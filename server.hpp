#ifndef APPD_SERVER
#define APPD_SERVER

#include <memory>
#include <iostream>
#include <fstream>
#include <list>
#include <inttypes.h>

#include <atomic>

#include <thread>

#include <set>

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "adhost.cpp"
#include "bcast.hpp"
#include "conn.hpp"

using namespace boost;

class Server{
private:
	int server_port;

	asio::io_service io_service_;
	asio::ip::tcp::acceptor acceptor_;
	asio::ip::tcp::socket socket_;

	asio::deadline_timer master_heartbeat_timer;

	std::thread broadcast_thread;

	std::vector<AppDaemonHost> appd_hosts;
	std::list<BroadcastConnection> broadcasters;

	void do_accept();
	void master_heartbeat_broadcast();
	void master_request_broadcast();

public:
	Server(int port);

	void master_heartbeat_timeout(const system::error_code& ec);
	asio::deadline_timer master_heartbeat_timeout_timer;

	std::atomic<bool> is_master = {false};
	std::atomic<uint64_t> master_request_time = {0};

	std::atomic<int> yes_votes;
	std::atomic<int> no_votes;
};

#endif
