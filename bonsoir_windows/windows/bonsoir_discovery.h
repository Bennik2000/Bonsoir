#pragma once

#include <flutter/method_channel.h>
#include <flutter/event_channel.h>
#include <flutter/event_stream_handler_functions.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>
#include <memory>
#include <dns_sd.h>
#include <thread>

struct Service
{
public:
	std::string serviceName;
	std::string serviceType;
	std::string serviceIp;
	int port = 0;

	Service() {};
	Service(std::string& serviceName,
		std::string& serviceType,
		std::string& serviceIp,
		int port);
};

class BonsoirDiscovery
{
private:
	std::unique_ptr<DNSServiceRef> browse;
	std::unique_ptr<std::thread> messagePumpThread;
	std::shared_ptr<flutter::EventSink<flutter::EncodableValue>> eventSink;
	std::shared_ptr<flutter::EventChannel<flutter::EncodableValue>> eventChannel;

	std::map<std::string, Service> services;

public:
	BonsoirDiscovery(flutter::BinaryMessenger* messenger, int id)
	{
		eventChannel = std::make_shared<flutter::EventChannel<flutter::EncodableValue>>(
			messenger,
			"fr.skyost.bonsoir.discovery." + std::to_string(id),
			&flutter::StandardMethodCodec::GetInstance());

		eventChannel->SetStreamHandler(
			std::make_unique<flutter::StreamHandlerFunctions<flutter::EncodableValue>>(
				[this](const flutter::EncodableValue* arguments,
					std::unique_ptr<flutter::EventSink<flutter::EncodableValue>>&& events)
				-> std::unique_ptr<flutter::StreamHandlerError<flutter::EncodableValue>>
				{
					this->eventSink = std::move(events);
					return nullptr;
				},
				[](const flutter::EncodableValue* arguments)
					-> std::unique_ptr<flutter::StreamHandlerError<flutter::EncodableValue>>
				{
					return nullptr;
				}));
	};

	void discoverServices(std::string& type);
	void stopDiscovery();

	bool onServiceFound(Service& service);
	void onServiceResolved(Service& service);
	void onServiceLost(std::string& serviceName);

private:
	void messagePump();
};


struct ResolveContext {
public:
	BonsoirDiscovery* bonsoirDiscovery;
	Service serviceToResolve;
};
