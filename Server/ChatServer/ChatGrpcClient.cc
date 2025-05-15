#include "ChatGrpcClient.h"
#include "RedisMgr.h"
#include "ConfigMgr.h"
#include "UserMgr.h"
#include "CSession.h"
#include "MysqlMgr.h"
#include "Logger.h"

ChatGrpcClient::ChatGrpcClient()
{
    auto& cfg         = ConfigMgr::Inst();
    auto  server_list = cfg["PeerServer"]["Servers"];

    std::vector<std::string> words;

    std::stringstream ss(server_list);
    std::string       word;

    while (std::getline(ss, word, ','))
    {
        words.push_back(word);
    }

    for (auto& word : words)
    {
        if (cfg[word]["Name"].empty())
        {
            continue;
        }
        _pools[cfg[word]["Name"]] = std::make_unique<ChatConPool>(
            5, cfg[word]["Host"], cfg[word]["Port"]);
    }
}

AddFriendRsp ChatGrpcClient::NotifyAddFriend(
    std::string server_ip, const AddFriendReq& req)
{
    AddFriendRsp rsp;
    Defer        defer([&rsp, &req]() {
        rsp.set_error(ErrorCodes::Success);
        rsp.set_applyuid(req.applyuid());
        rsp.set_touid(req.touid());
    });

    auto find_iter = _pools.find(server_ip);
    if (find_iter == _pools.end())
    {
        LOG_ERROR("gRPC client pool don't have this server, server_ip: {}",
            server_ip);
        return rsp;
    }

    LOG_INFO("gRPC client NotifyAddFriend begin, server_ip: {}", server_ip);

    auto&         pool = find_iter->second;
    ClientContext context;
    auto          stub   = pool->getConnection();
    Status        status = stub->NotifyAddFriend(&context, req, &rsp);
    Defer         defercon(
        [&stub, this, &pool]() { pool->returnConnection(std::move(stub)); });

    if (!status.ok())
    {
        LOG_ERROR("gRPC client NotifyAddFriend failed");
        rsp.set_error(ErrorCodes::RPCFailed);
        return rsp;
    }

    LOG_INFO("gRPC client NotifyAddFriend succeed, server_ip: {}",
        server_ip);

    return rsp;
}

bool ChatGrpcClient::GetBaseInfo(
    std::string base_key, int uid, std::shared_ptr<UserInfo>& userinfo)
{
    // 优先查redis中查询用户信息
    LOG_INFO("Redis get user info key: {}", base_key);
    std::string info_str = "";
    bool        b_base   = RedisMgr::GetInstance()->Get(base_key, info_str);
    if (b_base)
    {
        Json::Reader reader;
        Json::Value  root;
        reader.parse(info_str, root);
        userinfo->uid   = root["uid"].asInt();
        userinfo->name  = root["name"].asString();
        userinfo->pwd   = root["pwd"].asString();
        userinfo->email = root["email"].asString();
        userinfo->nick  = root["nick"].asString();
        userinfo->desc  = root["desc"].asString();
        userinfo->sex   = root["sex"].asInt();
        userinfo->icon  = root["icon"].asString();
        LOG_INFO("Redis get user info succeed, key: {}, uid: {}, name: {}, pwd: {}, email: {}",
            base_key, userinfo->uid, userinfo->name, userinfo->pwd, userinfo->email);
    }
    else
    {
        LOG_INFO("Redis get user info failed, key: {}", base_key);
        LOG_INFO("Turn to SQL to get user info, uid: {}", uid);
        // redis中没有则查询mysql
        // 查询数据库
        std::shared_ptr<UserInfo> user_info = nullptr;
        user_info = MysqlMgr::GetInstance()->GetUser(uid);
        if (user_info == nullptr)
        {
            LOG_ERROR("Get user info from mysql failed, uid: {}", uid);
            return false;
        }

        LOG_INFO("Get user info from mysql succeed, uid: {}, name: {}, pwd: {}, email: {}, nick: {}, desc: {}, sex: {}, icon: {}",
            uid, user_info->name, user_info->pwd, user_info->email,
            user_info->nick, user_info->desc, user_info->sex, user_info->icon);

        userinfo = user_info;

        LOG_INFO("Redis set user info key: {}", base_key);
        // 将数据库内容写入redis缓存
        Json::Value redis_root;
        redis_root["uid"]   = uid;
        redis_root["pwd"]   = userinfo->pwd;
        redis_root["name"]  = userinfo->name;
        redis_root["email"] = userinfo->email;
        redis_root["nick"]  = userinfo->nick;
        redis_root["desc"]  = userinfo->desc;
        redis_root["sex"]   = userinfo->sex;
        redis_root["icon"]  = userinfo->icon;
        RedisMgr::GetInstance()->Set(base_key, redis_root.toStyledString());
    }
    return true;
}

