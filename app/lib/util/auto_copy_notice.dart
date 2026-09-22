import 'dart:async';

import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';

const _channel = MethodChannel('localsend/auto_copy_notice');

/// Shows the small native confirmation overlay after an automatic copy.
void showAutoCopyNotice() {
  if (defaultTargetPlatform != TargetPlatform.windows) {
    return;
  }

  unawaited(_showAutoCopyNotice());
}

Future<void> _showAutoCopyNotice() async {
  try {
    await _channel.invokeMethod<void>('show');
  } catch (error) {
    debugPrint('Could not show auto-copy notice: $error');
  }
}
