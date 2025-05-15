// StatusServer.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "StatusServiceImpl.h"
#include "RedisMgr.h"
#include "MysqlMgr.h"
#include "AsioIOServicePool.h"
#include "const.h"
#include "ConfigMgr.h"
#include "Logger.h"

#include <iostream>
#include <jsoncpp/json/json.h>
#include <jsoncpp/json/value.h>
#include <jsoncpp/json/reader.h>
#include <hiredis/hiredis.h>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <boost/asio.hpp>

void RunServer()
{
    auto& cfg = ConfigMgr::Inst();

    std::string server_address(
        cfg["StatusServer"]["Host"] + ":" + cfg["StatusServer"]["Port"]);
    StatusServiceImpl service;

    grpc::ServerBuilder builder;
    // 监听端口和添加服务
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    // 构建并启动gRPC服务器
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    LOG_INFO("RPC Server listening on {}", server_address);

    // 创建Boost.Asio的io_context
    boost::asio::io_context io_context;
    // 创建signal_set用于捕获SIGINT
    boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);

    // 设置异步等待SIGINT信号
    signals.async_wait(
        [&server, &io_context](
            const boost::system::error_code& error, int signal_number) {
            if (!error)
            {
                LOG_INFO("Received signal {}: shutting down server...", signal_number);
                server->Shutdown();  // 优雅地关闭服务器
                io_context.stop();   // 停止io_context
            }
        });

    // 在单独的线程中运行io_context
    std::thread([&io_context]() { io_context.run(); }).detach();

    // 等待服务器关闭
    server->Wait();
}

int main(int argc, char** argv)
{
    try
    {
        RunServer();
    }
    catch (std::exception const& e)
    {
        LOG_ERROR("Exception: {}", e.what());
        return EXIT_FAILURE;
    }

    return 0;
}
