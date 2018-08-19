//
// Created by capitalg on 2018/8/17.
//

#include <boost/interprocess/creation_tags.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include "SimpleChannel.h"

using namespace boost::interprocess;
SimpleChannel::SimpleChannel(SimpleChannelInfo *shm_channel_info) : shm_channel_info(shm_channel_info) {
    shm_name = shm_channel_info->shm_name;
    shared_memory_object shm(open_only, shm_name.c_str(), read_only);
    offset_t shm_size = 0;
    assert(shm.get_size(shm_size));
    region_ptr = std::make_unique<mapped_region>(shm, read_write, 0, shm_size);
}

/**
 * 向通道写入数据，通道为循环队列
 * @param buffer
 * @param buf_len
 * @return 0 on success, -1 on failure
 */
int SimpleChannel::channel_send(const char *buffer, size_t buf_len) {
    uint32_t size = get_shm_size();
    uint32_t write_shift = get_write_index();
    uint32_t read_shift = get_read_index();
    char *dest = get_shm_ptr();
    if (write_shift >= read_shift) {
        uint32_t write_to_tail_bytes = size - write_shift;
        // 能在队列尾直接写完
        if (buf_len < write_to_tail_bytes) {
            memcpy(dest + write_shift, buffer, buf_len);
            set_write_index(write_shift + buf_len);
            return 0;
        }

        // 需要在空闲的队列头部写
        if (buf_len < write_to_tail_bytes + read_shift) {
            memcpy(dest + write_shift, buffer, write_to_tail_bytes);
            memcpy(dest, buffer + write_to_tail_bytes, buf_len - write_shift);
            set_write_index(buf_len - write_to_tail_bytes);
            return 0;
        }
    }
    else {
        uint32_t write_to_tail_bytes = read_shift - write_shift;
        if (buf_len < write_to_tail_bytes) {
            memcpy(dest + write_shift, buffer, buf_len);
            set_write_index(write_shift + buf_len);
            return 0;
        }
        else {
            return -1;
        }
    }
    return -1;
}

int SimpleChannel::channel_resv(char *buffer, size_t &max_len) {
    uint32_t size = get_shm_size();
    uint32_t read_shift = get_read_index();
    uint32_t write_shift = get_write_index();

    char *src = get_shm_ptr();

    if (read_shift == write_shift) { // nothing to read
        return -1;
    }

    if (read_shift < write_shift) {
        uint32_t read_bytes = std::min(write_shift-read_shift, static_cast<uint32_t>(max_len));
        memcpy(buffer, src + read_shift, read_bytes);
        set_read_index(read_shift + read_bytes);
        max_len = read_bytes;
        return 0;
    }
    else {
        uint32_t read_to_tail_bytes = size - read_shift;
        if (max_len <= read_to_tail_bytes) {
            memcpy(buffer, src + read_shift, max_len);
            set_read_index(read_shift + max_len);
            return 0;
        }
        else {
            memcpy(buffer, src + read_shift, read_to_tail_bytes);
            uint32_t head_remaining_bytes = write_shift;
            uint32_t remain_to_read = std::min(head_remaining_bytes, static_cast<uint32_t>(max_len) - read_to_tail_bytes);
            memcpy(buffer + read_to_tail_bytes, src, remain_to_read);
            set_read_index(remain_to_read);
            max_len = read_to_tail_bytes + remain_to_read;
            return 0;
        }
    }
    return -1;
}
