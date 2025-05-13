#!/bin/bash

root_path=$(dirname $(readlink -f $0))

cd ChatServer && ./generate_message_grpc.sh
cd $root_path
cd GateServer && ./generate_message_grpc.sh
cd $root_path
cd StatusServer && ./generate_message_grpc.sh

cd $root_path

ln -sf build/compile_commands.json

mkdir -p build && cd build && cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON .. && make -j4

cd $root_path

mkdir -p bin
mkdir -p bin/ChatServer
mkdir -p bin/ChatServer2
mkdir -p bin/GateServer
mkdir -p bin/StatusServer

cp -f build/StatusServer/StatusServer bin/StatusServer/StatusServer
cp -f StatusServer/config.ini bin/StatusServer/config.ini

cp -f build/GateServer/GateServer bin/GateServer/GateServer
cp -f GateServer/config.ini bin/GateServer/config.ini

cp -f build/ChatServer/ChatServer bin/ChatServer/ChatServer
cp -f ChatServer/config.ini bin/ChatServer/config.ini

cp -f build/ChatServer/ChatServer bin/ChatServer2/ChatServer
cp -f ChatServer/config2.ini bin/ChatServer2/config.ini