#include "mysql.h"

#include <iostream>
#include <memory.h>

bool mysql_time_to_time_t(const MYSQL_TIME& mt, time_t& ts)
{
    struct tm tm;
    ts = 0;
    localtime_r(&ts, &tm);
    tm.tm_year = mt.year - 1900;
    tm.tm_mon  = mt.month - 1;
    tm.tm_mday = mt.day;
    tm.tm_hour = mt.hour;
    tm.tm_min  = mt.minute;
    tm.tm_sec  = mt.second;
    ts         = mktime(&tm);
    if (ts < 0)
    {
        ts = 0;
    }
    return true;
}

bool time_t_to_mysql_time(const time_t& ts, MYSQL_TIME& mt)
{
    struct tm tm;
    localtime_r(&ts, &tm);
    mt.year   = tm.tm_year + 1900;
    mt.month  = tm.tm_mon + 1;
    mt.day    = tm.tm_mday;
    mt.hour   = tm.tm_hour;
    mt.minute = tm.tm_min;
    mt.second = tm.tm_sec;
    return true;
}

namespace
{

struct MySQLThreadIniter
{
    MySQLThreadIniter() { mysql_thread_init(); }

    ~MySQLThreadIniter() { mysql_thread_end(); }
};
}  // namespace

static MYSQL* mysql_init(const std::string& host, int32_t port,
    const std::string& user, const std::string& passwd,
    const std::string& dbname, const int timeout)
{

    // 多线程下每个线程都需要初始化 mysql 环境
    static thread_local MySQLThreadIniter s_thread_initer;

    MYSQL* mysql = ::mysql_init(nullptr);
    if (mysql == nullptr)
    {
        std::cout << "mysql_init error";
        return nullptr;
    }

    if (timeout > 0)
    {
        // 设置连接的超时时间
        mysql_options(mysql, MYSQL_OPT_CONNECT_TIMEOUT, &timeout);
        mysql_options(mysql, MYSQL_OPT_READ_TIMEOUT, &timeout);
        mysql_options(mysql, MYSQL_OPT_WRITE_TIMEOUT, &timeout);
    }
    // 字符集设置为 UTF-8 编码的 utf8mb4 格式
    mysql_options(mysql, MYSQL_SET_CHARSET_NAME, "utf8mb4");

    if (mysql_real_connect(mysql,
            host.c_str(),
            user.c_str(),
            passwd.c_str(),
            dbname.c_str(),
            port,
            NULL,
            0) == nullptr)
    {
        std::cout << "mysql_real_connect(" << host << ", " << port << ", "
                  << dbname << ") error: " << mysql_error(mysql);
        mysql_close(mysql);
        return nullptr;
    }
    return mysql;
}

MySQL::MySQL(const std::string& host, const int32_t port,
    const std::string& user, const std::string& passwd,
    const std::string& dbname, const int32_t timeout)
    : m_host(host),
      m_port(port),
      m_user(user),
      m_passwd(passwd),
      m_dbname(dbname),
      m_timeout(timeout),
      m_lastUsedTime(0),
      m_hasError(false)
{}

bool MySQL::connect()
{
    if (m_mysql && !m_hasError)
    {
        return true;
    }

    MYSQL* m =
        mysql_init(m_host, m_port, m_user, m_passwd, m_dbname, m_timeout);
    if (!m)
    {
        m_hasError = true;
        return false;
    }
    m_hasError = false;
    m_mysql.reset(m, mysql_close);
    return true;
}

IStmt::ptr MySQL::prepare(const std::string& sql)
{
    return MySQLStmt::Create(shared_from_this(), sql);
}

ITransaction::ptr MySQL::openTransaction(bool auto_commit)
{
    return MySQLTransaction::Create(shared_from_this(), auto_commit);
}

int64_t MySQL::getLastInsertId() { return mysql_insert_id(m_mysql.get()); }

bool MySQL::isNeedCheck()
{
    if ((time(0) - m_lastUsedTime) < 5 && !m_hasError)
    {
        return false;
    }
    return true;
}

bool MySQL::ping()
{
    if (!m_mysql)
    {
        return false;
    }
    if (mysql_ping(m_mysql.get()))
    {
        m_hasError = true;
        return false;
    }
    m_hasError = false;
    return true;
}

