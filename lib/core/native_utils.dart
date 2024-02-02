import "package:flutter/services.dart";


class NativeUtils {
  NativeUtils._();

  static const _native = MethodChannel("overwatch_region_control.nightfeather.dev/native_utils");

  static Future<bool> isUserAdmin() async {
    return await _native.invokeMethod("isUserAdmin");
  }
}
