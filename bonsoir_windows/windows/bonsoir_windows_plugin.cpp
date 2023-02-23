#include "bonsoir_windows_plugin.h"
#include <windows.h>

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>

#include <memory>
#include <sstream>
#include <stdio.h>
#include <memory>

#include <dns_sd.h>



namespace bonsoir_windows
{

	// static
	void BonsoirWindowsPlugin::RegisterWithRegistrar(
		flutter::PluginRegistrarWindows* registrar)
	{
		auto messenger = registrar->messenger();

		auto channel =
			std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
				messenger, "fr.skyost.bonsoir",
				&flutter::StandardMethodCodec::GetInstance());

		auto plugin = std::make_unique<BonsoirWindowsPlugin>(messenger);

		channel->SetMethodCallHandler(
			[plugin_pointer = plugin.get()](const auto& call, auto result)
		{
			plugin_pointer->HandleMethodCall(call, std::move(result));
		});

		registrar->AddPlugin(std::move(plugin));
	}

	BonsoirWindowsPlugin::~BonsoirWindowsPlugin() {}

	void BonsoirWindowsPlugin::HandleMethodCall(
		const flutter::MethodCall<flutter::EncodableValue>& method_call,
		std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result)
	{
		std::cout << "HandleMethodCall " << method_call.method_name() << std::endl;

		int id = this->GetInt32Argument("id", method_call);

		if (method_call.method_name().compare("broadcast.initialize") == 0)
		{
			// TODO: Setup BonsoirBroadcastListener
			result->Success(true);
		}
		else if (method_call.method_name().compare("broadcast.start") == 0)
		{
			int servicePort = this->GetInt32Argument("service.port", method_call);
			std::string serviceType = this->GetStringArgument("service.type", method_call);
			std::string serviceName = this->GetStringArgument("service.name", method_call);

			this->broadcasts[id] = std::make_unique<DNSServiceRef>();

			DNSServiceRegister(
				this->broadcasts[id].get(),
				0,
				0,
				serviceName.c_str(),
				serviceType.c_str(),
				NULL,
				NULL,
				(uint16_t)servicePort,
				0,
				NULL,
				NULL,
				NULL
			);

			result->Success(true);
		}
		else if (method_call.method_name().compare("broadcast.stop") == 0)
		{
			DNSServiceRefDeallocate(
				*this->broadcasts[id].release()
			);

			this->broadcasts.erase(id);

			result->Success(true);
		}
		else if (method_call.method_name().compare("discovery.initialize") == 0)
		{
			this->discovery[id] = std::make_shared<BonsoirDiscovery>(this->messenger, id);

			// TODO: Setup discovery listener
			// Event Channel: EventChannel(messenger, "${BonsoirPlugin.channel}.discovery.$id")
			// Get eventSink from eventChannel.setStreamHandler
			// Event: eventSink?.success(SuccessObject("discoveryStarted").toJson())
			// Event: eventSink?.error("discoveryError", "Bonsoir failed to start discovery", errorCode)
			// Event: eventSink?.success(SuccessObject("discoveryServiceFound", service).toJson(resolver.getResolvedServiceIpAddress(service)))
			// Event: eventSink?.success(SuccessObject("discoveryServiceLost", service).toJson(resolvedServiceInfo))
			// Event: eventSink?.success(SuccessObject("discoveryStopped").toJson())
			// Event: eventSink?.error("discoveryError", "Bonsoir has encountered an error while stopping the discovery", errorCode)
			// Event: eventSink?.success(SuccessObject("discoveryServiceResolved", service).toJson(resolver.getResolvedServiceIpAddress(service)))
			// Event: eventSink?.success(SuccessObject("discoveryServiceResolveFailed", service).toJson(resolver.getResolvedServiceIpAddress(service)))
			

			//bool printLogs = this->GetBooleanArgument("printLogs", method_call);
			result->Success(true);
		}
		else if (method_call.method_name().compare("discovery.start") == 0)
		{
			std::string serviceType = this->GetStringArgument("type", method_call);
			std::cout << "serviceType: " << serviceType << std::endl;
			
			discovery[id]->discoverServices(serviceType);

			result->Success(true);
		}
		else if (method_call.method_name().compare("discovery.stop") == 0)
		{
			discovery[id]->stopDiscovery();
			discovery.erase(id);

			result->Success(true);
		}
		else
		{
			std::cout << "not implemented: " << method_call.method_name() << std::endl;
			result->NotImplemented();
		}
	}

	std::string BonsoirWindowsPlugin::GetStringArgument(std::string key, const flutter::MethodCall<flutter::EncodableValue>& method_call)
	{
		const auto* arguments = std::get_if<flutter::EncodableMap>(method_call.arguments());

		if (!arguments) return std::string();

		auto value_entry = arguments->find(flutter::EncodableValue(key));
		if (value_entry == arguments->end()) return std::string();

		return std::get<std::string>(value_entry->second);
	}

	bool BonsoirWindowsPlugin::GetBooleanArgument(std::string key, const flutter::MethodCall<flutter::EncodableValue>& method_call)
	{
		const auto* arguments = std::get_if<flutter::EncodableMap>(method_call.arguments());

		if (!arguments) return false;

		auto value_entry = arguments->find(flutter::EncodableValue(key));
		if (value_entry == arguments->end()) return nullptr;

		return std::get<bool>(value_entry->second);
	}

	int BonsoirWindowsPlugin::GetInt32Argument(std::string key, const flutter::MethodCall<flutter::EncodableValue>& method_call)
	{
		const auto* arguments = std::get_if<flutter::EncodableMap>(method_call.arguments());

		if (!arguments) return false;

		auto value_entry = arguments->find(flutter::EncodableValue(key));
		if (value_entry == arguments->end()) return 0;

		return std::get<int>(value_entry->second);
	}

}
