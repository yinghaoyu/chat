#include "RedisMgr.h"
#include "const.h"
#include "ConfigMgr.h"
#include "DistLock.h"

RedisMgr::RedisMgr()
{
    auto& gCfgMgr = ConfigMgr::Inst();
    auto  host    = gCfgMgr["Redis"]["Host"];
    auto  port    = gCfgMgr["Redis"]["Port"];
    auto  pwd     = gCfgMgr["Redis"]["Passwd"];
    _con_pool.reset(new RedisPool(host, stoi(port), pwd, 10));
}

RedisMgr::~RedisMgr() {}

bool RedisMgr::Get(const std::string& key, std::string& value)
{
    auto connect = _con_pool->get();
    if (connect == nullptr)
    {
        return false;
    }
    auto reply = connect->cmd("GET %s", key.c_str());
    if (reply == nullptr)
    {
        std::cout << "[ GET  " << key << " ] failed" << std::endl;
        return false;
    }

    if (reply->type != REDIS_REPLY_STRING)
    {
        std::cout << "[ GET  " << key << " ] failed" << std::endl;
        return false;
    }

    value = reply->str;

    std::cout << "Succeed to execute command [ GET " << key << "  ]"
              << std::endl;
    return true;
}

bool RedisMgr::Set(const std::string& key, const std::string& value)
{
    // 执行redis命令行
    auto connect = _con_pool->get();
    if (connect == nullptr)
    {
        return false;
    }
    auto reply = connect->cmd("SET %s %s", key.c_str(), value.c_str());

    // 如果返回NULL则说明执行失败
    if (nullptr == reply)
    {
        std::cout << "Execut command [ SET " << key << "  " << value
                  << " ] failure" << std::endl;
        return false;
    }

    // 如果执行失败则释放连接
    if (!(reply->type == REDIS_REPLY_STATUS &&
            (strcmp(reply->str, "OK") == 0 || strcmp(reply->str, "ok") == 0)))
    {
        std::cout << "Execut command [ SET " << key << "  " << value
                  << " ] failure" << std::endl;
        return false;
    }

    std::cout << "Execut command [ SET " << key << "  " << value << " ] success"
              << std::endl;
    return true;
}

bool RedisMgr::LPush(const std::string& key, const std::string& value)
{
    auto connect = _con_pool->get();
    if (connect == nullptr)
    {
        return false;
    }
    auto reply = connect->cmd("LPUSH %s %s", key.c_str(), value.c_str());
    if (nullptr == reply)
    {
        std::cout << "Execut command [ LPUSH " << key << "  " << value
                  << " ] failure" << std::endl;

        return false;
    }

    if (reply->type != REDIS_REPLY_INTEGER || reply->integer <= 0)
    {
        std::cout << "Execut command [ LPUSH " << key << "  " << value
                  << " ] failure" << std::endl;

        return false;
    }

    std::cout << "Execut command [ LPUSH " << key << "  " << value
              << " ] success" << std::endl;

    return true;
}

bool RedisMgr::LPop(const std::string& key, std::string& value)
{
    auto connect = _con_pool->get();
    if (connect == nullptr)
    {
        return false;
    }
    auto reply = connect->cmd("LPOP %s ", key.c_str());
    if (reply == nullptr)
    {
        std::cout << "Execut command [ LPOP " << key << " ] failure"
                  << std::endl;

        return false;
    }

    if (reply->type == REDIS_REPLY_NIL)
    {
        std::cout << "Execut command [ LPOP " << key << " ] failure"
                  << std::endl;

        return false;
    }

    value = reply->str;
    std::cout << "Execut command [ LPOP " << key << " ] success" << std::endl;

    return true;
}

