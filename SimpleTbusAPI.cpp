//
// Created by capitalg on 2018/8/17.
//
#include <memory>
#include <thread>
#include "SimpleTbus.h"
#include "SimpleTbusAPI.h"

static std::shared_ptr<SimpleTbus> tbus_ptr;

int bind(const std::string &process_id) {
    boost::asio::io_context io_context;
    boost::asio::ip::tcp::resolver resolver(io_context);
    std::string tbus_ip = "localhost",
                port = std::to_string(TBUSD_PORT);
    auto endpoints = resolver.resolve(tbus_ip, port);
    tbus_ptr = std::make_shared<SimpleTbus>(process_id, TBUS_SHM_NAME, io_context);

    std::thread t([&io_context]() {io_context.run();});
}

int send(const std::string &dest_id, const char *buff, size_t len) {
    return tbus_ptr->send_msg_impl(dest_id, buff, len);
}

int recv(char *buff, size_t &max_len) {
    return tbus_ptr->resv_msg_impl(buff, max_len);
}