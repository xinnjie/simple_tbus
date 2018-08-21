//
// Created by capitalg on 2018/8/20.
//
#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>
#include "SimpleTbusdConn.h"
#include "structs/TbusMsg.h"


void SimpleTbusdConn::start() {
    do_read_message_type();
}

void SimpleTbusdConn::do_read_message_type() {
    auto self(shared_from_this());
    boost::asio::async_read(socket_,
                            boost::asio::buffer(&message_type, sizeof(message_type)),
                            [this, self](boost::system::error_code ec, std::size_t bytes_transferred) {
                                if (!ec && bytes_transferred == sizeof(message_type)) {
                                    switch (static_cast<MessageType>(message_type)) {
                                        case MessageType::TBUSMSG: {
                                            do_read_tbusmsg();
                                            break;
                                        }
                                        case MessageType::DATA: {
                                            do_read_data_header();
                                            break;
                                        }
                                        case MessageType::TELL_PRC_IP: {
                                            do_read_local_proc_id();
                                            break;
                                        }
                                        default:{
                                            BOOST_LOG_TRIVIAL(error) << "read message type error, type:" << message_type;
                                        }
                                    }
                                    BOOST_LOG_TRIVIAL(debug) << "read message type: " << message_type;
                                }
                                else {
                                    BOOST_LOG_TRIVIAL(error) << "read message type failure" << ec.message();
                                }
                            });
}


void SimpleTbusdConn::do_read_local_proc_id() {
    auto self(shared_from_this());
    boost::asio::async_read(socket_,
                            boost::asio::buffer(&buffer_proc_id, sizeof(buffer_proc_id)),
                            [this, self](boost::system::error_code ec, std::size_t bytes_transferred) {
                                if (!ec && bytes_transferred == sizeof(buffer_proc_id)) {
                                    BOOST_LOG_TRIVIAL(info) << "read local proc id" << addr_ntoa(buffer_proc_id);
                                    local_conns[buffer_proc_id] = shared_from_this();
                                    do_read_message_type();
                                } else {
                                    BOOST_LOG_TRIVIAL(error) << "read TbusMsg error";
                                }
                            });
}

void SimpleTbusdConn::do_read_tbusmsg() {
    auto self(shared_from_this());
    boost::asio::async_read(socket_,
                            boost::asio::buffer(&tbus_msg, sizeof(tbus_msg)),
                            [this, self](boost::system::error_code ec, std::size_t bytes_transferred) {
                                if (!ec && bytes_transferred == sizeof(tbus_msg)) {
                                    BOOST_LOG_TRIVIAL(debug) << "read TbusMsg: " << tbus_msg;
                                    // todo do something
                                    if (tbus_msg.from_reader) {
//                                        if ()

                                    }

                                } else {
                                    BOOST_LOG_TRIVIAL(error) << "read TbusMsg error";
                                }
                            });
}


void SimpleTbusdConn::do_read_data_header() {
    auto self(shared_from_this());
    boost::asio::async_read(socket_,
            boost::asio::buffer(&len, sizeof(len)),
            [this, self](boost::system::error_code ec, std::size_t bytes_transferred) {
                if (!ec && bytes_transferred == sizeof(len)) {
                    BOOST_LOG_TRIVIAL(debug) << "read data_ length: " << len;
                    do_read_data_body();
                }
                else {
                    BOOST_LOG_TRIVIAL(error) << "read TbusMsg error, TbusMsg len_ is not right: " << len;
                }
            });
}


// todo 收到后将数据写入对应的通道中
void SimpleTbusdConn::do_read_data_body(){
//    auto self(shared_from_this());
//    boost::asio::async_read(socket_,
//                            boost::asio::buffer(&len_, sizeof(len_)),
//                            [this, self](boost::system::error_code ec, std::size_t bytes_transferred) {
//                                if (!ec && len_ == sizeof(TbusMsg) && bytes_transferred == sizeof(TbusMsg)) {
//                                    len_ = 0;
//                                    do_read_body();
//                                }
//                                else {
//                                    BOOST_LOG_TRIVIAL(error) << "read TbusMsg error, TbusMsg len_ is not right: " << len_;
//                                }
//                            });
}

SimpleTbusdConn::SimpleTbusdConn(boost::asio::ip::tcp::socket socket,
                                 std::map<std::pair<uint32_t, uint32_t>, std::unique_ptr<SimpleChannel>> &channels,
                                 std::map<uint32_t, uint32_t> &process_id2ip,
                                 std::map<uint32_t, std::shared_ptr<SimpleTbusdConn>> &local_conns) : socket_(std::move(socket)),
                                                                                         channels(channels),
                                                                                         process_id2ip(process_id2ip),
                                                                                         local_conns(local_conns){
}