bool RedisMgr::RPush(const std::string& key, const std::string& value)
{
    auto connect = _con_pool->get();
    if (connect == nullptr)
    {
        return false;
    }
    auto reply = connect->cmd("RPUSH %s %s", key.c_str(), value.c_str());
    if (nullptr == reply)
    {
        std::cout << "Execut command [ RPUSH " << key << "  " << value
                  << " ] failure" << std::endl;

        return false;
    }

    if (reply->type != REDIS_REPLY_INTEGER || reply->integer <= 0)
    {
        std::cout << "Execut command [ RPUSH " << key << "  " << value
                  << " ] failure" << std::endl;

        return false;
    }

    std::cout << "Execut command [ RPUSH " << key << "  " << value
              << " ] success" << std::endl;

    return true;
}
bool RedisMgr::RPop(const std::string& key, std::string& value)
{
    auto connect = _con_pool->get();
    if (connect == nullptr)
    {
        return false;
    }
    auto reply = connect->cmd("RPOP %s ", key.c_str());
    if (reply == nullptr)
    {
        std::cout << "Execut command [ RPOP " << key << " ] failure"
                  << std::endl;

        return false;
    }

    if (reply->type == REDIS_REPLY_NIL)
    {
        std::cout << "Execut command [ RPOP " << key << " ] failure"
                  << std::endl;

        return false;
    }
    value = reply->str;
    std::cout << "Execut command [ RPOP " << key << " ] success" << std::endl;

    return true;
}

bool RedisMgr::HSet(
    const std::string& key, const std::string& hkey, const std::string& value)
{
    auto connect = _con_pool->get();
    if (connect == nullptr)
    {
        return false;
    }
    auto reply =
        connect->cmd("HSET %s %s %s", key.c_str(), hkey.c_str(), value.c_str());
    if (reply == nullptr)
    {
        std::cout << "Execut command [ HSet " << key << "  " << hkey << "  "
                  << value << " ] failure" << std::endl;

        return false;
    }

    if (reply->type != REDIS_REPLY_INTEGER)
    {
        std::cout << "Execut command [ HSet " << key << "  " << hkey << "  "
                  << value << " ] failure" << std::endl;

        return false;
    }

    std::cout << "Execut command [ HSet " << key << "  " << hkey << "  "
              << value << " ] success" << std::endl;

    return true;
}

bool RedisMgr::HSet(
    const char* key, const char* hkey, const char* hvalue, size_t hvaluelen)
{
    auto connect = _con_pool->get();
    if (connect == nullptr)
    {
        return false;
    }

    auto reply = connect->cmd({"HSET", key, hkey, hvalue});

    if (reply == nullptr)
    {
        std::cout << "Execut command [ HSet " << key << "  " << hkey << "  "
                  << hvalue << " ] failure" << std::endl;

        return false;
    }

    if (reply->type != REDIS_REPLY_INTEGER)
    {
        std::cout << "Execut command [ HSet " << key << "  " << hkey << "  "
                  << hvalue << " ] failure" << std::endl;

        return false;
    }
    std::cout << "Execut command [ HSet " << key << "  " << hkey << "  "
              << hvalue << " ] success" << std::endl;

    return true;
}

std::string RedisMgr::HGet(const std::string& key, const std::string& hkey)
{
    auto connect = _con_pool->get();
    if (connect == nullptr)
    {
        return "";
    }

    auto reply = connect->cmd({"HGET", key, hkey});
    if (reply == nullptr)
    {
        std::cout << "Execut command [ HGet " << key << " " << hkey
                  << "  ] failure" << std::endl;

        return "";
    }

    if (reply->type == REDIS_REPLY_NIL)
    {

        std::cout << "Execut command [ HGet " << key << " " << hkey
                  << "  ] failure" << std::endl;

        return "";
    }

    std::string value = reply->str;

    std::cout << "Execut command [ HGet " << key << " " << hkey << " ] success"
              << std::endl;
    return value;
}

bool RedisMgr::HDel(const std::string& key, const std::string& field)
{
    auto connect = _con_pool->get();
    if (connect == nullptr)
    {
        return false;
    }

    auto reply = connect->cmd("HDEL %s %s", key.c_str(), field.c_str());
    if (reply == nullptr)
    {
        std::cerr << "HDEL command failed" << std::endl;
        return false;
    }

    bool success = false;
    if (reply->type == REDIS_REPLY_INTEGER)
    {
        success = reply->integer > 0;
    }

    return success;
}

bool RedisMgr::Del(const std::string& key)
{
    auto connect = _con_pool->get();
    if (connect == nullptr)
    {
        return false;
    }
    auto reply = connect->cmd("DEL %s", key.c_str());
    if (reply == nullptr)
    {
        std::cout << "Execut command [ Del " << key << " ] failure"
                  << std::endl;

        return false;
    }

    if (reply->type != REDIS_REPLY_INTEGER)
    {
        std::cout << "Execut command [ Del " << key << " ] failure"
                  << std::endl;

        return false;
    }

    std::cout << "Execut command [ Del " << key << " ] success" << std::endl;

    return true;
}

