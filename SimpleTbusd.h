//
// Created by capitalg on 2018/8/20.
//

#ifndef TENCENT_INTERN_SIMPLETBUSD_H
#define TENCENT_INTERN_SIMPLETBUSD_H
#include <map>
#include <set>

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
     * @param process_id2endpoint   路由信息
     */
    SimpleTbusd(boost::asio::io_context &io_context,
                const boost::asio::ip::tcp::endpoint &accept_endpoint,
                const std::string &tbus_shm_name,
                std::map<uint32_t, std::pair<uint32_t, uint32_t>> process_id2endpoint);

private:
    void do_accept();

    void read_tbus_info(const std::string &tbus_shm_name);

/*
 * members
 */
    boost::asio::ip::tcp::acceptor acceptor_;

    // 通道信息  通道<processA,processB> -> channel
    std::unique_ptr<std::map<std::pair<uint32_t, uint32_t>, std::unique_ptr<SimpleChannel>>> channels_ptr;

    // 路由信息
    std::map<uint32_t, std::pair<uint32_t, uint32_t>> process_id2endpoint;
    std::map<std::pair<uint32_t, uint32_t>, std::shared_ptr<boost::asio::ip::tcp::socket>> remote_endpoint2socket;
    std::set<uint32_t> local_proc_ids;


    /*****************所有连接*********************/
//    std::vector<std::shared_ptr<SimpleTbusdConn>> conns;    // todo 是不是没有必要保存所有连接
    // 本地连接到tbusd的连接 process_id -> conn
    std::map<uint32_t, std::shared_ptr<SimpleTbusdConn>> local_conns;
    // 连接到其它tbusd的连接  ip -> conn
    std::unique_ptr<std::map<uint32_t, std::unique_ptr<SimpleTbusdConn>>> other_tbus_conns;

    // tbus共享内存信息
    std::unique_ptr<boost::interprocess::shared_memory_object> shm_obj_ptr;
    std::unique_ptr<boost::interprocess::mapped_region> region_ptr;
    SimpleTbusInfo *tbus_info;
};


#endif //TENCENT_INTERN_SIMPLETBUSD_H
