#pragma once

#include "db.h"

#include <mysql/mysql.h>
#include <functional>
#include <memory>
#include <mutex>
#include <list>
#include <vector>

class MySQL;
class MySQLStmt;

struct MySQLTime
{
    MySQLTime(time_t t) : ts(t) {}
    time_t ts;
};

bool mysql_time_to_time_t(const MYSQL_TIME& mt, time_t& ts);
bool time_t_to_mysql_time(const time_t& ts, MYSQL_TIME& mt);

class MySQLRes : public ISQLData
{
  public:
    typedef std::shared_ptr<MySQLRes> ptr;
    typedef std::function<bool(MYSQL_ROW row, int field_count, int row_no)>
        data_cb;
    MySQLRes(MYSQL_RES* res, int eno, const char* estr);

    MYSQL_RES* get() const { return m_data.get(); }

    int                getErrno() const override { return m_errno; }
    const std::string& getErrStr() const override { return m_errstr; }

    bool foreach (data_cb cb);

    int         getDataCount() override;
    int         getColumnCount() override;
    int         getColumnBytes(int idx) override;
    int         getColumnType(int idx) override;
    std::string getColumnName(int idx) override;

    bool        isNull(int idx) override;
    int8_t      getInt8(int idx) override;
    uint8_t     getUint8(int idx) override;
    int16_t     getInt16(int idx) override;
    uint16_t    getUint16(int idx) override;
    int32_t     getInt32(int idx) override;
    uint32_t    getUint32(int idx) override;
    int64_t     getInt64(int idx) override;
    uint64_t    getUint64(int idx) override;
    float       getFloat(int idx) override;
    double      getDouble(int idx) override;
    std::string getString(int idx) override;
    std::string getBlob(int idx) override;
    time_t      getTime(int idx) override;
    bool        next() override;

  private:
    int                        m_errno;
    std::string                m_errstr;
    MYSQL_ROW                  m_cur;
    unsigned long*             m_curLength;
    std::shared_ptr<MYSQL_RES> m_data;
};

class MySQLStmtRes : public ISQLData
{
    friend class MySQLStmt;

  public:
    typedef std::shared_ptr<MySQLStmtRes> ptr;
    static MySQLStmtRes::ptr Create(std::shared_ptr<MySQLStmt> stmt);
    ~MySQLStmtRes();

    int                getErrno() const override { return m_errno; }
    const std::string& getErrStr() const override { return m_errstr; }

    int         getDataCount() override;
    int         getColumnCount() override;
    int         getColumnBytes(int idx) override;
    int         getColumnType(int idx) override;
    std::string getColumnName(int idx) override;

    bool        isNull(int idx) override;
    int8_t      getInt8(int idx) override;
    uint8_t     getUint8(int idx) override;
    int16_t     getInt16(int idx) override;
    uint16_t    getUint16(int idx) override;
    int32_t     getInt32(int idx) override;
    uint32_t    getUint32(int idx) override;
    int64_t     getInt64(int idx) override;
    uint64_t    getUint64(int idx) override;
    float       getFloat(int idx) override;
    double      getDouble(int idx) override;
    std::string getString(int idx) override;
    std::string getBlob(int idx) override;
    time_t      getTime(int idx) override;
    bool        next() override;

  protected:
    MySQLStmtRes(
        std::shared_ptr<MySQLStmt> stmt, int eno, const std::string& estr);
    struct Data
    {
        Data();
        ~Data();

        void alloc(size_t size);

        bool             is_null;
        bool             error;
        enum_field_types type;
        unsigned long    length;
        int32_t          data_length;
        char*            data;
    };

  private:
    int                        m_errno;
    std::string                m_errstr;
    std::shared_ptr<MySQLStmt> m_stmt;
    std::vector<MYSQL_BIND>    m_binds;
    std::vector<Data>          m_datas;
};

class MySQLPool;
class MySQL : public IDB, public std::enable_shared_from_this<MySQL>
{
    friend class MySQLPool;

  public:
    typedef std::shared_ptr<MySQL> ptr;

    MySQL(const std::string& host, const int32_t port, const std::string& user,
        const std::string& passwd, const std::string& dbname,
        const int32_t timeout);

    bool connect();
    bool ping();

