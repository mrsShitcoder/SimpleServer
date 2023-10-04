#include <boost/asio.hpp>
#include <iostream>
#include <array>
#include <boost/asio/detached.hpp>

#include "NetUtils/TcpAcceptor.hpp"

using boost::asio::ip::tcp;

class Session : public std::enable_shared_from_this<Session> {
public:
    Session(tcp::socket socket) : socket_(std::move(socket)) {}

    void start() {
        read();
    }

private:
    void read() {
        auto self(shared_from_this());
        socket_.async_read_some(
                boost::asio::buffer(data_),
                [this, self](boost::system::error_code ec, std::size_t length) {
                    if (!ec) {
                        write(length);
                    }
                });
    }

    void write(std::size_t length) {
        auto self(shared_from_this());
        boost::asio::async_write(
                socket_,
                boost::asio::buffer(data_, length),
                [this, self](boost::system::error_code ec, std::size_t /*length*/) {
                    if (!ec) {
                        read();
                    }
                });
    }

    tcp::socket socket_;
    std::array<char, 1024> data_;
};

class Server {
public:
    Server(boost::asio::io_context& io_context, short port)
        : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
        accept();
    }

private:
    void accept() {
        acceptor_.async_accept(
                [this](boost::system::error_code ec, tcp::socket socket) {
                    if (!ec) {
                        std::make_shared<Session>(std::move(socket))->start();
                    }
                    accept();
                });
    }

    tcp::acceptor acceptor_;
};

int main(int argc, char* argv[]) {
    try {
        if (argc != 2) {
            std::cerr << "Usage: async_tcp_echo_server <port>\n";
            return 1;
        }

        boost::asio::io_context io_context;

		SimpleServer::TcpAcceptor acceptor(io_context, tcp::endpoint(tcp::v4(), std::atoi(argv[1])));

		co_spawn(io_context,
				 acceptor.asyncAccept(
					 [](boost::asio::ip::tcp::socket &&socket) -> boost::asio::awaitable<bool>
					 {
						 std::make_shared<Session>(std::move(socket))->start();
						 co_return true;
					 }),
				 boost::asio::detached);

		io_context.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
