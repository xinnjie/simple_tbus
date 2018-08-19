//
// Created by capitalg on 2018/8/17.
//

#ifndef TENCENT_INTERN_SIMPLETBUS_H
#define TENCENT_INTERN_SIMPLETBUS_H
#include <unordered_map>
#include <string>
#include <boost/interprocess/mapped_region.hpp>
#include "structs/SimpleChannelInfo.h"
#include "structs/SimpleTbusInfo.h"
#include "SimpleChannel.h"

class SimpleTbus {
public:
    /**
     * 从名为shm_name的共享内存中找到所有通道信息，构建出 send,recv channel 的字典
     * @param self_address
     * @param shm_name
     */
    SimpleTbus(const std::string &self_address, const std::string &shm_name);


    /**
     *
     * @param send_channel_name
     * @return 对应的channel
     * @throw 如果没有该channel，抛出std::out_of_range
     */
    SimpleChannel &get_send_channel(const std::string &send_channel_name);

    SimpleChannel &get_recv_channel(const std::string &read_channel_name);

private:
    std::unordered_map<uint32_t, std::unique_ptr<SimpleChannel>> send_channels;
    std::unordered_map<uint32_t, std::unique_ptr<SimpleChannel>> recv_channels;

    std::string self_address;  // use for debug
    uint32_t self_address_n;
    std::string shm_name;

    std::unique_ptr<boost::interprocess::shared_memory_object> shm_obj_ptr;
    std::unique_ptr<boost::interprocess::mapped_region> region_ptr;

    // 存储在共享内存中，只读
    SimpleTbusInfo *tbus_info;

};


#endif //TENCENT_INTERN_SIMPLETBUS_H
