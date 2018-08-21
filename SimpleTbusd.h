//
// Created by capitalg on 2018/8/20.
//

#ifndef TENCENT_INTERN_SIMPLETBUSD_H
#define TENCENT_INTERN_SIMPLETBUSD_H
#include <map>

#include <boost/asio.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>

#include "structs/SimpleChannelInfo.h"
#include "structs/SimpleTbusInfo.h"
#include "SimpleTbusdConn.h"
#include "SimpleChannel.h"

/**
 * 单线程tbusd
 */
class SimpleTbusd {
public:
    /**
     *
     * @param io_context
     * @param accept_endpoint tbusd监听地址
     * @param channel_addrs_p 通道的共享内存地址
     * @param channel_infos_p 通道信息
     * @param process_id2ip   路由信息
     */
    SimpleTbusd(boost::asio::io_context &io_context,
                const boost::asio::ip::tcp::endpoint &accept_endpoint,
                const std::string &tbus_shm_name,
                std::map<uint32_t, uint32_t> *process_id2ip);

    void read_tbus(const std::string &tbus_shm_name);

private:
    void do_accept();

    boost::asio::ip::tcp::acceptor acceptor_;

    // 通道信息  通道<processA,processB> -> channel
    std::unique_ptr<std::map<std::pair<uint32_t, uint32_t>, std::unique_ptr<SimpleChannel>>> channels_ptr;

    //路由信息
    std::unique_ptr<std::map<uint32_t, uint32_t>> process_id2ip;

    std::vector<std::unique_ptr<SimpleTbusdConn>> conns;
    std::unique_ptr<std::map<uint32_t, std::unique_ptr<SimpleTbusdConn>>> other_tbus_conns;

    // tbus共享内存信息
    std::unique_ptr<boost::interprocess::shared_memory_object> shm_obj_ptr;
    std::unique_ptr<boost::interprocess::mapped_region> region_ptr;
    SimpleTbusInfo *tbus_info;
};


#endif //TENCENT_INTERN_SIMPLETBUSD_H
