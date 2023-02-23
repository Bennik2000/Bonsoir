import 'package:plugin_platform_interface/plugin_platform_interface.dart';

import 'bonsoir_windows_method_channel.dart';

abstract class BonsoirWindowsPlatform extends PlatformInterface {
  /// Constructs a BonsoirWindowsPlatform.
  BonsoirWindowsPlatform() : super(token: _token);

  static final Object _token = Object();

  static BonsoirWindowsPlatform _instance = MethodChannelBonsoirWindows();

  /// The default instance of [BonsoirWindowsPlatform] to use.
  ///
  /// Defaults to [MethodChannelBonsoirWindows].
  static BonsoirWindowsPlatform get instance => _instance;

  /// Platform-specific implementations should set this with their own
  /// platform-specific class that extends [BonsoirWindowsPlatform] when
  /// they register themselves.
  static set instance(BonsoirWindowsPlatform instance) {
    PlatformInterface.verifyToken(instance, _token);
    _instance = instance;
  }

  Future<String?> getPlatformVersion() {
    throw UnimplementedError('platformVersion() has not been implemented.');
  }
}
