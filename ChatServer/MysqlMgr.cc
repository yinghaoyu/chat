#include "MysqlMgr.h"

MysqlMgr::~MysqlMgr() {}

int MysqlMgr::RegUser(const std::string& name, const std::string& email,
    const std::string& pwd, const std::string& icon)
{
    return _dao.RegUser(name, email, pwd, icon);
}

bool MysqlMgr::CheckEmail(const std::string& name, const std::string& email)
{
    return _dao.CheckEmail(name, email);
}

bool MysqlMgr::UpdatePwd(const std::string& name, const std::string& pwd)
{
    return _dao.UpdatePwd(name, pwd);
}

MysqlMgr::MysqlMgr() {}

bool MysqlMgr::CheckPwd(
    const std::string& email, const std::string& pwd, UserInfo& userInfo)
{
    return _dao.CheckPwd(email, pwd, userInfo);
}

bool MysqlMgr::AddFriendApply(const int from, const int to)
{
    return _dao.AddFriendApply(from, to);
}

bool MysqlMgr::AuthFriendApply(const int from, const int to)
{
    return _dao.AuthFriendApply(from, to);
}

bool MysqlMgr::AddFriend(
    const int from, const int to, const std::string& back_name)
{
    return _dao.AddFriend(from, to, back_name);
}

std::shared_ptr<UserInfo> MysqlMgr::GetUser(int uid)
{
    return _dao.GetUser(uid);
}

std::shared_ptr<UserInfo> MysqlMgr::GetUser(const std::string& name)
{
    return _dao.GetUser(name);
}

bool MysqlMgr::GetApplyList(const int        touid,
    std::vector<std::shared_ptr<ApplyInfo>>& applyList, int begin, int limit)
{

    return _dao.GetApplyList(touid, applyList, begin, limit);
}

bool MysqlMgr::GetFriendList(
    const int self_id, std::vector<std::shared_ptr<UserInfo>>& user_info)
{
    return _dao.GetFriendList(self_id, user_info);
}
