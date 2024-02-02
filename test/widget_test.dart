import "package:overwatch_region_control/core/network.dart";

void main() async {
  await Network.ping(Uri.parse("https://hk-ed.metercdn.net"));
}
