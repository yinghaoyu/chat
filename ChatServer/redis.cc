#include "redis.h"

#include <memory.h>
#include <iostream>
#include <functional>

redisReply* RedisReplyClone(redisReply* r)
{
    redisReply* c = (redisReply*)calloc(1, sizeof(*c));
    c->type       = r->type;

    switch (r->type)
    {
        case REDIS_REPLY_INTEGER: c->integer = r->integer; break;
        case REDIS_REPLY_ARRAY:
            if (r->element != NULL && r->elements > 0)
            {
                c->element  = (redisReply**)calloc(r->elements, sizeof(r));
                c->elements = r->elements;
                for (size_t i = 0; i < r->elements; ++i)
                {
                    c->element[i] = RedisReplyClone(r->element[i]);
                }
            }
            break;
        case REDIS_REPLY_ERROR:
        case REDIS_REPLY_STATUS:
        case REDIS_REPLY_STRING:
            if (r->str == NULL)
            {
                c->str = NULL;
            }
            else
            {
                // c->str = strndup(r->str, r->len);
                c->str = (char*)malloc(r->len + 1);
                memcpy(c->str, r->str, r->len);
                c->str[r->len] = '\0';
            }
            c->len = r->len;
            break;
    }
    return c;
}

Redis::Redis(const std::string& host, int32_t port, const std::string& passwd)
{
    m_host           = host;
    m_port           = port;
    m_passwd         = passwd;
    uint64_t timeout = 1000;  // ms

    m_cmdTimeout.tv_sec  = timeout / 1000;
    m_cmdTimeout.tv_usec = timeout % 1000 * 1000;
}

bool Redis::reconnect()
{
    if (redisReconnect(m_context.get()) == REDIS_ERR)
    {
        std::cout << "redisReconnect error: (" << m_host << ":" << m_port << ")"
                  << std::endl;
        return false;
    }
    if (!m_passwd.empty())
    {
        auto r = (redisReply*)redisCommand(
            m_context.get(), "auth %s", m_passwd.c_str());

        if (!r)
        {
            std::cout << "auth error:(" << m_host << ":" << m_port << ")"
                      << std::endl;
            return false;
        }

        ReplyPtr rt(r, freeReplyObject);

        if (rt->type != REDIS_REPLY_STATUS)
        {
            std::cout << "auth reply type error:" << rt->type << "(" << m_host
                      << ":" << m_port << ")" << std::endl;
            return false;
        }
        if (!rt->str)
        {
            std::cout << "auth reply str error: NULL(" << m_host << ":"
                      << m_port << ")" << std::endl;
            return false;
        }
        if (strcmp(rt->str, "OK") == 0)
        {
            return true;
        }
        else
        {
            std::cout << "auth error: " << rt->str << "(" << m_host << ":"
                      << m_port << ")" << std::endl;
            return false;
        }
    }
    return true;
}

bool Redis::connect() { return connect(m_host, m_port, 50); }

bool Redis::connect(const std::string& ip, int port, uint64_t ms)
{
    m_host      = ip;
    m_port      = port;
    m_connectMs = ms;
    if (m_context)
    {
        return true;
    }
    timeval tv = {(int)ms / 1000, (int)ms % 1000 * 1000};
    auto    c  = redisConnectWithTimeout(ip.c_str(), port, tv);
    if (c)
    {
        m_context.reset(c, redisFree);
        if (m_cmdTimeout.tv_sec || m_cmdTimeout.tv_usec)
        {
            setTimeout(
                m_cmdTimeout.tv_sec * 1000 + m_cmdTimeout.tv_usec / 1000);
        }

        if (!m_passwd.empty())
        {
            auto r = (redisReply*)redisCommand(c, "auth %s", m_passwd.c_str());

            if (!r)
            {
                std::cout << "auth error:(" << m_host << ":" << m_port << ")"
                          << std::endl;
                return false;
            }

            ReplyPtr rt(r, freeReplyObject);

            if (rt->type != REDIS_REPLY_STATUS)
            {
                std::cout << "auth reply type error:" << rt->type << "("
                          << m_host << ":" << m_port << ")" << std::endl;
                return false;
            }
            if (!rt->str)
            {
                std::cout << "auth reply str error: NULL(" << m_host << ":"
                          << m_port << ")" << std::endl;
                return false;
            }
            if (strcmp(rt->str, "OK") == 0)
            {
                return true;
            }
            else
            {
                std::cout << "auth error: " << rt->str << "(" << m_host << ":"
                          << m_port << ")" << std::endl;
                return false;
            }
        }
        return true;
    }
    return false;
}

