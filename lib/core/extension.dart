import "dart:convert";
import "dart:io";

extension IntListUtil on List<int> {
  int sum() {
    if (isEmpty) return 0;
    return reduce((a, b) => a + b);
  }

  double average() {
    if (isEmpty) return 0;
    return sum() / length;
  }
}

extension HttpClientResponseUtil on HttpClientResponse {
  Future<String> body({bool allowMalformed = true}) async {
    return utf8.decode(
      await expand((e) => e).toList(),
      allowMalformed: allowMalformed
    );
  }
}
