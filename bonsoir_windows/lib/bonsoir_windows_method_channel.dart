import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';

import 'bonsoir_windows_platform_interface.dart';

/// An implementation of [BonsoirWindowsPlatform] that uses method channels.
class MethodChannelBonsoirWindows extends BonsoirWindowsPlatform {
  /// The method channel used to interact with the native platform.
  @visibleForTesting
  final methodChannel = const MethodChannel('bonsoir_windows');

  @override
  Future<String?> getPlatformVersion() async {
    final version = await methodChannel.invokeMethod<String>('getPlatformVersion');
    return version;
  }
}
