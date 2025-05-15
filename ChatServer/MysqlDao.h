#pragma once
#include "data.h"
#include "mysql.h"
#include "const.h"

#include <thread>
#include <memory>
#include <queue>
#include <mutex>
#include <chrono>

class MysqlDao
{
  public:
    MysqlDao();
    ~MysqlDao();
    int  RegUser(const std::string& name, const std::string& email,
         const std::string& pwd, const std::string& icon);
    bool CheckEmail(const std::string& name, const std::string& email);
    bool UpdatePwd(const std::string& name, const std::string& newpwd);
    bool CheckPwd(
        const std::string& email, const std::string& pwd, UserInfo& userInfo);
    bool AddFriendApply(const int from, const int to);
    bool AuthFriendApply(const int from, const int to);
    bool AddFriend(const int from, const int to, const std::string& back_name);
    std::shared_ptr<UserInfo> GetUser(const int uid);
    std::shared_ptr<UserInfo> GetUser(const std::string& name);
    bool                      GetApplyList(const int                  touid,
                             std::vector<std::shared_ptr<ApplyInfo>>& applyList, int offset,
                             int limit);
    bool                      GetFriendList(
                             const int self_id, std::vector<std::shared_ptr<UserInfo>>& user_info);

  private:
    std::unique_ptr<MySQLPool> pool_;
};
