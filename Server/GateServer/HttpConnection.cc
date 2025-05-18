#include "HttpConnection.h"
#include "Logger.h"
#include "LogicSystem.h"

namespace
{
// 工具函数
unsigned char ToHex(unsigned char x) { return x > 9 ? x + 55 : x + 48; }
unsigned char FromHex(unsigned char x)
{
    if (x >= 'A' && x <= 'Z')
        return x - 'A' + 10;
    if (x >= 'a' && x <= 'z')
        return x - 'a' + 10;
    if (x >= '0' && x <= '9')
        return x - '0';
    assert(0);
    return 0;
}
std::string UrlEncode(const std::string& str)
{
    std::string strTemp;
    for (auto ch : str)
    {
        if (isalnum((unsigned char)ch) || ch == '-' || ch == '_' || ch == '.' ||
            ch == '~')
            strTemp += ch;
        else if (ch == ' ')
            strTemp += "+";
        else
        {
            strTemp += '%';
            strTemp += ToHex((unsigned char)ch >> 4);
            strTemp += ToHex((unsigned char)ch & 0x0F);
        }
    }
    return strTemp;
}
std::string UrlDecode(const std::string& str)
{
    std::string strTemp;
    for (size_t i = 0; i < str.length(); ++i)
    {
        if (str[i] == '+')
            strTemp += ' ';
        else if (str[i] == '%')
        {
            assert(i + 2 < str.length());
            unsigned char high = FromHex((unsigned char)str[++i]);
            unsigned char low  = FromHex((unsigned char)str[++i]);
            strTemp += high * 16 + low;
        }
        else
            strTemp += str[i];
    }
    return strTemp;
}
}  // namespace

HttpConnection::HttpConnection(boost::asio::io_context& ioc) : socket_(ioc) {}

void HttpConnection::Start()
{
    auto self = shared_from_this();
    http::async_read(
        socket_, buffer_, request_, [self](beast::error_code ec, std::size_t) {
            try
            {
                if (ec)
                {
                    LOG_ERROR("Http read err, {}", ec.message());
                    return;
                }
                self->HandleReq();
                self->CheckDeadline();
            }
            catch (std::exception& exp)
            {
                LOG_ERROR("Exception: {}", exp.what());
            }
        });
}

void HttpConnection::HandleReq()
{
    response_.version(request_.version());
    response_.keep_alive(false);
    response_.set(boost::beast::http::field::access_control_allow_origin, "*");

    switch (request_.method())
    {
        case http::verb::get: HandleGetRequest(); break;
        case http::verb::post: HandlePostRequest(); break;
        default:
            SendErrorResponse(
                http::status::bad_request, "Unsupported HTTP method");
            break;
    }
}

void HttpConnection::HandleGetRequest()
{
    PreParseGetParam();
    if (!LogicSystem::GetInstance()->HandleGet(url_, shared_from_this()))
    {
        SendErrorResponse(http::status::not_found, "url not found\r\n");
        return;
    }
    response_.result(http::status::ok);
    response_.set(http::field::server, "GateServer");
    WriteResponse();
}

void HttpConnection::HandlePostRequest()
{
    if (!LogicSystem::GetInstance()->HandlePost(
            request_.target(), shared_from_this()))
    {
        SendErrorResponse(http::status::not_found, "url not found\r\n");
        return;
    }
    response_.result(http::status::ok);
    response_.set(http::field::server, "GateServer");
    WriteResponse();
}

void HttpConnection::SendErrorResponse(
    http::status status, const std::string& message)
{
    response_.result(status);
    response_.set(http::field::content_type, "text/plain");
    beast::ostream(response_.body()) << message;
    WriteResponse();
}

void HttpConnection::PreParseGetParam()
{
    auto uri       = request_.target();
    auto query_pos = uri.find('?');
    if (query_pos == std::string::npos)
    {
        url_ = uri;
        return;
    }
    url_                     = uri.substr(0, query_pos);
    std::string query_string = uri.substr(query_pos + 1);
    size_t      pos          = 0;
    while ((pos = query_string.find('&')) != std::string::npos)
    {
        auto   pair   = query_string.substr(0, pos);
        size_t eq_pos = pair.find('=');
        if (eq_pos != std::string::npos)
        {
            params_[UrlDecode(pair.substr(0, eq_pos))] =
                UrlDecode(pair.substr(eq_pos + 1));
        }
        query_string.erase(0, pos + 1);
    }
    if (!query_string.empty())
    {
        size_t eq_pos = query_string.find('=');
        if (eq_pos != std::string::npos)
        {
            params_[UrlDecode(query_string.substr(0, eq_pos))] =
                UrlDecode(query_string.substr(eq_pos + 1));
        }
    }
}

void HttpConnection::CheckDeadline()
{
    auto self = shared_from_this();
    deadline_.async_wait([self](beast::error_code ec) {
        if (!ec)
        {
            self->socket_.close(ec);
        }
    });
}

void HttpConnection::WriteResponse()
{
    auto self = shared_from_this();
    response_.content_length(response_.body().size());
    http::async_write(
        socket_, response_, [self](beast::error_code ec, std::size_t) {
            if (ec)
            {
                self->socket_.close(ec);
            }
            else
            {
                self->socket_.shutdown(tcp::socket::shutdown_send, ec);
            }
            self->deadline_.cancel();
        });
}
