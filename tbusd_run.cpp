#include <boost/asio.hpp>
#include <iostream>
#include "SimpleTbusd.h"

using boost::asio::ip::tcp;

int main(int argc, char *argv[]) {
    try {
        if (argc < 2) {
            std::cerr << "Usage: simple_tbusd <port>\n";
            return 1;
        }

        int port = std::atoi(argv[1]);
        boost::asio::io_context io_context;
        tcp::endpoint endpoint(tcp::v4(), port);

        SimpleTbusd tbusd(io_context);
        io_context.run();
    }
    catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}