//
// Created by capitalg on 2018/8/20.
//

#ifndef TENCENT_INTERN_SIMPLETBUSDCONN_H
#define TENCENT_INTERN_SIMPLETBUSDCONN_H
#include <unordered_map>
#include <boost/asio.hpp>
#include "structs/TbusMsg.h"
#include "structs/SimpleChannelInfo.h"
#include "NetworkUtil.h"
#include "SimpleChannel.h"


/**
 * SimpleTbusdConn需要改变tbus共享内存中对应通道的信息以及通道内容
 * 因此需要知道
 *          1.所有通道对应读写指针的地址，即通道->simplechannelInfo*的映射关系
 *          2.每个通道对应的共享内存地址, 即通道->通道共享内存地址 的映射关系
 */
class SimpleTbusdConn {
public:
    SimpleTbusdConn(boost::asio::ip::tcp::socket socket,
                    std::map<std::pair<uint32_t, uint32_t>, std::unique_ptr<SimpleChannel>> &channels,
                    std::map<uint32_t, uint32_t> &process_id2ip);

    void start();

    void do_read_message_type();

    void do_read_data_header();

    void do_read_data_body();

    void do_read_tbusmsg();

    void do_send_message_type();

    void do_send_tbusmsg();

    // todo
    void do_send_data(const void *msg_buffer, size_t message_len);

private:
    boost::asio::ip::tcp::socket socket_;
    TbusMsg tbus_msg;
    uint32_t len, cur_read_len, message_type;
    std::map<std::pair<uint32_t, uint32_t>, std::unique_ptr<SimpleChannel>> &channels;
    std::map<uint32_t, uint32_t> &process_id2ip;

    // todo 修改！
    inline bool is_local_host(uint32_t addr_n) {
        static uint32_t local_n = addr_aton("127.0.0.1");
        return addr_n == local_n;
    }
};


#endif //TENCENT_INTERN_SIMPLETBUSDCONN_H
