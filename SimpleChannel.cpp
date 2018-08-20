//
// Created by capitalg on 2018/8/17.
//

#include <boost/interprocess/creation_tags.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include "SimpleChannel.h"

using namespace boost::interprocess;
SimpleChannel::SimpleChannel(SimpleChannelInfo *shm_channel_info) : shm_channel_info(shm_channel_info) {
    shm_channel_name = shm_channel_info->shm_name;
    shared_memory_object shm(open_only, shm_channel_name.c_str(), read_write);
    offset_t shm_size = 0;
    assert(shm.get_size(shm_size));
    region_ptr = std::make_unique<mapped_region>(shm, read_write, 0, shm_size);
}

int SimpleChannel::channel_write_raw(const void *buffer, size_t buf_len) {
    if (get_remaining_write_bytes() < buf_len) {
        return -1;
    }
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
            memcpy(dest, static_cast<const char*>(buffer) + write_to_tail_bytes, buf_len - write_shift);
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

int SimpleChannel::channel_read_raw(void *buffer, size_t len) {
    if (get_remaining_read_bytes() < len) {
        return -1;
    }

    uint32_t size = get_shm_size();
    uint32_t read_shift = get_read_index();
    uint32_t write_shift = get_write_index();

    char *src = get_shm_ptr();

    if (read_shift <= write_shift) {
        memcpy(buffer, src + read_shift, len);
        set_read_index(read_shift + len);
        return 0;
    }
    else {
        uint32_t read_to_tail_bytes = size - read_shift;
        if (len <= read_to_tail_bytes) {
            memcpy(buffer, src + read_shift, len);
            set_read_index(read_shift + len);
            return 0;
        }
        else {
            memcpy(buffer, src + read_shift, read_to_tail_bytes);
            uint32_t remain_to_read = len - read_to_tail_bytes;
            memcpy(static_cast<char*>(buffer) + read_to_tail_bytes, src, remain_to_read);
            set_read_index(remain_to_read);
            return 0;
        }
    }
    return -1;
}

int SimpleChannel::channel_peek_raw(void *buffer, size_t len) {
    if (get_remaining_read_bytes() < len) {
        return -1;
    }

    uint32_t size = get_shm_size();
    uint32_t read_shift = get_read_index();
    uint32_t write_shift = get_write_index();

    char *src = get_shm_ptr();

    if (read_shift <= write_shift) {
        memcpy(buffer, src + read_shift, len);
        return 0;
    }
    else {
        uint32_t read_to_tail_bytes = size - read_shift;
        if (len <= read_to_tail_bytes) {
            memcpy(buffer, src + read_shift, len);
            return 0;
        }
        else {
            memcpy(buffer, src + read_shift, read_to_tail_bytes);
            uint32_t remain_to_read = len - read_to_tail_bytes;
            memcpy(static_cast<char*>(buffer) + read_to_tail_bytes, src, remain_to_read);
            return 0;
        }
    }
    return -1;
}

int SimpleChannel::channel_send_msg(const void *msg_buffer, size_t message_len) {
    if (get_remaining_write_bytes() < message_len + sizeof(uint32_t)) {
        return -1;
    }
    uint32_t len = message_len;
    channel_write_raw(reinterpret_cast<char*>(&len), sizeof(len));
    channel_write_raw(msg_buffer, message_len);
    return 0;
}

int SimpleChannel::channel_resv_msg(void *msg_buffer, size_t &max_msg_len) {
    if (get_remaining_read_bytes() < sizeof(uint32_t)) {
        return -1;
    }

    uint32_t len = 0;
    channel_peek_raw(&len, sizeof(len));
    if (get_remaining_read_bytes() < sizeof(uint32_t) + len) {
        return -1;
    }
    max_msg_len = len;
    channel_read_raw(&len, sizeof(len));
    channel_read_raw(msg_buffer, len);

    return 0;
}
