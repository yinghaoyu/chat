#include "HttpConnection.h"
#include "LogicSystem.h"
#include "Logger.h"

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

HttpConnection::HttpConnection(boost::asio::io_context& ioc) : m_socket(ioc) {}

void HttpConnection::Start()
{
    auto self = shared_from_this();
    http::async_read(m_socket,
        m_buffer,
        m_request,
        [self](beast::error_code ec, std::size_t) {
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
    m_response.version(m_request.version());
    m_response.keep_alive(false);
    m_response.set(boost::beast::http::field::access_control_allow_origin, "*");

    switch (m_request.method())
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
    if (!LogicSystem::GetInstance()->HandleGet(m_url, shared_from_this()))
    {
        SendErrorResponse(http::status::not_found, "url not found\r\n");
        return;
    }
    m_response.result(http::status::ok);
    m_response.set(http::field::server, "GateServer");
    WriteResponse();
}

void HttpConnection::HandlePostRequest()
{
    if (!LogicSystem::GetInstance()->HandlePost(
            m_request.target(), shared_from_this()))
    {
        SendErrorResponse(http::status::not_found, "url not found\r\n");
        return;
    }
    m_response.result(http::status::ok);
    m_response.set(http::field::server, "GateServer");
    WriteResponse();
}

void HttpConnection::SendErrorResponse(
    http::status status, const std::string& message)
{
    m_response.result(status);
    m_response.set(http::field::content_type, "text/plain");
    beast::ostream(m_response.body()) << message;
    WriteResponse();
}

void HttpConnection::PreParseGetParam()
{
    auto uri       = m_request.target();
    auto query_pos = uri.find('?');
    if (query_pos == std::string::npos)
    {
        m_url = uri;
        return;
    }
    m_url                 = uri.substr(0, query_pos);
    std::string query_string = uri.substr(query_pos + 1);
    size_t      pos          = 0;
    while ((pos = query_string.find('&')) != std::string::npos)
    {
        auto   pair   = query_string.substr(0, pos);
        size_t eq_pos = pair.find('=');
        if (eq_pos != std::string::npos)
        {
            m_params[UrlDecode(pair.substr(0, eq_pos))] =
                UrlDecode(pair.substr(eq_pos + 1));
        }
        query_string.erase(0, pos + 1);
    }
    if (!query_string.empty())
    {
        size_t eq_pos = query_string.find('=');
        if (eq_pos != std::string::npos)
        {
            m_params[UrlDecode(query_string.substr(0, eq_pos))] =
                UrlDecode(query_string.substr(eq_pos + 1));
        }
    }
}

void HttpConnection::CheckDeadline()
{
    auto self = shared_from_this();
    m_deadline.async_wait([self](beast::error_code ec) {
        if (!ec)
        {
            self->m_socket.close(ec);
        }
    });
}

void HttpConnection::WriteResponse()
{
    auto self = shared_from_this();
    m_response.content_length(m_response.body().size());
    http::async_write(
        m_socket, m_response, [self](beast::error_code ec, std::size_t) {
            if(ec)
            {
                self->m_socket.close(ec);
            }
            else
            {
                self->m_socket.shutdown(tcp::socket::shutdown_send, ec);
            }
            self->m_deadline.cancel();
        });
}
