#include "UserMgr.h"
#include "Logger.h"
#include "Session.h"

UserMgr::~UserMgr() { sessions_.clear(); }

std::shared_ptr<Session> UserMgr::GetSession(int uid)
{
    std::lock_guard<std::mutex> lock(mutex_);

    auto iter = sessions_.find(uid);

    if (iter == sessions_.end())
    {
        return nullptr;
    }

    return iter->second.lock();
}

void UserMgr::SetUserSession(int uid, std::shared_ptr<Session> session)
{
    std::lock_guard<std::mutex> lock(mutex_);
    sessions_[uid] = session;
}

void UserMgr::RmvUserSession(int uid, const std::string& session_id)
{
    std::lock_guard<std::mutex> lock(mutex_);

    auto iter = sessions_.find(uid);

    if (iter == sessions_.end())
    {
        return;
    }

    auto ptr = iter->second.lock();
    if (ptr == nullptr)
    {
        LOG_ERROR("session removed failure, uid: {}, session : {}", uid, session_id);
        // 清理无效 weak_ptr
        sessions_.erase(uid);
        return;
    }

    auto session_id_ = ptr->GetSessionId();
    // 不相等说明是其他地方登录了
    if (session_id_ != session_id)
    {
        LOG_ERROR("session removed failure, uid: {}, old_session: {}, "
                  "new_session: {}",
                  uid, session_id, session_id_);
        return;
    }
    sessions_.erase(uid);
    LOG_INFO("session removed, uid: {}, session : {}", uid, session_id);
}
