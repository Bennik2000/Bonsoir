
import 'bonsoir_windows_platform_interface.dart';

class BonsoirWindows {
  Future<String?> getPlatformVersion() {
    return BonsoirWindowsPlatform.instance.getPlatformVersion();
  }
}
