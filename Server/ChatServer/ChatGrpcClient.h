#pragma once

#include "Singleton.h"
#include "data.h"
#include "message.grpc.pb.h"
#include "message.pb.h"

#include <condition_variable>
#include <grpcpp/grpcpp.h>
#include <jsoncpp/json/json.h>
#include <jsoncpp/json/reader.h>
#include <jsoncpp/json/value.h>
#include <queue>

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using message::AddFriendReq;
using message::AddFriendRsp;

using message::AuthFriendReq;
using message::AuthFriendRsp;

using message::ChatService;
using message::GetChatServerRsp;
using message::LoginReq;
using message::LoginRsp;

using message::TextChatData;
using message::TextChatMsgReq;
using message::TextChatMsgRsp;

using message::KickUserReq;
using message::KickUserRsp;

class ChatConPool
{
  public:
    ChatConPool(
        const size_t poolSize, const std::string& host, const std::string& port)
        : poolSize_(poolSize), host_(host), port_(port), stopped_(false)
    {
        for (size_t i = 0; i < poolSize_; ++i)
        {

            std::shared_ptr<Channel> channel = grpc::CreateChannel(
                host + ":" + port, grpc::InsecureChannelCredentials());

            connections_.push(ChatService::NewStub(channel));
        }
    }

    ~ChatConPool()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        Close();
        while (!connections_.empty())
        {
            connections_.pop();
        }
    }

    std::unique_ptr<ChatService::Stub> getConnection()
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

    void returnConnection(std::unique_ptr<ChatService::Stub> context)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        connections_.push(std::move(context));
        cond_.notify_one();
    }

    void Close()
    {
        stopped_ = true;
        cond_.notify_all();
    }

  private:
    std::atomic<bool>                              stopped_;
    size_t                                         poolSize_;
    std::string                                    host_;
    std::string                                    port_;
    std::queue<std::unique_ptr<ChatService::Stub>> connections_;
    std::mutex                                     mutex_;
    std::condition_variable                        cond_;
};

class ChatGrpcClient : public Singleton<ChatGrpcClient>
{
    friend class Singleton<ChatGrpcClient>;

  public:
    ~ChatGrpcClient() {}

    AddFriendRsp NotifyAddFriend(
        const std::string& server_ip, const AddFriendReq& req);

    AuthFriendRsp NotifyAuthFriend(
        const std::string& server_ip, const AuthFriendReq& req);

    bool GetBaseInfo(const std::string& base_key, int uid,
        std::shared_ptr<UserInfo>& userinfo);

    TextChatMsgRsp NotifyTextChatMsg(const std::string& server_ip,
        const TextChatMsgReq& req, const Json::Value& rtvalue);

    KickUserRsp NotifyKickUser(
        const std::string& server_ip, const KickUserReq& req);

  private:
    ChatGrpcClient();
    std::unordered_map<std::string, std::unique_ptr<ChatConPool>> pools_;
};
