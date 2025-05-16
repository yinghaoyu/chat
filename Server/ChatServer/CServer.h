#pragma once

#include "CSession.h"

#include <boost/asio.hpp>
#include <memory.h>
#include <map>
#include <mutex>
#include <boost/asio/steady_timer.hpp>

using boost::asio::ip::tcp;

class CServer : public std::enable_shared_from_this<CServer>
{
    typedef std::map<std::string, std::shared_ptr<CSession>> SESSION_MAP;

  public:
    CServer(boost::asio::io_context& io_context, short port);

    ~CServer();

    void CleanSession(const std::string& session_id);

    bool CheckValid(const std::string& session_id);

    void Start();
    void Shutdown();

  private:
    void on_timer(const boost::system::error_code& ec);
    void start_timer();
    void HandleAccept(
        std::shared_ptr<CSession>, const boost::system::error_code& error);
    void StartAccept();

    boost::asio::io_context&  _io_context;
    short                     _port;
    tcp::acceptor             _acceptor;
    SESSION_MAP               _sessions;
    std::mutex                _mutex;
    boost::asio::steady_timer _timer;
};
