#include <memory>
#include <iostream>
#include <fstream>

#include <atomic>

#include <thread>

#include <set>

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "adhost.cpp"
#include "server.hpp"
#include "bcast.hpp"
#include "conn.hpp"

using namespace boost;

void Server::do_accept(){
	acceptor_.async_accept(socket_, [this](system::error_code ec){
		if (ec){
			std::cout << "async_accept error (" << ec.value() << "): " << ec.message() << std::endl;
			return;
		}

		std::make_shared<Connection>(this, std::move(socket_))->start();

		do_accept();
	});

	//std::cout << "Accepting..." << std::endl;
}

void Server::master_heartbeat_broadcast(){
	std::cout << "Master Heartbeat broadcasting..." << std::endl;

	asio::io_service broadcast_service;

	std::string request = "";

	request.append(Connection::packet_types[3]);
	request.append("\n");

	for (auto& host : appd_hosts){
		if (host.port != server_port){
			broadcasters.emplace_back(this, &host, broadcast_service, &request);
		}
	}

	try{
		broadcast_service.run();
	}catch (std::exception& e){
		std::cerr << "broadcast_service.run() exception: " << e.what() << "\n";
	}

	std::cout << "Master Heartbeat broadcast complete!" << std::endl;
	broadcasters.clear();

	master_heartbeat_timer.expires_from_now(posix_time::seconds(2));
	master_heartbeat_timer.async_wait(bind(&Server::master_heartbeat_broadcast, this));
}

void Server::master_request_broadcast(){
	std::cout << "Master Heartbeat timed out, broadcasting Master Request..." << std::endl;

	asio::io_service broadcast_service;

	yes_votes = 0;
	no_votes = 0;

	std::string request = "";

	request.append(Connection::packet_types[0]);
	request.append("\n");
	master_request_time = chrono::time_point_cast<chrono::nanoseconds>(chrono::high_resolution_clock::now()).time_since_epoch().count();
	request.append(std::to_string(master_request_time));
	request.append("\n");

	for (auto& host : appd_hosts){
		if (host.port != server_port){
			broadcasters.emplace_back(this, &host, broadcast_service, &request);
		}
	}

	try{
		broadcast_service.run();
	}catch (std::exception& e){
		std::cerr << "broadcast_service.run() exception: " << e.what() << "\n";
	}

	std::cout << "Master Request broadcast complete! Votes: " << yes_votes.load() << " YES " << no_votes.load() << " NO." << std::endl;

	broadcasters.clear();

	if (yes_votes.load() > no_votes.load()){
		std::cout << "-------->>>>>>> this IS     the master" << std::endl;
		is_master = true;
		master_heartbeat_broadcast();
	}else{
		std::cout << "-------->>>>>>> this IS NOT the master" << std::endl;
		master_request_time = 0;
	}

	master_heartbeat_timeout_timer.expires_from_now(posix_time::seconds(4));
	master_heartbeat_timeout_timer.async_wait(bind(&Server::master_heartbeat_timeout, this, asio::placeholders::error));
}

void Server::master_heartbeat_timeout(const system::error_code& ec){
	if (ec){
		if (ec == asio::error::operation_aborted){
			//Trashed this timerwith cancel();
		}else{
			std::cout << "async_accept error (" << ec.value() << "): " << ec.message() << std::endl;
		}
	}else if (!is_master){
		if (broadcast_thread.joinable()){
			broadcast_thread.join();
		}
		broadcast_thread = std::thread(&Server::master_request_broadcast, this);
	}
}

Server::Server(int port)
: server_port(port),
acceptor_(io_service_, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), server_port)),
socket_(io_service_),
master_heartbeat_timer(io_service_),
master_heartbeat_timeout_timer(io_service_){

	std::ifstream infile("daemons.list", std::ifstream::in);
	if(!infile.good()){
		std::cerr << "No daemons.list! Bye." << std::endl;
		return;
	}
	std::string line;
	while (std::getline(infile, line)){
		AppDaemonHost next;
		if (strcmp(line.c_str(), "tcp") == 0){
			next.type = TCP;
		}else{
			next.type = IDK;
		}

		if (!std::getline(infile, line)){
			std::cout << "invalid daemon address in list" << std::endl;
			//throw std::exception("Invalid daemon list.");
		}
		next.hostname = line;

		if (!std::getline(infile, line)){
			std::cout << "invalid daemon port in list" << std::endl;
			//throw std::exception("Invalid daemon list.");
		}
		next.port = atoi(line.c_str());

		std::cout << "Host App Daemon @ " << next.to_string() << std::endl;

		appd_hosts.push_back(next);
	}
	infile.close();

	master_heartbeat_timeout_timer.expires_from_now(posix_time::seconds(4));
	master_heartbeat_timeout_timer.async_wait(bind(&Server::master_heartbeat_timeout, this, asio::placeholders::error));

	try{
		do_accept();
	}catch (std::exception& e){
		std::cerr << "do_accept() exception: " << e.what() << "\n";
	}

	try{
		io_service_.run();
	}catch (std::exception& e){
		std::cerr << "io_service_.run() exception: " << e.what() << "\n";
	}
}
