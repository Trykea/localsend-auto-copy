#include "copy_notice.h"

#include "resource.h"

#include <d2d1.h>
#include <dwrite.h>
#include <wrl/client.h>

namespace {

using Microsoft::WRL::ComPtr;

constexpr wchar_t kClassName[] = L"LocalSendAutoCopyNotice";
constexpr wchar_t kWindowTitle[] = L"LocalSend notification";
constexpr UINT_PTR kHideTimer = 1;
constexpr UINT kDisplayDurationMs = 2200;
constexpr int kBaseWidth = 135;
constexpr int kBaseHeight = 34;
constexpr int kBaseBottomMargin = 56;

HWND g_notice_window = nullptr;
ComPtr<ID2D1Factory> g_d2d_factory;
ComPtr<IDWriteFactory> g_write_factory;

int Scale(int value, UINT dpi) { return MulDiv(value, static_cast<int>(dpi), 96); }

bool EnsureRenderers() {
  if (!g_d2d_factory) {
    if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED,
                                 g_d2d_factory.GetAddressOf()))) {
      return false;
    }
  }
  if (!g_write_factory) {
    if (FAILED(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
                                   __uuidof(IDWriteFactory),
                                   reinterpret_cast<IUnknown**>(
                                       g_write_factory.GetAddressOf())))) {
      return false;
    }
  }
  return true;
}

LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wparam,
                            LPARAM lparam) {
  switch (message) {
    case WM_NCHITTEST:
      return HTTRANSPARENT;
    case WM_TIMER:
      if (wparam == kHideTimer) {
        KillTimer(window, kHideTimer);
        ShowWindow(window, SW_HIDE);
      }
      return 0;
    case WM_ERASEBKGND:
      return 1;
    case WM_DESTROY:
      if (g_notice_window == window) g_notice_window = nullptr;
      return 0;
  }
  return DefWindowProc(window, message, wparam, lparam);
}

void EnsureWindowClass(HINSTANCE instance) {
  static bool registered = false;
  if (registered) return;
  WNDCLASSW window_class = {};
  window_class.hInstance = instance;
  window_class.lpfnWndProc = WindowProc;
  window_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
  window_class.lpszClassName = kClassName;
  RegisterClassW(&window_class);
  registered = true;
}

