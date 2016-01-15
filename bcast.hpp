#ifndef APPD_BCAST
#define APPD_BCAST

#include <iostream>
#include <istream>
#include <ostream>
#include <string>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/chrono.hpp>

#include "adhost.cpp"
#include "conn.hpp"

using namespace boost;

class Server;

class BroadcastConnection{
public:
	BroadcastConnection(Server* server, AppDaemonHost* host, asio::io_service& broadcast_service, std::string* request);
private:
	Server* server_reference;
	AppDaemonHost* bcast_host;

	asio::ip::tcp::socket host_socket;

	std::string* host_request;
	char response[1024];

	void did_connect(const system::error_code& ec);
	void did_write(const system::error_code& ec, std::size_t);
	void did_read(const system::error_code& ec, std::size_t length);
};

#endif
