import 'package:localsend_app/util/auto_copy_received_text.dart';
import 'package:test/test.dart';

void main() {
  test('copies accepted text from an authenticated favorite desktop sender', () async {
    String? copiedText;
    var noticeCount = 0;

    await copyReceivedTextIfAllowed(
      message: 'hello',
      accepted: true,
      enabled: true,
      isDesktop: true,
      isFavorite: true,
      isAuthenticated: true,
      writeClipboard: (text) async => copiedText = text,
      onCopied: () => noticeCount++,
      onClipboardError: (_, _) {},
    );

    expect(copiedText, 'hello');
    expect(noticeCount, 1);
  });

  test('eligible text is allowed to auto-accept without a Copy button interaction', () {
    expect(
      shouldAutoCopyReceivedText(
        message: 'hello',
        enabled: true,
        isDesktop: true,
        isFavorite: true,
        isAuthenticated: true,
      ),
      isTrue,
    );
  });

  test('does not copy declined text', () async {
    var writeCount = 0;

    await copyReceivedTextIfAllowed(
      message: 'hello',
      accepted: false,
      enabled: true,
      isDesktop: true,
      isFavorite: true,
      isAuthenticated: true,
      writeClipboard: (_) async => writeCount++,
      onClipboardError: (_, _) {},
    );

    expect(writeCount, 0);
  });

  test('does not copy text from a non-favorite sender', () async {
    var writeCount = 0;

    await copyReceivedTextIfAllowed(
      message: 'hello',
      accepted: true,
      enabled: true,
      isDesktop: true,
      isFavorite: false,
      isAuthenticated: true,
      writeClipboard: (_) async => writeCount++,
      onClipboardError: (_, _) {},
    );

    expect(writeCount, 0);
  });

  test('does not copy when disabled, non-desktop, non-text, or unverified', () async {
    for (final options in [
      (enabled: false, isDesktop: true, message: 'hello', isAuthenticated: true),
      (enabled: true, isDesktop: false, message: 'hello', isAuthenticated: true),
      (enabled: true, isDesktop: true, message: null, isAuthenticated: true),
      (enabled: true, isDesktop: true, message: 'hello', isAuthenticated: false),
    ]) {
      var writeCount = 0;

      await copyReceivedTextIfAllowed(
        message: options.message,
        accepted: true,
        enabled: options.enabled,
        isDesktop: options.isDesktop,
        isFavorite: true,
        isAuthenticated: options.isAuthenticated,
        writeClipboard: (_) async => writeCount++,
        onClipboardError: (_, _) {},
      );

      expect(writeCount, 0);
    }
  });

  test('swallows clipboard failures and reports them', () async {
    Object? error;
    StackTrace? stackTrace;

    await copyReceivedTextIfAllowed(
      message: 'hello',
      accepted: true,
      enabled: true,
      isDesktop: true,
      isFavorite: true,
      isAuthenticated: true,
      writeClipboard: (_) async => throw StateError('clipboard unavailable'),
      onClipboardError: (caughtError, caughtStackTrace) {
        error = caughtError;
        stackTrace = caughtStackTrace;
      },
    );

    expect(error, isA<StateError>());
    expect(stackTrace, isNotNull);
  });
}
