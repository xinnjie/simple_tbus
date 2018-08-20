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
     * 发送一条消息
     * @param msg_buffer
     * @param message_len
     * @return 0 on success, -1 on failure
     */
    int send_msg(const std::string &send_channel_name, const void *msg_buffer, size_t message_len);

    int resv_msg(const std::string &recv_channel_name, void *msg_buffer, size_t &max_msg_len);

    int send_msg(uint32_t channel_id, const void *msg_buffer, size_t message_len);

    int resv_msg(uint32_t channel_id, void *msg_buffer, size_t &max_msg_len);

    /**
     * @param send_channel_name
     * @return 对应的channel
     * @throw 如果没有该channel，抛出std::out_of_range
     */
    SimpleChannel &get_send_channel(const std::string &send_channel_name);

    SimpleChannel &get_recv_channel(const std::string &read_channel_name);
private:
    std::unordered_map<uint32_t, std::unique_ptr<SimpleChannel>> send_channels;
    std::unordered_map<uint32_t, std::unique_ptr<SimpleChannel>> recv_channels;

    // 点分十进制的进程id
    std::string self_address;  // use for debug

    /*
     * self_address_n是数字类型的进程id
     * self_address_n = aton(self_address)
     */
    uint32_t self_address_n;
    // tbus信息所在的共享内存名称
    std::string shm_name;

    std::unique_ptr<boost::interprocess::shared_memory_object> shm_obj_ptr;
    std::unique_ptr<boost::interprocess::mapped_region> region_ptr;

    // 存储在共享内存中的只读tbus_info
    SimpleTbusInfo *tbus_info;
};


#endif //TENCENT_INTERN_SIMPLETBUS_H
