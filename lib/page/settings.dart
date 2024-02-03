import "dart:io";

import "package:flutter/material.dart";
import "package:hive_flutter/hive_flutter.dart";
import "package:overwatch_region_control/core/firewall.dart";
import "package:overwatch_region_control/core/ip_config.dart";
import "package:overwatch_region_control/core/utils.dart";
import "package:overwatch_region_control/page/setup.dart";


class SettingsPage extends StatefulWidget {
  const SettingsPage({super.key});

  @override
  State<StatefulWidget> createState() => _SettingsPageState();
}

class _SettingsPageState extends State<SettingsPage> {
  static final Box _sharedPref = Hive.box("sharedPref");
  
  final TextEditingController _gamePathController = TextEditingController(
    text: _sharedPref.get("gameExePath", defaultValue: "N/A")
  );

  @override
  void dispose() {
    super.dispose();
    _gamePathController.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: ListView(
        padding: const EdgeInsets.all(12),
        children: [
          Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              const Padding(
                padding: EdgeInsets.only(bottom: 6),
                child: Text(
                  "Game Path",
                  style: TextStyle(
                    fontWeight: FontWeight.bold,
                    fontSize: 20
                  ),
                ),
              ),
              Row(
                crossAxisAlignment: CrossAxisAlignment.center,
                children: [
                  IconButton.filled(
                    style: ButtonStyle(
                      shape: MaterialStatePropertyAll(
                        RoundedRectangleBorder(
                          borderRadius: BorderRadius.circular(12)
                        )
                      )
                    ),
                    onPressed: () async {
                      bool result = await Utils.updateGamePath();
                      if (result && mounted) {
                        _gamePathController.text = _sharedPref.get("gameExePath", defaultValue: "N/A");
                      }
                    },
                    icon: const Icon(Icons.edit_rounded),
                  ),
                  Expanded(
                    child: Padding(
                      padding: const EdgeInsets.only(left: 12),
                      child: TextField(
                        maxLines: 1,
                        controller: _gamePathController,
                        readOnly: true,
                      )
                    ),
                  )
                ],
              )
            ],
          ),
          const SizedBox(height: 30,),
          Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              const Padding(
                padding: EdgeInsets.only(bottom: 6),
                child: Text(
                  "Reset Config",
                  style: TextStyle(
                    fontWeight: FontWeight.bold,
                    fontSize: 20
                  ),
                ),
              ),
              Row(
                crossAxisAlignment: CrossAxisAlignment.center,
                children: [
                  ElevatedButton(
                    onPressed: () async {
                      bool? quitApp = await showDialog(
                        context: context,
                        builder: (context) {
                          return AlertDialog(
                            title: const Text("User Action Required"),
                            content: const Text("Choose action after process done"),
                            actions: [
                              OutlinedButton(
                                onPressed: () {
                                  Navigator.of(context).pop();
                                },
                                child: const Text("Cancel"),
                              ),
                              ElevatedButton(
                                onPressed: () {
                                  Navigator.of(context).pop(false);
                                },
                                child: const Text("Restart App"),
                              ),
                              ElevatedButton(
                                onPressed: () {
                                  Navigator.of(context).pop(true);
                                },
                                child: const Text("Quit"),
                              ),
                            ],
                          );
                        },
                      );

                      if (quitApp == null) return;

                      Directory appDir = await Utils.getAppDocDir();
                      for (FileSystemEntity file in await appDir.list().toList()){
                        await file.delete();
                      }
                      
                      if (quitApp) {
                        exit(0);
                      } else {
                        if (mounted) {
                          Navigator.of(context).pushReplacement(
                            MaterialPageRoute(
                              builder: (context) => const SetupPage(),
                            )
                          );
                        }
                      }
                    },
                    child: const Text("Reset Source Data & IP Config"),
                  ),
                  const SizedBox(width: 10,),
                  ElevatedButton(
                    onPressed: () async {
                      bool? quitApp = await showDialog(
                        context: context,
                        builder: (context) {
                          return AlertDialog(
                            title: const Text("User Action Required"),
                            content: const Text("Choose action after process done"),
                            actions: [
                              OutlinedButton(
                                onPressed: () {
                                  Navigator.of(context).pop();
                                },
                                child: const Text("Cancel"),
                              ),
                              ElevatedButton(
                                onPressed: () {
                                  Navigator.of(context).pop(false);
                                },
                                child: const Text("Restart App"),
                              ),
                              ElevatedButton(
                                onPressed: () {
                                  Navigator.of(context).pop(true);
                                },
                                child: const Text("Quit"),
                              ),
                            ],
                          );
                        },
                      );

                      if (quitApp == null) return;

                      for (dynamic data in IpConfig.config["data"]) {
                        await Firewall.deleteRule(data["firewall_rule_name"]);
                      }

                      if (quitApp) {
                        exit(0);
                      } else {
                        if (mounted) {
                          Navigator.of(context).pushReplacement(
                            MaterialPageRoute(
                              builder: (context) => const SetupPage(),
                            )
                          );
                        }
                      }
                    },
                    child: const Text("Reset Windows Firewall Rules"),
                  ),
                ],
              )
            ],
          ),
        ]
      ),
    );
  }
}
