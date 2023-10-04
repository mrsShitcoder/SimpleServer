#pragma once

#include <boost/asio/awaitable.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <coroutine>

namespace SimpleServer
{
	template<typename TCoroutine>
	concept AcceptExecutor =
		requires(TCoroutine&& coroutine, boost::asio::ip::tcp::socket&& socket)
		{
			{coroutine(std::move(socket))} -> std::same_as<boost::asio::awaitable<bool>>;
		};

	class TcpAcceptor
	{
	private:
		boost::asio::ip::tcp::acceptor _acceptor;
		boost::asio::io_context &_context;


		void _completionHandler(std::exception_ptr eptr, bool result)
		{
			try
			{
				if (eptr)
				{
					std::rethrow_exception(eptr);
				}
				std::cout << "Connection handled with result: " << result << "\n";
			}
			catch (const std::exception &e)
			{
				std::cerr << "Exception while processing connection: " << e.what() << "\n";
			}
		}

	public:
		explicit TcpAcceptor(boost::asio::io_context &context, const boost::asio::ip::tcp::endpoint &endpoint)
			: _context(context), _acceptor(context, endpoint)
		{}

		template<AcceptExecutor TCoroutine>
		boost::asio::awaitable<void> asyncAccept(TCoroutine &&executor)
		{
			for (;;)
			{
				auto socket = co_await _acceptor.async_accept(boost::asio::use_awaitable);
				boost::asio::co_spawn(_context, executor(std::move(socket)),
									  [&](std::exception_ptr eptr, bool result) { _completionHandler(eptr, result); });
			}
		}

		boost::system::error_code close()
		{
			boost::system::error_code code;
			_acceptor.close(code);
			return code;
		}
	};
}// namespace SimpleServer