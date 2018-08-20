//
// Created by capitalg on 2018/8/19.
//
#include <iostream>
#include "../SimpleTbusCtl.h"
#include "../SimpleTbus.h"

using namespace std;
int main(int argc, char *argv[]) {
    if (argc != 5) {
        cout << "usage: " << argv[0] << "tbus_name size src_ip dest_ip" << endl;
        return -1;
    }
    string shm_name = argv[1];
    int size = stoi(argv[2]);
    string src_ip = argv[3];
    string dest_ip = argv[4];

    SimpleTbusCtl tbus_ctl(shm_name, size);
    tbus_ctl.add_channel("0.0.0.1", "0.0.0.2", shm_name, 4096);
    tbus_ctl.create();
    return 0;
}