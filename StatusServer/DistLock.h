#pragma once

#include "redis.h"

#include <string>
class DistLock
{
  public:
    static DistLock& Inst();
    ~DistLock() = default;
    std::string acquireLock(IRedis::ptr connect, const std::string& lockName,
        int lockTimeout, int acquireTimeout);

    bool releaseLock(IRedis::ptr connect, const std::string& lockName,
        const std::string& identifier);

  private:
    DistLock() = default;
};
