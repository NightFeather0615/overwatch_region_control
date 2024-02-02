import "dart:io";


import "package:overwatch_region_control/core/extension.dart";

class Network {
  Network._();

  static final HttpClient _httpClient = HttpClient();

  static Future<String> getUrlContent(Uri url, {bool allowMalformed = true}) async {
    HttpClientResponse res = await (await _httpClient.getUrl(url)).close();
    return (await res.body()).trim();
  }

  static Future<int> ping(Uri url, {int round = 5}) async {
    List<int> buf = [];
    for (int _ in List.generate(round, (i) => i)) {
      HttpClientRequest req = await _httpClient.openUrl("OPTIONS", url);
      int start = DateTime.now().millisecondsSinceEpoch;
      await req.close();
      int end = DateTime.now().millisecondsSinceEpoch;
      buf.add(end - start);
    }
    return buf.average().round();
  }
}
