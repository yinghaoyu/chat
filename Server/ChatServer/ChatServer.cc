#include "LogicSystem.h"
#include "AsioIOServicePool.h"
#include "CServer.h"
#include "ConfigMgr.h"
#include "RedisMgr.h"
#include "ChatServiceImpl.h"
#include "const.h"
#include "Logger.h"

#include <csignal>
#include <thread>

int main()
{
    auto& cfg         = ConfigMgr::Inst();
    auto  server_name = cfg["SelfServer"]["Name"];
    try
    {
        auto pool = AsioIOServicePool::GetInstance();
        // 将登录数设置为0
        RedisMgr::GetInstance()->HSet(LOGIN_COUNT, server_name, "0");

        Defer derfer([server_name]() {
            RedisMgr::GetInstance()->HDel(LOGIN_COUNT, server_name);
        });

        boost::asio::io_context io_context;

        auto port_str = cfg["SelfServer"]["Port"];

        auto cserver = std::make_shared<CServer>(io_context, stoi(port_str));

        cserver->Start();

        // 定义一个GrpcServer

        std::string server_address(
            cfg["SelfServer"]["Host"] + ":" + cfg["SelfServer"]["RPCPort"]);
        ChatServiceImpl     service;
        grpc::ServerBuilder builder;
        // 监听端口和添加服务
        builder.AddListeningPort(
            server_address, grpc::InsecureServerCredentials());
        builder.RegisterService(&service);
        service.RegisterServer(cserver);
        // 构建并启动gRPC服务器
        std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
        LOG_INFO("gRPC Server listening on {}", server_address);

        // 单独启动一个线程处理grpc服务
        std::thread grpc_server_thread([&server]() { server->Wait(); });

        boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait([&io_context, pool, &cserver, &server](auto, auto) {
            LOG_INFO("Stopping server...");
            // FIXME(yinghaoyu):
            // 这里timer.cancel()与io_context.stop()在Windows和Linux表现不一样
            // Windows先调用timer.cancel()，后调用io_context.stop()会产生dump
            cserver->Shutdown();
            io_context.stop();
            pool->Stop();
            server->Shutdown();
            LogicSystem::GetInstance()->Shutdown();
        });

        // 将Cserver注册给逻辑类方便以后清除连接
        LogicSystem::GetInstance()->SetServer(cserver);
        io_context.run();

        grpc_server_thread.join();
    }
    catch (std::exception& e)
    {
        LOG_ERROR("Exception: {}", e.what());
    }
}
