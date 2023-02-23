#include "bonsoir_broadcast.h"

using namespace flutter;

void DNSServiceRegisterCallback(
    DNSServiceRef sdRef,
    DNSServiceFlags flags,
    DNSServiceErrorType errorCode,
    const char *name,
    const char *regtype,
    const char *domain,
    void *context)
{
    auto bonsoirBroadcast = (BonsoirBroadcast *)context;

    if (errorCode != kDNSServiceErr_NoError)
    {
        bonsoirBroadcast->onBroadcastStartFailed(errorCode);
    }
    else
    {
        bonsoirBroadcast->onBroadcastStarted();
    }
}

BonsoirBroadcast::BonsoirBroadcast(BinaryMessenger *messenger, int id)
{
    eventChannel = std::make_shared<EventChannel<EncodableValue>>(
        messenger,
        "fr.skyost.bonsoir.broadcast." + std::to_string(id),
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

void BonsoirBroadcast::startBroadcast(Service &serviceToBroadcast)
{
    this->service = serviceToBroadcast;
    this->broadcast = std::make_unique<DNSServiceRef>();

    auto error = DNSServiceRegister(
        this->broadcast.get(),
        0,
        0,
        service.serviceName.c_str(),
        service.serviceType.c_str(),
        NULL,
        NULL,
        (uint16_t)service.port,
        0,
        NULL,
        DNSServiceRegisterCallback, // TODO: Callback
        this);

    if (error != kDNSServiceErr_NoError && eventSink != nullptr)
    {
        this->eventSink->Error("discoveryError", "Bonsoir failed to start discovery", error);
    }
}

void BonsoirBroadcast::stopBroadcast()
{
    DNSServiceRefDeallocate(*this->broadcast.release());
    this->broadcast = nullptr;

    if (eventSink != nullptr)
    {
        this->eventSink->Success(EncodableMap{{EncodableValue("id"), EncodableValue("broadcastStopped")}});
    }
}

void BonsoirBroadcast::onBroadcastStarted()
{
    if (eventSink != nullptr)
    {
        this->eventSink->Success(
            EncodableMap{
                {EncodableValue("id"), EncodableValue("broadcastStarted")},
                {EncodableValue("service"), EncodableMap{
                                                {EncodableValue("service.name"), EncodableValue(service.serviceName)},
                                                {EncodableValue("service.type"), EncodableValue(service.serviceType)},
                                                {EncodableValue("service.port"), EncodableValue(service.port)},
                                                {EncodableValue("service.ip"), EncodableValue(service.serviceIp)},
                                                {EncodableValue("service.attributes"), EncodableMap{}},
                                            }}});
    }
}

void BonsoirBroadcast::onBroadcastStartFailed(DNSServiceErrorType error)
{
    if (eventSink != nullptr)
    {
        this->eventSink->Error("broadcastError", "Bonsoir service registration failed.", error);
    }
}