int MySQL::execute(const char* format, ...)
{
    va_list ap;
    va_start(ap, format);
    int rt = execute(format, ap);
    va_end(ap);
    return rt;
}

std::string formatv(const char* fmt, va_list ap)
{
    char* buf = nullptr;
    auto  len = vasprintf(&buf, fmt, ap);
    if (len == -1)
    {
        return "";
    }
    std::string ret(buf, len);
    free(buf);
    return ret;
}

int MySQL::execute(const char* format, va_list ap)
{
    m_cmd = formatv(format, ap);
    int r = ::mysql_query(m_mysql.get(), m_cmd.c_str());
    if (r)
    {
        std::cout << "cmd=" << cmd() << ", error: " << getErrStr();
        m_hasError = true;
    }
    else
    {
        m_hasError = false;
    }
    return r;
}

int MySQL::execute(const std::string& sql)
{
    m_cmd = sql;
    int r = ::mysql_query(m_mysql.get(), m_cmd.c_str());
    if (r)
    {
        std::cout << "cmd=" << cmd() << ", error: " << getErrStr();
        m_hasError = true;
    }
    else
    {
        m_hasError = false;
    }
    return r;
}

std::shared_ptr<MYSQL> MySQL::getRaw() { return m_mysql; }

uint64_t MySQL::getAffectedRows()
{
    if (!m_mysql)
    {
        return 0;
    }
    return mysql_affected_rows(m_mysql.get());
}

static MYSQL_RES* my_mysql_query(MYSQL* mysql, const char* sql)
{
    if (mysql == nullptr)
    {
        std::cout << "mysql_query mysql is null";
        return nullptr;
    }

    if (sql == nullptr)
    {
        std::cout << "mysql_query sql is null";
        return nullptr;
    }

    if (::mysql_query(mysql, sql))
    {
        std::cout << "mysql_query(" << sql << ") error:" << mysql_error(mysql);
        return nullptr;
    }

    MYSQL_RES* res = mysql_store_result(mysql);
    if (res == nullptr)
    {
        std::cout << "mysql_store_result() error:" << mysql_error(mysql);
    }
    return res;
}

template <class T, class... Args>
inline std::shared_ptr<T> protected_make_shared(Args&&... args)
{
    struct Helper : T
    {
        Helper(Args&&... args) : T(std::forward<Args>(args)...) {}
    };
    return std::make_shared<Helper>(std::forward<Args>(args)...);
}

MySQLStmt::ptr MySQLStmt::Create(MySQL::ptr db, const std::string& stmt)
{
    auto st = mysql_stmt_init(db->getRaw().get());
    if (!st)
    {
        return nullptr;
    }
    if (mysql_stmt_prepare(st, stmt.c_str(), stmt.size()))
    {
        std::cout << "stmt=" << stmt << " errno=" << mysql_stmt_errno(st)
                  << " errstr=" << mysql_stmt_error(st);
        mysql_stmt_close(st);
        return nullptr;
    }
    int            count = mysql_stmt_param_count(st);
    MySQLStmt::ptr rt    = protected_make_shared<MySQLStmt>(db, st);
    rt->m_binds.resize(count);
    for (int i = 0; i < count; ++i)
    {
        memset(&rt->m_binds[i], 0, sizeof(rt->m_binds[i]));
    }
    return rt;
}

MySQLStmt::MySQLStmt(MySQL::ptr db, MYSQL_STMT* stmt)
    : m_mysql(db), m_stmt(stmt)
{}

MySQLStmt::~MySQLStmt()
{
    if (m_stmt)
    {
        mysql_stmt_close(m_stmt);
    }

    for (auto& i : m_binds)
    {
        if (i.buffer)
        {
            free(i.buffer);
        }
        // if(i.buffer_type == MYSQL_TYPE_TIMESTAMP
        //     || i.buffer_type == MYSQL_TYPE_DATETIME
        //     || i.buffer_type == MYSQL_TYPE_DATE
        //     || i.buffer_type == MYSQL_TYPE_TIME) {
        //     if(i.buffer) {
        //         free(i.buffer);
        //     }
        // }
    }
}

int MySQLStmt::bind(int idx, const int8_t& value)
{
    return bindInt8(idx, value);
}

int MySQLStmt::bind(int idx, const uint8_t& value)
{
    return bindUint8(idx, value);
}

