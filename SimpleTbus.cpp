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
void _restore_read_channel_queue(SimpleChannel &channel, boost::lockfree::queue<uint32_t> &read_queue);

SimpleTbus::SimpleTbus(const std::string &self_process_id, const std::string &tbus_shm_name,
                       boost::asio::io_context &io_context, const boost::asio::ip::tcp::resolver::results_type &tbusd_endpoints):
          shm_name(tbus_shm_name),
          io_context_(io_context),
          socket_tbusd_(io_context),
          read_queue(1024) /*todo 超过1024个消息时queue会重新分配内存，这时不是thread safe的*/,
          msg_queue(1024){
    read_tbus_shm(self_process_id, tbus_shm_name);
    do_connect_to_tbusd(tbusd_endpoints);
}


SimpleChannel &SimpleTbus::get_send_channel(const std::string &send_channel_name) {
    return *send_channels.at(addr_aton(send_channel_name.c_str()));
}

SimpleChannel &SimpleTbus::get_recv_channel(const std::string &read_channel_name) {
    return *recv_channels.at(addr_aton(read_channel_name.c_str()));
}

SimpleChannel &SimpleTbus::get_send_channel(uint32_t proc_id) {
    return *send_channels.at(proc_id);
}

SimpleChannel &SimpleTbus::get_recv_channel(uint32_t proc_id) {
    return *recv_channels.at(proc_id);
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
            recv_channels[channel_info->from] = std::make_unique<SimpleChannel>(channel_info);
            SimpleChannel &resv_channel = *recv_channels[channel_info->from];

            _restore_read_channel_queue(resv_channel, read_queue);
        }
    }
}

void _restore_read_channel_queue(SimpleChannel &channel, boost::lockfree::queue<uint32_t> &read_queue) {
    char *shm_start = channel.get_shm_ptr();
    uint32_t read_index = channel.get_read_index(),
            write_index = channel.get_write_index();
    // todo 需要从全局通道信息中恢复可读队列read_queue 暂时只考虑一个情况 fixme

    if (read_index < write_index) {
        while (read_index < write_index) {
            uint32_t msg_size = *reinterpret_cast<uint32_t*>(shm_start + read_index);
            read_queue.push(channel.get_from_ip());
            read_index += msg_size + sizeof(uint32_t);
        }
    }
}

void SimpleTbus::do_connect_to_tbusd(const tcp::resolver::results_type &tbusd_endpoints) {
    boost::asio::async_connect(socket_tbusd_, tbusd_endpoints,
                               [this](boost::system::error_code ec, tcp::endpoint endpoint) {
                                   if (!ec) {
                                       BOOST_LOG_TRIVIAL(info) << "connected to tbusd";
                                       send_process_id();
                                   }
                                   else {
                                       BOOST_LOG_TRIVIAL(error) << "can not connect to tbusd: " << endpoint;
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


void SimpleTbus::notify_tbusd_after_send(TbusMsg tbusMsg) {
    uint32_t msg_len = sizeof(TbusMsg) + sizeof(uint32_t);

    char *buffer = new char[msg_len];
    *reinterpret_cast<uint32_t*>(buffer) = static_cast<uint32_t>(MessageType::TBUSMSG);
    memcpy(buffer + sizeof(uint32_t), &tbusMsg, sizeof(TbusMsg));
    handle_async_write(buffer, msg_len);
    delete[] buffer;
}

void SimpleTbus::send_process_id() {
//    auto buffer = std::make_shared<std::array<uint32_t,2>>();
    uint32_t buffer[2];
    buffer[0] = static_cast<uint32_t>(MessageType::TELL_PRC_IP);
    buffer[1] = self_address_n;
    handle_async_write(buffer, 2* sizeof(uint32_t));
    do_read_message_type();
}




void SimpleTbus::handle_async_write(void *data, uint32_t len) {
    bool write_in_progress = !msg_queue.empty();
    msg_queue.push(new SimpleMsg(data, len));
    if (!write_in_progress)
    {
        _aysn_write();
    }
}

void SimpleTbus::_aysn_write() {
    SimpleMsg *msg_ptr;
    if (msg_queue.pop(msg_ptr)) {
        async_write(socket_tbusd_, boost::asio::buffer(msg_ptr->get_data(), msg_ptr->get_len()),
                    [this, msg_ptr](const boost::system::error_code &ec, std::size_t size) {
                        if (!ec) {
                            /***********************************debug****************************************************/
                            char *start = static_cast<char *>(msg_ptr->get_data());
                            uint32_t message_type = *reinterpret_cast<uint32_t *>(msg_ptr->get_data());
                            switch (static_cast<MessageType>(message_type)) {
                                case MessageType::TBUSMSG: {
                                    TbusMsg *tbusmsg = reinterpret_cast<TbusMsg *>(start + sizeof(uint32_t));
                                    BOOST_LOG_TRIVIAL(debug) << "send TbusMsg " << *tbusmsg;
                                    break;
                                }
                                case MessageType::TELL_PRC_IP: {
                                    uint32_t *proc_id = reinterpret_cast<uint32_t *>(start + sizeof(uint32_t));
                                    BOOST_LOG_TRIVIAL(debug) << "send pro_id " << addr_ntoa(*proc_id);
                                    break;
                                }
                            }
                            /***********************************debug****************************************************/
                            delete msg_ptr;
                            if (!msg_queue.empty()) {
                                _aysn_write();
                            }
                        } else {
                            BOOST_LOG_TRIVIAL(error) << "write error: " << ec.message();
                        }
                    });
    }
}

int SimpleTbus::send_msg_impl(const std::string &send_channel_name, const void *msg_buffer, size_t message_len) {
    int success = send_msg(send_channel_name, msg_buffer, message_len);
    if (success == 0) { //success
        TbusMsg notify_msg;
        notify_msg.from_reader = 0; //false
        notify_msg.from = self_address_n;
        notify_msg.to = addr_aton(send_channel_name.c_str());
        notify_msg.read_index = get_send_channel(send_channel_name.c_str()).get_read_index();
        notify_msg.write_index = get_send_channel(send_channel_name.c_str()).get_write_index();
        notify_msg.write_data_len = message_len + sizeof(uint32_t);
        notify_tbusd_after_send(notify_msg);
    }
    return success;
}

int SimpleTbus::resv_msg_impl(void *msg_buffer, size_t &max_msg_len, std::string &src_proc_id) {
    uint32_t resv_prc = 0;
    if (read_queue.pop(resv_prc)) {
        src_proc_id = addr_ntoa(resv_prc);
        return resv_msg(resv_prc, msg_buffer, max_msg_len);
    }
    return -1;
}
