//
// Created by capitalg on 2018/8/20.
//
#include <boost/asio.hpp>
#include <iostream>
#include "SimpleTbusd.h"

using boost::asio::ip::tcp;

SimpleTbusd::SimpleTbusd(boost::asio::io_context &io_context,
        const boost::asio::ip::tcp::endpoint &endpoint): acceptor_(io_context, endpoint) {
    do_accept();
}

void SimpleTbusd::do_accept() {
//    acceptor_.async_accept(
//            [this](boost::system::error_code ec, tcp::socket socket) {
//                if (!ec) {
//                    std::make_shared<chat_session>(std::move(socket), room_)->start();
//                }
//
//                do_accept();
//            });
}