bool Redis::setTimeout(uint64_t v)
{
    m_cmdTimeout.tv_sec  = v / 1000;
    m_cmdTimeout.tv_usec = v % 1000 * 1000;
    redisSetTimeout(m_context.get(), m_cmdTimeout);
    return true;
}

ReplyPtr Redis::cmd(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    ReplyPtr rt = cmd(fmt, ap);
    va_end(ap);
    return rt;
}

ReplyPtr Redis::cmd(const char* fmt, va_list ap)
{
    auto r = (redisReply*)redisvCommand(m_context.get(), fmt, ap);

    if (!r)
    {
        std::cout << "redisCommand error: (" << fmt << ")(" << m_host << ":"
                  << m_port << ")" << std::endl;
        return nullptr;
    }

    ReplyPtr rt(r, freeReplyObject);
    if (r->type != REDIS_REPLY_ERROR)
    {
        return rt;
    }

    std::cout << "redisCommand error: (" << fmt << ")(" << m_host << ":"
              << m_port << ")"
              << ": " << r->str << std::endl;
    return nullptr;
}

ReplyPtr Redis::cmd(const std::vector<std::string>& argv)
{
    std::vector<const char*> v;
    std::vector<size_t>      l;
    for (auto& i : argv)
    {
        v.push_back(i.c_str());
        l.push_back(i.size());
    }

    auto r = (redisReply*)redisCommandArgv(
        m_context.get(), argv.size(), &v[0], &l[0]);
    if (!r)
    {
        std::cout << "redisCommandArgv error: (" << m_host << ":" << m_port
                  << ")" << std::endl;
        return nullptr;
    }
    ReplyPtr rt(r, freeReplyObject);
    if (r->type != REDIS_REPLY_ERROR)
    {
        return rt;
    }

    std::cout << "redisCommandArgv error: (" << m_host << ":" << m_port << ")("
              << r->str << ")" << std::endl;
    return nullptr;
}

ReplyPtr Redis::getReply()
{
    redisReply* r = nullptr;
    if (redisGetReply(m_context.get(), (void**)&r) == REDIS_OK)
    {
        ReplyPtr rt(r, freeReplyObject);
        return rt;
    }
    std::cout << "redisGetReply error: (" << m_host << ":" << m_port << ")"
              << std::endl;
    return nullptr;
}

int Redis::appendCmd(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int rt = appendCmd(fmt, ap);
    va_end(ap);
    return rt;
}

int Redis::appendCmd(const char* fmt, va_list ap)
{
    return redisvAppendCommand(m_context.get(), fmt, ap);
}

int Redis::appendCmd(const std::vector<std::string>& argv)
{
    std::vector<const char*> v;
    std::vector<size_t>      l;
    for (auto& i : argv)
    {
        v.push_back(i.c_str());
        l.push_back(i.size());
    }
    return redisAppendCommandArgv(m_context.get(), argv.size(), &v[0], &l[0]);
}

IRedis::ptr RedisPool::get()
{
    std::unique_lock<std::mutex> lock(m_mutex);

    IRedis* r = nullptr;
    if (!m_conns.empty())
    {
        r = m_conns.front();
        m_conns.pop_front();
        lock.unlock();
        auto rr = dynamic_cast<ISyncRedis*>(r);
        if ((time(0) - rr->getLastActiveTime()) > 30)
        {
            if (!rr->cmd("ping"))
            {
                delete r;
                return nullptr;
            }
            if (!rr->reconnect())
            {
                delete r;
                return nullptr;
            }
        }
        rr->setLastActiveTime(time(0));
        return std::shared_ptr<IRedis>(
            r, std::bind(&RedisPool::freeRedis, this, std::placeholders::_1));
    }
    lock.unlock();
    Redis* rds = new Redis(m_host, m_port, m_passwd);
    if (!rds->connect())
    {
        delete rds;
        return nullptr;
    }
    rds->setLastActiveTime(time(0));
    return std::shared_ptr<IRedis>(
        rds, std::bind(&RedisPool::freeRedis, this, std::placeholders::_1));
}

void RedisPool::freeRedis(IRedis* r)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_conns.size() < m_maxConn)
    {
        m_conns.push_back(r);
        return;
    }
    delete r;
}

RedisPool::RedisPool(const std::string& host, int32_t port,
    const std::string& passwd, int poolSize)
    : m_host(host), m_port(port), m_passwd(passwd), m_maxConn(poolSize)
{}

RedisPool::~RedisPool()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto& i : m_conns)
    {
        delete i;
    }
}