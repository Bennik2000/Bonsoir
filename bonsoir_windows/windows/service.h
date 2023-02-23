#pragma once

#include <string>

struct Service
{
public:
	std::string serviceName;
	std::string serviceType;
	std::string serviceIp;
	int port = 0;

	Service(){};
	Service(std::string &serviceName,
			std::string &serviceType,
			std::string &serviceIp,
			int port);
};
