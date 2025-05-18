#include "GateServer.h"
#include "HttpConnection.h"
#include "AsioIOServicePool.h"
#include "Logger.h"

GateServer::GateServer(boost::asio::io_context& ioc, unsigned short& port)
    : m_ioc(ioc), m_acceptor(ioc, tcp::endpoint(tcp::v4(), port))
{}

void GateServer::Start()
{
    auto  self       = shared_from_this();
    auto& io_context = AsioIOServicePool::GetInstance()->GetIOService();

    std::shared_ptr<HttpConnection> new_con =
        std::make_shared<HttpConnection>(io_context);
        
    m_acceptor.async_accept(
        new_con->GetSocket(), [self, new_con](beast::error_code ec) {
            try
            {
                // 出错则放弃这个连接，继续监听新链接
                if (ec)
                {
                    self->Start();
                    return;
                }

                // 处理新链接，创建HpptConnection类管理新连接
                new_con->Start();
                // 继续监听
                self->Start();
            }
            catch (std::exception& exp)
            {
                LOG_ERROR("Exception: {}", exp.what());
                self->Start();
            }
        });
}
