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
                                    BOOST_LOG_TRIVIAL(info) << "read local proc id: " << addr_ntoa(buffer_proc_id);
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

                                    uint32_t send_proc = tbus_msg.from;
                                    uint32_t resv_proc = tbus_msg.to;
                                    if (tbus_msg.from_reader != 0) { //有写事件发生 由 from 发送
                                        if (_is_local(send_proc)) { // 发送方为本地进程
                                            if (_is_local(resv_proc)) { // 转发这条消息给to
                                                auto &recv_sock = local_conns[resv_proc]->get_socket();
                                                boost::system::error_code ignored_error; //todo  ignored
                                                boost::asio::write(recv_sock, boost::asio::buffer(&tbus_msg, sizeof(tbus_msg)), ignored_error);
                                            }
                                            else { // 转发这条消息给远程tbusd, 并发送data给远程tbusd
                                                auto &remote_tbus_endpoint = process_id2endpoint[resv_proc];
                                                auto &remote_tbusd_sock = *(remote_endpoint2socket[remote_tbus_endpoint]);

                                                auto &channel = *channels[std::make_pair(send_proc, resv_proc)];
                                                char *shm_start = channel.get_shm_ptr();

                                                boost::system::error_code ignored_error; //todo  ignored
                                                boost::asio::write(remote_tbusd_sock, boost::asio::buffer(&tbus_msg, sizeof(tbus_msg)), ignored_error);

                                                // todo 增加一个write_len字段
                                                // todo 循环队列还需要特殊考虑
                                                uint32_t write_len = tbus_msg.write_data_len,
                                                        last_write_index = channel.get_write_index() - write_len;
                                                boost::asio::write(remote_tbusd_sock, boost::asio::buffer(shm_start + last_write_index, write_len), ignored_error);
                                            }
                                        }
                                        else { // 发送方为远程tbusd
                                            if (_is_local(resv_proc)) { // 远程发送到本地，接收data
                                                // todo 接收的大小有限制
                                                char data_buffer[1024];
                                                boost::system::error_code ignored_error; //todo  ignored
                                                boost::asio::read(socket_, boost::asio::buffer(data_buffer, sizeof(uint32_t)), ignored_error);
                                                uint32_t msg_len = *reinterpret_cast<uint32_t*>(data_buffer);
                                                boost::asio::read(socket_, boost::asio::buffer(data_buffer + sizeof(uint32_t), msg_len), ignored_error);

                                                auto &channel = *channels[std::make_pair(send_proc, resv_proc)];
                                                channel.channel_write_raw(data_buffer, msg_len);

                                            }
                                            else { // 远程到远程，这是不应该发生的情况
                                                BOOST_LOG_TRIVIAL(error) << "recv write event error: remote to remote";
                                            }

                                        }
                                    }
                                    else { //读事件发生 由 to 发送
                                        if (_is_local(resv_proc)) {
                                            if (_is_local(send_proc)) {
                                                // do nothing
                                            }
                                            else { // 转发给send_proc所在的tbusd 读的情况

                                            }
                                        }
                                        else {
                                            if (_is_local(send_proc)) {

                                            }
                                            else {
                                                BOOST_LOG_TRIVIAL(error) << "recv read event error: remote to remote";
                                            }
                                        }

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
                                 std::map<uint32_t, std::pair<uint32_t, uint32_t>> &process_id2endpoint,
                                 std::map<uint32_t, std::shared_ptr<SimpleTbusdConn>> &local_conns,
                                 std::map<std::pair<uint32_t, uint32_t>, std::shared_ptr<boost::asio::ip::tcp::socket>> &remote_endpoint2socket) : socket_(std::move(socket)),
                                                                                         channels(channels),
                                                                                         process_id2endpoint(process_id2endpoint),
                                                                                         local_conns(local_conns),
                                                                                         remote_endpoint2socket(remote_endpoint2socket){
}



