import "package:flutter/material.dart";

import "package:bitsdojo_window/bitsdojo_window.dart";
import "package:hive_flutter/hive_flutter.dart";

import "package:overwatch_region_control/config.dart";
import "package:overwatch_region_control/page/home.dart";
import "package:overwatch_region_control/page/settings.dart";
import "package:overwatch_region_control/page/setup.dart";


void main() async {
  WidgetsFlutterBinding.ensureInitialized();

  doWhenWindowReady(() {
    Size initialSize = const Size(610, 415);
    appWindow.minSize = initialSize;
    appWindow.size = initialSize;
    appWindow.title = Config.appTitle;
    appWindow.show();
  });
  
  await Hive.initFlutter();
  await Hive.openBox("sharedPref");

  runApp(const MainApp());
}

class MainApp extends StatelessWidget {
  const MainApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: Config.appTitle,
      theme: ThemeData(
        colorScheme: ColorScheme.fromSeed(
          seedColor: const Color.fromARGB(255, 0, 116, 224),
          brightness: Brightness.dark,
        ),
        useMaterial3: true,
        brightness: Brightness.dark
      ),
      debugShowCheckedModeBanner: false,
      home: const SetupPage(),
    );
  }
}

class MainPage extends StatefulWidget {
  const MainPage({super.key});

  @override
  State<StatefulWidget> createState() => _MainPageState();
}

class _MainPageState extends State<MainPage> {
  int _selectedIndex = 0;

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: Row(
        children: [
          NavigationRail(
            selectedIndex: _selectedIndex,
            labelType: NavigationRailLabelType.selected,
            groupAlignment: 0.0,
            onDestinationSelected: (value) {
              setState(() {
                _selectedIndex = value;
              });
            },
            destinations: const [
              NavigationRailDestination(
                icon: Icon(Icons.home_rounded),
                label: Text("Home")
              ),
              NavigationRailDestination(
                icon: Icon(Icons.settings_rounded),
                label: Text("Settings")
              ),
            ],
          ),
          Expanded(
            child: IndexedStack(
              index: _selectedIndex,
              children: const [
                HomePage(),
                SettingsPage(),
              ],
            )
          )
        ],
      ),
    );
  }
}
