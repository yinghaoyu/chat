#pragma once
#include "Singleton.h"
#include "CSession.h"
#include "data.h"

#include <jsoncpp/json/json.h>
#include <jsoncpp/json/value.h>
#include <jsoncpp/json/reader.h>
#include <vector>
#include <map>
#include <functional>
#include <thread>

class CServer;
typedef function<void(
    shared_ptr<CSession>, const short& msg_id, const string& msg_data)>
    FunCallBack;
class LogicSystem : public Singleton<LogicSystem>
{
    friend class Singleton<LogicSystem>;

  public:
    void Shutdown();
    void PostMsgToQue(shared_ptr<LogicNode> msg);
    void SetServer(std::shared_ptr<CServer> pserver);

  private:
    LogicSystem();
    void DealMsg();
    void RegisterCallBacks();
    void LoginHandler(shared_ptr<CSession> session, const short& msg_id,
        const string& msg_data);
    void SearchInfo(std::shared_ptr<CSession> session, const short& msg_id,
        const string& msg_data);
    void AddFriendApply(std::shared_ptr<CSession> session, const short& msg_id,
        const string& msg_data);
    void AuthFriendApply(std::shared_ptr<CSession> session, const short& msg_id,
        const string& msg_data);
    void DealChatTextMsg(std::shared_ptr<CSession> session, const short& msg_id,
        const string& msg_data);
    void HeartBeatHandler(std::shared_ptr<CSession> session,
        const short& msg_id, const string& msg_data);
    bool isPureDigit(const std::string& str);
    void GetUserByUid(std::string uid_str, Json::Value& rtvalue);
    void GetUserByName(std::string name, Json::Value& rtvalue);
    bool GetBaseInfo(
        std::string base_key, int uid, std::shared_ptr<UserInfo>& userinfo);
    bool GetFriendApplyInfo(
        int to_uid, std::vector<std::shared_ptr<ApplyInfo>>& list);
    bool GetFriendList(
        int self_id, std::vector<std::shared_ptr<UserInfo>>& user_list);
    std::vector<std::thread>           _worker_threads;
    std::vector<shared_ptr<LogicNode>> _msg_que;
    std::mutex                         _mutex;
    std::condition_variable            _consume;
    bool                               _b_stop;
    std::map<short, FunCallBack>       _fun_callbacks;
    std::shared_ptr<CServer>           _p_server;
};
