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

/**
 * Callback function called by the Bonjour dns service when a service was resolved.
 *
 * Expects a pointer to ResolveContext as context;
 */
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
	void *context);

/**
 * Callback function called by the Bonjour dns service when a service was browsed.
 * At this stage the IP and port of the service is not known.
 *
 * Expects a pointer to BonsoirDiscovery as context;
 */
void DNSServiceBrowseCallback(
	DNSServiceRef sdRef,
	DNSServiceFlags flags,
	uint32_t interfaceIndex,
	DNSServiceErrorType errorCode,
	const char *serviceName,
	const char *regtype,
	const char *replyDomain,
	void *context);


/**
 * Handler class which manages the mDNS discovery and translates between flutter
 * and dns_sd.h
*/
class BonsoirDiscovery
{
private:
	DNSServiceRef browse;
	std::atomic<int> isBrowsing = 0;
	std::thread messagePumpThread;
	std::shared_ptr<flutter::EventSink<flutter::EncodableValue>> eventSink;
	std::shared_ptr<flutter::EventChannel<flutter::EncodableValue>> eventChannel;

	std::map<std::string, Service> services;

public:
	BonsoirDiscovery(flutter::BinaryMessenger *messenger, int id);

	void discoverServices(std::string &type);
	void stopDiscovery();

	bool onServiceFound(Service &service);
	void onServiceResolved(Service &service);
	void onServiceLost(std::string &serviceName);

private:
	void messagePump();
};


struct ResolveContext
{
public:
	BonsoirDiscovery *bonsoirDiscovery;
	Service serviceToResolve;
};
