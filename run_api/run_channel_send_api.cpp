//
// Created by capitalg on 2018/8/21.
//

#include <iostream>
#include <thread>
#include "../SimpleTbusAPI.h"

using namespace std;

void check_success(int result, std::string fail_message) {
    if (result != 0) {
        cout << "check faild: " << fail_message << endl;
        exit(1);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        cout << "usage: " << argv[0] << "bind_ip dest_ip send_message" << endl;
        return -1;
    }
    string bind_ip = argv[1];
    string dest_ip = argv[2];
    string message = argv[3];

    check_success(simple_tbus::bind(bind_ip), "fail to bind");

    std::this_thread::sleep_for(2s);

    int i = 0;
    while (true) {
        ++i;
        string message_send = message + std::to_string(i);
        check_success(simple_tbus::send(dest_ip, message_send.c_str(), message_send.size() + 1), "fail to send");
        std::this_thread::sleep_for(3s);
    }

//    cout << "read_index: " << send_channel.get_read_index() << "; write_index: " << send_channel.get_write_index();
    return 0;
}