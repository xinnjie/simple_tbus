//
// Created by capitalg on 2018/8/17.
//
#include <iostream>

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/log/trivial.hpp>
#include <boost/asio.hpp>

#include "SimpleTbus.h"
#include "NetworkUtil.h"
#include "structs/TbusMsg.h"


using boost::asio::ip::tcp;

SimpleTbus::SimpleTbus(const std::string &self_process_id, const std::string &tbus_shm_name,
                       boost::asio::io_context &io_context, const boost::asio::ip::tcp::resolver::results_type &tbusd_endpoints):
          shm_name(tbus_shm_name),
          io_context_(io_context),
          socket_tbusd_(io_context),
          read_queue(1024) /*todo 超过1024个消息时queue会重新分配内存，这时不是thread safe的*/{
    read_tbus_shm(self_process_id, tbus_shm_name);
    do_connect_to_tbusd(tbusd_endpoints);
}


SimpleChannel &SimpleTbus::get_send_channel(const std::string &send_channel_name) {
    return *send_channels.at(addr_aton(send_channel_name.c_str()));
}

SimpleChannel &SimpleTbus::get_recv_channel(const std::string &read_channel_name) {
    return *recv_channels.at(addr_aton(read_channel_name.c_str()));
}

int
SimpleTbus::resv_msg(uint32_t channel_id, void *msg_buffer, size_t &max_msg_len) {
    SimpleChannel &resv_channel = *recv_channels.at(channel_id);
    return resv_channel.channel_resv_msg(msg_buffer, max_msg_len);
}

int SimpleTbus::resv_msg(const std::string &recv_channel_name, void *msg_buffer, size_t &max_msg_len) {
    uint32_t channel_id = addr_aton(recv_channel_name.c_str());
    return resv_msg(channel_id, msg_buffer, max_msg_len);
}

int SimpleTbus::send_msg(uint32_t channel_id, const void *msg_buffer, size_t message_len) {
    SimpleChannel &send_channel = *send_channels.at(channel_id);
    return send_channel.channel_send_msg(msg_buffer, message_len);
}

int SimpleTbus::send_msg(const std::string &send_channel_name, const void *msg_buffer, size_t message_len) {
    uint32_t channel_id = addr_aton(send_channel_name.c_str());
    return send_msg(channel_id, msg_buffer, message_len);
}



void SimpleTbus::read_tbus_shm(const std::string &self_process_id, const std::string &tbus_shm_name) {
    using namespace boost::interprocess;
    self_address_n = addr_aton(self_process_id.c_str());
    shm_obj_ptr = std::make_unique<shared_memory_object>(open_only, tbus_shm_name.c_str(), read_write);
    offset_t shm_size = 0;
    assert(shm_obj_ptr->get_size(shm_size));
    region_ptr = std::make_unique<mapped_region>(*shm_obj_ptr, read_write, 0, shm_size);
    void *shm_p = region_ptr->get_address();

    /*
     * 通道信息从共享内存中读出
     */
    tbus_info = static_cast<SimpleTbusInfo*>(shm_p);
    SimpleChannelInfo *channel_info = reinterpret_cast<SimpleChannelInfo*>(
            static_cast<char*>(shm_p) + sizeof(SimpleTbusInfo));
    int channel_count = tbus_info->channel_count;
    for (int i = 0; i < channel_count; ++i, ++channel_info) {
        if (self_address_n == channel_info->from) {
            BOOST_LOG_TRIVIAL(info) << "get send channel: " << addr_ntoa(channel_info->from)
                                    << " -> "
                                    << addr_ntoa(channel_info->to);
            send_channels[channel_info->to] = std::make_unique<SimpleChannel>(channel_info);
        }

        if (self_address_n == channel_info->to) {
            BOOST_LOG_TRIVIAL(info) << "get recv channel: " << addr_ntoa(channel_info->from)
                                    << " -> "
                                    << addr_ntoa(channel_info->to);
            // todo 需要从全局通道信息中恢复可读队列read_queue
            recv_channels[channel_info->from] = std::make_unique<SimpleChannel>(channel_info);
            SimpleChannel &resv_channel = *recv_channels[channel_info->from];
            if (resv_channel.get_remaining_read_bytes() > 0) {
                read_queue.push(channel_info->from);
            }
        }
    }
}

