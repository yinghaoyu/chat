#include "AsioIOServicePool.h"

#include <iostream>

using namespace std;

AsioIOServicePool::AsioIOServicePool(std::size_t size)
    : _ioServices(size), _workGuards(size), _nextIOService(0)
{
    for (std::size_t i = 0; i < size; ++i)
    {
        // 使用 make_work_guard 创建 WorkGuard
        _workGuards[i] = std::make_unique<WorkGuard>(
            boost::asio::make_work_guard(_ioServices[i]));
    }

    // 遍历多个 io_service，创建多个线程，每个线程内部启动 io_service
    for (std::size_t i = 0; i < _ioServices.size(); ++i)
    {
        _threads.emplace_back([this, i]() { _ioServices[i].run(); });
    }
}

AsioIOServicePool::~AsioIOServicePool()
{
    Stop();
    std::cout << "AsioIOServicePool dtor~" << endl;
}

boost::asio::io_context& AsioIOServicePool::GetIOService()
{
    auto& service = _ioServices[_nextIOService++];
    if (_nextIOService == _ioServices.size())
    {
        _nextIOService = 0;
    }
    return service;
}

void AsioIOServicePool::Stop()
{
    // 释放 WorkGuard，允许 io_context 停止
    _workGuards.clear();  // 这将自动释放所有 WorkGuard

    for (auto& t : _threads)
    {
        if (t.joinable())
        {
            t.join();
        }
    }
}