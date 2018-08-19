//
// Created by capitalg on 2018/8/19.
//
#include <arpa/inet.h>
#include <cstring>
#include <cassert>
#include "NetworkUtil.h"

const char *addr_ntoa(uint32_t addr) {
    static struct in_addr addr_struct;
    memcpy(&addr_struct, &addr, sizeof(addr));
    return inet_ntoa(addr_struct);
}

uint32_t addr_aton(const char *addr_str) {
    assert(addr_str != nullptr);
    return inet_addr(addr_str);
}
