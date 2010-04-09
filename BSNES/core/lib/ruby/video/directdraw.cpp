#include <ddraw.h>

namespace ruby {

class pVideoDD {
public:
  LPDIRECTDRAW lpdd;
  LPDIRECTDRAW7 lpdd7;
  LPDIRECTDRAWSURFACE7 screen, raster;
  LPDIRECTDRAWCLIPPER clipper;
  DDSURFACEDESC2 ddsd;
  DDSCAPS2 ddscaps;
  unsigned iwidth, iheight;

  struct {
    HWND handle;
    bool synchronize;

    unsigned width;
    unsigned height;
  } settings;

  bool cap(const string& name) {
    if(name == Video::Handle) return true;
    if(name == Video::Synchronize) return true;
    return false;
  }

  any get(const string& name) {
    if(name == Video::Handle) return (uintptr_t)settings.handle;
    if(name == Video::Synchronize) return settings.synchronize;
    return false;
  }

  bool set(const string& name, const any& value) {
    if(name == Video::Handle) {
      settings.handle = (HWND)any_cast<uintptr_t>(value);
      return true;
    }

    if(name == Video::Synchronize) {
      settings.synchronize = any_cast<bool>(value);
      return true;
    }

    return false;
  }

  void resize(unsigned width, unsigned height) {
    if(iwidth >= width && iheight >= height) return;

    iwidth  = max(width,  iwidth);
    iheight = max(height, iheight);

    if(raster) raster->Release();

    screen->GetSurfaceDesc(&ddsd);
    int depth = ddsd.ddpfPixelFormat.dwRGBBitCount;
    if(depth == 32) goto try_native_surface;

    memset(&ddsd, 0, sizeof(DDSURFACEDESC2));
    ddsd.dwSize = sizeof(DDSURFACEDESC2);
    ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
    ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY;  //DDSCAPS_SYSTEMMEMORY
    ddsd.dwWidth  = iwidth;
    ddsd.dwHeight = iheight;

    ddsd.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    ddsd.ddpfPixelFormat.dwFlags = DDPF_RGB;
    ddsd.ddpfPixelFormat.dwRGBBitCount = 32;
    ddsd.ddpfPixelFormat.dwRBitMask = 0xff0000;
    ddsd.ddpfPixelFormat.dwGBitMask = 0x00ff00;
    ddsd.ddpfPixelFormat.dwBBitMask = 0x0000ff;

    if(lpdd7->CreateSurface(&ddsd, &raster, 0) == DD_OK) return clear();

    try_native_surface:
    memset(&ddsd, 0, sizeof(DDSURFACEDESC2));
    ddsd.dwSize = sizeof(DDSURFACEDESC2);
    ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
    ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY;  //DDSCAPS_SYSTEMMEMORY
    ddsd.dwWidth  = iwidth;
    ddsd.dwHeight = iheight;

    if(lpdd7->CreateSurface(&ddsd, &raster, 0) == DD_OK) return clear();
  }

  void clear() {
    DDBLTFX fx;
    fx.dwSize = sizeof(DDBLTFX);
    fx.dwFillColor = 0x00000000;
    screen->Blt(0, 0, 0, DDBLT_WAIT | DDBLT_COLORFILL, &fx);
    raster->Blt(0, 0, 0, DDBLT_WAIT | DDBLT_COLORFILL, &fx);
  }

  bool lock(uint32_t *&data, unsigned &pitch, unsigned width, unsigned height) {
    if(width != settings.width || height != settings.height) {
      resize(settings.width = width, settings.height = height);
    }

    if(raster->Lock(0, &ddsd, DDLOCK_WAIT, 0) != DD_OK) {
      raster->Restore();
      if(raster->Lock(0, &ddsd, DDLOCK_WAIT, 0) != DD_OK) return false;
    }
    pitch = ddsd.lPitch;
    return data = (uint32_t*)ddsd.lpSurface;
  }

  void unlock() {
    raster->Unlock(0);
  }

  void refresh() {
    if(settings.synchronize) {
      while(true) {
        BOOL in_vblank;
        lpdd7->GetVerticalBlankStatus(&in_vblank);
        if(in_vblank == true) break;
      }
    }

    HRESULT hr;
    RECT rd, rs;
    SetRect(&rs, 0, 0, settings.width, settings.height);

    POINT p = { 0, 0 };
    ClientToScreen(settings.handle, &p);
    GetClientRect(settings.handle, &rd);
    OffsetRect(&rd, p.x, p.y);

    if(screen->Blt(&rd, raster, &rs, DDBLT_WAIT, 0) == DDERR_SURFACELOST) {
      screen->Restore();
      raster->Restore();
    }
  }

  bool init() {
    term();

    DirectDrawCreate(0, &lpdd, 0);
    lpdd->QueryInterface(IID_IDirectDraw7, (void**)&lpdd7);
    if(lpdd) { lpdd->Release(); lpdd = 0; }

    lpdd7->SetCooperativeLevel(settings.handle, DDSCL_NORMAL);

    memset(&ddsd, 0, sizeof(DDSURFACEDESC2));
    ddsd.dwSize = sizeof(DDSURFACEDESC2);

    ddsd.dwFlags = DDSD_CAPS;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
    lpdd7->CreateSurface(&ddsd, &screen, 0);

    lpdd7->CreateClipper(0, &clipper, 0);
    clipper->SetHWnd(0, settings.handle);
    screen->SetClipper(clipper);

    raster  = 0;
    iwidth  = 0;
    iheight = 0;
    resize(settings.width = 256, settings.height = 256);

    return true;
  }

  void term() {
    if(clipper) { clipper->Release(); clipper = 0; }
    if(raster) { raster->Release(); raster = 0; }
    if(screen) { screen->Release(); screen = 0; }
    if(lpdd7) { lpdd7->Release(); lpdd7 = 0; }
    if(lpdd) { lpdd->Release(); lpdd = 0; }
  }

  pVideoDD() {
    lpdd = 0;
    lpdd7 = 0;
    screen = 0;
    raster = 0;
    clipper = 0;

    settings.handle = 0;
  }
};

DeclareVideo(DD)

};