void SimpleTbus::do_connect_to_tbusd(const tcp::resolver::results_type &tbusd_endpoints) {
    boost::asio::async_connect(socket_tbusd_, tbusd_endpoints,
                               [this](boost::system::error_code ec, tcp::endpoint endpoint) {
                                   if (!ec) {
                                       BOOST_LOG_TRIVIAL(info) << "connected to tbusd" << endpoint;
                                       do_read_message_type();
                                   }
                                   else {
                                       BOOST_LOG_TRIVIAL(error) << "can not connect to tbusd" << endpoint;
                                       socket_tbusd_.close();
                                   }
                               });
}

void SimpleTbus::do_read_message_type() {
    boost::asio::async_read(socket_tbusd_,
                            boost::asio::buffer(&message_type, sizeof(message_type)),
                            [this](boost::system::error_code ec, std::size_t bytes_transferred) {
                                if (!ec && bytes_transferred == sizeof(message_type)) {
                                    switch (static_cast<MessageType>(message_type)) {
                                        case MessageType::TBUSMSG: {
                                            do_read_tbusmsg();
                                            break;
                                        }
                                        default:{
                                            BOOST_LOG_TRIVIAL(error) << "read message type error, type:" << message_type;
                                        }
                                    }
                                }
                                else {
                                    BOOST_LOG_TRIVIAL(error) << "read message type error failure";
                                }
                            });
}

void SimpleTbus::do_read_tbusmsg() {
    boost::asio::async_read(socket_tbusd_,
                            boost::asio::buffer(&tbus_msg, sizeof(tbus_msg)),
                            [this](boost::system::error_code ec, std::size_t bytes_transferred) {
                                if (!ec && bytes_transferred == sizeof(tbus_msg)) {
                                    BOOST_LOG_TRIVIAL(debug) << "read TbusMsg: " << tbus_msg;
                                    if (!tbus_msg.from_reader) { // 来自发送者，需要在读队列中增加一个新的可读消息
                                        read_queue.push(tbus_msg.read_index);
                                    }
                                    else {
                                        BOOST_LOG_TRIVIAL(error) << "should not receive from reader";
                                    }
                                    do_read_message_type();
                                } else {
                                    BOOST_LOG_TRIVIAL(error) << "read TbusMsg error";
                                }
                            });
}

void SimpleTbus::do_send_message_type(const TbusMsg &tbusMsg) {
    auto tbusmsg_ptr = std::make_shared<TbusMsg>(tbusMsg);
    boost::asio::async_write(socket_tbusd_,
                             boost::asio::buffer(&send_message_type,
                                                 sizeof(send_message_type)),
                             [this, tbusmsg_ptr](boost::system::error_code ec, std::size_t /*length*/) {
                                 if (!ec) {
                                     do_send_tbusmsg(tbusmsg_ptr);
                                 } else {
                                     socket_tbusd_.close();
                                     BOOST_LOG_TRIVIAL(error) << "write error, socket close";
                                 }
                             });
}

void SimpleTbus::notify_tbusd_after_send(const TbusMsg &tbusMsg) {
    do_send_message_type(tbusMsg);
}

void SimpleTbus::do_send_tbusmsg(std::shared_ptr<TbusMsg> tbusmsg_ptr) {
    boost::asio::async_write(socket_tbusd_,
                             boost::asio::buffer(&(*tbusmsg_ptr),
                                                 sizeof(TbusMsg)),
                                                 [this, tbusmsg_ptr](boost::system::error_code ec, std::size_t length) {
                                                     if (!ec && length == sizeof(TbusMsg)) {
                                                         BOOST_LOG_TRIVIAL(debug) << "send TbusMsg: " << *tbusmsg_ptr;
                                                     } else {
                                                         socket_tbusd_.close();
                                                         BOOST_LOG_TRIVIAL(error) << "write error, socket close";
                                                     }
                                                 });
}
