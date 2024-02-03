import "dart:io";

import "package:flutter/material.dart";
import "package:hive_flutter/hive_flutter.dart";
import "package:overwatch_region_control/core/firewall.dart";
import "package:overwatch_region_control/core/ip_config.dart";
import "package:overwatch_region_control/core/native_utils.dart";
import "package:overwatch_region_control/core/utils.dart";
import "package:overwatch_region_control/main.dart";


class ProgressBar extends StatefulWidget {
  const ProgressBar({
    super.key,
    required this.length,
    required this.progress
  });

  final int length;
  final int progress;
  
  @override
  State<StatefulWidget> createState() => _ProgressBarState();
}

class _ProgressBarState extends State<ProgressBar> {
  static const _progressBarThumbSize = 2.438;

  @override
  Widget build(BuildContext context) {
    return SliderTheme(
      data: SliderThemeData(
        overlayShape: SliderComponentShape.noOverlay,
        thumbShape: const RoundSliderThumbShape(
          enabledThumbRadius: _progressBarThumbSize,
          disabledThumbRadius: _progressBarThumbSize,
          elevation: 0
        ),
        trackHeight: 3,
        trackShape: const RoundedRectSliderTrackShape(),
      ),
      child: Slider(
        value: widget.progress.toDouble(),
        max: widget.length.toDouble(),
        min: 0,
        divisions: widget.length,
        onChanged: (_) {},
      ),
    );
  }
}


class SetupPage extends StatefulWidget {
  const SetupPage({super.key});

  @override
  State<SetupPage> createState() => _SetupPageState();
}

class _SetupPageState extends State<SetupPage> {
  final Box _sharedPref = Hive.box("sharedPref");

  final List<String> _steps = [
    "Checking permission",
    "Checking firewall status",
    "Checking Overwatch game path",
    "Checking update for source data",
    "Loading source data",
    "Checking update for IP config",
    "Loading IP config",
    "Initializing firewall",
  ];
  int _stepIndex = 0;

  void _progressBarNext() {
    if (mounted) {
      setState(() {
        _stepIndex += 1;
      });
    }
  }

  Future<void> _startupCheck() async {
    if (!await NativeUtils.isUserAdmin()) {
      if (mounted) {
        await Navigator.of(context).push(
          MaterialPageRoute(
            builder: (context) => RequestActionPage(
              title: "This program is not running as an administrator",
              description: "Editing Windows Firewall rules requires administrator privileges.",
              action: () => exit(0),
              buttonText: "Exit",
            )
          )
        );
      }
    }
    _progressBarNext();

    if (!await Firewall.isEnabled()) {
      if (mounted) {
        await Navigator.of(context).push(
          MaterialPageRoute(
            builder: (context) => RequestActionPage(
              title: "Windows Firewall not enabled",
              description: "Firewall needs to be enabled to block specific connections.",
              action: () async {
                await Firewall.setEnabled();
                if (await Firewall.isEnabled() && mounted) Navigator.of(context).pop();
              },
              buttonText: "Enable Windows Firewall",
            )
          )
        );
      }
    }
    _progressBarNext();

    if (_sharedPref.get("gameExePath") == null || (_sharedPref.get("gameExePath") as String).isEmpty) {
      if (mounted) {
        await Navigator.of(context).push(
          MaterialPageRoute(
            builder: (context) => RequestActionPage(
              title: "Overwatch game path not assigned",
              description: "The game path needs to be specified to set up firewall rules.\nGame path should end with \"Overwatch\\_retail_\\Overwatch.exe\" on Battle.net\nor \"steamapps\\common\\Overwatch\\Overwatch.exe\" on Steam.",
              action: () async {
                bool result = await Utils.updateGamePath();
                if (result && mounted) Navigator.of(context).pop();
              },
              buttonText: "Choose Path",
            )
          )
        );
      }
    }
    _progressBarNext();

    await IpConfig.updateSourceData();
    _progressBarNext();

    await IpConfig.loadSourceData();
    _progressBarNext();

    await IpConfig.updateRegionData();
    _progressBarNext();

    await IpConfig.loadConfig();
    _progressBarNext();

    await IpConfig.initFirewall();
    _progressBarNext();

    if (mounted) {
      Navigator.of(context).pushReplacement(
        MaterialPageRoute(builder: (context) => const MainPage())
      );
    }
  }

  @override
  void initState() {
    super.initState();
    _startupCheck();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: [
            const Spacer(),
            const Text(
              "Running Startup Check",
              style: TextStyle(
                fontSize: 30,
                fontWeight: FontWeight.bold
              ),
            ),
            if (_stepIndex < _steps.length) Text(
              "${_steps[_stepIndex]}...",
              style: const TextStyle(
                fontSize: 20
              ),
            ) else const Text(
              "Finishing up...",
              style: TextStyle(
                fontSize: 20
              ),
            ),
            const Spacer(),
            SizedBox(
              height: MediaQuery.of(context).size.height / 8,
              width: MediaQuery.of(context).size.width / 1.2,
              child: ProgressBar(length: _steps.length, progress: _stepIndex),
            )
          ],
        ),
      ),
    );
  }
}


class RequestActionPage extends StatelessWidget {
  const RequestActionPage({super.key, required this.title, required this.description, this.action, required this.buttonText});

  final String title;
  final String description;
  final void Function()? action;
  final String buttonText;

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: [
            const Spacer(flex: 20,),
            Text(
              title,
              style: const TextStyle(
                fontSize: 28,
                fontWeight: FontWeight.bold
              ),
            ),
            Text(
              description,
              textAlign: TextAlign.center,
              style: const TextStyle(
                fontSize: 16
              ),
            ),
            const Spacer(flex: 2,),
            OutlinedButton(
              onPressed: action,
              child: Padding(
                padding: const EdgeInsets.symmetric(horizontal: 0, vertical: 10),
                child: Text(
                  buttonText,
                  style: const TextStyle(
                    fontSize: 18
                  ),
                ),
              ),
            ),
            const Spacer(flex: 20,)
          ],
        ),
      ),
    );
  }
}
