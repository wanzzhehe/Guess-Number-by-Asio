#include <array>
#include <iostream>

#include <boost/asio.hpp>

using boost::asio::ip::tcp;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cout << "Usage : " << argv[0] << " <Host>" << std::endl;
        return -1;
    }

    boost::asio::io_context io_context;
    tcp::resolver resolver{io_context};

    auto endpoints = resolver.resolve(argv[1], "233");

    tcp::socket socket{io_context};
    boost::asio::connect(socket, endpoints);

    {
        std::array<int, 2> range;
        std::string str;

        std::cout << "�Ƿ��Զ��巶Χ�� [y/N]";
        std::getline(std::cin, str);
        if (tolower(str[0]) != 'y') {
            range[0] = 0, range[1] = -1;
        } else {
            std::cout << "���뷶Χ���ÿո������";
            std::cin >> range[0] >> range[1];
        }
        boost::asio::write(socket, boost::asio::buffer(range));

        boost::asio::read(socket, boost::asio::buffer(range));
        std::cout << "ʵ�ʲ����ķ�ΧΪ��" << range[0] << ' ' << range[1]
                  << '\n';
    }

    for (;;) {
        int t;

        std::cout << "������µ�����: ";
        std::cin >> t;

        boost::asio::write(socket, boost::asio::buffer(&t, sizeof(int)));
        boost::asio::read(socket, boost::asio::buffer(&t, sizeof(int)));

        if (t == 0) {
            std::cout << "�ۣ���¶��ˣ�" << std::endl;
            break;
        } else if (t < -10) {
            std::cout << "̫С�ˡ�" << std::endl;
        } else if (t < 0) {
            std::cout << "�ȽϽӽ���������С�ˡ�" << std::endl;
        } else if (t > 10) {
            std::cout << "̫���ˡ�" << std::endl;
        } else {
            std::cout << "�ȽϽӽ��������Ǵ��ˡ�" << std::endl;
        }
    }
    return 0;
}