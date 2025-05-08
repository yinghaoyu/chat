cmake_minimum_required(VERSION 3.28)

project(BlueBird)

set(CXX_FLAGS
 -g
 -ggdb
  )

set(LINK_LIBRARIES
  -lmysqlclient
  -ljsoncpp
  )

# SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
# SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

find_package(Boost REQUIRED)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
else()
    message(FATAL_ERROR "Boost not found")
endif()

# find_package(MySQL REQUIRED)
# if(MySQL_FOUND)
#     include_directories(${MYSQL_INCLUDE_DIR})
# else()
#   message(FATAL_ERROR "mysql not found")
# endif()

# find_package(jsoncpp REQUIRED)
# if(JSONCPP_FOUND)
#   include_directories(${JSONCPP_INCLUDE_DIRS})
# else()
#   message(FATAL_ERROR "jsoncpp not found")
# endif()

# find_package(hiredis REQUIRED)
# if(HIREDIS_FOUND)
#     include_directories(${HIREDIS_INCLUDE_DIRS})
# else()
#     message(FATAL_ERROR "hiredis not found")
# endif()

find_package(Protobuf REQUIRED)
if(Protobuf_FOUND)
    include_directories(${Protobuf_INCLUDE_DIRS})
else()
    message(FATAL_ERROR "Protobuf not found")
endif()

find_package(gRPC REQUIRED)
if(gRPC_FOUND)
    include_directories(${gRPC_INCLUDE_DIRS})
else()
    message(FATAL_ERROR "gRPC not found")
endif()

add_subdirectory(ChatServer)
add_subdirectory(GateServer)
add_subdirectory(StatusServer)