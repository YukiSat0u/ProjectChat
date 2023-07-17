cmake_minimum_required(VERSION 3.8 FATAL_ERROR)

project(Server)

set(INCLUDES
    include/Server.h
)

set(SOURCES
    source/Server.cpp
    source/Main.cpp
)

add_executable(Server ${SOURCES} ${INCLUDES})


if(WIN32)
    target_link_libraries(Server ws2_32)
endif()

