cmake_minimum_required(VERSION 3.10)
project(tencent_intern)

set(CMAKE_CXX_STANDARD 14)

SET(Boost_USE_STATIC_LIBS ON)
find_package(Boost COMPONENTS system log)

include_directories(${Boost_INCLUDE_DIR})

add_executable(try_uname_share trys/try_uname_share.cpp)
target_link_libraries(try_uname_share ${Boost_LIBRARIES})

add_executable(try_mapped_region_write trys/try_mapped_region_write.cpp)
target_link_libraries(try_mapped_region_write ${Boost_LIBRARIES})

add_executable(try_mapped_region_read trys/try_mapped_region_read.cpp)
target_link_libraries(try_mapped_region_read ${Boost_LIBRARIES})


add_library(simple_tbus STATIC
        SimpleTbus.cpp
        SimpleTbus.h
        SimpleTbusAPI.cpp
        SimpleTbusAPI.h
        SimpleTbusCtl.cpp
        SimpleTbusCtl.h
        structs/SimpleChannelInfo.h
        structs/SimpleTbusInfo.h
        SimpleChannel.cpp
        SimpleChannel.h
        NetworkUtil.cpp
        NetworkUtil.h
        SimpleTbusd.cpp
        SimpleTbusd.h SimpleTbusdConn.cpp SimpleTbusdConn.h structs/TbusMsg.h SimpleMsg.cpp SimpleMsg.h)
target_link_libraries(simple_tbus ${Boost_LIBRARIES})


add_executable(create_channels sh/create_channels.cpp)
target_link_libraries(create_channels simple_tbus)


#  new

add_executable(tbusd_run run_tbusd.cpp)
target_link_libraries(tbusd_run simple_tbus)

add_executable(api_channel_receive run_api/run_channel_recv_api.cpp)
target_link_libraries(api_channel_receive simple_tbus)

add_executable(api_channel_send run_api/run_channel_send_api.cpp)
target_link_libraries(api_channel_send simple_tbus)
