project(NetUtils LANGUAGES CXX)

find_package(Threads REQUIRED)
find_package(Boost CONFIG 1.83.0 COMPONENTS system coroutine REQUIRED)

file(GLOB NetUtils_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")

add_library(NetUtils INTERFACE ${NetUtils_HEADERS})

target_include_directories(NetUtils INTERFACE ${Boost_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(NetUtils INTERFACE Threads::Threads Boost::system Boost::coroutine)
