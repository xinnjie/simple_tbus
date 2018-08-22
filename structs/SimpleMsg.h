//
// Created by capitalg on 2018/8/21.
//

#ifndef TENCENT_INTERN_SIMPLEMSG_H
#define TENCENT_INTERN_SIMPLEMSG_H


#include <cstdint>

class SimpleMsg {
public:
    SimpleMsg(void *data, uint32_t len);

    ~SimpleMsg();

    inline void *get_data() { return data_;}
    inline uint32_t get_len() {return len_;}

private:
    char *data_;
    uint32_t len_;
};


#endif //TENCENT_INTERN_SIMPLEMSG_H
