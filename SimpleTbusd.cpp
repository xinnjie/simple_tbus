//
// Created by capitalg on 2018/8/20.
//
#include <boost/asio.hpp>
#include <iostream>
#include "SimpleTbusd.h"
#include "SimpleTbusdConn.h"

using boost::asio::ip::tcp;

void SimpleTbusd::do_accept() {
    acceptor_.async_accept(
            [this](boost::system::error_code ec, tcp::socket socket) {
                if (!ec) {
                    auto new_conn = std::make_unique<SimpleTbusdConn>(std::move(socket), *channel_addrs_p, *channel_infos_p, *process_id2ip);
                    new_conn->start();
                    local_process_conns.push_back(std::move(new_conn));
                }
                do_accept();
            });
}

/*
 * tbusd需要做的事情：
 *      连接到其它的tbusd，保存连接
 */
SimpleTbusd::SimpleTbusd(boost::asio::io_context &io_context,
                         const boost::asio::ip::tcp::endpoint &endpoint,
                         std::map<std::pair<uint32_t, uint32_t>, void *> *channel_addrs_p,
                         std::map<std::pair<uint32_t, uint32_t>, SimpleChannelInfo *> *channel_infos_p,
                         std::map<uint32_t, uint32_t> *process_id2ip) : acceptor_(io_context, endpoint), channel_addrs_p(
        channel_addrs_p), channel_infos_p(channel_infos_p), process_id2ip(process_id2ip) {
    do_accept();
//    endpoint.address()

}



