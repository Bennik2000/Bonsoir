#include <flutter/method_channel.h>
#include <flutter/event_channel.h>
#include <flutter/event_stream_handler_functions.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>
#include <memory>
#include <dns_sd.h>
#include <thread>

class BonsoirDiscovery
{
private:
    std::unique_ptr<DNSServiceRef> browse;
    std::unique_ptr<std::thread> messagePumpThread;
    std::shared_ptr<flutter::EventSink<flutter::EncodableValue>> eventSink;
    std::shared_ptr<flutter::EventChannel<flutter::EncodableValue>> eventChannel;

public:
    BonsoirDiscovery(flutter::BinaryMessenger *messenger, int id)
    {
        eventChannel = std::make_shared<flutter::EventChannel<flutter::EncodableValue>>(
            messenger,
            "fr.skyost.bonsoir.discovery." + std::to_string(id),
            &flutter::StandardMethodCodec::GetInstance());

        eventChannel->SetStreamHandler(
            std::make_unique<flutter::StreamHandlerFunctions<flutter::EncodableValue>>(
                [this](const flutter::EncodableValue *arguments,
                       std::unique_ptr<flutter::EventSink<flutter::EncodableValue>> &&events)
                    -> std::unique_ptr<flutter::StreamHandlerError<flutter::EncodableValue>>
                {
                    this->eventSink = std::move(events);
                    return nullptr;
                },
                [](const flutter::EncodableValue *arguments)
                    -> std::unique_ptr<flutter::StreamHandlerError<flutter::EncodableValue>>
                {
                    return nullptr;
                }));
    };

    void discoverServices(std::string &type);
    void stopDiscovery();

    void onServiceFound(std::string &serviceName,
                        std::string &serviceType,
                        std::string &serviceIp,
                        int port);

    void onServiceResolved(std::string &serviceName,
                           std::string &serviceType,
                           std::string &serviceIp,
                           int port);

    void onServiceLost(std::string &serviceName,
                       std::string &serviceType,
                       std::string &serviceIp,
                       int port);

private:
    void messagePump();
};