bool RedisMgr::ExistsKey(const std::string& key)
{
    auto connect = _con_pool->get();
    if (connect == nullptr)
    {
        return false;
    }

    auto reply = connect->cmd("exists %s", key.c_str());
    if (reply == nullptr)
    {
        std::cout << "Not Found [ Key " << key << " ] " << std::endl;

        return false;
    }

    if (reply->type != REDIS_REPLY_INTEGER || reply->integer == 0)
    {
        std::cout << "Not Found [ Key " << key << " ] " << std::endl;

        return false;
    }
    std::cout << " Found [ Key " << key << " ] exists" << std::endl;

    return true;
}

std::string RedisMgr::acquireLock(
    const std::string& lockName, int lockTimeout, int acquireTimeout)
{

    auto connect = _con_pool->get();
    if (connect == nullptr)
    {
        return "";
    }

    Defer defer([&connect, this]() {});

    return DistLock::Inst().acquireLock(
        connect, lockName, lockTimeout, acquireTimeout);
}

bool RedisMgr::releaseLock(
    const std::string& lockName, const std::string& identifier)
{
    if (identifier.empty())
    {
        return true;
    }
    auto connect = _con_pool->get();
    if (connect == nullptr)
    {
        return false;
    }

    Defer defer([&connect, this]() {});

    return DistLock::Inst().releaseLock(connect, lockName, identifier);
}

void RedisMgr::IncreaseCount(std::string server_name)
{
    auto lock_key   = LOCK_COUNT;
    auto identifier = RedisMgr::GetInstance()->acquireLock(
        lock_key, LOCK_TIME_OUT, ACQUIRE_TIME_OUT);
    // 利用defer解锁
    Defer defer2([this, identifier, lock_key]() {
        RedisMgr::GetInstance()->releaseLock(lock_key, identifier);
    });

    // 将登录数量增加
    auto rd_res = RedisMgr::GetInstance()->HGet(LOGIN_COUNT, server_name);
    int  count  = 0;
    if (!rd_res.empty())
    {
        count = std::stoi(rd_res);
    }

    count++;
    auto count_str = std::to_string(count);
    RedisMgr::GetInstance()->HSet(LOGIN_COUNT, server_name, count_str);
}

void RedisMgr::DecreaseCount(std::string server_name)
{
    auto lock_key   = LOCK_COUNT;
    auto identifier = RedisMgr::GetInstance()->acquireLock(
        lock_key, LOCK_TIME_OUT, ACQUIRE_TIME_OUT);
    // 利用defer解锁
    Defer defer2([this, identifier, lock_key]() {
        RedisMgr::GetInstance()->releaseLock(lock_key, identifier);
    });

    // 将登录数量减少
    auto rd_res = RedisMgr::GetInstance()->HGet(LOGIN_COUNT, server_name);
    int  count  = 0;
    if (!rd_res.empty())
    {
        count = std::stoi(rd_res);
        if (count > 0)
        {
            count--;
        }
    }

    auto count_str = std::to_string(count);
    RedisMgr::GetInstance()->HSet(LOGIN_COUNT, server_name, count_str);
}

void RedisMgr::InitCount(std::string server_name)
{
    auto lock_key   = LOCK_COUNT;
    auto identifier = RedisMgr::GetInstance()->acquireLock(
        lock_key, LOCK_TIME_OUT, ACQUIRE_TIME_OUT);
    // 利用defer解锁
    Defer defer2([this, identifier, lock_key]() {
        RedisMgr::GetInstance()->releaseLock(lock_key, identifier);
    });

    RedisMgr::GetInstance()->HSet(LOGIN_COUNT, server_name, "0");
}

void RedisMgr::DelCount(std::string server_name)
{
    auto lock_key   = LOCK_COUNT;
    auto identifier = RedisMgr::GetInstance()->acquireLock(
        lock_key, LOCK_TIME_OUT, ACQUIRE_TIME_OUT);
    // 利用defer解锁
    Defer defer2([this, identifier, lock_key]() {
        RedisMgr::GetInstance()->releaseLock(lock_key, identifier);
    });

    RedisMgr::GetInstance()->HDel(LOGIN_COUNT, server_name);
}
