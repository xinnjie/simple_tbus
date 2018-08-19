//
// Created by capitalg on 2018/8/17.
//

#ifndef TENCENT_INTERN_SIMPLETBUS_H
#define TENCENT_INTERN_SIMPLETBUS_H
#include <unordered_map>
#include <string>
#include "structs/SimpleChannelInfo.h"
#include "structs/SimpleTbusInfo.h"

class SimpleTbus {
public:
    SimpleTbus(const std::string &self_address, const std::string &shm_name);

private:
    std::unordered_map<std::string, SimpleChannelInfo> send_channel_infos;
    std::unordered_map<std::string, SimpleChannelInfo> write_channel_infos;

    std::string self_address;
    std::string shm_name;

    // 存储在共享内存中，只读
    SimpleTbusInfo *tbus_info;

};


#endif //TENCENT_INTERN_SIMPLETBUS_H