AuthFriendRsp ChatGrpcClient::NotifyAuthFriend(
    std::string server_ip, const AuthFriendReq& req)
{
    AuthFriendRsp rsp;
    rsp.set_error(ErrorCodes::Success);

    Defer defer([&rsp, &req]() {
        rsp.set_fromuid(req.fromuid());
        rsp.set_touid(req.touid());
    });

    auto find_iter = _pools.find(server_ip);
    if (find_iter == _pools.end())
    {
        LOG_ERROR("gRPC client pool don't have this server, server_ip: {}",
            server_ip);
        return rsp;
    }

    auto&         pool = find_iter->second;
    ClientContext context;
    auto          stub = pool->getConnection();

    LOG_INFO("gRPC client NotifyAuthFriend begin, server_ip: {}", server_ip);
    Status status = stub->NotifyAuthFriend(&context, req, &rsp);
    Defer  defercon(
        [&stub, this, &pool]() { pool->returnConnection(std::move(stub)); });

    if (!status.ok())
    {
        LOG_ERROR("gRPC client NotifyAuthFriend failed");
        rsp.set_error(ErrorCodes::RPCFailed);
        return rsp;
    }
    LOG_INFO("gRPC client NotifyAuthFriend succeed, server_ip: {}",
        server_ip);

    return rsp;
}

TextChatMsgRsp ChatGrpcClient::NotifyTextChatMsg(std::string server_ip,
    const TextChatMsgReq& req, const Json::Value& rtvalue)
{

    TextChatMsgRsp rsp;
    rsp.set_error(ErrorCodes::Success);

    Defer defer([&rsp, &req]() {
        rsp.set_fromuid(req.fromuid());
        rsp.set_touid(req.touid());
        for (const auto& text_data : req.textmsgs())
        {
            TextChatData* new_msg = rsp.add_textmsgs();
            new_msg->set_msgid(text_data.msgid());
            new_msg->set_msgcontent(text_data.msgcontent());
        }
    });

    auto find_iter = _pools.find(server_ip);
    if (find_iter == _pools.end())
    {
        LOG_ERROR("gRPC client pool don't have this server, server_ip: {}",
            server_ip);
        return rsp;
    }

    auto&         pool = find_iter->second;
    ClientContext context;
    auto          stub = pool->getConnection();

    LOG_INFO("gRPC client NotifyTextChatMsg begin, server_ip: {}", server_ip);

    Status status = stub->NotifyTextChatMsg(&context, req, &rsp);
    Defer  defercon(
        [&stub, this, &pool]() { pool->returnConnection(std::move(stub)); });

    if (!status.ok())
    {
        LOG_ERROR("gRPC client NotifyTextChatMsg failed");
        rsp.set_error(ErrorCodes::RPCFailed);
        return rsp;
    }
    LOG_INFO("gRPC client NotifyTextChatMsg succeed, server_ip: {}",
        server_ip);
    return rsp;
}

KickUserRsp ChatGrpcClient::NotifyKickUser(
    std::string server_ip, const KickUserReq& req)
{
    KickUserRsp rsp;
    Defer       defer([&rsp, &req]() {
        rsp.set_error(ErrorCodes::Success);
        rsp.set_uid(req.uid());
    });

    auto find_iter = _pools.find(server_ip);
    if (find_iter == _pools.end())
    {
        LOG_ERROR("gRPC client pool don't have this server, server_ip: {}",
            server_ip);
        return rsp;
    }

    auto&         pool = find_iter->second;
    ClientContext context;
    auto          stub = pool->getConnection();
    Defer         defercon(
        [&stub, this, &pool]() { pool->returnConnection(std::move(stub)); });

    LOG_INFO("gRPC client NotifyKickUser begin, server_ip: {}", server_ip);
    Status status = stub->NotifyKickUser(&context, req, &rsp);

    if (!status.ok())
    {
        LOG_INFO("gRPC client NotifyKickUser failed");
        rsp.set_error(ErrorCodes::RPCFailed);
        return rsp;
    }
    LOG_INFO("gRPC client NotifyKickUser succeed, server_ip: {}",
        server_ip);
    return rsp;
}
