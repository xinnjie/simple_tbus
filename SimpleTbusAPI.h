//
// Created by capitalg on 2018/8/17.
//

#ifndef TENCENT_INTERN_SIMPLETBUSAPI_H
#define TENCENT_INTERN_SIMPLETBUSAPI_H

#define TBUS_SHM_NAME "simple_tbus_test"
#define TBUSD_PORT 12323

#include <string>
#include "SimpleTbus.h"

namespace simple_tbus {
    int bind(const std::string &process_id);

    int send(const std::string &dest_id, const char *buff, size_t len);

    int recv(char *buff, size_t &max_len, std::string &src_proc_id);
}

//int unbind()


#endif //TENCENT_INTERN_SIMPLETBUSAPI_H
