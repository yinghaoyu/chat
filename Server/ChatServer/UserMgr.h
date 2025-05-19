#pragma once
#include "Singleton.h"

#include <memory>
#include <mutex>
#include <unordered_map>

class Session;
class UserMgr : public Singleton<UserMgr>
{
    friend class Singleton<UserMgr>;

  public:
    ~UserMgr();

    std::shared_ptr<Session> GetSession(int uid);

    void SetUserSession(int uid, std::shared_ptr<Session> session);
    void RmvUserSession(int uid, const std::string& session_id);

  private:
    UserMgr() {}

    std::mutex mutex_;

    std::unordered_map<int, std::weak_ptr<Session>> sessions_;
};
