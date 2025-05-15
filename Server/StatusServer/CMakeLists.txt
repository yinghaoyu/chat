file(GLOB StatusServer_SRCS
    "*.cc"
)

add_executable(StatusServer ${StatusServer_SRCS})
target_link_libraries(StatusServer
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
