#include "ChatServer.h"
#include "AsioIOServicePool.h"
#include "Session.h"
#include "ConfigMgr.h"
#include "Logger.h"
#include "RedisMgr.h"
#include "UserMgr.h"

#include <memory>
#include <vector>

ChatServer::ChatServer(boost::asio::io_context& io_context, short port)
    : io_context_(io_context),
      port_(port),
      acceptor_(io_context, tcp::endpoint(tcp::v4(), port)),
      timer_(io_context_)
{}

ChatServer::~ChatServer() { LOG_TRACE("Server dtor~"); }

void ChatServer::HandleAccept(
    shared_ptr<Session> new_session, const boost::system::error_code& error)
{
    if (!error)
    {
        new_session->Start();

        lock_guard<mutex> lock(mutex_);

        sessions_.emplace(new_session->GetSessionId(), new_session);
    }
    else
    {
        LOG_ERROR("session accept failed, error: {}", error.what());
    }

    StartAccept();
}

void ChatServer::StartAccept()
{
    auto& io_context = AsioIOServicePool::GetInstance()->GetIOService();

    auto new_session = std::make_shared<Session>(io_context, this);

    acceptor_.async_accept(new_session->GetSocket(),
        std::bind(
            &ChatServer::HandleAccept, this, new_session, placeholders::_1));
}

// 根据session 的id删除session，并移除用户和session的关联
void ChatServer::CleanSession(const std::string& session_id)
{
    lock_guard<mutex> lock(mutex_);
    if (sessions_.count(session_id))
    {
        LOG_INFO("CleanSession session: {}", session_id);
        auto uid = sessions_[session_id]->GetUserId();
        // 移除用户和session的关联
        UserMgr::GetInstance()->RmvUserSession(uid, session_id);
    }

    sessions_.erase(session_id);
}

bool ChatServer::CheckValid(const std::string& session_id)
{
    lock_guard<mutex> lock(mutex_);
    return sessions_.count(session_id) == 1;
}

void ChatServer::on_timer(const boost::system::error_code& ec)
{
    std::vector<std::shared_ptr<Session>> expired_sessions;

    int count = 0;

    {
        lock_guard<mutex> lock(mutex_);

        time_t now = std::time(nullptr);

        for (auto& session : sessions_)
        {
            bool expired = session.second->IsHeartbeatExpired(now);
            if (expired)
            {
                // 关闭socket, 其实这里也会触发async_read的错误处理
                session.second->Close();
                expired_sessions.emplace_back(session.second);
                continue;
            }
            count++;
        }
    }

    // 设置session数量
    auto& cfg       = ConfigMgr::Inst();
    auto  self_name = cfg["SelfServer"]["Name"];
    auto  count_str = std::to_string(count);
    RedisMgr::GetInstance()->HSet(LOGIN_COUNT, self_name, count_str);

    // 处理过期session, 单独提出，防止死锁
    for (auto& session : expired_sessions)
    {
        session->DealExceptionSession();
    }

    start_timer();
}

void ChatServer::Start()
{
    LOG_INFO("Server start success, listen on port: {}", port_);
    StartAccept();
    start_timer();
}

void ChatServer::Shutdown() { timer_.cancel(); }

void ChatServer::start_timer()
{
    timer_.expires_after(std::chrono::seconds(60));

    auto self = shared_from_this();

    timer_.async_wait(
        [self](boost::system::error_code ec) { self->on_timer(ec); });
}
