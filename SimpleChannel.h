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
private:
    SimpleChannelInfo *shm_channel_info;
    std::string shm_channel_name;  // channel 所在共享内存的名字
    std::unique_ptr<boost::interprocess::mapped_region> region_ptr;

    /**
     * 向通道写入数据，通道为循环队列
     * @param buffer
     * @param buf_len
     * @return 0 on success, -1 on failure
     */
    int channel_write_raw(const void *buffer, size_t buf_len);

    /**
     * 从通道读取指定长度的数据，如果通道中没有这么多数据，返回失败
     * @param buffer
     * @param len
     * @return 0 on success, -1 on failure
     */
    int channel_read_raw(void *buffer, size_t len);

    /**
     * 从通道读取peek指定长度数据，但是不改变读指针
     * @param buffer
     * @param len
     * @return
     */
    int channel_peek_raw(void *buffer, size_t len);


public:
    /**
     * @param shm_channel_info 存在共享内存中的 SimpleChannelInfo 结构，SimpleChannel会对读写指针做操作
     */
    explicit SimpleChannel(SimpleChannelInfo *shm_channel_info);


    /**
     * 发送一条消息，每条消息头包含一个消息长度信息
     * @param msg_buffer
     * @param message_len
     * @return
     */
    int channel_send_msg(const void *msg_buffer, size_t message_len);

    int channel_resv_msg(void *msg_buffer, size_t &max_msg_len);


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

    inline uint32_t get_remaining_write_bytes() {
        return get_write_index() < get_read_index() ?
            get_read_index() - get_write_index() : get_shm_size() - (get_write_index() - get_read_index());
    }

    inline uint32_t get_remaining_read_bytes() {
        return get_read_index() <= get_write_index() ?
            get_write_index() - get_read_index() : get_shm_size() - (get_read_index() - get_write_index());
    }
};


#endif //TENCENT_INTERN_SIMPLECHANNEL_H