    virtual int            execute(const char* format, ...) override;
    int                    execute(const char* format, va_list ap);
    virtual int            execute(const std::string& sql) override;
    int64_t                getLastInsertId() override;
    std::shared_ptr<MYSQL> getRaw();

    uint64_t getAffectedRows();

    ISQLData::ptr query(const char* format, ...) override;
    ISQLData::ptr query(const char* format, va_list ap);
    ISQLData::ptr query(const std::string& sql) override;

    ITransaction::ptr openTransaction(bool auto_commit) override;
    IStmt::ptr        prepare(const std::string& sql) override;

    template <typename... Args>
    int execStmt(const char* stmt, Args&&... args);

    template <class... Args>
    ISQLData::ptr queryStmt(const char* stmt, Args&&... args);

    const char* cmd();

    bool        use(const std::string& dbname);
    int         getErrno() override;
    std::string getErrStr() override;
    uint64_t    getInsertId();

  private:
    bool isNeedCheck();

  private:
    std::shared_ptr<MYSQL> m_mysql;

    std::string m_cmd;

    std::string m_host;
    int32_t     m_port;
    std::string m_user;
    std::string m_passwd;
    std::string m_dbname;
    int32_t     m_timeout;

    uint64_t m_lastUsedTime;
    bool     m_hasError;
};

class MySQLTransaction : public ITransaction
{
  public:
    typedef std::shared_ptr<MySQLTransaction> ptr;

    static MySQLTransaction::ptr Create(MySQL::ptr mysql, bool auto_commit);
    ~MySQLTransaction();

    bool begin() override;
    bool commit() override;
    bool rollback() override;

    virtual int            execute(const char* format, ...) override;
    int                    execute(const char* format, va_list ap);
    virtual int            execute(const std::string& sql) override;
    int64_t                getLastInsertId() override;
    std::shared_ptr<MySQL> getMySQL();

    bool isAutoCommit() const { return m_autoCommit; }
    bool isFinished() const { return m_isFinished; }
    bool isError() const { return m_hasError; }

  protected:
    MySQLTransaction(MySQL::ptr mysql, bool auto_commit);

  private:
    MySQL::ptr m_mysql;
    bool       m_autoCommit;
    bool       m_isFinished;
    bool       m_hasError;
};

class MySQLStmt : public IStmt, public std::enable_shared_from_this<MySQLStmt>
{
  public:
    typedef std::shared_ptr<MySQLStmt> ptr;
    static MySQLStmt::ptr Create(MySQL::ptr db, const std::string& stmt);

    ~MySQLStmt();
    int bind(int idx, const int8_t& value);
    int bind(int idx, const uint8_t& value);
    int bind(int idx, const int16_t& value);
    int bind(int idx, const uint16_t& value);
    int bind(int idx, const int32_t& value);
    int bind(int idx, const uint32_t& value);
    int bind(int idx, const int64_t& value);
    int bind(int idx, const uint64_t& value);
    int bind(int idx, const float& value);
    int bind(int idx, const double& value);
    int bind(int idx, const std::string& value);
    int bind(int idx, const char* value);
    int bind(int idx, const void* value, int len);
    // int bind(int idx, const MYSQL_TIME& value, int type =
    // MYSQL_TYPE_TIMESTAMP); for null type
    int bind(int idx);

    int bindInt8(int idx, const int8_t& value) override;
    int bindUint8(int idx, const uint8_t& value) override;
    int bindInt16(int idx, const int16_t& value) override;
    int bindUint16(int idx, const uint16_t& value) override;
    int bindInt32(int idx, const int32_t& value) override;
    int bindUint32(int idx, const uint32_t& value) override;
    int bindInt64(int idx, const int64_t& value) override;
    int bindUint64(int idx, const uint64_t& value) override;
    int bindFloat(int idx, const float& value) override;
    int bindDouble(int idx, const double& value) override;
    int bindString(int idx, const char* value) override;
    int bindString(int idx, const std::string& value) override;
    int bindBlob(int idx, const void* value, int64_t size) override;
    int bindBlob(int idx, const std::string& value) override;
    // int bindTime(int idx, const MYSQL_TIME& value, int type =
    // MYSQL_TYPE_TIMESTAMP);
    int bindTime(int idx, const time_t& value) override;
    int bindNull(int idx) override;

