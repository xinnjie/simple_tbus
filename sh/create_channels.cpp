//
// Created by capitalg on 2018/8/19.
//
#include <iostream>
#include "../SimpleTbusCtl.h"
#include "../SimpleTbus.h"

using namespace std;
int main(int argc, char *argv[]) {
    if (argc != 7) {
        cout << "usage: " << argv[0] << "tbus_name tbus_shm_size channel_name channel_shm_size src_ip dest_ip" << endl;
        return -1;
    }
    string tbus_shm_name = argv[1];
    int tbus_shm_size = stoi(argv[2]);
    string channel_shm_name = argv[3];
    int channel_shm_size = stoi(argv[4]);
    string src_ip = argv[5];
    string dest_ip = argv[6];

    SimpleTbusCtl tbus_ctl(tbus_shm_name, tbus_shm_size);
    tbus_ctl.add_channel("0.0.0.1", "0.0.0.2", channel_shm_name, channel_shm_size);
    tbus_ctl.create();
    return 0;
}