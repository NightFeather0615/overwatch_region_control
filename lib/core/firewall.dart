import "package:flutter/services.dart";


enum FirewallProtocol {
  tcp,
  udp;

  static FirewallProtocol fromInt(int value) {
    switch(value) {
      case 6: return FirewallProtocol.tcp;
      case 17: return FirewallProtocol.udp;
      default: return FirewallProtocol.tcp;
    }
  }

  int toInt() {
    switch(this) {
      case FirewallProtocol.tcp: return 6;
      case FirewallProtocol.udp: return 17;
      default: return 6;
    }
  }
}

enum FirewallRuleDirection {
  directionIn,
  directionOut,
  directionMax;

  static FirewallRuleDirection fromInt(int value) {
    switch(value) {
      case 1: return FirewallRuleDirection.directionIn;
      case 2: return FirewallRuleDirection.directionOut;
      default: return FirewallRuleDirection.directionMax;
    }
  }

  int toInt() {
    switch(this) {
      case FirewallRuleDirection.directionIn: return 1;
      case FirewallRuleDirection.directionOut: return 2;
      default: return 3;
    }
  }
}

enum FirewallAction {
  actionBlock,
  actionAllow,
  actionMax;

  static FirewallAction fromInt(int value) {
    switch(value) {
      case 0: return FirewallAction.actionBlock;
      case 1: return FirewallAction.actionAllow;
      default: return FirewallAction.actionMax;
    }
  }

  int toInt() {
    switch(this) {
      case FirewallAction.actionBlock: return 0;
      case FirewallAction.actionAllow: return 1;
      default: return 2;
    }
  }
}

enum FirewallProfileType2 {
  profile2Domain,
  profile2Private,
  profile2Public,
  profile2All;

  static FirewallProfileType2 fromInt(int value) {
    switch(value) {
      case 0x1: return FirewallProfileType2.profile2Domain;
      case 0x2: return FirewallProfileType2.profile2Private;
      case 0x4: return FirewallProfileType2.profile2Public;
      case 0x7fffffff: return FirewallProfileType2.profile2All;
      default: return FirewallProfileType2.profile2All;
    }
  }

  int toInt() {
    switch(this) {
      case FirewallProfileType2.profile2Domain: return 0x1;
      case FirewallProfileType2.profile2Private: return 0x2;
      case FirewallProfileType2.profile2Public: return 0x4;
      case FirewallProfileType2.profile2All: return 0x7fffffff;
      default: return 0x7fffffff;
    }
  }
}

class FirewallRule {
  String name;
  String description;
  String appName;
  String? serviceName;
  FirewallProtocol? protocol;
  String icmpType;
  String localPorts;
  String remotePorts;
  String localAdresses;
  String remoteAddresses;
  FirewallProfileType2? profiles;
  FirewallRuleDirection direction;
  FirewallAction action;
  String interfaceTypes;
  bool enabled;
  String grouping;
  bool edgeTraversal;

  FirewallRule({
    required this.name,
    this.description = "",
    this.appName = "",
    this.serviceName,
    this.protocol,
    this.icmpType = "",
    this.localPorts = "",
    this.remotePorts = "",
    this.localAdresses = "",
    this.remoteAddresses = "",
    this.profiles,
    this.direction = FirewallRuleDirection.directionIn,
    this.action = FirewallAction.actionBlock,
    this.interfaceTypes = "",
    this.enabled = false,
    this.grouping = "",
    this.edgeTraversal = false,
  });

