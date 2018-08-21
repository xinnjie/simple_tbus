//
// Created by capitalg on 2018/8/21.
//

#ifndef TENCENT_INTERN_TBUSMSG_H
#define TENCENT_INTERN_TBUSMSG_H
#include <cstdint>
#include <ostream>
#include "../NetworkUtil.h"

enum class MessageType : uint32_t {
    TBUSMSG = 101,
    DATA = 202
};

struct TbusMsg {
    uint32_t from;
    uint32_t to;
    uint32_t read_index;
    uint32_t write_index;
    uint32_t is_reader;  // 用来标识发送该消息的reader还是writer

    friend std::ostream &operator<<(std::ostream &os, const TbusMsg &msg) {
        os << "from: " << addr_ntoa(msg.from)  << " to: " << addr_ntoa(msg.to) << " read_index: " << msg.read_index << " write_index: "
           << msg.write_index <<  "is_reader: " << (msg.is_reader ?  "true" : "false");
        return os;
    }
};
#endif //TENCENT_INTERN_TBUSMSG_H