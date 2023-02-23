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
    if (errorCode != kDNSServiceErr_NoError)
    {
        // TODO: Handle error
        return;
    }

    auto ip = gethostbyname(hosttarget);
    const char *ipAddress = inet_ntoa(*(in_addr *)ip->h_addr_list[0]);

    auto resolveContext = (ResolveContext *)context;

    Service service = resolveContext->serviceToResolve;
    service.port = port;
    service.serviceIp = std::string(ipAddress);

    resolveContext->bonsoirDiscovery->onServiceResolved(service);
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

        if (!isAlreadyKnownAtOtherInterface)
        {
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
    }
}

BonsoirDiscovery::BonsoirDiscovery(BinaryMessenger *messenger, int id)
{
    eventChannel = std::make_shared<EventChannel<EncodableValue>>(
        messenger,
        "fr.skyost.bonsoir.discovery." + std::to_string(id),
        &StandardMethodCodec::GetInstance());

    eventChannel->SetStreamHandler(
        std::make_unique<StreamHandlerFunctions<EncodableValue>>(
            [this](const EncodableValue *arguments,
                   std::unique_ptr<EventSink<EncodableValue>> &&events)
                -> std::unique_ptr<StreamHandlerError<EncodableValue>>
            {
                this->eventSink = std::move(events);
                return nullptr;
            },
            [](const EncodableValue *arguments)
                -> std::unique_ptr<StreamHandlerError<EncodableValue>>
            {
                return nullptr;
            }));
}

void BonsoirDiscovery::discoverServices(std::string &serviceType)
{
    this->browse = DNSServiceRef();

    auto error = DNSServiceBrowse(
        &this->browse,
        0,
        0,
        serviceType.c_str(),
        NULL,
        DNSServiceBrowseCallback,
        this);

    if (error != kDNSServiceErr_NoError)
    {

        if (eventSink != nullptr)
        {
            this->eventSink->Error("discoveryError", "Bonsoir failed to start discovery", error);
        }
        return;
    }

    isBrowsing.store(1, std::memory_order_release);

    this->messagePumpThread = std::thread(&BonsoirDiscovery::messagePump, this);
    
    if (eventSink != nullptr)
    {
        this->eventSink->Success(EncodableMap{{EncodableValue("id"), EncodableValue("discoveryStarted")}});
    }
}

void BonsoirDiscovery::stopDiscovery()
{
    isBrowsing.store(0, std::memory_order_release);
    
    DNSServiceRefDeallocate(this->browse);

    messagePumpThread.join();

    if (eventSink != nullptr)
    {
        this->eventSink->Success(EncodableMap{{EncodableValue("id"), EncodableValue("discoveryStopped")}});
    }
}

bool BonsoirDiscovery::onServiceFound(Service &service)
{
    if (this->services.find(service.serviceName) != this->services.end())
    {
        return true;
    }

    this->services[service.serviceName] = service;

    if (eventSink != nullptr)
    {
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
    }

    return false;
}

void BonsoirDiscovery::onServiceResolved(Service &service)
{
    this->services[service.serviceName] = service;

    if (eventSink != nullptr)
    {
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
}

void BonsoirDiscovery::onServiceLost(std::string &serviceName)
{
    if (this->services.find(serviceName) != this->services.end())
    {
        Service service = this->services[serviceName];

        if (eventSink != nullptr)
        {
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
        }

        this->services.erase(serviceName);
    }
}

void BonsoirDiscovery::messagePump()
{
    while (isBrowsing.load(std::memory_order_acquire) != 0)
    {
        DNSServiceProcessResult(this->browse);
    }
}
