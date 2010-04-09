#include <assert.h>

namespace ruby {

class pVideoGDI {
public:
  uint32_t *buffer;
  HBITMAP bitmap;
  HDC bitmapdc;
  BITMAPINFO bmi;

  struct {
    HWND handle;

    unsigned width;
    unsigned height;
  } settings;

  bool cap(const string& name) {
    if(name == Video::Handle) return true;
    return false;
  }

  any get(const string& name) {
    if(name == Video::Handle) return (uintptr_t)settings.handle;
    return false;
  }

  bool set(const string& name, const any& value) {
    if(name == Video::Handle) {
      settings.handle = (HWND)any_cast<uintptr_t>(value);
      return true;
    }

    return false;
  }

  bool lock(uint32_t *&data, unsigned &pitch, unsigned width, unsigned height) {
    settings.width  = width;
    settings.height = height;

    pitch = 1024 * 4;
    return data = buffer;
  }

  void unlock() {}

  void clear() {}

  void refresh() {
    RECT rc;
    GetClientRect(settings.handle, &rc);

    SetDIBits(bitmapdc, bitmap, 0, settings.height, (void*)buffer, &bmi, DIB_RGB_COLORS);
    HDC hdc = GetDC(settings.handle);
    StretchBlt(hdc, rc.left, rc.top, rc.right, rc.bottom, bitmapdc, 0, 1024 - settings.height, settings.width, settings.height, SRCCOPY);
    ReleaseDC(settings.handle, hdc);
  }

  bool init() {
    HDC hdc = GetDC(settings.handle);
    bitmapdc = CreateCompatibleDC(hdc);
    assert(bitmapdc);
    bitmap = CreateCompatibleBitmap(hdc, 1024, 1024);
    assert(bitmap);
    SelectObject(bitmapdc, bitmap);
    ReleaseDC(settings.handle, hdc);

    memset(&bmi, 0, sizeof(BITMAPINFO));
    bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth       = 1024;
    bmi.bmiHeader.biHeight      = -1024;
    bmi.bmiHeader.biPlanes      = 1;
    bmi.bmiHeader.biBitCount    = 32; //biBitCount of 15 is invalid, biBitCount of 16 is really RGB555
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage   = 1024 * 1024 * sizeof(uint32_t);

    settings.width  = 256;
    settings.height = 256;
    return true;
  }

  void term() {
    DeleteObject(bitmap);
    DeleteDC(bitmapdc);
  }

  pVideoGDI() {
    buffer = (uint32_t*)malloc(1024 * 1024 * sizeof(uint32_t));
    settings.handle = 0;
  }

  ~pVideoGDI() {
    if(buffer) free(buffer);
  }
};

DeclareVideo(GDI)

};
