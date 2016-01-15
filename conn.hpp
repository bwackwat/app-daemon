#ifndef APPD_CONN
#define APPD_CONN

#include <memory>
#include <iostream>
#include <utility>

#include <boost/asio.hpp>

class Server;

using namespace boost;

class Connection : public std::enable_shared_from_this<Connection>{
public:
	static const char* packet_types[];
	static int get_packet_type(const char* packet_type);

	void start();

	Connection(Server* server, asio::ip::tcp::socket socket);
private:
	Server* server_reference;
	asio::ip::tcp::socket socket_;

	char request[1024];
	std::string response;

	void do_read();
	void do_write(std::size_t length);
};

#endif
