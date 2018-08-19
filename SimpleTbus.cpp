//
// Created by capitalg on 2018/8/17.
//
#include <boost/interprocess/shared_memory_object.hpp>
#include <iostream>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/log/trivial.hpp>

#include "SimpleTbus.h"
#include "NetworkUtil.h"

SimpleTbus::SimpleTbus(const std::string &self_address, const std::string &shm_name) : self_address(self_address),
                                                                                       shm_name(shm_name) {
    using namespace boost::interprocess;
    self_address_n = addr_aton(self_address.c_str());
    shm_obj_ptr = std::make_unique<shared_memory_object>(open_only, shm_name.c_str(), read_write);
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
