#ifndef APPD_HOST
#define APPD_HOST

#include <string>
#include <iostream>

enum HostType {
	TCP,
	IPC,
	IDK
};

struct AppDaemonHost {
	HostType type;
	std::string hostname;
	int port;

	std::string to_string(){
		std::string res = "";
		switch (type){
		case TCP:
			res += "TCP://";
			break;
		case IPC:
			res += "IPC://";
			break;
		default:
			res += "IDK://";
			break;
		}
		res += hostname;
		res += ":";
		res += std::to_string(port);
		return res;
	}
};

#endif