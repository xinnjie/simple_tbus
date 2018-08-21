//
// Created by capitalg on 2018/8/20.
//

#ifndef TENCENT_INTERN_SIMPLETBUSD_H
#define TENCENT_INTERN_SIMPLETBUSD_H
#include <boost/asio.hpp>
#include <unordered_map>
#include "structs/SimpleChannelInfo.h"
#include "SimpleTbusdConn.h"

/**
 * 单线程tbusd
 */
class SimpleTbusd {
public:
    /**
     *
     * @param io_context
     * @param endpoint
     * @param channel_addrs_p 通道的共享内存地址
     * @param channel_infos_p 通道信息
     * @param process_id2ip   路由信息
     */
    SimpleTbusd(boost::asio::io_context &io_context,
                const boost::asio::ip::tcp::endpoint &endpoint,
                std::map<std::pair<uint32_t, uint32_t>, void *> *channel_addrs_p,
                std::map<std::pair<uint32_t, uint32_t>, SimpleChannelInfo *> *channel_infos_p,
                std::map<uint32_t, uint32_t> *process_id2ip);

private:
    void do_accept();

    boost::asio::ip::tcp::acceptor acceptor_;

    std::unique_ptr<std::map<std::pair<uint32_t, uint32_t>, void*>> channel_addrs_p;
    std::unique_ptr<std::map<std::pair<uint32_t, uint32_t>, SimpleChannelInfo*>> channel_infos_p;
    //路由信息
    std::unique_ptr<std::map<uint32_t, uint32_t>> process_id2ip;

    std::vector<std::unique_ptr<SimpleTbusdConn>> local_process_conns;
    std::unique_ptr<std::map<uint32_t, std::unique_ptr<SimpleTbusdConn>>> other_tbus_conns;

};


#endif //TENCENT_INTERN_SIMPLETBUSD_H