bool RenderAndPresent(HWND window, int width, int height,
                      const std::wstring& message) {
  if (!EnsureRenderers()) return false;

  HDC screen_dc = GetDC(nullptr);
  HDC bitmap_dc = CreateCompatibleDC(screen_dc);
  BITMAPINFO bitmap_info = {};
  bitmap_info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bitmap_info.bmiHeader.biWidth = width;
  bitmap_info.bmiHeader.biHeight = -height;
  bitmap_info.bmiHeader.biPlanes = 1;
  bitmap_info.bmiHeader.biBitCount = 32;
  bitmap_info.bmiHeader.biCompression = BI_RGB;
  void* pixels = nullptr;
  HBITMAP bitmap = CreateDIBSection(screen_dc, &bitmap_info, DIB_RGB_COLORS,
                                   &pixels, nullptr, 0);
  if (!bitmap || !bitmap_dc) {
    if (bitmap) DeleteObject(bitmap);
    if (bitmap_dc) DeleteDC(bitmap_dc);
    ReleaseDC(nullptr, screen_dc);
    return false;
  }
  HGDIOBJ previous_bitmap = SelectObject(bitmap_dc, bitmap);

  const UINT dpi = GetDpiForWindow(window);
  const float scale = static_cast<float>(dpi) / 96.0f;
  const float radius = 17.0f * scale;
  const D2D1_SIZE_U size = D2D1::SizeU(width, height);
  const auto properties = D2D1::RenderTargetProperties(
      D2D1_RENDER_TARGET_TYPE_DEFAULT,
      D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM,
                        D2D1_ALPHA_MODE_PREMULTIPLIED));
  ComPtr<ID2D1DCRenderTarget> target;
  const RECT target_rect = {0, 0, width, height};
  if (FAILED(g_d2d_factory->CreateDCRenderTarget(&properties,
                                                  target.GetAddressOf())) ||
      FAILED(target->BindDC(bitmap_dc, &target_rect))) {
    SelectObject(bitmap_dc, previous_bitmap);
    DeleteObject(bitmap);
    DeleteDC(bitmap_dc);
    ReleaseDC(nullptr, screen_dc);
    return false;
  }

  target->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
  target->BeginDraw();
  target->Clear(D2D1::ColorF(0, 0.0f));
  const auto pill = D2D1::RoundedRect(
      D2D1::RectF(0.5f, 0.5f, static_cast<float>(width) - 0.5f,
                  static_cast<float>(height) - 0.5f),
      radius, radius);

  ComPtr<ID2D1SolidColorBrush> fill;
  ComPtr<ID2D1SolidColorBrush> text;
  target->CreateSolidColorBrush(D2D1::ColorF(0x000000, 0.90f),
                                fill.GetAddressOf());
  target->CreateSolidColorBrush(D2D1::ColorF(0xFFFFFF, 1.0f),
                                text.GetAddressOf());
  target->FillRoundedRectangle(pill, fill.Get());

  ComPtr<IDWriteTextFormat> format;
  const float font_size = 13.0f * scale;
  g_write_factory->CreateTextFormat(L"Segoe UI", nullptr,
                                     DWRITE_FONT_WEIGHT_SEMI_BOLD,
                                     DWRITE_FONT_STYLE_NORMAL,
                                     DWRITE_FONT_STRETCH_NORMAL, font_size,
                                     L"en-US", format.GetAddressOf());
  format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
  format->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
  format->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
  const float text_padding = static_cast<float>(Scale(8, dpi));
  const int icon_size = Scale(16, dpi);
  const int icon_gap = Scale(6, dpi);
  const auto text_rect = D2D1::RectF(
      text_padding + icon_size + icon_gap, 0.0f,
      static_cast<float>(width - Scale(8, dpi)),
                                     static_cast<float>(height));
  ComPtr<IDWriteTextLayout> layout;
  g_write_factory->CreateTextLayout(message.c_str(), static_cast<UINT32>(message.size()),
                                    format.Get(), text_rect.right - text_rect.left,
                                    text_rect.bottom, layout.GetAddressOf());
  target->DrawTextLayout(D2D1::Point2F(text_rect.left, 0.0f), layout.Get(),
                         text.Get(), D2D1_DRAW_TEXT_OPTIONS_CLIP);
  target->EndDraw();

  HICON icon = LoadIconW(GetModuleHandle(nullptr),
                         MAKEINTRESOURCEW(IDI_APP_ICON));
  if (icon != nullptr) {
    DrawIconEx(bitmap_dc, Scale(8, dpi), (height - icon_size) / 2, icon,
               icon_size, icon_size, 0, nullptr, DI_NORMAL);
  }

  POINT destination = {};
  RECT window_rect;
  GetWindowRect(window, &window_rect);
  destination.x = window_rect.left;
  destination.y = window_rect.top;
  SIZE window_size = {width, height};
  POINT source = {};
  BLENDFUNCTION blend = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
  const BOOL updated = UpdateLayeredWindow(
      window, screen_dc, &destination, &window_size, bitmap_dc, &source, 0,
      &blend, ULW_ALPHA);

  SelectObject(bitmap_dc, previous_bitmap);
  DeleteObject(bitmap);
  DeleteDC(bitmap_dc);
  ReleaseDC(nullptr, screen_dc);
  return updated == TRUE;
}

}  // namespace

namespace copy_notice {

void Show(HWND owner, const std::wstring& message) {
  HINSTANCE instance = GetModuleHandle(nullptr);
  EnsureWindowClass(instance);
  if (g_notice_window == nullptr) {
    g_notice_window = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE | WS_EX_LAYERED |
            WS_EX_TRANSPARENT,
        kClassName, kWindowTitle, WS_POPUP, 0, 0, 1, 1, owner, nullptr,
        instance, nullptr);
    if (g_notice_window == nullptr) return;
  }

  const UINT dpi = GetDpiForWindow(owner);
  const int width = Scale(kBaseWidth, dpi);
  const int height = Scale(kBaseHeight, dpi);
  const int bottom_margin = Scale(kBaseBottomMargin, dpi);
  POINT cursor;
  GetCursorPos(&cursor);
  HMONITOR monitor = MonitorFromPoint(cursor, MONITOR_DEFAULTTONEAREST);
  MONITORINFO monitor_info = {sizeof(monitor_info)};
  GetMonitorInfoW(monitor, &monitor_info);
  const RECT work_area = monitor_info.rcWork;
  const int x = work_area.left + ((work_area.right - work_area.left - width) / 2);
  const int y = work_area.bottom - height - bottom_margin;
  SetWindowPos(g_notice_window, HWND_TOPMOST, x, y, width, height,
               SWP_NOACTIVATE | SWP_SHOWWINDOW);
  RenderAndPresent(g_notice_window, width, height, message);
  SetTimer(g_notice_window, kHideTimer, kDisplayDurationMs, nullptr);
}

}  // namespace copy_notice
