import "package:flutter/material.dart";
import "package:overwatch_region_control/core/firewall.dart";
import "package:overwatch_region_control/core/ip_config.dart";
import "package:overwatch_region_control/widget/region_card.dart";


class HomePage extends StatefulWidget {
  const HomePage({super.key});

  @override
  State<StatefulWidget> createState() => _HomePageState();
}

class _HomePageState extends State<HomePage> {
  UniqueKey _listKey = UniqueKey();

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      floatingActionButton: Row(
        mainAxisAlignment: MainAxisAlignment.center,
        children: [
          const Spacer(flex: 6,),
          ElevatedButton(
            onPressed: () async {
              for (dynamic data in IpConfig.config["data"]) {
                await Firewall.toggleRule(data["firewall_rule_name"], enabled: false);
              }
              setState(() => _listKey = UniqueKey());
            },
            child: const Text("Enable All"),
          ),
          const Spacer(),
          ElevatedButton(
            onPressed: () async {
              for (dynamic data in IpConfig.config["data"]) {
                await Firewall.toggleRule(data["firewall_rule_name"], enabled: true);
              }
              setState(() => _listKey = UniqueKey());
            },
            child: const Text("Disable All"),
          ),
          const Spacer(flex: 6,)
        ],
      ),
      body: ListView.builder(
        key: _listKey,
        padding: const EdgeInsets.only(top: 6, bottom: 56),
        itemCount: IpConfig.config["data"].length,
        itemBuilder: (context, index) {
          return Padding(
            padding: const EdgeInsets.symmetric(horizontal: 12, vertical: 6),
            child: RegionCard(regionData: IpConfig.config["data"][index]),
          );
        },
      )
    );
  }
}
