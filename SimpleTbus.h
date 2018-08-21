//
// Created by capitalg on 2018/8/17.
//

#ifndef TENCENT_INTERN_SIMPLETBUS_H
#define TENCENT_INTERN_SIMPLETBUS_H
#include <unordered_map>
#include <string>
#include <queue>
#include <boost/lockfree/queue.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include "structs/SimpleChannelInfo.h"
#include "structs/SimpleTbusInfo.h"
#include "SimpleChannel.h"
#include "structs/TbusMsg.h"
#include "SimpleMsg.h"

class SimpleTbus {
public:
    /**
     * 从名为shm_name的共享内存中找到所有通道信息，构建出 send,recv channel 的字典
     * @param self_process_id
     * @param tbus_shm_name
     */
    SimpleTbus(const std::string &self_process_id, const std::string &tbus_shm_name,
               boost::asio::io_context &io_context, const boost::asio::ip::tcp::resolver::results_type &tbusd_endpoints);



    /*
     * api
     */

    int send_msg_impl(const std::string &send_channel_name, const void *msg_buffer, size_t message_len);

    int resv_msg_impl(void *msg_buffer, size_t &max_msg_len);

    // todo 将下面的这些方法改为不抛出异常的
    /**
     * 发送一条消息
     * @param msg_buffer
     * @param message_len
     * @return 0 on success, -1 on failure
     */
    int send_msg(const std::string &send_channel_name, const void *msg_buffer, size_t message_len);

    int resv_msg(const std::string &recv_channel_name, void *msg_buffer, size_t &max_msg_len);

    int send_msg(uint32_t channel_id, const void *msg_buffer, size_t message_len);

    int resv_msg(uint32_t channel_id, void *msg_buffer, size_t &max_msg_len);

    /**
     * @param send_channel_name
     * @return 对应的channel
     * @throw 如果没有该channel，抛出std::out_of_range
     */
    SimpleChannel &get_send_channel(const std::string &send_channel_name);

    SimpleChannel &get_recv_channel(const std::string &read_channel_name);

    SimpleChannel &get_send_channel(uint32_t proc_id);

    SimpleChannel &get_recv_channel(uint32_t proc_id);
private:

    /**
     * 将tbus全局通道信息表从共享内存中读出
     * @param self_process_id
     * @param tbus_shm_name
     */
    void read_tbus_shm(const std::string &self_process_id, const std::string &tbus_shm_name);


    /*****************************网络***************************/
    void do_connect_to_tbusd(const boost::asio::ip::tcp::resolver::results_type &tbusd_endpoints);

    void do_read_message_type();

    void do_read_tbusmsg();

    /**
     * 告知自己的process_id
     */
     void send_process_id();




    /**
     * 在写完数据后通知tbusd
     */

    void handle_async_write(void *data, uint32_t len);
    void _aysn_write();
    void notify_tbusd_after_send(TbusMsg tbusMsg);
//    void do_send_tbusmsg_type(TbusMsg tbusMsg);
//    void do_send_tbusmsg(TbusMsg tbusmsg);

    /*
     * members
     */

    std::unordered_map<uint32_t, std::unique_ptr<SimpleChannel>> send_channels;
    std::unordered_map<uint32_t, std::unique_ptr<SimpleChannel>> recv_channels;

    // 点分十进制的进程id
    std::string self_address;  // use for debug

    /*
     * self_address_n是数字类型的进程id
     * self_address_n = aton(self_address)
     */
    uint32_t self_address_n;

    //tbus全局通道信息表
    std::string shm_name;
    std::unique_ptr<boost::interprocess::shared_memory_object> shm_obj_ptr;
    std::unique_ptr<boost::interprocess::mapped_region> region_ptr;

    // 存储在共享内存中的只读tbus_info
    SimpleTbusInfo *tbus_info;

    //可读通道号
    boost::lockfree::queue<uint32_t> read_queue;
    boost::lockfree::queue<SimpleMsg*> msg_queue;

    boost::asio::ip::tcp::socket socket_tbusd_;
    boost::asio::io_context &io_context_;

    uint32_t message_type, send_message_type = static_cast<uint32_t>(MessageType::TBUSMSG);
    TbusMsg tbus_msg;



};


#endif //TENCENT_INTERN_SIMPLETBUS_H
