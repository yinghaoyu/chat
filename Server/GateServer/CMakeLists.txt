file(GLOB GateServer_SRCS
    "*.cc"
)

add_executable(GateServer ${GateServer_SRCS})
target_link_libraries(GateServer
                      Common
                      pthread
                      boost_filesystem
                      boost_system
                      protobuf
                      gRPC::grpc++_reflection
                      gRPC::grpc++
                      jsoncpp
                      mysqlclient
                      hiredis
                      spdlog::spdlog
                      )
