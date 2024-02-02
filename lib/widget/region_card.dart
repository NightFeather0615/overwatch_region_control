import "dart:async";

import "package:flutter/material.dart";
import "package:flutter_svg/svg.dart";
import "package:overwatch_region_control/core/firewall.dart";
import "package:overwatch_region_control/core/ip_config.dart";
import "package:overwatch_region_control/core/network.dart";

class RegionCard extends StatefulWidget {
  const RegionCard({super.key, required this.regionData});

  final Map<String, dynamic> regionData;

  @override
  State<StatefulWidget> createState() => _RegionCardState();
}

class _RegionCardState extends State<RegionCard> with AutomaticKeepAliveClientMixin {
  Timer? _pingTimer;
  String _latency = "---";
  bool _enabled = false;

  Future<void> _updateFirewallStatus() async {
    FirewallRule rule = (await Firewall.getRule("${widget.regionData["firewall_rule_name"]}_In"))!;
    if (mounted) setState(() => _enabled = !rule.enabled);
  }
  
  Future<void> _updateLatency(Timer? t) async {
    int latency;

    if (mounted) {
      setState(() {
        _latency = "---";
      });
    } else {
      t?.cancel();
      return;
    }

    try {
      latency = await Network.ping(Uri.parse(widget.regionData["ping_server"])).timeout(
        const Duration(seconds: 30)
      );
    } catch (_) {
      return;
    }

    if (mounted) {
      setState(() {
        _latency = latency.toString();
      });
    } else {
      t?.cancel();
      return;
    }
  }

  @override
  void initState() {
    super.initState();
    _updateFirewallStatus();
    _updateLatency(_pingTimer);
    _pingTimer = Timer.periodic(
      const Duration(minutes: 30),
      _updateLatency
    );
  }

  @override
  void dispose() {
    super.dispose();
    _pingTimer?.cancel();
  }

  @override
  Widget build(BuildContext context) {
    super.build(context);
    return InkWell(
      borderRadius: BorderRadius.circular(12),
      onTap: () {
        _updateLatency(_pingTimer);
      },
      highlightColor: Colors.transparent,
      splashFactory: NoSplash.splashFactory,
      child: Padding(
        padding: const EdgeInsets.symmetric(vertical: 10, horizontal: 16),
        child: Row(
          mainAxisAlignment: MainAxisAlignment.start,
          crossAxisAlignment: CrossAxisAlignment.center,
          children: [
            SvgPicture.asset(
              "assets/emoji/${widget.regionData["emoji"]}.svg",
              height: 42,
              semanticsLabel: widget.regionData["emoji"],
              placeholderBuilder: (context) => const CircularProgressIndicator(),
            ),
            const SizedBox(width: 12,),
            Column(
              mainAxisAlignment: MainAxisAlignment.center,
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Text(
                  widget.regionData["name"],
                  style: const TextStyle(
                    fontWeight: FontWeight.bold,
                    fontSize: 16
                  ),
                ),
                Text(
                  IpRegion.fromString(widget.regionData["region"]).toString(),
                  style: TextStyle(
                    fontSize: 13,
                    color: Colors.white.withAlpha(165)
                  ),
                ),
              ],
            ),
            const Spacer(),
            Text("$_latency ms"),
            const SizedBox(width: 10,),
            Checkbox(
              value: _enabled,
              onChanged: (value) async {
                await Firewall.toggleRule(
                  widget.regionData["firewall_rule_name"],
                  enabled: !(value ?? false)
                );
                setState(() {
                  _enabled = value ?? false;
                });
              },
            )
          ],
        ),
      ),
    );
  }
  
  @override
  bool get wantKeepAlive => true;
}
