//
// Created by capitalg on 2018/8/19.
//

#include <iostream>
#include "../SimpleTbusCtl.h"
#include "../SimpleTbus.h"

using namespace std;
int main(int argc, char *argv[]) {
    if (argc != 5) {
        cout << "usage: " << argv[0] << "tbus_name src_ip dest_ip send_message" << endl;
        return -1;
    }
    string shm_name = argv[1];
    string src_ip = argv[2];
    string dest_ip = argv[3];
    string message = argv[4];

    SimpleTbus tbus(src_ip, shm_name);
    SimpleChannel &send_channel = tbus.get_send_channel(dest_ip);
    int success = tbus.send_msg(dest_ip, message.c_str(), message.size() + 1);
    if (success == 0) {
        cout << "send: " << message << endl;
    }
    else {
        cout << "fail to send" << endl;
    }

    cout << "read_index: " << send_channel.get_read_index() << "; write_index: " << send_channel.get_write_index();
    return 0;
}