  static FirewallRule fromMap(Map<dynamic, dynamic> data) {
    if (!data.containsKey("name")) {
      throw Exception("Missing rule name");
    }

    FirewallRule result = FirewallRule(name: data["name"]);

    if (data.containsKey("description")) {
      result.description = data["description"];
    }
    if (data.containsKey("app_name")) {
      result.appName = data["app_name"];
    }
    if (data.containsKey("service_name")) {
      result.serviceName = data["service_name"];
    }
    if (data.containsKey("protocol")) {
      result.protocol = FirewallProtocol.fromInt(data["protocol"]);
    }
    if (data.containsKey("icmp_type")) {
      result.icmpType = data["icmp_type"];
    }
    if (data.containsKey("local_ports")) {
      result.localPorts = data["local_ports"];
    }
    if (data.containsKey("remote_ports")) {
      result.remotePorts = data["remote_ports"];
    }
    if (data.containsKey("local_adresses")) {
      result.localAdresses = data["local_adresses"];
    }
    if (data.containsKey("profiles")) {
      result.profiles = FirewallProfileType2.fromInt(data["profiles"]);
    }
    if (data.containsKey("direction")) {
      result.direction = FirewallRuleDirection.fromInt(data["direction"]);
    }
    if (data.containsKey("action")) {
      result.action = FirewallAction.fromInt(data["action"]);
    }
    if (data.containsKey("interface_types")) {
      result.interfaceTypes = data["interface_types"];
    }
    if (data.containsKey("enabled")) {
      result.enabled = data["enabled"];
    }
    if (data.containsKey("grouping")) {
      result.grouping = data["grouping"];
    }
    if (data.containsKey("edge_traversal")) {
      result.edgeTraversal = data["edge_traversal"];
    }

    return result;
  }

  Map<String, dynamic> toMap() {
    Map<String, dynamic> data = {
      "name": name,
      "description": description,
      "app_name": appName,
      "icmp_type": icmpType,
      "local_ports": localPorts,
      "remote_ports": remotePorts,
      "local_adresses": localAdresses,
      "remote_addresses": remoteAddresses,
      "direction": direction.toInt(),
      "action": action.toInt(),
      "interface_types": interfaceTypes,
      "enabled": enabled,
      "grouping": grouping,
      "edge_traversal": edgeTraversal
    };

    if (serviceName != null) {
      data["service_Name"] = serviceName;
    }
    if (profiles != null) {
      data["profiles"] = profiles!.toInt();
    }
    if (protocol != null) {
      data["protocol"] = protocol!.toInt();
    }

    return data;
  }
}

class Firewall {
  Firewall._();

  static const _native = MethodChannel("overwatch_region_control.nightfeather.dev/firewall");

  static Future<bool> isEnabled() async {
    return await _native.invokeMethod("isEnabled");
  }

  static Future<void> setEnabled({bool enabled = true}) async {
    await _native.invokeMethod("setEnabled", enabled);
  }

  static Future<void> addRule(FirewallRule rule, {bool bothDirection = true}) async {
    if (bothDirection) {
      String baseName = rule.name;
      
      rule.name = "${baseName}_In";
      rule.direction = FirewallRuleDirection.directionIn;
      await _native.invokeListMethod("addRule", rule.toMap());

      rule.name = "${baseName}_Out";
      rule.direction = FirewallRuleDirection.directionOut;
      await _native.invokeListMethod("addRule", rule.toMap());
    } else {
      await _native.invokeListMethod("addRule", rule.toMap());
    }
  }

  static Future<void> deleteRule(String name, {bool bothDirection = true}) async {
    if (bothDirection) {
      await _native.invokeListMethod("deleteRule", "${name}_In");
      await _native.invokeListMethod("deleteRule", "${name}_Out");
    } else {
      await _native.invokeListMethod("deleteRule", name);
    }
  }

  static Future<void> toggleRule(String name, {bool enabled = false, bool bothDirection = true}) async {
    if (bothDirection) {
      await _native.invokeListMethod("toggleRule", {"name": "${name}_In", "enabled": enabled});
      await _native.invokeListMethod("toggleRule", {"name": "${name}_Out", "enabled": enabled});
    } else {
      await _native.invokeListMethod("toggleRule", {"name": name, "enabled": enabled});
    }
  }

  static Future<FirewallRule?> getRule(String name) async {
    try {
      Map<dynamic, dynamic> rule = await _native.invokeMethod("getRule", name);
      return FirewallRule.fromMap(rule);
    } catch (_) {
      return null;
    }
  }

  static Future<List<FirewallRule>> getRules() async {
    List<Map<dynamic, dynamic>> rules = await _native.invokeListMethod("getRules") ?? [];
    return rules.map((data) => FirewallRule.fromMap(data)).toList();
  }
}
