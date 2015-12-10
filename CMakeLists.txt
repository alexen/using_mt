cmake_minimum_required(VERSION 2.8.12)

set(CMAKE_FRAMEWORK_PATH ${CMAKE_FRAMEWORK_PATH} "/opt/itcs/lib")
set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "/opt/itcs/include")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "/home/alexen/devel/g2g/trunk/cmake")
add_definitions(-std=c++11)
add_definitions(-Wall -Wextra -Wpedantic -Werror)

find_package(Pqxx REQUIRED)
find_package(Hiredis REQUIRED)
find_package(Boost 1.55 REQUIRED COMPONENTS thread system)

include_directories(${CMAKE_INCLUDE_PATH})

set(NAME test.bin)

add_executable(${NAME}
    main.cpp
)

target_link_libraries(${NAME}
    ${Boost_THREAD_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
)