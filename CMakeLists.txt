cmake_minimum_required(VERSION 3.10)
project(tencent_intern)

set(CMAKE_CXX_STANDARD 14)

#find_package(Boost COMPONENTS system)

include_directories(${Boost_INCLUDE_DIR})

add_executable(try_uname_share trys/try_uname_share.cpp)
target_link_libraries(try_uname_share ${Boost_LIBRARIES})

add_executable(try_mapped_region_write trys/try_mapped_region_write.cpp)
target_link_libraries(try_mapped_region_write ${Boost_LIBRARIES})

add_executable(try_mapped_region_read trys/try_mapped_region_read.cpp)
target_link_libraries(try_mapped_region_read ${Boost_LIBRARIES})


add_library(simple_tbus SimpleTbus.cpp SimpleTbus.h SimpleTbusAPI.cpp SimpleTbusAPI.h ShmInfos_delete/SimpleTbusInfo.h SimpleTbusCtl.cpp SimpleTbusCtl.h structs/SimpleChannelInfo.h structs/SimpleTbusInfo.h SimpleChannel.cpp SimpleChannel.h)
target_link_libraries(simple_tbus ${Boost_LIBRARIES})
