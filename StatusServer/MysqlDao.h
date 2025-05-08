#pragma once

#include "mysql.h"

#include <thread>

struct UserInfo
{
    std::string name;
    std::string pwd;
    int         uid;
    std::string email;
};

class MysqlDao
{
  public:
    MysqlDao();
    ~MysqlDao();
    int  RegUser(const std::string& name, const std::string& email,
         const std::string& pwd);
    int  RegUserTransaction(const std::string& name, const std::string& email,
         const std::string& pwd, const std::string& icon);
    bool CheckEmail(const std::string& name, const std::string& email);
    bool UpdatePwd(const std::string& name, const std::string& newpwd);
    bool CheckPwd(
        const std::string& name, const std::string& pwd, UserInfo& userInfo);

  private:
    std::unique_ptr<MySQLPool> pool_;
};
