file(GLOB Common_SRCS
    "*.cc"
)

add_library(Common ${Common_SRCS})
target_link_libraries(Common
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
