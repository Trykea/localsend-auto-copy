import 'package:localsend_app/provider/settings_provider.dart';
import 'package:mockito/mockito.dart';
import 'package:refena_flutter/refena_flutter.dart';
import 'package:test/test.dart';

import '../../mocks.mocks.dart';

void main() {
  test('auto-copy received text is disabled by default', () {
    final settings = SettingsService(MockPersistenceService()).init();

    expect(settings.autoCopyReceivedText, isFalse);
  });

  test('enabling auto-copy persists and updates settings', () async {
    final persistence = MockPersistenceService();
    final tester = NotifierTester(notifier: SettingsService(persistence));

    await tester.notifier.setAutoCopyReceivedText(true);

    expect(tester.state.autoCopyReceivedText, isTrue);
    verify(persistence.setAutoCopyReceivedText(true));
  });
}
