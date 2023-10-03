cmake_minimum_required(VERSION 3.26)
project(SimpleServer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)

find_package(Threads REQUIRED)
find_package(Boost CONFIG 1.83.0 COMPONENTS system coroutine REQUIRED)

add_subdirectory(NetUtils)

add_executable(SimpleServer main.cpp)

target_include_directories(SimpleServer PUBLIC ${Boost_INCLUDE_DIRS})
target_link_libraries(SimpleServer PUBLIC Threads::Threads Boost::system Boost::coroutine NetUtils)
