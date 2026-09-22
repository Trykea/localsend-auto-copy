import 'dart:async';

import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';

const _channel = MethodChannel('localsend/auto_copy_notice');

/// Shows the small native confirmation overlay after an automatic copy.
void showAutoCopyNotice({String message = 'Text copied'}) {
  if (defaultTargetPlatform != TargetPlatform.windows) {
    return;
  }

  unawaited(_showAutoCopyNotice(message));
}

Future<void> _showAutoCopyNotice(String message) async {
  try {
    await _channel.invokeMethod<void>('show', message);
  } catch (error) {
    debugPrint('Could not show auto-copy notice: $error');
  }
}
