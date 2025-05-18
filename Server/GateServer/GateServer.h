#pragma once

#include <memory>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/http.hpp>

namespace net = boost::asio;           // from <boost/asio.hpp>
using tcp     = boost::asio::ip::tcp;  // from <boost/asio/ip/tcp.hpp>

class GateServer : public std::enable_shared_from_this<GateServer>
{
  public:
    GateServer(boost::asio::io_context& ioc, unsigned short& port);
    void Start();

  private:
    tcp::acceptor    m_acceptor;
    net::io_context& m_ioc;
};
