import "package:flutter/services.dart";


class NativeUtils {
  static const native = MethodChannel("overwatch_region_control.nightfeather.dev/native_utils");

  static Future<bool> isUserAdmin() async {
    return await native.invokeMethod("isUserAdmin");
  }
}
