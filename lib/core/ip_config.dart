import "dart:convert";
import "dart:io";

import "package:flutter/services.dart";
import "package:hive_flutter/hive_flutter.dart";
import "package:overwatch_region_control/config.dart";
import "package:overwatch_region_control/core/firewall.dart";
import "package:overwatch_region_control/core/network.dart";
import "package:path_provider/path_provider.dart";

enum IpRegion {
  asia,
  middleEast,
  europe,
  northAmerica,
  southAndCentralAmerica,
  oceania,
  unknown;

  static IpRegion fromString(String value) {
    switch (value.toLowerCase()) {
      case "asia" || "as": return IpRegion.asia;
      case "middle_east" || "me": return IpRegion.middleEast;
      case "europe" || "eu": return IpRegion.europe;
      case "north_america" || "na": return IpRegion.northAmerica;
      case "south_and_central_america" || "sa": return IpRegion.southAndCentralAmerica;
      case "oceania" || "oc": return IpRegion.oceania;
      default: return IpRegion.unknown;
    }
  }

  @override
  String toString() {
    switch (this) {
      case IpRegion.asia: return "Asia";
      case IpRegion.middleEast: return "Middle East";
      case IpRegion.europe: return "Europe";
      case IpRegion.northAmerica: return "North America";
      case IpRegion.southAndCentralAmerica: return "South and Central America";
      case IpRegion.oceania: return "Oceania";
      case IpRegion.unknown: return "Unknown";
    }
  }
}

class IpData {
  final String name;
  final IpRegion region;
  final List<String> ipRanges;

  IpData({required this.name, required this.region, required this.ipRanges});
}

class IpConfig {
  IpConfig._();

  static final Box _sharedPref = Hive.box("sharedPref");
  static const JsonEncoder _jsonEncoder = JsonEncoder.withIndent("  ");

  static dynamic sourceData;
  static dynamic config;

  static Future<List<String>> downloadIpList(String url) async {
    String rawData = await Network.getUrlContent(Uri.parse(url));
    return rawData.split("\r").map((l) => l.trim()).where((l) => l.isNotEmpty).toList();
  }

  static Future<Directory> _getAppDocDir() async {
    Directory docDir = await getApplicationDocumentsDirectory();
    return await Directory("${docDir.path}\\OverwatchRegionControl").create();
  }

  static Future<void> updateSourceData() async {
    Directory appDir = await  _getAppDocDir();
    File dataFile = File("${appDir.path}\\regionSourceData.json");
    dynamic latestData = jsonDecode(await Network.getUrlContent(Uri.parse(Config.regionSourceDataUrl)));

    if (!await dataFile.exists()) {
      await dataFile.create();
      await dataFile.writeAsString("{}");
    }
    dynamic data = jsonDecode(await dataFile.readAsString());

    if (data["version"] == latestData["version"]) return;

    await dataFile.writeAsString(_jsonEncoder.convert(latestData));
  }

  static Future<void> loadSourceData() async {
    Directory docDir = await getApplicationDocumentsDirectory();
    Directory appDir = await Directory("${docDir.path}\\OverwatchRegionControl").create();
    File dataFile = File("${appDir.path}\\regionSourceData.json");
    sourceData = jsonDecode(await dataFile.readAsString());
  }

  static Future<void> updateRegionData() async {
    String latestVersion = await Network.getUrlContent(Uri.parse(sourceData["ip_source_verison_url"]));
    Directory appDir = await  _getAppDocDir();
    File configFile = File("${appDir.path}\\ipConfig.json");

    if (!await configFile.exists()) {
      await configFile.create();
      await configFile.writeAsString("{}");
    }

    dynamic data = jsonDecode(await configFile.readAsString());
    if (data["version"] == latestVersion) return;

    Map<String, dynamic> jsonBuf = {};

    jsonBuf["version"] = latestVersion;
    jsonBuf["data"] = [];

    AssetManifest assetManifest = await AssetManifest.loadFromAssetBundle(rootBundle);

    for (dynamic regionData in sourceData["regions"]) {
      regionData["ip_list"] = await downloadIpList(regionData["ip_source"]);
      regionData["firewall_rule_name"] = "Config_${regionData["name"]}_${regionData["region"]}".replaceAll(" ", "");
      if (assetManifest.getAssetVariants("assets/emoji/${regionData["emoji"]}.svg") == null) {
        regionData["emoji"] = "globe_with_meridians";
      }
      (jsonBuf["data"] as List<dynamic>).add(regionData);
    }

    (jsonBuf["data"] as List<dynamic>).sort(
      (a, b) => (a["name"] as String).toLowerCase().compareTo((b["name"] as String).toLowerCase())
    );
    (jsonBuf["data"] as List<dynamic>).sort(
      (a, b) => (a["region"] as String).toLowerCase().compareTo((b["region"] as String).toLowerCase())
    );

    await configFile.writeAsString(_jsonEncoder.convert(jsonBuf));
  }

  static Future<void> loadConfig() async {
    Directory docDir = await getApplicationDocumentsDirectory();
    Directory appDir = await Directory("${docDir.path}\\OverwatchRegionControl").create();
    File configFile = File("${appDir.path}\\ipConfig.json");
    config = jsonDecode(await configFile.readAsString());
  }

  static Future<void> initFirewall() async {
    for (dynamic data in config["data"]) {
      FirewallRule? inRule = await Firewall.getRule("${data["firewall_rule_name"]}_In");
      FirewallRule? outRule = await Firewall.getRule("${data["firewall_rule_name"]}_Out");
      if (inRule == null || outRule == null) {
        await Firewall.addRule(
          FirewallRule(
            name: data["firewall_rule_name"],
            description: "Block all connections to ${data["name"]}'s Overwatch servers",
            appName: _sharedPref.get("gameExePath"),
            action: FirewallAction.actionBlock,
            remoteAddresses: (data["ip_list"] as List<dynamic>).join(","),
            grouping: Config.appTitle
          )
        );
      }
    }
  }
}
