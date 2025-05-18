#include "AsioIOServicePool.h"

#include <iostream>

using namespace std;

AsioIOServicePool::AsioIOServicePool(std::size_t size)
    : ioServices_(size), workGuards_(size), nextIOService_(0)
{
    for (std::size_t i = 0; i < size; ++i)
    {
        // 使用 make_work_guard 创建 WorkGuard
        workGuards_[i] = std::make_unique<WorkGuard>(
            boost::asio::make_work_guard(ioServices_[i]));
    }

    // 遍历多个 io_service，创建多个线程，每个线程内部启动 io_service
    for (std::size_t i = 0; i < ioServices_.size(); ++i)
    {
        threads_.emplace_back([this, i]() { ioServices_[i].run(); });
    }
}

AsioIOServicePool::~AsioIOServicePool()
{
    Stop();
    std::cout << "AsioIOServicePool dtor~" << endl;
}

boost::asio::io_context& AsioIOServicePool::GetIOService()
{
    auto& service = ioServices_[nextIOService_++];
    if (nextIOService_ == ioServices_.size())
    {
        nextIOService_ = 0;
    }
    return service;
}

void AsioIOServicePool::Stop()
{
    // 释放 WorkGuard，允许 io_context 停止
    workGuards_.clear();  // 这将自动释放所有 WorkGuard

    for (auto& t : threads_)
    {
        if (t.joinable())
        {
            t.join();
        }
    }
}
