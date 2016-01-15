#include <iostream>
#include <istream>
#include <ostream>
#include <string>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/chrono.hpp>
#include <boost/lexical_cast.hpp>

#include "bcast.hpp"
#include "server.hpp"
#include "conn.hpp"

using namespace boost;

BroadcastConnection::BroadcastConnection(Server* server, AppDaemonHost* host, asio::io_service& broadcast_service, std::string* request)
: server_reference(server),
bcast_host(host),
host_socket(broadcast_service),
host_request(request){

	//std::cout << "broadcasting to " << bcast_host->to_string() << std::endl;

	try{
		host_socket.async_connect(
			asio::ip::tcp::endpoint(asio::ip::address::from_string(bcast_host->hostname), bcast_host->port),
			bind(&BroadcastConnection::did_connect, this, asio::placeholders::error));
	}catch(std::exception& e){
		std::cerr << "host_socket.async_connect exception: " << e.what() << std::endl;
	}
}

void BroadcastConnection::did_connect(const system::error_code& ec){
	if (ec){
		if (ec.value() == 10061){
			std::cout << "APPD DOWN @ " << bcast_host->to_string() << std::endl;
		}else{
			std::cout << "async_connect error (" << ec.value() << "): " << ec.message() << std::endl;
		}
		return;
	}

	host_socket.async_write_some(asio::buffer(*host_request, (*host_request).length()), bind(&BroadcastConnection::did_write, this, asio::placeholders::error, asio::placeholders::bytes_transferred));
	//std::cout << "async wr(i/o)te" << std::endl;
}

void BroadcastConnection::did_write(const system::error_code& ec, std::size_t){
	if (ec){
		std::cout << "async_write_some error (" << ec.value() << "): " << ec.message() << std::endl;
		return;
	}

	host_socket.async_read_some(asio::buffer(response, 1024), bind(&BroadcastConnection::did_read, this, asio::placeholders::error, asio::placeholders::bytes_transferred));
	//std::cout << "async read" << std::endl;
}

void BroadcastConnection::did_read(const system::error_code& ec, std::size_t length){
	if (ec){
		std::cout << "async_read_some error (" << ec.value() << "): " << ec.message() << std::endl;
		return;
	}

	//std::cout << "RECEIVED PACKET" << std::endl;

	response[length] = '\0';
	std::string packet_type;
	std::string line;
	std::stringstream packet_parser(response);

	std::getline(packet_parser, packet_type, '\n');
	int packet_type_int = Connection::get_packet_type(packet_type.c_str());

	switch (packet_type_int){
	case 1:
		std::cout << "Got Yes Vote!" << std::endl;
		server_reference->yes_votes++;
		break;
	case 2:
		std::cout << "Got No Vote!" << std::endl;
		server_reference->no_votes++;
		break;
	case 4:
		std::cout << "Slave Heartbeat @ " << bcast_host->to_string() << std::endl;
		break;
	default:
		std::cout << "packet_type: " << packet_type_int << " (" << packet_type << ")" << std::endl;
		while (std::getline(packet_parser, line, '\n')){
			std::cout << "packet_data for unknown packet_type: " << line << std::endl;
		}
		break;
	}
}
