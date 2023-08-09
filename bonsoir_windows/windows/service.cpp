#include "service.h"

Service::Service(std::string &serviceName, std::string &serviceType, std::string &serviceIp, int port)
    : serviceName(serviceName),
      serviceType(serviceType),
      serviceIp(serviceIp),
      port(port)
{
}
