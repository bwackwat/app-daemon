#include <iostream>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>

#include "server.hpp"

int APPD_PORT;

int main(int argc, char** argv){
	//std::system("start /wait notepad");

	if (argc < 1 || argv[1] == 0){
		std::cout << "Usage: appd.exe <port 10000 - 20000>" << std::endl;
		std::cout << "No port given, please enter a port number: ";
		std::string line;
		std::getline(std::cin, line);
		
		try{
			APPD_PORT = std::atoi(line.c_str());
		}catch (std::exception& e){
			std::cerr << "std::atoi exception parsing '" << line.c_str() << "': " << e.what() << "\n";
			std::getchar();
			return 1;
		}
	}
	else{
		try{
			APPD_PORT = std::atoi(argv[1]);
		}catch (std::exception& e){
			std::cerr << "std::atoi exception parsing '" << argv[1] << "': " << e.what() << "\n";
			std::getchar();
			return 1;
		}
	}

	if (APPD_PORT < 10000 || APPD_PORT > 20000){
		std::cout << "App Daemon must be run on a port between 10000 and 20000." << std::endl;
		std::getchar();
		return 1;
	}
	std::cout << "This App Daemon @ port " << APPD_PORT << std::endl;

	try{
		Server s(APPD_PORT);
	}catch(std::exception& e){
		std::cerr << "Global exception: " << e.what() << "\n";
		std::getchar();
		return 1;
	}

	return 0;
}