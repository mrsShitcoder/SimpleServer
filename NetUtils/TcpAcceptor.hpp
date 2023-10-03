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
		requires(TCoroutine coroutine, boost::system::error_code ec, boost::asio::ip::tcp::socket &&socket)
		{
			{coroutine(ec, socket)} -> std::same_as<boost::asio::awaitable<bool>>;
		};

	class TcpAcceptor
	{
	private:
		boost::asio::ip::tcp::acceptor _acceptor;
		boost::asio::io_context &_context;

	public:
		explicit TcpAcceptor(boost::asio::io_context &context, const boost::asio::ip::tcp::endpoint &endpoint)
			: _context(context), _acceptor(context, endpoint)
		{}

		template<AcceptExecutor TCoroutine>
		void asyncAccept(TCoroutine &&executor)
		{
			_acceptor.async_accept(
				boost::asio::bind_executor(_context.get_executor(),
										   [this, executor = std::forward<TCoroutine>(executor)](
											   boost::system::error_code ec, boost::asio::ip::tcp::socket &&socket)
										   {
											   if (!ec)
											   {
												   auto result = co_await executor(ec, std::move(socket));
												   if (result)
												   {
													   asyncAccept(std::forward<TCoroutine>(executor));
												   }
											   }
										   }));
		}

		boost::system::error_code close()
		{
			boost::system::error_code code;
			_acceptor.close(code);
			return code;
		}
	};
}// namespace SimpleServer