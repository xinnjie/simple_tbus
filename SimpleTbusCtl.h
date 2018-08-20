//
// Created by capitalg on 2018/8/17.
//

#ifndef TENCENT_INTERN_SIMPLETBUSCTL_H
#define TENCENT_INTERN_SIMPLETBUSCTL_H
#include <vector>
#include <string>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include "structs/SimpleChannelInfo.h"

class SimpleTbusCtl {
private:
    std::string shm_name;
    std::vector<SimpleChannelInfo> channel_infos;
    std::unique_ptr<boost::interprocess::shared_memory_object> shm_object_ptr;
    std::unique_ptr<boost::interprocess::mapped_region> region_ptr;
public:
    SimpleTbusCtl(const std::string &shm_name, uint32_t size);

    /**
     * 在共享内存中新增一条通道
     * @param src_addr
     * @param dest_addr
     * @param shm_name
     * @param shm_size
     */
    void add_channel(const std::string &src_addr, const std::string &dest_addr, const std::string &shm_name, uint32_t shm_size);

    /**
     * 真正向共享内存写入通道信息，并且（如果需要）创建通道
     */
    void create();

    /**
     * 清除共享内存中的通道信息
     */
    void drop();
};


#endif //TENCENT_INTERN_SIMPLETBUSCTL_H