int MySQLStmt::bind(int idx, const int16_t& value)
{
    return bindInt16(idx, value);
}

int MySQLStmt::bind(int idx, const uint16_t& value)
{
    return bindUint16(idx, value);
}

int MySQLStmt::bind(int idx, const int32_t& value)
{
    return bindInt32(idx, value);
}

int MySQLStmt::bind(int idx, const uint32_t& value)
{
    return bindUint32(idx, value);
}

int MySQLStmt::bind(int idx, const int64_t& value)
{
    return bindInt64(idx, value);
}

int MySQLStmt::bind(int idx, const uint64_t& value)
{
    return bindUint64(idx, value);
}

int MySQLStmt::bind(int idx, const float& value)
{
    return bindFloat(idx, value);
}

int MySQLStmt::bind(int idx, const double& value)
{
    return bindDouble(idx, value);
}

// int MySQLStmt::bind(int idx, const MYSQL_TIME& value, int type) {
//     return bindTime(idx, value, type);
// }

int MySQLStmt::bind(int idx, const std::string& value)
{
    return bindString(idx, value);
}

int MySQLStmt::bind(int idx, const char* value)
{
    return bindString(idx, value);
}

int MySQLStmt::bind(int idx, const void* value, int len)
{
    return bindBlob(idx, value, len);
}

int MySQLStmt::bind(int idx)
{
    idx -= 1;
    m_binds[idx].buffer_type = MYSQL_TYPE_NULL;
    return 0;
}

int MySQLStmt::getErrno() { return mysql_stmt_errno(m_stmt); }

std::string MySQLStmt::getErrStr()
{
    const char* e = mysql_stmt_error(m_stmt);
    if (e)
    {
        return e;
    }
    return "";
}

int MySQLStmt::getAffectedRows() const
{
    if (!m_stmt)
    {
        return 0;
    }
    return mysql_stmt_affected_rows(m_stmt);
}

int MySQLStmt::bindNull(int idx) { return bind(idx); }

int MySQLStmt::bindInt8(int idx, const int8_t& value)
{
    idx -= 1;
    m_binds[idx].buffer_type = MYSQL_TYPE_TINY;
#define BIND_COPY(ptr, size)                \
    if (m_binds[idx].buffer == nullptr)     \
    {                                       \
        m_binds[idx].buffer = malloc(size); \
    }                                       \
    memcpy(m_binds[idx].buffer, ptr, size);
    BIND_COPY(&value, sizeof(value));
    m_binds[idx].is_unsigned   = false;
    m_binds[idx].buffer_length = sizeof(value);
    return 0;
}

int MySQLStmt::bindUint8(int idx, const uint8_t& value)
{
    idx -= 1;
    m_binds[idx].buffer_type = MYSQL_TYPE_TINY;
    BIND_COPY(&value, sizeof(value));
    m_binds[idx].is_unsigned   = true;
    m_binds[idx].buffer_length = sizeof(value);
    return 0;
}

int MySQLStmt::bindInt16(int idx, const int16_t& value)
{
    idx -= 1;
    m_binds[idx].buffer_type = MYSQL_TYPE_SHORT;
    BIND_COPY(&value, sizeof(value));
    m_binds[idx].is_unsigned   = false;
    m_binds[idx].buffer_length = sizeof(value);
    return 0;
}

int MySQLStmt::bindUint16(int idx, const uint16_t& value)
{
    idx -= 1;
    m_binds[idx].buffer_type = MYSQL_TYPE_SHORT;
    BIND_COPY(&value, sizeof(value));
    m_binds[idx].is_unsigned   = true;
    m_binds[idx].buffer_length = sizeof(value);
    return 0;
}

int MySQLStmt::bindInt32(int idx, const int32_t& value)
{
    idx -= 1;
    m_binds[idx].buffer_type = MYSQL_TYPE_LONG;
    BIND_COPY(&value, sizeof(value));
    m_binds[idx].is_unsigned   = false;
    m_binds[idx].buffer_length = sizeof(value);
    return 0;
}

int MySQLStmt::bindUint32(int idx, const uint32_t& value)
{
    idx -= 1;
    m_binds[idx].buffer_type = MYSQL_TYPE_LONG;
    BIND_COPY(&value, sizeof(value));
    m_binds[idx].is_unsigned   = true;
    m_binds[idx].buffer_length = sizeof(value);
    return 0;
}

