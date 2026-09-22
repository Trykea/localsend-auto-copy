typedef ClipboardWriter = Future<void> Function(String text);
typedef ClipboardErrorHandler = void Function(Object error, StackTrace stackTrace);

bool shouldAutoCopyReceivedText({
  required String? message,
  required bool enabled,
  required bool isDesktop,
  required bool isFavorite,
  required bool isAuthenticated,
}) => message != null && enabled && isDesktop && isFavorite && isAuthenticated;

Future<void> copyReceivedTextIfAllowed({
  required String? message,
  required bool accepted,
  required bool enabled,
  required bool isDesktop,
  required bool isFavorite,
  required bool isAuthenticated,
  required ClipboardWriter writeClipboard,
  required ClipboardErrorHandler onClipboardError,
}) async {
  if (!accepted ||
      !shouldAutoCopyReceivedText(
        message: message,
        enabled: enabled,
        isDesktop: isDesktop,
        isFavorite: isFavorite,
        isAuthenticated: isAuthenticated,
      )) {
    return;
  }

  try {
    await writeClipboard(message!);
  } catch (error, stackTrace) {
    onClipboardError(error, stackTrace);
  }
}
