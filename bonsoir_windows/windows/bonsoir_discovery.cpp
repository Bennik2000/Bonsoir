#include "bonsoir_discovery.h"

using namespace flutter;

void DNSServiceResolveCallback(
    DNSServiceRef sdRef,
    DNSServiceFlags flags,
    uint32_t interfaceIndex,
    DNSServiceErrorType errorCode,
    const char *fullname,
    const char *hosttarget,
    uint16_t port,
    uint16_t txtLen,
    const unsigned char *txtRecord,
    void *context)
{
    auto ip = gethostbyname(hosttarget);
    const char *ipAddress = inet_ntoa(*(in_addr *)ip->h_addr_list[0]);

    auto resolveContext = (ResolveContext *)context;

    std::string ipAddrStr = std::string(ipAddress);
    std::string fullnameStr = std::string(fullname);
    std::string hosttargetStr = std::string(hosttarget);


    Service service = resolveContext->serviceToResolve;
    service.port = port;
    service.serviceIp = ipAddrStr;

    resolveContext->bonsoirDiscovery->onServiceResolved(service);

    printf(
        "RESOLVE:\n  fullname: %s\n  target: %s\n  rxtRecord: %s\n  port: %d\n  ip: %s\n",
        fullname,
        hosttarget,
        txtRecord,
        port,
        ipAddress);
    std::cout << std::endl;
}

void DNSServiceBrowseCallback(
    DNSServiceRef sdRef,
    DNSServiceFlags flags,
    uint32_t interfaceIndex,
    DNSServiceErrorType errorCode,
    const char *serviceName,
    const char *regtype,
    const char *replyDomain,
    void *context)
{
    if (errorCode != kDNSServiceErr_NoError)
    {
        // TODO: Handle error
        return;
    }

    auto bonsoirDiscovery = (BonsoirDiscovery *)context;

    std::string serviceNameStr = std::string(serviceName);

    bool isNewBrowsed = (flags & kDNSServiceFlagsAdd) != 0;

    if (isNewBrowsed)
    {
        std::string regtypeStr = std::string(regtype);
        std::string serviceIpStr = std::string("");

        Service service = Service(
            serviceNameStr,
            regtypeStr,
            serviceIpStr,
            0);

        bool isAlreadyKnownAtOtherInterface = bonsoirDiscovery->onServiceFound(service);

        if (!isAlreadyKnownAtOtherInterface) {
            printf("FOUND: %s %s %s\n", serviceName, regtype, replyDomain);
            std::cout << std::endl;

            ResolveContext resolveContext;
            resolveContext.bonsoirDiscovery = bonsoirDiscovery;
            resolveContext.serviceToResolve = service;

            DNSServiceRef ref;
            DNSServiceResolve(
                &ref,
                0,
                0,
                serviceName,
                regtype,
                replyDomain,
                DNSServiceResolveCallback,
                &resolveContext);

            DNSServiceProcessResult(ref);

        }
            }
    else
    {
        bonsoirDiscovery->onServiceLost(serviceNameStr);

        printf("LOST: %s %s %s\n", serviceName, regtype, replyDomain);
        std::cout << std::endl;
    }
}

void BonsoirDiscovery::discoverServices(std::string &serviceType)
{
    this->browse = std::make_unique<DNSServiceRef>();

    auto error = DNSServiceBrowse(
        this->browse.get(),
        0,
        0,
        serviceType.c_str(),
        NULL,
        DNSServiceBrowseCallback,
        this);

    if (error != kDNSServiceErr_NoError)
    {
        this->eventSink->Error("discoveryError", "Bonsoir failed to start discovery", error);
        return;
    }

    this->messagePumpThread = std::make_unique<std::thread>(&BonsoirDiscovery::messagePump, this);
    this->messagePumpThread->detach();

    this->eventSink->Success(EncodableMap{{EncodableValue("id"), EncodableValue("discoveryStarted")}});
}

void BonsoirDiscovery::stopDiscovery()
{
    DNSServiceRefDeallocate(*this->browse.release());

    this->browse = nullptr;
    this->eventSink->Success(EncodableMap{{EncodableValue("id"), EncodableValue("discoveryStopped")}});
}

bool BonsoirDiscovery::onServiceFound(Service &service)
{
    std::cout << "onServiceFound " << service.serviceName << std::endl;
    if (this->services.find(service.serviceName) != this->services.end())
    {
        return true;
    }

    this->services[service.serviceName] = service;

    this->eventSink->Success(
        EncodableMap{
            {EncodableValue("id"), EncodableValue("discoveryServiceFound")},
            {EncodableValue("service"), EncodableMap{
                                            {EncodableValue("service.name"), EncodableValue(service.serviceName)},
                                            {EncodableValue("service.type"), EncodableValue(service.serviceType)},
                                            {EncodableValue("service.port"), EncodableValue(service.port)},
                                            {EncodableValue("service.ip"), EncodableValue(service.serviceIp)},
                                            {EncodableValue("service.attributes"), EncodableMap{}},
                                        }}});

    return false;
}

void BonsoirDiscovery::onServiceResolved(Service &service)
{
    std::cout << "onServiceResolved " << service.serviceName << std::endl;
    this->services[service.serviceName] = service;

    this->eventSink->Success(
        EncodableMap{
            {EncodableValue("id"), EncodableValue("discoveryServiceResolved")},
            {EncodableValue("service"), EncodableMap{
                                            {EncodableValue("service.name"), EncodableValue(service.serviceName)},
                                            {EncodableValue("service.type"), EncodableValue(service.serviceType)},
                                            {EncodableValue("service.port"), EncodableValue(service.port)},
                                            {EncodableValue("service.ip"), EncodableValue(service.serviceIp)},
                                            {EncodableValue("service.attributes"), EncodableMap{}},
                                        }}});
}

void BonsoirDiscovery::onServiceLost(std::string &serviceName)
{
    std::cout << "onServiceLost " << serviceName << std::endl;
    if (this->services.find(serviceName) != this->services.end())
    {
        Service service = this->services[serviceName];

        this->eventSink->Success(
            EncodableMap{
                {EncodableValue("id"), EncodableValue("discoveryServiceLost")},
                {EncodableValue("service"), EncodableMap{
                                                {EncodableValue("service.name"), EncodableValue(service.serviceName)},
                                                {EncodableValue("service.type"), EncodableValue(service.serviceType)},
                                                {EncodableValue("service.port"), EncodableValue(service.port)},
                                                {EncodableValue("service.ip"), EncodableValue(service.serviceIp)},
                                                {EncodableValue("service.attributes"), EncodableMap{}},
                                            }}});

        this->services.erase(serviceName);
    }
}

void BonsoirDiscovery::messagePump()
{
    while (this->browse != nullptr)
    {
        DNSServiceProcessResult(*this->browse.get());
    }
}

Service::Service(std::string &serviceName, std::string &serviceType, std::string &serviceIp, int port)
    : serviceName(serviceName),
      serviceType(serviceType),
      serviceIp(serviceIp),
      port(port)
{
}
