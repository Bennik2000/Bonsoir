#pragma once

#include <flutter/method_channel.h>
#include <flutter/event_channel.h>
#include <flutter/event_stream_handler_functions.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>
#include <memory>
#include <dns_sd.h>
#include <thread>
#include "service.h"

void DNSServiceRegisterCallback(
    DNSServiceRef sdRef,
    DNSServiceFlags flags,
    DNSServiceErrorType errorCode,
    const char *name,
    const char *regtype,
    const char *domain,
    void *context);

/**
 * Handler class which manages the mDNS broadcast and translates between flutter
 * and dns_sd.h
 */
class BonsoirBroadcast
{
private:
    DNSServiceRef broadcast;
    std::shared_ptr<flutter::EventSink<flutter::EncodableValue>> eventSink;
    std::shared_ptr<flutter::EventChannel<flutter::EncodableValue>> eventChannel;

    Service service;

public:
    BonsoirBroadcast(flutter::BinaryMessenger *messenger, int id);

    void startBroadcast(Service &serviceToBroadcast);
    void stopBroadcast();

    void onBroadcastStarted();
    void onBroadcastStartFailed(DNSServiceErrorType error);
};
