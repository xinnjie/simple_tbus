//
// Created by capitalg on 2018/8/17.
//

#ifndef TENCENT_INTERN_SIMPLECHANNEL_H
#define TENCENT_INTERN_SIMPLECHANNEL_H


#include <cstddef>
#include <cstdint>
#include <string>
#include <boost/interprocess/mapped_region.hpp>
#include "structs/SimpleChannelInfo.h"

/*
 * 每个通道都是单向，A->B的通道只有A可以写入数据，B能读出数据
 * 因此A修改写指针，B修改读指针，可以不用加锁（除非进程A，B有多个线程同时修改）
 * SimpleChannel的channel信息包含在直接内存中的SimpleChannelInfo
 */
class SimpleChannel {
    SimpleChannelInfo *shm_channel_info;
    std::string shm_channel_name;  // channel 所在共享内存的名字
    std::unique_ptr<boost::interprocess::mapped_region> region_ptr;

public:
    /**
     * @param shm_channel_info 存在共享内存中的 SimpleChannelInfo 结构，SimpleChannel会对读写指针做操作
     */
    explicit SimpleChannel(SimpleChannelInfo *shm_channel_info);

    int channel_send(const char* buffer, size_t buf_len);

    int channel_resv(char *buffer, size_t &max_len);

    int channel_peek(char *buffer,size_t &max_len);


    /***************** getter & setter **********************/
    inline uint32_t get_shm_size() {
        return shm_channel_info->size;
    }

    inline uint32_t get_from_ip() {
        return shm_channel_info->from;
    }

    inline uint32_t get_to_ip() {
        return shm_channel_info->to;
    }

    inline uint32_t  get_read_index() {
        return shm_channel_info->read_index;
    }

    inline void set_read_index(uint32_t index) {
        shm_channel_info->read_index = index;
    }

    inline uint32_t get_write_index() {
        return shm_channel_info->write_index;
    }

    inline void set_write_index(uint32_t index) {
        shm_channel_info->write_index = index;
    }

    inline char *get_shm_ptr() {
        return static_cast<char*>(region_ptr->get_address());
    }
};


#endif //TENCENT_INTERN_SIMPLECHANNEL_H
