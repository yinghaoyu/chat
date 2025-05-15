#include "MysqlDao.h"
#include "ConfigMgr.h"
#include "Logger.h"

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

int MysqlDao::RegUser(const std::string& name, const std::string& email,
    const std::string& pwd, const std::string& icon)
{
    auto con = pool_->get();
    if (con == nullptr)
    {
        LOG_ERROR("Failed to get connection from pool");
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
        LOG_ERROR("Exception in RegUser: {}", e.what());
        return -1;
    }
}

bool MysqlDao::CheckEmail(const std::string& name, const std::string& email)
{
    auto con = pool_->get();
    if (con == nullptr)
    {
        LOG_ERROR("Failed to get connection from pool");
        return false;
    }

    try
    {
        auto result = con->queryStmt(
            "SELECT email FROM user WHERE name = ?", name.c_str());
        if (result && result->next())
        {
            std::string fetchedEmail = result->getString(0);
            return fetchedEmail == email;
        }
        return false;
    }
    catch (const std::exception& e)
    {
        LOG_ERROR("Exception in CheckEmail: {}", e.what());
        return false;
    }
}

bool MysqlDao::UpdatePwd(const std::string& name, const std::string& newpwd)
{
    auto con = pool_->get();
    if (con == nullptr)
    {
        LOG_ERROR("Failed to get connection from pool");
        return false;
    }

    try
    {
        int ret = con->execStmt("UPDATE user SET pwd = ? WHERE name = ?",
            newpwd.c_str(),
            name.c_str());
        if (ret == 0)
        {
            LOG_DEBUG("Password updated successfully for user: {}", name);
            return true;
        }
        else
        {
            LOG_ERROR("No rows affected for user: {}", name);
            return false;
        }
    }
    catch (const std::exception& e)
    {
        LOG_ERROR("Exception in UpdatePwd: ", e.what());
        return false;
    }
}

bool MysqlDao::CheckPwd(
    const std::string& email, const std::string& pwd, UserInfo& userInfo)
{
    auto con = pool_->get();
    if (con == nullptr)
    {
        LOG_ERROR("Failed to get connection from pool");
        return false;
    }

    try
    {
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
        LOG_ERROR("Exception in CheckPwd: {}", e.what());
        return false;
    }
}