    int         getErrno() override;
    std::string getErrStr() override;
    int         getAffectedRows() const;

    int           execute() override;
    int64_t       getLastInsertId() override;
    ISQLData::ptr query() override;

    MYSQL_STMT* getRaw() const { return m_stmt; }

  protected:
    MySQLStmt(MySQL::ptr db, MYSQL_STMT* stmt);

  private:
    MySQL::ptr              m_mysql;
    MYSQL_STMT*             m_stmt;
    std::vector<MYSQL_BIND> m_binds;
};

class MySQLPool
{
  public:
    MySQLPool(const std::string& host, const int32_t port,
        const std::string& user, const std::string& passwd,
        const std::string& dbname, const int32_t poolSize,
        const int32_t timeout);
    ~MySQLPool();

    MySQL::ptr get();

    void checkConnection(int sec = 30);

    uint32_t getMaxConn() const { return m_maxConn; }
    void     setMaxConn(uint32_t v) { m_maxConn = v; }

    int execute(const char* format, ...);
    int execute(const char* format, va_list ap);
    int execute(const std::string& sql);

    ISQLData::ptr query(const char* format, ...);
    ISQLData::ptr query(const char* format, va_list ap);
    ISQLData::ptr query(const std::string& sql);

    MySQLTransaction::ptr openTransaction(bool auto_commit = true);

  private:
    void freeMySQL(MySQL* m);

  private:
    std::string       m_host;
    int32_t           m_port;
    std::string       m_user;
    std::string       m_passwd;
    std::string       m_dbname;
    int32_t           m_maxConn;
    int32_t           m_timeout;
    std::mutex        m_mutex;
    std::list<MySQL*> m_conns;
};

namespace
{

template <size_t N, typename... Args>
struct MySQLBinder
{
    static int Bind(std::shared_ptr<MySQLStmt> stmt) { return 0; }
};

template <typename... Args>
int bindX(MySQLStmt::ptr stmt, Args&... args)
{
    return MySQLBinder<1, Args...>::Bind(stmt, args...);
}
}  // namespace

template <typename... Args>
int MySQL::execStmt(const char* stmt, Args&&... args)
{
    auto st = MySQLStmt::Create(shared_from_this(), stmt);
    if (!st)
    {
        return -1;
    }
    int rt = bindX(st, args...);
    if (rt != 0)
    {
        return rt;
    }
    return st->execute();
}

template <class... Args>
ISQLData::ptr MySQL::queryStmt(const char* stmt, Args&&... args)
{
    auto st = MySQLStmt::Create(shared_from_this(), stmt);
    if (!st)
    {
        return nullptr;
    }
    int rt = bindX(st, args...);
    if (rt != 0)
    {
        return nullptr;
    }
    return st->query();
}

namespace
{

template <size_t N, typename Head, typename... Tail>
struct MySQLBinder<N, Head, Tail...>
{
    static int Bind(MySQLStmt::ptr stmt, const Head&, Tail&...)
    {
        static_assert(sizeof...(Tail) < 0, "invalid type");
        return 0;
    }
};

#define XX(type, type2)                                                  \
    template <size_t N, typename... Tail>                                \
    struct MySQLBinder<N, type, Tail...>                                 \
    {                                                                    \
        static int Bind(MySQLStmt::ptr stmt, type2 value, Tail&... tail) \
        {                                                                \
            int rt = stmt->bind(N, value);                               \
            if (rt != 0)                                                 \
            {                                                            \
                return rt;                                               \
            }                                                            \
            return MySQLBinder<N + 1, Tail...>::Bind(stmt, tail...);     \
        }                                                                \
    };

XX(char*, char*);
XX(const char*, const char*);
XX(std::string, const std::string&);
XX(int8_t, int8_t&);
XX(uint8_t, uint8_t&);
XX(int16_t, int16_t&);
XX(uint16_t, uint16_t&);
XX(int32_t, int32_t&);
XX(uint32_t, uint32_t&);
XX(int64_t, int64_t&);
XX(uint64_t, uint64_t&);
XX(float, float&);
XX(double, double&);
// XX(MYSQL_TIME, MYSQL_TIME&);
#undef XX
}  // namespace
