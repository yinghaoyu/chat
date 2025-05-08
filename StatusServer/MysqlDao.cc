#include "MysqlDao.h"
#include "ConfigMgr.h"

MysqlDao::MysqlDao()
{
    auto&       cfg    = ConfigMgr::Inst();
    const auto& host   = cfg["Mysql"]["Host"];
    const auto& port   = cfg["Mysql"]["Port"];
    const auto& pwd    = cfg["Mysql"]["Passwd"];
    const auto& schema = cfg["Mysql"]["Schema"];
    const auto& user   = cfg["Mysql"]["User"];
    pool_.reset(new MySQLPool(host, stoi(port), user, pwd, schema, 5));
}

MysqlDao::~MysqlDao() {}

int MysqlDao::RegUser(
    const std::string& name, const std::string& email, const std::string& pwd)
{
    auto con = pool_->get();
    if (con == nullptr)
    {
        std::cerr << "Failed to get connection from pool" << std::endl;
        return -1;
    }

    try
    {
        // 使用存储过程注册用户
        auto stmt = con->prepare("CALL reg_user(?, ?, ?, @result)");
        stmt->bindString(1, name);
        stmt->bindString(2, email);
        stmt->bindString(3, pwd);

        // 执行存储过程
        if (stmt->execute() != 0)
        {
            std::cerr << "Failed to execute stored procedure for RegUser"
                      << std::endl;
            return -1;
        }

        // 获取存储过程的输出参数
        auto result = con->query("SELECT @result AS result");
        if (result && result->next())
        {
            int regResult = result->getInt32(0);
            std::cout << "RegUser result: " << regResult << std::endl;
            return regResult;
        }

        return -1;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception in RegUser: " << e.what() << std::endl;
        return -1;
    }
}

int MysqlDao::RegUserTransaction(const std::string& name,
    const std::string& email, const std::string& pwd, const std::string& icon)
{
    auto con = pool_->get();
    if (con == nullptr)
    {
        std::cerr << "Failed to get connection from pool" << std::endl;
        return -1;
    }

    try
    {
        // 开始事务
        MySQLTransaction::ptr tran = std::static_pointer_cast<MySQLTransaction>(
            con->openTransaction(false));
        if (!tran->begin())
        {
            std::cerr << "Failed to begin transaction" << std::endl;
            return -1;
        }

        // 检查 email 是否已存在
        auto emailCheck = tran->getMySQL()->queryStmt(
            "SELECT 1 FROM user WHERE email = ?", email.c_str());

        if (emailCheck && emailCheck->next())
        {
            tran->rollback();
            std::cerr << "Email " << email << " already exists" << std::endl;
            return 0;
        }

        // 检查 name 是否已存在
        auto nameCheck = tran->getMySQL()->queryStmt(
            "SELECT 1 FROM user WHERE name = ?", name.c_str());
        if (nameCheck && nameCheck->next())
        {
            tran->rollback();
            std::cerr << "Name " << name << " already exists" << std::endl;
            return 0;
        }

        // 更新 user_id 表
        if (tran->execute("UPDATE user_id SET id = id + 1") != 0)
        {
            tran->rollback();
            std::cerr << "Failed to update user_id" << std::endl;
            return -1;
        }

        // 获取新的用户 ID
        auto userIdResult =
            tran->getMySQL()->queryStmt("SELECT id FROM user_id");
        int newId = 0;
        if (userIdResult && userIdResult->next())
        {
            newId = userIdResult->getInt32(0);
        }
        else
        {
            tran->rollback();
            std::cerr << "Failed to retrieve new user ID" << std::endl;
            return -1;
        }

        // 插入新用户
        if (tran->getMySQL()->execStmt(
                "INSERT INTO user (uid, name, email, pwd, nick, icon) VALUES "
                "(?, ?, ?, ?, ?, ?)",
                newId,
                name.c_str(),
                email.c_str(),
                pwd.c_str(),
                name.c_str(),
                icon.c_str()) != 0)
        {
            tran->rollback();
            std::cerr << "Failed to insert new user" << std::endl;
            return -1;
        }

        // 提交事务
        if (!tran->commit())
        {
            std::cerr << "Failed to commit transaction" << std::endl;
            return -1;
        }

        std::cout << "User registered successfully with ID: " << newId
                  << std::endl;
        return newId;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception in RegUserTransaction: " << e.what()
                  << std::endl;
        return -1;
    }
}

bool MysqlDao::CheckEmail(const std::string& name, const std::string& email)
{
    auto con = pool_->get();
    if (con == nullptr)
    {
        std::cerr << "Failed to get connection from pool" << std::endl;
        return false;
    }

    try
    {
        auto result = con->queryStmt(
            "SELECT email FROM user WHERE name = ?", name.c_str());
        if (result && result->next())
        {
            std::string fetchedEmail = result->getString(0);
            std::cout << "Check Email: " << fetchedEmail << std::endl;
            return fetchedEmail == email;
        }
        return false;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception in CheckEmail: " << e.what() << std::endl;
        return false;
    }
}

bool MysqlDao::UpdatePwd(const std::string& name, const std::string& newpwd)
{
    auto con = pool_->get();
    if (con == nullptr)
    {
        std::cerr << "Failed to get connection from pool" << std::endl;
        return false;
    }

    try
    {
        int affectedRows =
            con->execStmt("UPDATE user SET pwd = ? WHERE name = ?",
                newpwd.c_str(),
                name.c_str());
        if (affectedRows == 0)
        {
            std::cout << "Password updated successfully for user: " << name
                      << std::endl;
            return true;
        }
        else
        {
            std::cerr << "No rows affected for user: " << name << std::endl;
            return false;
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception in UpdatePwd: " << e.what() << std::endl;
        return false;
    }
}

bool MysqlDao::CheckPwd(
    const std::string& email, const std::string& pwd, UserInfo& userInfo)
{
    auto con = pool_->get();
    if (con == nullptr)
    {
        std::cerr << "Failed to get connection from pool" << std::endl;
        return false;
    }

    try
    {
        // 使用 queryStmt 执行查询
        auto result = con->queryStmt(
            "SELECT uid, name, email, pwd FROM user WHERE email = ?",
            email.c_str());
        if (result && result->next())
        {
            std::string origin_pwd = result->getString(3);
            if (pwd != origin_pwd)
            {
                return false;
            }

            // 填充 userInfo
            userInfo.uid   = result->getInt32(0);
            userInfo.name  = result->getString(1);
            userInfo.email = result->getString(2);
            userInfo.pwd   = origin_pwd;

            return true;
        }
        return false;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception in CheckPwd: " << e.what() << std::endl;
        return false;
    }
}
