//
// Created by capitalg on 2018/8/17.
//

#include <iostream>
#include <boost/interprocess/creation_tags.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include "SimpleTbusCtl.h"
#include "NetworkUtil.h"
#include "structs/SimpleTbusInfo.h"


using namespace boost::interprocess;
/**
 * 创建一个TbusCtl对象
 * @param shm_name
 */
SimpleTbusCtl::SimpleTbusCtl(const std::string &shm_name, uint32_t size) : shm_name(shm_name) {
    shm_object_ptr = std::make_unique<shared_memory_object>(open_or_create, shm_name.c_str(), read_write);
    shm_object_ptr->truncate(size);
    region_ptr = std::make_unique<mapped_region>(*shm_object_ptr, read_write, 0, size);
}

void SimpleTbusCtl::add_channel(const std::string &src_addr, const std::string &dest_addr, const std::string &shm_name,
                                uint32_t shm_size) {
    SimpleChannelInfo channel_info = {
            shm_size,
            addr_aton(src_addr.c_str()),
            addr_aton(dest_addr.c_str()),
            0,
            0
    };
    strncpy(channel_info.shm_name, shm_name.c_str(), SIMPLE_TBUS_MAX_SHM_NAME_LEN-1);
    channel_infos.push_back(channel_info);
}

void SimpleTbusCtl::create() {
    char *shm_mem_ptr = static_cast<char *>(region_ptr->get_address());
    SimpleTbusInfo tbus_info;
    strncpy(tbus_info.name, shm_name.c_str(), SIMPLE_TBUS_MAX_SHM_NAME_LEN - 1);
    tbus_info.channel_count = channel_infos.size();
    memcpy(shm_mem_ptr, &tbus_info, sizeof(SimpleTbusInfo));
    shm_mem_ptr += sizeof(SimpleTbusInfo);

    for (auto channel_info : channel_infos) {
        memcpy(shm_mem_ptr, &channel_info, sizeof(SimpleChannelInfo));
        shm_mem_ptr += sizeof(SimpleChannelInfo);
        /*
         * 创建通道所需的共享内存
         */
        try {
            shared_memory_object shm_object(open_or_create, channel_info.shm_name, read_write, 0666);
            shm_object.truncate(channel_info.size);
        }
        catch (interprocess_exception &e) {
            std::cout << e.what() << std::endl;
        }
    }
}

void SimpleTbusCtl::drop() {
    for(auto &channel_info : channel_infos) {
        shared_memory_object::remove(channel_info.shm_name);
    }
    shared_memory_object::remove(shm_name.c_str());
}
