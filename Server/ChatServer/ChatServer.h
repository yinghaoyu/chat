#pragma once

#include "Session.h"

#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <map>
#include <memory.h>
#include <mutex>

using boost::asio::ip::tcp;

class ChatServer : public std::enable_shared_from_this<ChatServer>
{
    typedef std::map<std::string, std::shared_ptr<Session>> SESSION_MAP;

  public:
    ChatServer(boost::asio::io_context& io_context, short port);

    ~ChatServer();

    void CleanSession(const std::string& session_id);

    bool CheckValid(const std::string& session_id);

    void Start();
    void Shutdown();

  private:
    void on_timer(const boost::system::error_code& ec);
    void start_timer();
    void HandleAccept(
        std::shared_ptr<Session>, const boost::system::error_code& error);
    void StartAccept();

    boost::asio::io_context&  io_context_;
    short                     port_;
    tcp::acceptor             acceptor_;
    SESSION_MAP               sessions_;
    std::mutex                mutex_;
    boost::asio::steady_timer timer_;
};
