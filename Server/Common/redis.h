#pragma once

#include <hiredis/hiredis.h>
#include <stdlib.h>
#include <sys/time.h>
#include <memory>
#include <string>
#include <list>
#include <vector>
#include <mutex>

typedef std::shared_ptr<redisReply> ReplyPtr;

class IRedis
{
  public:
    enum Type
    {
        REDIS         = 1,
        REDIS_CLUSTER = 2,
    };
    typedef std::shared_ptr<IRedis> ptr;
    IRedis() {}
    virtual ~IRedis() {}

    virtual ReplyPtr cmd(const char* fmt, ...)                 = 0;
    virtual ReplyPtr cmd(const char* fmt, va_list ap)          = 0;
    virtual ReplyPtr cmd(const std::vector<std::string>& argv) = 0;

    const std::string& getPasswd() const { return m_passwd; }
    void               setPasswd(const std::string& v) { m_passwd = v; }

  protected:
    std::string m_passwd;
};

class ISyncRedis : public IRedis
{
  public:
    typedef std::shared_ptr<ISyncRedis> ptr;
    virtual ~ISyncRedis() {}

    virtual bool reconnect()                                               = 0;
    virtual bool connect(const std::string& ip, int port, uint64_t ms = 0) = 0;
    virtual bool connect()                                                 = 0;
    virtual bool setTimeout(uint64_t ms)                                   = 0;

    virtual int appendCmd(const char* fmt, ...)                 = 0;
    virtual int appendCmd(const char* fmt, va_list ap)          = 0;
    virtual int appendCmd(const std::vector<std::string>& argv) = 0;

    virtual ReplyPtr getReply() = 0;

    uint64_t getLastActiveTime() const { return m_lastActiveTime; }
    void     setLastActiveTime(uint64_t v) { m_lastActiveTime = v; }

  protected:
    uint64_t m_lastActiveTime;
};

class Redis : public ISyncRedis
{
  public:
    typedef std::shared_ptr<Redis> ptr;
    Redis(const std::string& host, int32_t port, const std::string& passwd);

    virtual bool reconnect();
    virtual bool connect(const std::string& ip, int port, uint64_t ms = 0);
    virtual bool connect();
    virtual bool setTimeout(uint64_t ms);

    virtual ReplyPtr cmd(const char* fmt, ...);
    virtual ReplyPtr cmd(const char* fmt, va_list ap);
    virtual ReplyPtr cmd(const std::vector<std::string>& argv);

    virtual int appendCmd(const char* fmt, ...);
    virtual int appendCmd(const char* fmt, va_list ap);
    virtual int appendCmd(const std::vector<std::string>& argv);

    virtual ReplyPtr getReply();

  private:
    std::string                   m_host;
    uint32_t                      m_port;
    uint32_t                      m_connectMs;
    struct timeval                m_cmdTimeout;
    std::shared_ptr<redisContext> m_context;
};

class RedisPool
{
  public:
    RedisPool(const std::string& host, int32_t port, const std::string& passwd,
        int poolSize);

    ~RedisPool();
    IRedis::ptr get();

  private:
    void freeRedis(IRedis* r);

  private:
    std::string m_host;
    int32_t     m_port;
    std::string m_passwd;
    int32_t     m_maxConn;

    std::mutex         m_mutex;
    std::list<IRedis*> m_conns;
};