int MySQLStmt::bindInt64(int idx, const int64_t& value)
{
    idx -= 1;
    m_binds[idx].buffer_type = MYSQL_TYPE_LONGLONG;
    BIND_COPY(&value, sizeof(value));
    m_binds[idx].is_unsigned   = false;
    m_binds[idx].buffer_length = sizeof(value);
    return 0;
}

int MySQLStmt::bindUint64(int idx, const uint64_t& value)
{
    idx -= 1;
    m_binds[idx].buffer_type = MYSQL_TYPE_LONGLONG;
    BIND_COPY(&value, sizeof(value));
    m_binds[idx].is_unsigned   = true;
    m_binds[idx].buffer_length = sizeof(value);
    return 0;
}

int MySQLStmt::bindFloat(int idx, const float& value)
{
    idx -= 1;
    m_binds[idx].buffer_type = MYSQL_TYPE_FLOAT;
    BIND_COPY(&value, sizeof(value));
    m_binds[idx].buffer_length = sizeof(value);
    return 0;
}

int MySQLStmt::bindDouble(int idx, const double& value)
{
    idx -= 1;
    m_binds[idx].buffer_type = MYSQL_TYPE_DOUBLE;
    BIND_COPY(&value, sizeof(value));
    m_binds[idx].buffer_length = sizeof(value);
    return 0;
}

int MySQLStmt::bindString(int idx, const char* value)
{
    idx -= 1;
    m_binds[idx].buffer_type = MYSQL_TYPE_STRING;
#define BIND_COPY_LEN(ptr, size)                                \
    if (m_binds[idx].buffer == nullptr)                         \
    {                                                           \
        m_binds[idx].buffer = malloc(size);                     \
    }                                                           \
    else if ((size_t)m_binds[idx].buffer_length < (size_t)size) \
    {                                                           \
        free(m_binds[idx].buffer);                              \
        m_binds[idx].buffer = malloc(size);                     \
    }                                                           \
    memcpy(m_binds[idx].buffer, ptr, size);                     \
    m_binds[idx].buffer_length = size;
    BIND_COPY_LEN(value, strlen(value));
    return 0;
}

int MySQLStmt::bindString(int idx, const std::string& value)
{
    idx -= 1;
    m_binds[idx].buffer_type = MYSQL_TYPE_STRING;
    BIND_COPY_LEN(value.c_str(), value.size());
    return 0;
}

int MySQLStmt::bindBlob(int idx, const void* value, int64_t size)
{
    idx -= 1;
    m_binds[idx].buffer_type = MYSQL_TYPE_BLOB;
    BIND_COPY_LEN(value, size);
    return 0;
}

int MySQLStmt::bindBlob(int idx, const std::string& value)
{
    idx -= 1;
    m_binds[idx].buffer_type = MYSQL_TYPE_BLOB;
    BIND_COPY_LEN(value.c_str(), value.size());
    return 0;
}

std::string time2str(time_t ts, const std::string& format = "%Y-%m-%d %H:%M:%S")
{
    struct tm tm;
    localtime_r(&ts, &tm);
    char buf[64];
    strftime(buf, sizeof(buf), format.c_str(), &tm);
    return buf;
}

time_t str2time(const char* str, const char* format = "%Y-%m-%d %H:%M:%S")
{
    struct tm t;
    memset(&t, 0, sizeof(t));
    if (!strptime(str, format, &t))
    {
        return 0;
    }
    return mktime(&t);
}

int MySQLStmt::bindTime(int idx, const time_t& value)
{
    return bindString(idx, time2str(value));
}

int MySQLStmt::execute()
{
    mysql_stmt_bind_param(m_stmt, &m_binds[0]);
    return mysql_stmt_execute(m_stmt);
}

int64_t MySQLStmt::getLastInsertId() { return mysql_stmt_insert_id(m_stmt); }

ISQLData::ptr MySQLStmt::query()
{
    mysql_stmt_bind_param(m_stmt, &m_binds[0]);
    return MySQLStmtRes::Create(shared_from_this());
}

MySQLRes::MySQLRes(MYSQL_RES* res, int eno, const char* estr)
    : m_errno(eno), m_errstr(estr), m_cur(nullptr), m_curLength(nullptr)
{
    if (res)
    {
        m_data.reset(res, mysql_free_result);
    }
}

