//
// Created by capitalg on 2018/8/20.
//

#ifndef TENCENT_INTERN_SIMPLETBUSD_H
#define TENCENT_INTERN_SIMPLETBUSD_H
#include <boost/asio.hpp>


class SimpleTbusd {
public:
    SimpleTbusd(boost::asio::io_context &io_context,
            const boost::asio::ip::tcp::endpoint &endpoint);

private:
    void do_accept();
    boost::asio::ip::tcp::acceptor acceptor_;
};


#endif //TENCENT_INTERN_SIMPLETBUSD_H
