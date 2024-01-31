import "package:flutter/material.dart";
import "package:flutter/services.dart";

void main() {
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: "Flutter Demo",
      theme: ThemeData(
        colorScheme: ColorScheme.fromSeed(seedColor: Colors.deepPurple),
        useMaterial3: true,
      ),
      home: const MyHomePage(title: "INetFwPolicy2"),
    );
  }
}

class MyHomePage extends StatefulWidget {
  const MyHomePage({super.key, required this.title});

  final String title;

  @override
  State<MyHomePage> createState() => _MyHomePageState();
}

class _MyHomePageState extends State<MyHomePage> {
  static const platform = MethodChannel("overwatch_region_control.nightfeather.dev/firewall");

  List<String> _rules = [];

  Future<void> _netFwTest() async {
    List<String> rules;
    try {
      List<Object?> status = await platform.invokeMethod<List<Object?>>("getFirewallRules") ?? [];
      if (status.isEmpty) {
        return;
      } else {
        rules = status.map((e) {
          return (e as Map<Object?, Object?>)["name"] as String;
        }).toList();
      }
    } on PlatformException catch (_) {
      return;
    }

    setState(() {
      _rules = rules;
    });
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        backgroundColor: Theme.of(context).colorScheme.inversePrimary,
        title: Text(widget.title),
      ),
      body: Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.spaceEvenly,
          children: [
            ElevatedButton(
              onPressed: _netFwTest,
              child: const Text("Get Net Fw Rules"),
            ),
            Expanded(
              child: ListView(
                children: _rules.map((e) {
                  return Text(e);
                }).toList(),
              ),
            )
          ],
        ),
      ),
    );
  }
}
