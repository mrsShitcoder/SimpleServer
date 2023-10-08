project(Lib LANGUAGES CXX)

find_package(Threads REQUIRED)
find_package(Boost CONFIG 1.83.0 COMPONENTS system coroutine REQUIRED)

file(GLOB Lib_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")

add_library(Lib INTERFACE ${Lib_HEADERS})

target_include_directories(Lib INTERFACE ${Boost_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(Lib INTERFACE Threads::Threads Boost::system Boost::coroutine)
