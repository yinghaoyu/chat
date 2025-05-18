#pragma once

#include "MsgNode.h"
#include "const.h"

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/http.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <memory>
#include <mutex>
#include <queue>

using tcp = boost::asio::ip::tcp;  // from <boost/asio/ip/tcp.hpp>

class ChatServer;
class LogicSystem;

class Session : public std::enable_shared_from_this<Session>
{
  public:
    Session(boost::asio::io_context& io_context, ChatServer* server);
    ~Session();

    tcp::socket&       GetSocket();
    const std::string& GetSessionId();

    void SetUserId(int uid);
    int  GetUserId();
    void Start();

    void Send(const char* msg, const short max_length, const short msgid);
    void Send(const std::string& msg, const short msgid);
    void ShutDownWrite();
    void Close();

    std::shared_ptr<Session> SharedSelf();

    void AsyncReadBody(int length);
    void AsyncReadHead(int total_len);
    void NotifyOffline(int uid);

    // 判断心跳是否过期
    bool IsHeartbeatExpired(std::time_t& now);

    // 更新心跳
    void UpdateHeartbeat();
    // 处理异常连接
    void DealExceptionSession();

  private:
    void asyncReadFull(std::size_t maxLength,
        std::function<void(const boost::system::error_code&, std::size_t)>
            handler);
    void asyncReadLen(std::size_t read_len, std::size_t total_len,
        std::function<void(const boost::system::error_code&, std::size_t)>
            handler);
    void HandleWrite(
        const boost::system::error_code&, std::shared_ptr<Session>);

    tcp::socket socket_;
    std::string session_id_;
    char        data_[MAX_LENGTH];
    ChatServer* server_;
    bool        closed_;

    std::queue<shared_ptr<SendNode>> send_que_;

    std::mutex send_mutex_;
    // 收到的消息结构
    std::shared_ptr<RecvNode> recv_msg_node_;
    bool                      head_parse_;
    // 收到的头部结构
    std::shared_ptr<MsgNode> recv_head_node_;
    int                      user_uid_;
    // 记录上次接受数据的时间
    std::atomic<time_t> last_heartbeat_;
    // session 锁
    std::mutex session_mutex_;
};

class LogicNode
{
    friend class LogicSystem;

  public:
    LogicNode(shared_ptr<Session>, shared_ptr<RecvNode>);

  private:
    shared_ptr<Session>  session_;
    shared_ptr<RecvNode> recv_node_;
};
