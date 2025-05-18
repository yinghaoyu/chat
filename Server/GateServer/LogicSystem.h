#pragma once

#include "Singleton.h"

#include <functional>
#include <unordered_map>

class HttpConnection;
typedef std::function<void(std::shared_ptr<HttpConnection>)> HttpHandler;
class LogicSystem : public Singleton<LogicSystem>
{
    friend class Singleton<LogicSystem>;

  public:
    ~LogicSystem();
    bool HandleGet(const std::string&, std::shared_ptr<HttpConnection>);
    void RegGet(const std::string&, HttpHandler handler);
    void RegPost(const std::string&, HttpHandler handler);
    bool HandlePost(const std::string&, std::shared_ptr<HttpConnection>);

  private:
    LogicSystem();
    std::unordered_map<std::string, HttpHandler> m_post_handlers;
    std::unordered_map<std::string, HttpHandler> m_get_handlers;
};
