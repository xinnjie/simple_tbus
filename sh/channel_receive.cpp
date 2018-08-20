//
// Created by capitalg on 2018/8/19.
//

#include <iostream>
#include "../SimpleTbusCtl.h"
#include "../SimpleTbus.h"

using namespace std;
int main(int argc, char *argv[]) {
    if (argc != 4) {
        cout << "usage: " << argv[0] << "tbus_name src_ip dest_ip" << endl;
        return -1;
    }
    string shm_name = argv[1];
    string src_ip = argv[2];
    string dest_ip = argv[3];

    char buffer[512];
    size_t len = sizeof(buffer);
    SimpleTbus tbus(dest_ip, shm_name);
    SimpleChannel &recv_channel = tbus.get_recv_channel(src_ip);
    int success = tbus.resv_msg(src_ip, buffer, len);
    if (success == 0) {
        cout << "received: " << buffer << endl;
    }
    else {
        cout << "fail to receive" << endl;
    }

    cout << "read_index: " << recv_channel.get_read_index() << "; write_index: " << recv_channel.get_write_index();
    return 0;
}