bool MySQLRes::foreach (data_cb cb)
{
    MYSQL_ROW row;
    uint64_t  fields = getColumnCount();
    int       i      = 0;
    while ((row = mysql_fetch_row(m_data.get())))
    {
        if (!cb(row, fields, i++))
        {
            break;
        }
    }
    return true;
}

int MySQLRes::getDataCount() { return mysql_num_rows(m_data.get()); }

int MySQLRes::getColumnCount() { return mysql_num_fields(m_data.get()); }

int MySQLRes::getColumnBytes(int idx) { return m_curLength[idx]; }

int MySQLRes::getColumnType(int idx) { return 0; }

std::string MySQLRes::getColumnName(int idx) { return ""; }

bool MySQLRes::isNull(int idx)
{
    if (m_cur[idx] == nullptr)
    {
        return true;
    }
    return false;
}

int8_t MySQLRes::getInt8(int idx) { return getInt64(idx); }

uint8_t MySQLRes::getUint8(int idx) { return getInt64(idx); }

int16_t MySQLRes::getInt16(int idx) { return getInt64(idx); }

uint16_t MySQLRes::getUint16(int idx) { return getInt64(idx); }

int32_t MySQLRes::getInt32(int idx) { return getInt64(idx); }

uint32_t MySQLRes::getUint32(int idx) { return getInt64(idx); }

int64_t MySQLRes::getInt64(int idx)
{
    return strtoull(m_cur[idx], nullptr, 10);
}

uint64_t MySQLRes::getUint64(int idx) { return getInt64(idx); }

float MySQLRes::getFloat(int idx) { return getDouble(idx); }

double MySQLRes::getDouble(int idx) { return atof(m_cur[idx]); }

std::string MySQLRes::getString(int idx)
{
    return std::string(m_cur[idx], m_curLength[idx]);
}

std::string MySQLRes::getBlob(int idx)
{
    return std::string(m_cur[idx], m_curLength[idx]);
}

time_t MySQLRes::getTime(int idx)
{
    if (!m_cur[idx])
    {
        return 0;
    }
    return str2time(m_cur[idx]);
}

bool MySQLRes::next()
{
    m_cur       = mysql_fetch_row(m_data.get());
    m_curLength = mysql_fetch_lengths(m_data.get());
    return m_cur;
}

MySQLStmtRes::ptr MySQLStmtRes::Create(std::shared_ptr<MySQLStmt> stmt)
{
    int               eno    = mysql_stmt_errno(stmt->getRaw());
    const char*       errstr = mysql_stmt_error(stmt->getRaw());
    MySQLStmtRes::ptr rt =
        protected_make_shared<MySQLStmtRes>(stmt, eno, errstr);
    if (eno)
    {
        return rt;
    }
    MYSQL_RES* res = mysql_stmt_result_metadata(stmt->getRaw());
    if (!res)
    {
        return protected_make_shared<MySQLStmtRes>(
            stmt, stmt->getErrno(), stmt->getErrStr());
    }

    int          num    = mysql_num_fields(res);
    MYSQL_FIELD* fields = mysql_fetch_fields(res);

    rt->m_binds.resize(num);
    for (int i = 0; i < num; ++i)
    {
        memset(&rt->m_binds[i], 0, sizeof(rt->m_binds[i]));
    }
    rt->m_datas.resize(num);

    for (int i = 0; i < num; ++i)
    {
        rt->m_datas[i].type = fields[i].type;
        switch (fields[i].type)
        {
#define XX(m, t) \
    case m: rt->m_datas[i].alloc(sizeof(t)); break;
            XX(MYSQL_TYPE_TINY, int8_t);
            XX(MYSQL_TYPE_SHORT, int16_t);
            XX(MYSQL_TYPE_LONG, int32_t);
            XX(MYSQL_TYPE_LONGLONG, int64_t);
            XX(MYSQL_TYPE_FLOAT, float);
            XX(MYSQL_TYPE_DOUBLE, double);
            XX(MYSQL_TYPE_TIMESTAMP, MYSQL_TIME);
            XX(MYSQL_TYPE_DATETIME, MYSQL_TIME);
            XX(MYSQL_TYPE_DATE, MYSQL_TIME);
            XX(MYSQL_TYPE_TIME, MYSQL_TIME);
#undef XX
            default: rt->m_datas[i].alloc(fields[i].length); break;
        }

        rt->m_binds[i].buffer_type   = rt->m_datas[i].type;
        rt->m_binds[i].buffer        = rt->m_datas[i].data;
        rt->m_binds[i].buffer_length = rt->m_datas[i].data_length;
        rt->m_binds[i].length        = &rt->m_datas[i].length;
        rt->m_binds[i].is_null       = &rt->m_datas[i].is_null;
        rt->m_binds[i].error         = &rt->m_datas[i].error;
    }

    if (mysql_stmt_bind_result(stmt->getRaw(), &rt->m_binds[0]))
    {
        return protected_make_shared<MySQLStmtRes>(
            stmt, stmt->getErrno(), stmt->getErrStr());
    }

    stmt->execute();

    if (mysql_stmt_store_result(stmt->getRaw()))
    {
        return protected_make_shared<MySQLStmtRes>(
            stmt, stmt->getErrno(), stmt->getErrStr());
    }
    return rt;
}

