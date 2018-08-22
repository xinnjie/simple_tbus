//
// Created by capitalg on 2018/8/17.
//
#include <memory>
#include <thread>
#include "SimpleTbus.h"
#include "SimpleTbusAPI.h"

namespace simple_tbus {
//    todo 之后换一种写法
    static boost::asio::io_context io_context;
    static SimpleTbus *tbus_ptr;
    static std::thread *thread_ptr;

    int bind(const std::string &process_id) {
        boost::asio::ip::tcp::resolver resolver(io_context);
        std::string tbus_ip = "localhost",
                port = std::to_string(TBUSD_PORT);
        auto endpoints = resolver.resolve(tbus_ip, port);
        tbus_ptr = new SimpleTbus(process_id, TBUS_SHM_NAME, io_context, endpoints);
        thread_ptr = new std::thread([&]() { io_context.run(); });
        return 0;
    }

    int send(const std::string &dest_id, const char *buff, size_t len) {
        if (tbus_ptr == nullptr) return -1;
        return tbus_ptr->send_msg_impl(dest_id, buff, len);
    }

    int recv(char *buff, size_t &max_len) {
        if (tbus_ptr == nullptr) return -1;
        return tbus_ptr->resv_msg_impl(buff, max_len);
    }
}