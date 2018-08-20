//
// Created by capitalg on 2018/8/20.
//

#ifndef TENCENT_INTERN_SIMPLETBUSDCONN_H
#define TENCENT_INTERN_SIMPLETBUSDCONN_H
#include <boost/asio.hpp>
#include "structs/TbusMsg.h"

class SimpleTbusdConn {
public:
    void do_read_message_type();

    void do_read_data_header();

    void do_read_data_body();

    void do_read_tbusmsg();

private:
    boost::asio::ip::tcp::socket socket_;
    TbusMsg tbus_msg;
    uint32_t len, cur_read_len, message_type;
};


#endif //TENCENT_INTERN_SIMPLETBUSDCONN_H
