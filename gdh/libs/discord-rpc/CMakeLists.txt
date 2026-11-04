cmake_minimum_required(VERSION 3.21)
project(DiscordRPC LANGUAGES CXX)

add_library(discord-rpc STATIC
    src/discord_rpc.cpp
    src/rpc_connection.cpp
    src/serialization.cpp
    src/connection_win.cpp
    src/discord_register_win.cpp
)

target_include_directories(discord-rpc PUBLIC src)
target_link_libraries(discord-rpc PRIVATE psapi advapi32)