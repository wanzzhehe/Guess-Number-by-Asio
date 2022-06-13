#include <boost/asio.hpp>

#include <functional>
#include <iostream>
#include <memory>
#include <random>

using boost::asio::ip::tcp;

class tcp_connection : public std::enable_shared_from_this<tcp_connection> {
public:
    using pointer = std::shared_ptr<tcp_connection>;
    static pointer create(boost::asio::io_context &io) {
        return pointer(new tcp_connection(io));
    }

    tcp::socket &socket() { return m_socket; }
    void start() {
        boost::asio::async_read(
            m_socket, boost::asio::buffer(range),
            [this, self = shared_from_this()](
                const boost::system::error_code &ec, size_t) {
                if (range[0] > range[1]) {
                    range = {1, 1000};
                }
                boost::asio::async_write(
                    m_socket, boost::asio::buffer(range),
                    std::bind(&tcp_connection::handle_start, shared_from_this(),
                              std::placeholders::_1, std::placeholders::_2));
            });
    }

private:
    tcp_connection(boost::asio::io_context &io) : m_socket(io) {}

    void handle_start(const boost::system::error_code & /*ec*/,
                      size_t /*bytes_transferred*/) {
        static std::random_device rd;
        static std::default_random_engine rng{rd()};
        static std::uniform_int_distribution<int> u(range[0], range[1]);
        m_number = u(rng);

        std::cout << "New Player from " << m_socket.remote_endpoint().address()
                  << ':' << m_socket.remote_endpoint().port()
                  << " be in this server. The answer is " << m_number
                  << ". The range is [" << range[0] << ", " << range[1] << "]"
                  << std::endl;

        boost::asio::async_read(
            m_socket, boost::asio::buffer(buffer),
            std::bind(&tcp_connection::handle_guess, shared_from_this(),
                      std::placeholders::_1, std::placeholders::_2));
    }

    void handle_guess(const boost::system::error_code &ec,
                      size_t /*bytes_transferred*/) {
        if (ec) {
            std::cerr << ec.message() << std::endl;
            return;
        }
        int number = buffer[0];
        int ret = (number - m_number);
        memcpy(buffer.data(), std::addressof(ret), sizeof(int));

        std::cout << "Player from " << m_socket.remote_endpoint().address()
                  << ':' << m_socket.remote_endpoint().port() << " guesses "
                  << number << std::endl;

        if (number == m_number) {
            boost::asio::async_write(
                m_socket, boost::asio::buffer(buffer),
                std::bind(&tcp_connection::handle_success, shared_from_this(),
                          std::placeholders::_1, std::placeholders::_2));
        } else {
            boost::asio::async_write(
                m_socket, boost::asio::buffer(buffer),
                std::bind(&tcp_connection::handle_report, shared_from_this(),
                          std::placeholders::_1, std::placeholders::_2));
        }
    }

    void handle_report(const boost::system::error_code &ec,
                       size_t /*bytes_transferred*/) {
        if (ec) {
            std::cerr << ec.message() << " [" << ec << "]" << std::endl;
            return;
        }
        boost::asio::async_read(
            m_socket, boost::asio::buffer(buffer),
            std::bind(&tcp_connection::handle_guess, shared_from_this(),
                      std::placeholders::_1, std::placeholders::_2));
    }

    void handle_success(const boost::system::error_code /*ec*/,
                        size_t /*bytes_transferred*/) {
        std::cout << "Player from " << m_socket.remote_endpoint().address()
                  << ':' << m_socket.remote_endpoint().port() << " wines "
                  << std::endl;
    }

    tcp::socket m_socket;
    int m_number;
    std::array<int, 1> buffer;
    std::array<int, 2> range{1, 1000};
};

class tcp_server {
public:
    tcp_server(boost::asio::io_context &io)
        : m_io(io), m_acceptor{io, tcp::endpoint(tcp::v4(), 233)} {
        start_accept();
    }

private:
    void start_accept() {
        auto new_connection = tcp_connection::create(m_io);
        m_acceptor.async_accept(
            new_connection->socket(),
            std::bind(&tcp_server::handle_accept, this, new_connection,
                      std::placeholders::_1));
    }

    void handle_accept(tcp_connection::pointer new_connection,
                       const boost::system::error_code &ec) {
        if (!ec) {
            new_connection->start();
        }
        start_accept();
    }

    boost::asio::io_context &m_io;
    tcp::acceptor m_acceptor;
};

int main() {
    try {
        boost::asio::io_context io;
        tcp_server server{io};
        io.run();
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
}