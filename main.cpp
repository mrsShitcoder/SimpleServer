#include <boost/asio.hpp>
#include <iostream>
#include <array>
#include <boost/asio/detached.hpp>

#include "Lib/TcpAcceptor.hpp"

using boost::asio::ip::tcp;

class Session{
public:
    Session(tcp::socket socket) : socket_(std::move(socket)) {}

	boost::asio::awaitable<void> start() {
        co_await read();
    }

private:
	boost::asio::awaitable<void> read()
	{
		for (;;)
		{
			std::size_t received = co_await boost::asio::async_read_until(socket_, boost::asio::dynamic_buffer(_buffer), "\n", boost::asio::use_awaitable);
			std::cout
				<< "Received: "
				<< std::string_view(reinterpret_cast<const char *>(_buffer.data()), received)
				<< "\n";
			co_await write(received);
			_buffer.clear();
		}
	}

	boost::asio::awaitable<void> write(std::size_t length) {
		std::cout << "Writing: " << std::string_view(reinterpret_cast<const char *>(_buffer.data()), length) << "\n";
        co_await boost::asio::async_write(
                socket_,
                boost::asio::buffer(_buffer, length),
               boost::asio::use_awaitable);
    }

    tcp::socket socket_;
    std::basic_string<std::byte> _buffer;
};

int main(int argc, char* argv[]) {
    try {
        if (argc != 2) {
            std::cerr << "Usage: async_tcp_echo_server <port>\n";
            return 1;
        }

        boost::asio::io_context io_context;

		SimpleServer::Lib::TcpAcceptor acceptor(io_context, tcp::endpoint(tcp::v4(), std::atoi(argv[1])));

		co_spawn(io_context,
				 acceptor.asyncAccept(
					 [](boost::asio::ip::tcp::socket &&socket) -> boost::asio::awaitable<bool>
					 {
						 co_await std::make_shared<Session>(std::move(socket))->start();
						 co_return true;
					 }),
				 boost::asio::detached);

		io_context.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