int MySQLStmtRes::getDataCount()
{
    return mysql_stmt_num_rows(m_stmt->getRaw());
}

int MySQLStmtRes::getColumnCount()
{
    return mysql_stmt_field_count(m_stmt->getRaw());
}

int MySQLStmtRes::getColumnBytes(int idx) { return m_datas[idx].length; }

int MySQLStmtRes::getColumnType(int idx) { return m_datas[idx].type; }

std::string MySQLStmtRes::getColumnName(int idx) { return ""; }

bool MySQLStmtRes::isNull(int idx) { return m_datas[idx].is_null; }

#define XX(type) return *(type*)m_datas[idx].data
int8_t MySQLStmtRes::getInt8(int idx) { XX(int8_t); }

uint8_t MySQLStmtRes::getUint8(int idx) { XX(uint8_t); }

int16_t MySQLStmtRes::getInt16(int idx) { XX(int16_t); }

uint16_t MySQLStmtRes::getUint16(int idx) { XX(uint16_t); }

int32_t MySQLStmtRes::getInt32(int idx) { XX(int32_t); }

uint32_t MySQLStmtRes::getUint32(int idx) { XX(uint32_t); }

int64_t MySQLStmtRes::getInt64(int idx) { XX(int64_t); }

uint64_t MySQLStmtRes::getUint64(int idx) { XX(uint64_t); }

float MySQLStmtRes::getFloat(int idx) { XX(float); }

double MySQLStmtRes::getDouble(int idx) { XX(double); }
#undef XX

std::string MySQLStmtRes::getString(int idx)
{
    return std::string(m_datas[idx].data, m_datas[idx].length);
}

std::string MySQLStmtRes::getBlob(int idx)
{
    return std::string(m_datas[idx].data, m_datas[idx].length);
}

time_t MySQLStmtRes::getTime(int idx)
{
    MYSQL_TIME* v  = (MYSQL_TIME*)m_datas[idx].data;
    time_t      ts = 0;
    mysql_time_to_time_t(*v, ts);
    return ts;
}

bool MySQLStmtRes::next() { return !mysql_stmt_fetch(m_stmt->getRaw()); }

MySQLStmtRes::Data::Data()
    : is_null(0), error(0), type(), length(0), data_length(0), data(nullptr)
{}

MySQLStmtRes::Data::~Data()
{
    if (data)
    {
        delete[] data;
    }
}

void MySQLStmtRes::Data::alloc(size_t size)
{
    if (data)
    {
        delete[] data;
    }
    data        = new char[size]();
    length      = size;
    data_length = size;
}

MySQLStmtRes::MySQLStmtRes(
    std::shared_ptr<MySQLStmt> stmt, int eno, const std::string& estr)
    : m_errno(eno), m_errstr(estr), m_stmt(stmt)
{}

MySQLStmtRes::~MySQLStmtRes()
{
    if (!m_errno)
    {
        mysql_stmt_free_result(m_stmt->getRaw());
    }
}

ISQLData::ptr MySQL::query(const char* format, ...)
{
    va_list ap;
    va_start(ap, format);
    auto rt = query(format, ap);
    va_end(ap);
    return rt;
}

