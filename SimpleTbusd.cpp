//
// Created by capitalg on 2018/8/20.
//
#include <boost/asio.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/log/trivial.hpp>

#include <iostream>
#include "SimpleTbusd.h"
#include "SimpleTbusdConn.h"
#include "structs/SimpleTbusInfo.h"

using boost::asio::ip::tcp;

void SimpleTbusd::do_accept() {
    acceptor_.async_accept(
            [this](boost::system::error_code ec, tcp::socket socket) {
                if (!ec) {
                    BOOST_LOG_TRIVIAL(info) << socket.remote_endpoint().address() << " connected";
                    std::make_shared<SimpleTbusdConn>(std::move(socket), *channels_ptr, process_id2endpoint, local_conns, remote_endpoint2socket)->start();
                }
                do_accept();
            });
}

/*
 * tbusd需要做的事情：
 *      连接到其它的tbusd，保存连接
 */
SimpleTbusd::SimpleTbusd(boost::asio::io_context &io_context,
                         const boost::asio::ip::tcp::endpoint &accept_endpoint,
                         const std::string &tbus_shm_name,
                         std::map<uint32_t, std::pair<uint32_t, uint32_t>> process_id2endpoint) : acceptor_(io_context, accept_endpoint),
                         process_id2endpoint(process_id2endpoint) {
    read_tbus_info(tbus_shm_name);
    do_accept();
}

void SimpleTbusd::read_tbus_info(const std::string &tbus_shm_name) {
    using namespace boost::interprocess;
    shm_obj_ptr = std::make_unique<shared_memory_object>(open_only, tbus_shm_name.c_str(), read_write);
    offset_t shm_size = 0;
    assert(shm_obj_ptr->get_size(shm_size));
    region_ptr = std::make_unique<mapped_region>(*shm_obj_ptr, read_write, 0, shm_size);
    void *shm_p = region_ptr->get_address();

    channels_ptr = std::make_unique<std::map<std::pair<uint32_t, uint32_t>, std::unique_ptr<SimpleChannel>>>();
    /*
     * 通道信息从共享内存中读出
     */
    tbus_info = static_cast<SimpleTbusInfo*>(shm_p);
    SimpleChannelInfo *channel_info = reinterpret_cast<SimpleChannelInfo*>(
            static_cast<char*>(shm_p) + sizeof(SimpleTbusInfo));
    int channel_count = tbus_info->channel_count;
    for (int i = 0; i < channel_count; ++i, ++channel_info) {
        uint32_t from = channel_info->from,
                to = channel_info->to;
        BOOST_LOG_TRIVIAL(info) << "get channel: " << addr_ntoa(from)
                                << " -> "
                                << addr_ntoa(to);
        (*channels_ptr)[std::make_pair(from, to)] = std::make_unique<SimpleChannel>(channel_info);
    }
}




