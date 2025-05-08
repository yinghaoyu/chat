file(GLOB ChatServer_SRCS
    "*.cc"
)

add_executable(ChatServer ${ChatServer_SRCS})
target_link_libraries(ChatServer
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
