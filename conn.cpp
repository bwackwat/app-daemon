#include <memory>
#include <iostream>
#include <utility>
#include <inttypes.h>

#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>

#include "conn.hpp"
#include "server.hpp"

using namespace boost;

void Connection::do_read(){
	auto self(shared_from_this());
	*request = 0;
	socket_.async_read_some(asio::buffer(request, 1024), [this, self](system::error_code ec, std::size_t length){
		if (ec){
			switch (ec.value()){
			case 2:
				//"End of file" (EOF) is OKAY. Ignored.
				break;
			default:
				std::cout << "server async_read_some error (" << ec.value() << "): " << ec.message() << std::endl;
				break;
			}
			return;
		}

		//std::cout << "RECEIVED PACKET" << std::endl;

		request[length] = '\0';

		std::string packet_type;
		std::string line;
		std::stringstream packet_parser(request);

		std::getline(packet_parser, packet_type, '\n');
		int packet_type_int = get_packet_type(packet_type.c_str());

		uint64_t request_time;

		response.clear();

		switch (packet_type_int){
		case 0:
			std::getline(packet_parser, line, '\n');
			request_time = lexical_cast<uint64_t>(line);
			std::cout << "Master has been requested at time: " << line << std::endl;
			//std::cout << request_time << std::endl;
			//std::cout << server_reference->master_request_time << std::endl;

			if (server_reference->master_request_time == 0){
				std::cout << "No other master request. Voting YES for this master." << std::endl;
				server_reference->master_request_time = request_time;
				response.append(Connection::packet_types[1]);
			}else if (request_time < server_reference->master_request_time){
				std::cout << "Older master request. Voting YES for this master." << std::endl;
				server_reference->master_request_time = request_time;
				response.append(Connection::packet_types[1]);
			}else{
				std::cout << "Newer master request. Voting NO for this master." << std::endl;
				response.append(Connection::packet_types[2]);
			}
			response.append("\n");

			break;
		case 3:
			std::cout << "Received Master Heartbeat. Sending Slave Heartbeat." << std::endl;
			server_reference->master_heartbeat_timeout_timer.cancel();
			server_reference->master_heartbeat_timeout_timer.expires_from_now(posix_time::seconds(4));
			server_reference->master_heartbeat_timeout_timer.async_wait(bind(&Server::master_heartbeat_timeout, server_reference, asio::placeholders::error));
			response.append(Connection::packet_types[4]);
			response.append("\n");
			break;
		default:
			std::cout << "packet_type: " << packet_type_int << " (" << packet_type << ")" << std::endl;
			while (std::getline(packet_parser, line, '\n')){
				std::cout << "packet_data for unknown packet_type: " << line << std::endl;
			}
			break;
		}

		do_write(length);
	});
}

void Connection::do_write(std::size_t length){
	auto self(shared_from_this());
	socket_.async_write_some(asio::buffer(response, response.length()), [this, self](system::error_code ec, std::size_t length){
		if (ec){
			std::cout << "server async_write_some error (" << ec.value() << "): " << ec.message() << std::endl;
			return;
		}

		do_read();
	});
}

const char* Connection::packet_types[] = {
	"Master Vote",
	"Slave Vote Yes",
	"Slave Vote No",
	"Master Heartbeat",
	"Slave Heartbeat",
	"Master App Request",
	"Slave App Response"
};

int Connection::get_packet_type(const char* packet_type){
	for (int i = 0; i < 5; i++){
		if (strcmp(packet_types[i], packet_type) == 0){
			return i;
		}
	}
	return -1;
}

Connection::Connection(Server* server, asio::ip::tcp::socket socket)
: server_reference(server),
socket_(std::move(socket)){
}

void Connection::start(){
	do_read();
}
