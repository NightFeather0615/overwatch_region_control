import "dart:io";

import "package:file_picker/file_picker.dart";
import "package:hive_flutter/hive_flutter.dart";
import "package:path_provider/path_provider.dart";

class Utils {
  Utils._();

  static final Box _sharedPref = Hive.box("sharedPref");
  static final RegExp _overwatchGamePathRegex = RegExp(r"(Overwatch\\_retail_\\Overwatch\.exe)|(steamapps\\common\\Overwatch\\Overwatch\.exe)");

  static Future<bool> updateGamePath() async {
    FilePickerResult? result = await FilePicker.platform.pickFiles(
      dialogTitle: "Choose Overwatch game path",
      initialDirectory: "C:\\",
      type: FileType.custom,
      allowedExtensions: ["exe"],
      lockParentWindow: true
    );
    if (result == null || !result.isSinglePick) return false;
    if (_overwatchGamePathRegex.hasMatch(result.files.first.path ?? "")) {
      _sharedPref.put("gameExePath", result.files.first.path);
      return true;
    }

    return false;
  }

  static Future<Directory> getAppDocDir() async {
    Directory docDir = await getApplicationDocumentsDirectory();
    return await Directory("${docDir.path}\\OverwatchRegionControl").create();
  }
}