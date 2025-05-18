#include "ConfigMgr.h"
#include "GateServer.h"
#include "Logger.h"
#include "MysqlMgr.h"
#include "RedisMgr.h"

int main()
{
    try
    {
        Logger::GetInstance();
        MysqlMgr::GetInstance();
        RedisMgr::GetInstance();

        auto&          gCfgMgr       = ConfigMgr::Inst();
        std::string    gate_port_str = gCfgMgr["GateServer"]["Port"];
        unsigned short gate_port     = stoi(gate_port_str);

        net::io_context         ioc{1};
        boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);

        signals.async_wait(
            [&ioc](const boost::system::error_code& error, int signal_number) {
                if (error)
                {
                    return;
                }
                ioc.stop();
            });

        std::make_shared<GateServer>(ioc, gate_port)->Start();
        LOG_INFO("GateServer is running on port {}", gate_port);
        ioc.run();
    }
    catch (std::exception const& e)
    {
        LOG_ERROR("Exception: {}", e.what());
        return EXIT_FAILURE;
    }
}
