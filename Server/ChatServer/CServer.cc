#include "CServer.h"
#include "AsioIOServicePool.h"
#include "Server/ChatServer/CSession.h"
#include "UserMgr.h"
#include "RedisMgr.h"
#include "ConfigMgr.h"
#include "Logger.h"
#include <memory>
#include <vector>

CServer::CServer(boost::asio::io_context& io_context, short port)
    : _io_context(io_context),
      _port(port),
      _acceptor(io_context, tcp::endpoint(tcp::v4(), port)),
      _timer(_io_context)
{}

CServer::~CServer() { LOG_TRACE("Server dtor~"); }

void CServer::HandleAccept(
    shared_ptr<CSession> new_session, const boost::system::error_code& error)
{
    if (!error)
    {
        new_session->Start();

        lock_guard<mutex> lock(_mutex);

        _sessions.emplace(new_session->GetSessionId(), new_session);
    }
    else
    {
        LOG_ERROR("session accept failed, error: {}", error.what());
    }

    StartAccept();
}

void CServer::StartAccept()
{
    auto& io_context = AsioIOServicePool::GetInstance()->GetIOService();

    auto new_session = std::make_shared<CSession>(io_context, this);

    _acceptor.async_accept(new_session->GetSocket(),
        std::bind(&CServer::HandleAccept, this, new_session, placeholders::_1));
}

// 根据session 的id删除session，并移除用户和session的关联
void CServer::CleanSession(const std::string& session_id)
{
    lock_guard<mutex> lock(_mutex);
    if (_sessions.count(session_id))
    {
        LOG_INFO("CleanSession session: {}", session_id);
        auto uid = _sessions[session_id]->GetUserId();
        // 移除用户和session的关联
        UserMgr::GetInstance()->RmvUserSession(uid, session_id);
    }

    _sessions.erase(session_id);
}

bool CServer::CheckValid(const std::string& session_id)
{
    lock_guard<mutex> lock(_mutex);
    return _sessions.count(session_id) == 1;
}

void CServer::on_timer(const boost::system::error_code& ec)
{
    std::vector<std::shared_ptr<CSession>> expired_sessions;

    int count = 0;

    {
        lock_guard<mutex> lock(_mutex);

        time_t now = std::time(nullptr);

        for (auto& session : _sessions)
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

void CServer::Start()
{
    LOG_INFO("Server start success, listen on port: {}", _port);
    StartAccept();
    start_timer();
}

void CServer::Shutdown() { _timer.cancel(); }

void CServer::start_timer()
{
    _timer.expires_after(std::chrono::seconds(60));

    auto self = shared_from_this();

    _timer.async_wait(
        [self](boost::system::error_code ec) { self->on_timer(ec); });
}
