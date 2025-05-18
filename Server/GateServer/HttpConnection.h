#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/http.hpp>

namespace beast = boost::beast;          // from <boost/beast.hpp>
namespace http  = beast::http;           // from <boost/beast/http.hpp>
namespace net   = boost::asio;           // from <boost/asio.hpp>
using tcp       = boost::asio::ip::tcp;  // from <boost/asio/ip/tcp.hpp>

class HttpConnection : public std::enable_shared_from_this<HttpConnection>
{
    friend class LogicSystem;

  public:
    HttpConnection(boost::asio::io_context& ioc);

    void Start();
    void PreParseGetParam();

    tcp::socket& GetSocket() { return socket_; }

    void HandleGetRequest();
    void HandlePostRequest();
    void SendErrorResponse(http::status status, const std::string& message);

  private:
    void CheckDeadline();
    void WriteResponse();
    void HandleReq();

    tcp::socket socket_;

    beast::flat_buffer buffer_{8192};

    http::request<http::dynamic_body> request_;

    http::response<http::dynamic_body> response_;

    net::steady_timer deadline_{
        socket_.get_executor(), std::chrono::seconds(60)};

    std::string                                  url_;
    std::unordered_map<std::string, std::string> params_;
};
