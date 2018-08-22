#include <boost/asio.hpp>
#include <iostream>
#include "../SimpleTbusd.h"
#include "../NetworkUtil.h"

using boost::asio::ip::tcp;
using namespace std;

int main(int argc, char *argv[]) {
//    try {
//        if (argc < 2) {
//            std::cerr << "Usage: simple_tbusd <port>\n";
//            return 1;
//        }

//        int port = std::atoi(argv[1]);

        int port = 12323;
        string tbus_shm_name = "simple_tbus_test" ;
        boost::asio::io_context io_context;
        tcp::endpoint endpoint(tcp::v4(), port);


        // 路由表
        auto route_info =  std::map<uint32_t, std::pair<uint32_t, uint32_t>>();
        route_info.insert({addr_aton("0.0.0.1"), {addr_aton("127.0.0.1"), 0}});
        route_info.insert({addr_aton("0.0.0.2"), {addr_aton("127.0.0.1"), 0}});


        SimpleTbusd tbusd(io_context, endpoint, tbus_shm_name, route_info);
        io_context.run();
//    }
//    catch (std::exception &e) {
//        std::cerr << "Exception: " << e.what() << "\n";
//    }

    return 0;
}