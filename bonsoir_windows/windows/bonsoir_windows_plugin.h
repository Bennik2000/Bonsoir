#ifndef FLUTTER_PLUGIN_BONSOIR_WINDOWS_PLUGIN_H_
#define FLUTTER_PLUGIN_BONSOIR_WINDOWS_PLUGIN_H_

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>

#include <memory>
#include <dns_sd.h>
#include <thread>
#include "bonsoir_discovery.h"

namespace bonsoir_windows {

	class BonsoirWindowsPlugin : public flutter::Plugin {

	private:
		std::map<int, std::unique_ptr<DNSServiceRef>> broadcasts;
		std::map<int, std::shared_ptr<BonsoirDiscovery>> discovery;
		flutter::BinaryMessenger* messenger;
		
	public:
		static void RegisterWithRegistrar(flutter::PluginRegistrarWindows* registrar);

		BonsoirWindowsPlugin(flutter::BinaryMessenger *messenger): messenger(messenger) { }

		virtual ~BonsoirWindowsPlugin();

		// Disallow copy and assign.
		BonsoirWindowsPlugin(const BonsoirWindowsPlugin&) = delete;
		BonsoirWindowsPlugin& operator=(const BonsoirWindowsPlugin&) = delete;

	private:
		// Called when a method is called on this plugin's channel from Dart.
		void HandleMethodCall(
			const flutter::MethodCall<flutter::EncodableValue>& method_call,
			std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);

		std::string GetStringArgument(std::string key, const flutter::MethodCall<flutter::EncodableValue>& method_call);
		bool GetBooleanArgument(std::string key, const flutter::MethodCall<flutter::EncodableValue>& method_call);
		int GetInt32Argument(std::string key, const flutter::MethodCall<flutter::EncodableValue>& method_call);

		void BonjourMessatePump();
	};
}

#endif
