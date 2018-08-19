//
// Created by capitalg on 2018/8/17.
//

#ifndef TENCENT_INTERN_SIMPLECHANNELINFO_H
#define TENCENT_INTERN_SIMPLECHANNELINFO_H

#define SIMPLE_TBUS_MAX_SHM_NAME_LEN 64

#include <cstddef>
#include <cstdint>

/**
 * SimpleChannelInfo 被存储在共享内存中
 */
struct SimpleChannelInfo {
    uint32_t  size; // channel的大小，round up 到 2^n
    uint32_t from;  // 用来唯一标识每一个进程
    uint32_t to;
    uint32_t read_index;
    uint32_t write_index;
    char shm_name[SIMPLE_TBUS_MAX_SHM_NAME_LEN  ];  // channel 所在共享内存的名字
};



#endif //TENCENT_INTERN_SIMPLECHANNELINFO_H
