#pragma once

#include "Singleton.h"
#include "const.h"
#include "message.grpc.pb.h"

#include <condition_variable>
#include <grpcpp/grpcpp.h>
#include <memory>
#include <queue>
#include <string>

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using message::GetVarifyReq;
using message::GetVarifyRsp;
using message::VarifyService;

class RPConPool
{
  public:
    RPConPool(size_t poolSize, std::string host, std::string port)
        : poolSize_(poolSize), host_(host), port_(port), stopped_(false)
    {
        for (size_t i = 0; i < poolSize_; ++i)
        {

            std::shared_ptr<Channel> channel = grpc::CreateChannel(
                host + ":" + port, grpc::InsecureChannelCredentials());

            connections_.push(VarifyService::NewStub(channel));
        }
    }

    ~RPConPool()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        Close();
        while (!connections_.empty())
        {
            connections_.pop();
        }
    }

    std::unique_ptr<VarifyService::Stub> getConnection()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_.wait(lock, [this] {
            if (stopped_)
            {
                return true;
            }
            return !connections_.empty();
        });
        // 如果停止则直接返回空指针
        if (stopped_)
        {
            return nullptr;
        }
        auto context = std::move(connections_.front());
        connections_.pop();
        return context;
    }

    void returnConnection(std::unique_ptr<VarifyService::Stub> context)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (stopped_)
        {
            return;
        }
        connections_.push(std::move(context));
        cond_.notify_one();
    }

    void Close()
    {
        stopped_ = true;
        cond_.notify_all();
    }

  private:
    atomic<bool>                                     stopped_;
    size_t                                           poolSize_;
    std::string                                      host_;
    std::string                                      port_;
    std::queue<std::unique_ptr<VarifyService::Stub>> connections_;
    std::mutex                                       mutex_;
    std::condition_variable                          cond_;
};

class VerifyGrpcClient : public Singleton<VerifyGrpcClient>
{
    friend class Singleton<VerifyGrpcClient>;

  public:
    ~VerifyGrpcClient() {}
    GetVarifyRsp GetVarifyCode(std::string email)
    {
        ClientContext context;
        GetVarifyRsp  reply;
        GetVarifyReq  request;
        request.set_email(email);
        auto   stub   = pool_->getConnection();
        Status status = stub->GetVarifyCode(&context, request, &reply);

        if (status.ok())
        {
            pool_->returnConnection(std::move(stub));
            return reply;
        }
        else
        {
            pool_->returnConnection(std::move(stub));
            reply.set_error(ErrorCodes::RPCFailed);
            return reply;
        }
    }

  private:
    VerifyGrpcClient();

    std::unique_ptr<RPConPool> pool_;
};
