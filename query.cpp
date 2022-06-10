#include <array>
#include <iostream>

#include <boost/asio.hpp>

using boost::asio::ip::tcp;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cout << "Usage : " << argv[0] << " <Host>" << std::endl;
        return -1;
    }

    std::array<char, sizeof(int)> send_buffer;
    std::array<char, 128> read_buffer;

    boost::asio::io_context io_context;
    tcp::resolver resolver{io_context};
    auto endpoints = resolver.resolve(
        tcp::endpoint(boost::asio::ip::address_v4::from_string(argv[1]), 233));

    try {
        tcp::socket socket{io_context};
        boost::asio::connect(socket, endpoints);

        {
            size_t len = boost::asio::read(
                socket, boost::asio::buffer(read_buffer, sizeof(int) * 2));
            if (len != sizeof(int) * 2) {
                std::cerr << "Unknown range." << std::endl;
                return 1;
            }
            int border[2];
            memcpy(border, read_buffer.data(), sizeof(int) * 2);
            std::cout << "猜数的范围为：" << border[0] << ' ' << border[1]
                      << '\n';
        }

        for (;;) {
            {
                std::cout << "输入你猜的数字: ";
                int t;
                std::cin >> t;
                memcpy(send_buffer.data(), &t, sizeof(int));
                boost::asio::write(socket, boost::asio::buffer(send_buffer));
            }
            {
                boost::system::error_code ec;
                boost::asio::read(
                    socket, boost::asio::buffer(read_buffer, sizeof(int)), ec);
                if (!ec || ec == boost::asio::error::eof) {
                    int t;
                    memcpy(&t, read_buffer.data(), sizeof(int));
                    if (t == 0) {
                        std::cout << "哇，你猜对了！" << std::endl;
                        break;
                    } else if (t < -10) {
                        std::cout << "太小了。" << std::endl;
                    } else if (t < 0) {
                        std::cout << "比较接近，但还是小了。" << std::endl;
                    } else if (t > 10) {
                        std::cout << "太大了。" << std::endl;
                    } else {
                        std::cout << "比较接近，但还是大了。" << std::endl;
                    }
                } else {
                    throw boost::system::system_error(ec);
                }
            }
        }
    } catch (std::exception &e) {
        std::cerr << e.what() << '\n';
    }

    return 0;
}