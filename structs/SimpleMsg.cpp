//
// Created by capitalg on 2018/8/21.
//

#include <cstring>
#include "SimpleMsg.h"


SimpleMsg::SimpleMsg(void *data, uint32_t len) {
    data_ = new char[len];
    len_ = len;
    memcpy(data_, data, len);
}

SimpleMsg::~SimpleMsg() {
    delete[] data_;
}
