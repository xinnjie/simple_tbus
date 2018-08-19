//
// Created by capitalg on 2018/8/17.
//
#include <boost/interprocess/shared_memory_object.hpp>
#include <iostream>
#include <boost/interprocess/mapped_region.hpp>
#include "SimpleTbus.h"

SimpleTbus::SimpleTbus(const std::string &self_address, const std::string &shm_name) : self_address(self_address),
                                                                                       shm_name(shm_name) {
    using namespace boost::interprocess;

    try {
        shared_memory_object shm(open_only, shm_name.c_str(), read_only);
        offset_t shm_size = 0;
        assert(shm.get_size(shm_size));
        mapped_region region(shm, read_only, 0, shm_size);
        void *shm_p = region.get_address();

        tbus_info = static_cast<SimpleTbusInfo*>(shm_p);
        int channel_count = tbus_info->channel_count;
        for (int i = 0; i < channel_count; ++i) {

        }

    }
    catch (interprocess_exception &e) {
        std::cout << e.what( ) << std::endl;
    }
}
