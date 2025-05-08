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
        int ret = con->execStmt("UPDATE user SET pwd = ? WHERE name = ?",
            newpwd.c_str(),
            name.c_str());
        if (ret == 0)
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
    const std::string& name, const std::string& pwd, UserInfo& userInfo)
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
            "SELECT uid, email, pwd FROM user WHERE name = ?", name.c_str());
        if (result && result->next())
        {
            std::string origin_pwd = result->getString(2);
            if (pwd != origin_pwd)
            {
                return false;
            }

            // 填充 userInfo
            userInfo.uid   = result->getInt32(0);
            userInfo.name  = name;
            userInfo.email = result->getString(1);
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

bool MysqlDao::AddFriendApply(const int from, const int to)
{
    auto con = pool_->get();
    if (con == nullptr)
    {
        std::cerr << "Failed to get connection from pool" << std::endl;
        return false;
    }

    try
    {
        int ret = con->execStmt(
            "INSERT INTO friend_apply (from_uid, to_uid) VALUES (?, ?) "
            "ON DUPLICATE KEY UPDATE from_uid = VALUES(from_uid), to_uid = "
            "VALUES(to_uid)",
            (int32_t)from,
            (int32_t)to);

        if (ret == 0)
        {
            std::cout << "Friend apply added or updated successfully: from "
                      << from << " to " << to << std::endl;
            return true;
        }
        else
        {
            std::cerr << "No rows affected for friend apply: from " << from
                      << " to " << to << std::endl;
            return false;
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception in AddFriendApply: " << e.what() << std::endl;
        return false;
    }
}

bool MysqlDao::AuthFriendApply(const int from, const int to)
{
    auto con = pool_->get();
    if (con == nullptr)
    {
        std::cerr << "Failed to get connection from pool" << std::endl;
        return false;
    }

    try
    {
        // 反过来的申请时from，验证时to
        int ret = con->execStmt("UPDATE friend_apply SET status = 1 "
                                "WHERE from_uid = ? AND to_uid = ?",
            (int32_t)to,
            (int32_t)from);

        if (ret == 0)
        {
            std::cout << "Friend apply authenticated successfully: from "
                      << from << " to " << to << std::endl;
            return true;
        }
        else
        {
            std::cerr
                << "No rows affected for friend apply authentication: from "
                << from << " to " << to << std::endl;
            return false;
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception in AuthFriendApply: " << e.what() << std::endl;
        return false;
    }
}

bool MysqlDao::AddFriend(
    const int from, const int to, const std::string& back_name)
{
    auto con = pool_->get();
    if (con == nullptr)
    {
        std::cerr << "Failed to get connection from pool" << std::endl;
        return false;
    }

    try
    {
        // 开始事务
        MySQLTransaction::ptr tran = std::static_pointer_cast<MySQLTransaction>(
            con->openTransaction(false));
        if (!tran->begin())
        {
            std::cerr << "Failed to begin transaction" << std::endl;
            return false;
        }

        // 插入认证方好友数据
        int ret =
            tran->getMySQL()->execStmt("INSERT IGNORE INTO friend (self_id, "
                                       "friend_id, back) VALUES (?, ?, ?)",
                (int32_t)from,
                (int32_t)to,
                back_name.c_str());
        if (ret != 0)
        {
            tran->rollback();
            std::cerr << "Failed to insert friend for user: " << from
                      << std::endl;
            return false;
        }

        // 插入申请方好友数据
        ret = tran->getMySQL()->execStmt("INSERT IGNORE INTO friend (self_id, "
                                         "friend_id, back) VALUES (?, ?, ?)",
            (int32_t)to,
            (int32_t)from,
            std::string(""));
        if (ret != 0)
        {
            tran->rollback();
            std::cerr << "Failed to insert friend for user: " << to
                      << std::endl;
            return false;
        }

        // 提交事务
        if (!tran->commit())
        {
            std::cerr << "Failed to commit transaction" << std::endl;
            return false;
        }

        std::cout << "Friend relationship added successfully: from " << from
                  << " to " << to << std::endl;
        return true;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception in AddFriend: " << e.what() << std::endl;
        return false;
    }
}

std::shared_ptr<UserInfo> MysqlDao::GetUser(const int uid)
{
    auto con = pool_->get();
    if (con == nullptr)
    {
        std::cerr << "Failed to get connection from pool" << std::endl;
        return nullptr;
    }

    try
    {
        // desc为 mysql 关键字
        auto result =
            con->queryStmt("SELECT uid, name, email, pwd, nick, `desc`, sex, "
                           "icon FROM user WHERE uid = ?",
                (int32_t)uid);
        if (result && result->next())
        {
            auto user_ptr   = std::make_shared<UserInfo>();
            user_ptr->uid   = uid;
            user_ptr->name  = result->getString(1);
            user_ptr->email = result->getString(2);
            user_ptr->pwd   = result->getString(3);
            user_ptr->nick  = result->getString(4);
            user_ptr->desc  = result->getString(5);
            user_ptr->sex   = result->getInt32(6);
            user_ptr->icon  = result->getString(7);
            return user_ptr;
        }
        return nullptr;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception in GetUser: " << e.what() << std::endl;
        return nullptr;
    }
}

std::shared_ptr<UserInfo> MysqlDao::GetUser(const std::string& name)
{
    auto con = pool_->get();
    if (con == nullptr)
    {
        std::cerr << "Failed to get connection from pool" << std::endl;
        return nullptr;
    }

    try
    {
        auto result =
            con->queryStmt("SELECT uid, name, email, pwd, nick, `desc`, sex, "
                           "icon FROM user WHERE name = ?",
                name.c_str());
        if (result && result->next())
        {
            auto user_ptr   = std::make_shared<UserInfo>();
            user_ptr->uid   = result->getInt32(0);
            user_ptr->name  = name;
            user_ptr->email = result->getString(2);
            user_ptr->pwd   = result->getString(3);
            user_ptr->nick  = result->getString(4);
            user_ptr->desc  = result->getString(5);
            user_ptr->sex   = result->getInt32(6);
            user_ptr->icon  = result->getString(7);
            return user_ptr;
        }
        return nullptr;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception in GetUser: " << e.what() << std::endl;
        return nullptr;
    }
}

bool MysqlDao::GetApplyList(const int        touid,
    std::vector<std::shared_ptr<ApplyInfo>>& applyList, int begin, int limit)
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
        auto result = con->queryStmt("SELECT apply.from_uid, apply.status, "
                                     "user.name, user.nick, user.sex "
                                     "FROM friend_apply AS apply "
                                     "JOIN user ON apply.from_uid = user.uid "
                                     "WHERE apply.to_uid = ? AND apply.id > ? "
                                     "ORDER BY apply.id ASC LIMIT ?",
            (int32_t)touid,
            begin,
            limit);

        // 遍历结果集
        while (result && result->next())
        {
            auto apply_ptr = std::make_shared<ApplyInfo>(result->getInt32(0),
                result->getString(2),
                "",  // email (未查询)
                "",  // pwd (未查询)
                result->getString(3),
                result->getInt32(4),
                result->getInt32(1));

            applyList.push_back(apply_ptr);
        }
        return true;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception in GetApplyList: " << e.what() << std::endl;
        return false;
    }
}

bool MysqlDao::GetFriendList(
    const int self_id, std::vector<std::shared_ptr<UserInfo>>& user_info_list)
{
    auto con = pool_->get();
    if (con == nullptr)
    {
        std::cerr << "Failed to get connection from pool" << std::endl;
        return false;
    }

    try
    {
        // 使用 queryStmt 查询好友列表
        auto result =
            con->queryStmt("SELECT friend.friend_id, friend.back, user.name, "
                           "user.email, user.nick, user.sex, user.icon "
                           "FROM friend "
                           "JOIN user ON friend.friend_id = user.uid "
                           "WHERE friend.self_id = ?",
                (int32_t)self_id);

        // 遍历结果集
        while (result && result->next())
        {
            auto user_ptr   = std::make_shared<UserInfo>();
            user_ptr->uid   = result->getInt32(0);
            user_ptr->name  = result->getString(2);
            user_ptr->email = result->getString(3);
            user_ptr->nick  = result->getString(4);
            user_ptr->sex   = result->getInt32(5);
            user_ptr->icon  = result->getString(6);
            user_ptr->back  = result->getString(1);

            user_info_list.push_back(user_ptr);
        }
        return true;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception in GetFriendList: " << e.what() << std::endl;
        return false;
    }
}