ISQLData::ptr MySQL::query(const char* format, va_list ap)
{
    m_cmd          = formatv(format, ap);
    MYSQL_RES* res = my_mysql_query(m_mysql.get(), m_cmd.c_str());
    if (!res)
    {
        m_hasError = true;
        return nullptr;
    }
    m_hasError       = false;
    ISQLData::ptr rt = std::make_shared<MySQLRes>(
        res, mysql_errno(m_mysql.get()), mysql_error(m_mysql.get()));
    return rt;
}

ISQLData::ptr MySQL::query(const std::string& sql)
{
    m_cmd          = sql;
    MYSQL_RES* res = my_mysql_query(m_mysql.get(), m_cmd.c_str());
    if (!res)
    {
        m_hasError = true;
        return nullptr;
    }
    m_hasError       = false;
    ISQLData::ptr rt = std::make_shared<MySQLRes>(
        res, mysql_errno(m_mysql.get()), mysql_error(m_mysql.get()));
    return rt;
}

const char* MySQL::cmd() { return m_cmd.c_str(); }

bool MySQL::use(const std::string& dbname)
{
    if (!m_mysql)
    {
        return false;
    }
    if (m_dbname == dbname)
    {
        return true;
    }
    if (mysql_select_db(m_mysql.get(), dbname.c_str()) == 0)
    {
        m_dbname   = dbname;
        m_hasError = false;
        return true;
    }
    else
    {
        m_dbname   = "";
        m_hasError = true;
        return false;
    }
}

std::string MySQL::getErrStr()
{
    if (!m_mysql)
    {
        return "mysql is null";
    }
    const char* str = mysql_error(m_mysql.get());
    if (str)
    {
        return str;
    }
    return "";
}

int MySQL::getErrno()
{
    if (!m_mysql)
    {
        return -1;
    }
    return mysql_errno(m_mysql.get());
}

uint64_t MySQL::getInsertId()
{
    if (m_mysql)
    {
        return mysql_insert_id(m_mysql.get());
    }
    return 0;
}

MySQLTransaction::ptr MySQLTransaction::Create(
    MySQL::ptr mysql, bool auto_commit)
{
    MySQLTransaction::ptr rt =
        protected_make_shared<MySQLTransaction>(mysql, auto_commit);
    if (rt->begin())
    {
        return rt;
    }
    return nullptr;
}

MySQLTransaction::~MySQLTransaction()
{
    if (m_autoCommit)
    {
        commit();
    }
    else
    {
        rollback();
    }
}

int64_t MySQLTransaction::getLastInsertId()
{
    return m_mysql->getLastInsertId();
}

bool MySQLTransaction::begin()
{
    int rt = execute("BEGIN");
    return rt == 0;
}

bool MySQLTransaction::commit()
{
    if (m_isFinished || m_hasError)
    {
        return !m_hasError;
    }
    int rt = execute("COMMIT");
    if (rt == 0)
    {
        m_isFinished = true;
    }
    else
    {
        m_hasError = true;
    }
    return rt == 0;
}

bool MySQLTransaction::rollback()
{
    if (m_isFinished)
    {
        return true;
    }
    int rt = execute("ROLLBACK");
    if (rt == 0)
    {
        m_isFinished = true;
    }
    else
    {
        m_hasError = true;
    }
    return rt == 0;
}

int MySQLTransaction::execute(const char* format, ...)
{
    va_list ap;
    va_start(ap, format);
    return execute(format, ap);
}

int MySQLTransaction::execute(const char* format, va_list ap)
{
    if (m_isFinished)
    {
        std::cout << "transaction is finished, format=" << format;
        return -1;
    }
    int rt = m_mysql->execute(format, ap);
    if (rt)
    {
        m_hasError = true;
    }
    return rt;
}

int MySQLTransaction::execute(const std::string& sql)
{
    if (m_isFinished)
    {
        std::cout << "transaction is finished, sql=" << sql;
        return -1;
    }
    int rt = m_mysql->execute(sql);
    if (rt)
    {
        m_hasError = true;
    }
    return rt;
}

std::shared_ptr<MySQL> MySQLTransaction::getMySQL() { return m_mysql; }

MySQLTransaction::MySQLTransaction(MySQL::ptr mysql, bool auto_commit)
    : m_mysql(mysql),
      m_autoCommit(auto_commit),
      m_isFinished(false),
      m_hasError(false)
{}

