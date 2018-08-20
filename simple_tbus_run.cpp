//
// Created by capitalg on 2018/8/19.
//
#include <iostream>
#include "SimpleTbusCtl.h"
#include "SimpleTbus.h"

using namespace std;
int main(int argc, char *argv[]) {
    string shm_name = "test_channel_1";
    SimpleTbusCtl tbus_ctl(shm_name, 4096);
    tbus_ctl.add_channel("0.0.0.1", "0.0.0.2", shm_name, 4096);
    tbus_ctl.create();

    SimpleTbus tbus_send("0.0.0.1", shm_name);
    SimpleChannel &send_channel = tbus_send.get_send_channel("0.0.0.2");

    SimpleTbus tbus_recv("0.0.0.2", shm_name);
    SimpleChannel &recv_channel = tbus_recv.get_recv_channel("0.0.0.1");

    send_channel.channel_send("hello,world!", sizeof("hello,world!"));

    char buffer[64];
    size_t len = sizeof(buffer);
    recv_channel.channel_resv(buffer, len);

    cout << buffer << endl;


    return 0;
}