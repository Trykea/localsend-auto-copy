import 'package:localsend_app/util/auto_copy_received_media.dart';
import 'package:localsend_isolates/model/dto/file_dto.dart';
import 'package:localsend_isolates/model/file_type.dart';
import 'package:test/test.dart';

FileDto file(FileType type, int size) => FileDto(
  id: 'file',
  fileName: 'file.bin',
  size: size,
  fileType: type,
  hash: null,
  preview: null,
  metadata: null,
);

void main() {
  test('accepts eligible favorite media within per-file and total limits', () {
    expect(
      shouldAutoCopyReceivedMedia(
        files: [file(FileType.image, maxAutoCopyImageBytes)],
        enabled: true,
        isFavorite: true,
        isDesktop: true,
      ),
      isTrue,
    );
  });

  test('rejects oversized images, files, and batches', () {
    expect(
      shouldAutoCopyReceivedMedia(
        files: [file(FileType.image, maxAutoCopyImageBytes + 1)],
        enabled: true,
        isFavorite: true,
        isDesktop: true,
      ),
      isFalse,
    );
    expect(
      shouldAutoCopyReceivedMedia(
        files: [file(FileType.video, maxAutoCopyFileBytes + 1)],
        enabled: true,
        isFavorite: true,
        isDesktop: true,
      ),
      isFalse,
    );
  });

  test('requires the setting, favorite sender, and desktop platform', () {
    for (final options in <({bool enabled, bool isFavorite, bool isDesktop})>[
      (enabled: false, isFavorite: true, isDesktop: true),
      (enabled: true, isFavorite: false, isDesktop: true),
      (enabled: true, isFavorite: true, isDesktop: false),
    ]) {
      expect(
        shouldAutoCopyReceivedMedia(
          files: [file(FileType.image, 1)],
          enabled: options.enabled,
          isFavorite: options.isFavorite,
          isDesktop: options.isDesktop,
        ),
        isFalse,
      );
    }
  });
}
