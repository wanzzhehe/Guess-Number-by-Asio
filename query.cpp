#include <array>
#include <iostream>

#include <boost/asio.hpp>

using boost::asio::ip::tcp;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Usage : " << argv[0] << " <Host>" << std::endl;
        return -1;
    }


    boost::asio::io_context io_context;
    tcp::resolver resolver{ io_context };

    auto endpoints = resolver.resolve(argv[1], "233");

    tcp::socket socket{ io_context };
    boost::asio::connect(socket, endpoints);

    {
        std::array<int, 2> read_buffer;
        size_t len = boost::asio::read(
            socket, boost::asio::buffer(read_buffer));
        if (len != sizeof(int) * 2) {
            std::cerr << "Unknown range." << std::endl;
            return 1;
        }
        std::cout << "猜数的范围为：" << read_buffer[0] << ' ' << read_buffer[1] << '\n';
    }

    for (;;) {
        int t;

        std::cout << "输入你猜的数字: ";
        std::cin >> t;

        boost::asio::write(socket, boost::asio::buffer(&t, sizeof(int)));
        boost::asio::read(socket, boost::asio::buffer(&t, sizeof(int)));

        if (t == 0) {
            std::cout << "哇，你猜对了！" << std::endl;
            break;
        }
        else if (t < -10) {
            std::cout << "太小了。" << std::endl;
        }
        else if (t < 0) {
            std::cout << "比较接近，但还是小了。" << std::endl;
        }
        else if (t > 10) {
            std::cout << "太大了。" << std::endl;
        }
        else {
            std::cout << "比较接近，但还是大了。" << std::endl;
        }
    }
    return 0;
}