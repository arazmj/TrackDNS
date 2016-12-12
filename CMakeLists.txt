cmake_minimum_required(VERSION 3.7.1)

project(trackdns)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(Boost REQUIRED program_options thread)
find_package(MySQL REQUIRED)
find_package(MySQLPP REQUIRED)
find_package(LDNS REQUIRED)
find_package(OpenSSL REQUIRED)

include_directories(SYSTEM ${Boost_INCLUDE_DIR})
include_directories(${MYSQL_INCLUDE_DIR})
include_directories(${MYSQLPP_INCLUDE_DIRS})
include_directories(${OPENSSL_INCLUDE_DIR})
include_directories(${LDNS_INCLUDE_DIRS})

set(SOURCE_FILES
        src/main.cpp
        src/Domain.h
        src/Domain.cpp
        src/Persistence.cpp
        src/Persistence.h)

add_executable(trackdns ${SOURCE_FILES})

target_link_libraries(trackdns
        ${Boost_LIBRARIES}
        ${MYSQLPP_LIBRARIES}
        ${LDNS_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT})
