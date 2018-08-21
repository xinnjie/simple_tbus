//
// Created by capitalg on 2018/8/17.
//
#include <boost/interprocess/shared_memory_object.hpp>
#include <iostream>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/log/trivial.hpp>

#include "SimpleTbus.h"
#include "NetworkUtil.h"

SimpleTbus::SimpleTbus(const std::string &self_address, const std::string &tbus_shm_name) : self_address(self_address),
                                                                                       shm_name(tbus_shm_name) {
    using namespace boost::interprocess;
    self_address_n = addr_aton(self_address.c_str());
    shm_obj_ptr = std::make_unique<shared_memory_object>(open_only, tbus_shm_name.c_str(), read_write);
    offset_t shm_size = 0;
    assert(shm_obj_ptr->get_size(shm_size));
    region_ptr = std::make_unique<mapped_region>(*shm_obj_ptr, read_write, 0, shm_size);
    void *shm_p = region_ptr->get_address();

    /*
     * 通道信息从共享内存中读出
     */
    tbus_info = static_cast<SimpleTbusInfo*>(shm_p);
    SimpleChannelInfo *channel_info = reinterpret_cast<SimpleChannelInfo*>(
            static_cast<char*>(shm_p) + sizeof(SimpleTbusInfo));
    int channel_count = tbus_info->channel_count;
    for (int i = 0; i < channel_count; ++i, ++channel_info) {
        if (self_address_n == channel_info->from) {
            BOOST_LOG_TRIVIAL(info) << "get send channel: " << addr_ntoa(channel_info->from)
                                    << " -> "
                                    << addr_ntoa(channel_info->to);
            send_channels[channel_info->to] = std::make_unique<SimpleChannel>(channel_info);
        }

        if (self_address_n == channel_info->to) {
            BOOST_LOG_TRIVIAL(info) << "get recv channel: " << addr_ntoa(channel_info->from)
                                    << " -> "
                                    << addr_ntoa(channel_info->to);
            recv_channels[channel_info->from] = std::make_unique<SimpleChannel>(channel_info);
        }
    }
}


SimpleChannel &SimpleTbus::get_send_channel(const std::string &send_channel_name) {
    return *send_channels.at(addr_aton(send_channel_name.c_str()));
}

SimpleChannel &SimpleTbus::get_recv_channel(const std::string &read_channel_name) {
    return *recv_channels.at(addr_aton(read_channel_name.c_str()));
}

int
SimpleTbus::resv_msg(uint32_t channel_id, void *msg_buffer, size_t &max_msg_len) {
    SimpleChannel &resv_channel = *recv_channels.at(channel_id);
    return resv_channel.channel_resv_msg(msg_buffer, max_msg_len);
}

int SimpleTbus::resv_msg(const std::string &recv_channel_name, void *msg_buffer, size_t &max_msg_len) {
    uint32_t channel_id = addr_aton(recv_channel_name.c_str());
    return resv_msg(channel_id, msg_buffer, max_msg_len);
}

int SimpleTbus::send_msg(uint32_t channel_id, const void *msg_buffer, size_t message_len) {
    SimpleChannel &send_channel = *send_channels.at(channel_id);
    return send_channel.channel_send_msg(msg_buffer, message_len);
}

int SimpleTbus::send_msg(const std::string &send_channel_name, const void *msg_buffer, size_t message_len) {
    uint32_t channel_id = addr_aton(send_channel_name.c_str());
    return send_msg(channel_id, msg_buffer, message_len);
}
