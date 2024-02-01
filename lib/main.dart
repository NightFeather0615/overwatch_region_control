import "package:flutter/material.dart";
import "package:hive_flutter/hive_flutter.dart";
import "package:overwatch_region_control/page/setup.dart";


void main() async {
  WidgetsFlutterBinding.ensureInitialized();

  await Hive.initFlutter();
  await Hive.openBox("sharedPref");

  runApp(const MainApp());
}

class MainApp extends StatelessWidget {
  const MainApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: "Overwatch Region Control",
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
