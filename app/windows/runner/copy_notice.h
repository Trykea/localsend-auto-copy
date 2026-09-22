#ifndef RUNNER_COPY_NOTICE_H_
#define RUNNER_COPY_NOTICE_H_

#include <windows.h>
#include <string>

namespace copy_notice {

void Show(HWND owner, const std::wstring& message);

}  // namespace copy_notice

#endif  // RUNNER_COPY_NOTICE_H_