MySQLPool::MySQLPool(const std::string& host, const int32_t port,
    const std::string& user, const std::string& passwd,
    const std::string& dbname, const int32_t poolSize, const int32_t timeout)
    : m_host(host),
      m_port(port),
      m_user(user),
      m_passwd(passwd),
      m_dbname(dbname),
      m_timeout(timeout),
      m_maxConn(poolSize)
{
    // 初始化 mysql 库
    mysql_library_init(0, nullptr, nullptr);
}

MySQLPool::~MySQLPool()
{
    mysql_library_end();
    for (auto& i : m_conns)
    {
        delete i;
    }
}

MySQL::ptr MySQLPool::get()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    MySQL*                       it = nullptr;

    if (!m_conns.empty())
    {
        MySQL* rt = m_conns.front();
        m_conns.pop_front();
        lock.unlock();
        if (!rt->isNeedCheck())
        {
            rt->m_lastUsedTime = time(0);
            return MySQL::ptr(rt,
                std::bind(&MySQLPool::freeMySQL, this, std::placeholders::_1));
        }
        if (rt->ping())
        {
            rt->m_lastUsedTime = time(0);
            return MySQL::ptr(rt,
                std::bind(&MySQLPool::freeMySQL, this, std::placeholders::_1));
        }
        else if (rt->connect())
        {
            rt->m_lastUsedTime = time(0);
            return MySQL::ptr(rt,
                std::bind(&MySQLPool::freeMySQL, this, std::placeholders::_1));
        }
        else
        {
            std::cout << "reconnect fail";
            return nullptr;
        }
    }
    lock.unlock();
    MySQL* rt =
        new MySQL(m_host, m_port, m_user, m_passwd, m_dbname, m_timeout);
    if (rt->connect())
    {
        rt->m_lastUsedTime = time(0);
        return MySQL::ptr(
            rt, std::bind(&MySQLPool::freeMySQL, this, std::placeholders::_1));
    }
    else
    {
        delete rt;
        return nullptr;
    }
}

void MySQLPool::checkConnection(int sec)
{
    time_t              now = time(0);
    std::vector<MySQL*> conns;

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto it = m_conns.begin(); it != m_conns.end();)
        {
            if ((int)(now - (*it)->m_lastUsedTime) >= sec)
            {
                auto tmp = *it;
                it       = m_conns.erase(it);
                conns.push_back(tmp);
            }
            else
            {
                ++it;
            }
        }
    }

    for (auto& i : conns)
    {
        delete i;
    }
}

int MySQLPool::execute(const char* format, ...)
{
    va_list ap;
    va_start(ap, format);
    int rt = execute(format, ap);
    va_end(ap);
    return rt;
}

int MySQLPool::execute(const char* format, va_list ap)
{
    auto conn = get();
    if (!conn)
    {
        std::cout << "MySQLPool::execute, get fail, format=" << format;
        return -1;
    }
    return conn->execute(format, ap);
}

int MySQLPool::execute(const std::string& sql)
{
    auto conn = get();
    if (!conn)
    {
        std::cout << "MySQLPool::execute, get fail, sql=" << sql;
        return -1;
    }
    return conn->execute(sql);
}

ISQLData::ptr MySQLPool::query(const char* format, ...)
{
    va_list ap;
    va_start(ap, format);
    auto res = query(format, ap);
    va_end(ap);
    return res;
}

ISQLData::ptr MySQLPool::query(const char* format, va_list ap)
{
    auto conn = get();
    if (!conn)
    {
        std::cout << "MySQLPool::query, get fail, format=" << format;
        return nullptr;
    }
    return conn->query(format, ap);
}

ISQLData::ptr MySQLPool::query(const std::string& sql)
{
    auto conn = get();
    if (!conn)
    {
        std::cout << "MySQLPool::query, get fail, sql=" << sql;
        return nullptr;
    }
    return conn->query(sql);
}

MySQLTransaction::ptr MySQLPool::openTransaction(bool auto_commit)
{
    auto conn = get();
    if (!conn)
    {
        std::cout << "MySQLPool::openTransaction, get fail";
        return nullptr;
    }
    MySQLTransaction::ptr trans(MySQLTransaction::Create(conn, auto_commit));
    return trans;
}

void MySQLPool::freeMySQL(MySQL* m)
{
    if (m->m_mysql)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_conns.size() < m_maxConn)
        {
            m_conns.push_back(m);
            return;
        }
    }
    delete m;
}
