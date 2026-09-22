import 'package:localsend_isolates/model/dto/file_dto.dart';
import 'package:localsend_isolates/model/file_type.dart';
import 'package:pasteboard/pasteboard.dart';

const maxAutoCopyImageBytes = 50 * 1024 * 1024;
const maxAutoCopyFileBytes = 250 * 1024 * 1024;
const maxAutoCopyTotalBytes = 500 * 1024 * 1024;

bool shouldAutoCopyReceivedMedia({
  required Iterable<FileDto> files,
  required bool enabled,
  required bool isFavorite,
  required bool isDesktop,
}) {
  if (!enabled || !isFavorite || !isDesktop) return false;
  final items = files.toList(growable: false);
  final total = items.fold<int>(0, (sum, file) => sum + file.size);
  return total <= maxAutoCopyTotalBytes &&
      items.every((file) => file.size <= (file.fileType == FileType.image ? maxAutoCopyImageBytes : maxAutoCopyFileBytes));
}

Future<bool> copyReceivedMediaToClipboard(Iterable<String> paths) {
  return Pasteboard.writeFiles(paths.toList(growable: false));
}
