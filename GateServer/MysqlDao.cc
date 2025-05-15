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
    pool_.reset(new MySQLPool(host, stoi(port), user, pwd, schema, 5, 5));
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

bool MysqlDao::AddFriendApply(const int from, const int to)
{
    auto con = pool_->get();
    if (con == nullptr)
    {
        LOG_ERROR("Failed to get connection from pool");
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
            LOG_DEBUG("Friend apply added or updated successfully, fromuid: {}, touid: {}", from, to);
            return true;
        }
        else
        {
            LOG_ERROR("No rows affected for friend apply, fromuid: {}, touid: {} ", from, to);
            return false;
        }
    }
    catch (const std::exception& e)
    {
        LOG_ERROR("Exception in AddFriendApply: {}", e.what());
        return false;
    }
}

bool MysqlDao::AuthFriendApply(const int from, const int to)
{
    auto con = pool_->get();
    if (con == nullptr)
    {
        LOG_ERROR("Failed to get connection from pool");
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
            LOG_DEBUG("Friend apply authenticated successfully, fromuid: {}, touid: {}", from, to);
            return true;
        }
        else
        {
            LOG_ERROR("No rows affected for friend apply authentication, fromuid: {}, touid: {}", from, to);
            return false;
        }
    }
    catch (const std::exception& e)
    {
        LOG_ERROR("Exception in AuthFriendApply: {}", e.what());
        return false;
    }
}

bool MysqlDao::AddFriend(
    const int from, const int to, const std::string& back_name)
{
    auto con = pool_->get();
    if (con == nullptr)
    {
        LOG_ERROR("Failed to get connection from pool");
        return false;
    }

    try
    {
        // 开始事务
        MySQLTransaction::ptr tran = std::static_pointer_cast<MySQLTransaction>(
            con->openTransaction(false));
        if (!tran->begin())
        {
            LOG_ERROR("Failed to begin transaction");
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
            LOG_ERROR("Failed to insert friend for user: {}", from);
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
            LOG_ERROR("Failed to insert friend for user: {}", to);
            return false;
        }

        // 提交事务
        if (!tran->commit())
        {
            LOG_ERROR("Failed to commit transaction");
            return false;
        }

        LOG_DEBUG("Friend relationship added successfully, fromuid: {}, touid: {}", from, to);
        return true;
    }
    catch (const std::exception& e)
    {
        LOG_ERROR("Exception in AddFriend: {}", e.what());
        return false;
    }
}

std::shared_ptr<UserInfo> MysqlDao::GetUser(const int uid)
{
    auto con = pool_->get();
    if (con == nullptr)
    {
        LOG_ERROR("Failed to get connection from pool");
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
        LOG_ERROR("Exception in GetUser: {}" ,e.what());
        return nullptr;
    }
}

std::shared_ptr<UserInfo> MysqlDao::GetUser(const std::string& name)
{
    auto con = pool_->get();
    if (con == nullptr)
    {
        LOG_ERROR("Failed to get connection from pool");
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
        LOG_ERROR("Exception in GetUser: {}", e.what());
        return nullptr;
    }
}

bool MysqlDao::GetApplyList(const int        touid,
    std::vector<std::shared_ptr<ApplyInfo>>& applyList, int begin, int limit)
{
    auto con = pool_->get();
    if (con == nullptr)
    {
        LOG_ERROR("Failed to get connection from pool");
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
                result->getInt16(1));

            applyList.push_back(apply_ptr);
        }
        return true;
    }
    catch (const std::exception& e)
    {
        LOG_ERROR("Exception in GetApplyList: {}", e.what());
        return false;
    }
}

bool MysqlDao::GetFriendList(
    const int self_id, std::vector<std::shared_ptr<UserInfo>>& user_info_list)
{
    auto con = pool_->get();
    if (con == nullptr)
    {
        LOG_ERROR("Failed to get connection from pool");
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
        LOG_ERROR("Exception in GetFriendList: {}", e.what());
        return false;
    }
}
