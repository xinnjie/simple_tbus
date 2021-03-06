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
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        cout << "usage: " << argv[0] << "bind_ip" << endl;
        return -1;
    }
    string bind_ip = argv[1];

    check_success(simple_tbus::bind(bind_ip), "fail to bind");

    std::this_thread::sleep_for(2s);


    while (true) {
        char buffer[1024];
        size_t len = 1024;
        std::string source;
        auto success = simple_tbus::recv(buffer, len, source);
        check_success(success, "fail to resv");

        if (success == 0) {
            cout << "resv: " << buffer << endl;
        }
        std::this_thread::sleep_for(1s);
    }


//    cout << "read_index: " << send_channel.get_read_index() << "; write_index: " << send_channel.get_write_index();
    return 0;
}