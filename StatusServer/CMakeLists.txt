set(StatusServer_SRCS
    AsioIOServicePool.cc
    ChatGrpcClient.cc
    ConfigMgr.cc
    DistLock.cc
    message.grpc.pb.cc
    message.pb.cc
    mysql.cc
    MysqlDao.cc
    MysqlMgr.cc
    RedisMgr.cc
    StatusServer.cc
    StatusServiceImpl.cc
    )

add_executable(StatusServer ${StatusServer_SRCS})
target_link_libraries(StatusServer
                      pthread
                      boost_filesystem
                      boost_system
                      protobuf
                      gRPC::grpc++_reflection
                      gRPC::grpc++
                      jsoncpp
                      mysqlclient
                      hiredis
                      )
