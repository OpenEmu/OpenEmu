#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
#include <X11/extensions/Xv.h>
#include <X11/extensions/Xvlib.h>

extern "C" XvImage* XvShmCreateImage(Display*, XvPortID, int, char*, int, int, XShmSegmentInfo*);

namespace ruby {

class pVideoXv {
public:
  uint32_t *buffer;
  uint8_t  *ytable, *utable, *vtable;

  enum XvFormat {
    XvFormatRGB32,
    XvFormatRGB24,
    XvFormatRGB16,
    XvFormatRGB15,
    XvFormatYUY2,
    XvFormatUYVY,
    XvFormatUnknown
  };

  struct {
    Display *display;
    GC gc;
    Window window;
    Colormap colormap;
    XShmSegmentInfo shminfo;

    int port;
    int depth;
    int visualid;

    XvImage *image;
    XvFormat format;
    uint32_t fourcc;
  } device;

  struct {
    Window handle;
    bool synchronize;

    unsigned width;
    unsigned height;
  } settings;

  bool cap(const string& name) {
    if(name == Video::Handle) return true;
    if(name == Video::Synchronize) {
      return XInternAtom(XOpenDisplay(0), "XV_SYNC_TO_VBLANK", true) != None;
    }
    return false;
  }

  any get(const string& name) {
    if(name == Video::Handle) return settings.handle;
    if(name == Video::Synchronize) return settings.synchronize;
    return false;
  }

  bool set(const string& name, const any& value) {
    if(name == Video::Handle) {
      settings.handle = any_cast<uintptr_t>(value);
      return true;
    }

    if(name == Video::Synchronize) {
      Display *display = XOpenDisplay(0);
      Atom atom = XInternAtom(display, "XV_SYNC_TO_VBLANK", true);
      if(atom != None && device.port >= 0) {
        settings.synchronize = any_cast<bool>(value);
        XvSetPortAttribute(display, device.port, atom, settings.synchronize);
        return true;
      }
      return false;
    }

    return false;
  }

  bool lock(uint32_t *&data, unsigned &pitch, unsigned width, unsigned height) {
    settings.width  = width;
    settings.height = height;

    pitch = 1024 * 4;
    return data = buffer;
  }

  void unlock() {
  }

  void clear() {
    memset(buffer, 0, 1024 * 1024 * sizeof(uint32_t));
    //clear twice in case video is double buffered ...
    refresh();
    refresh();
  }

  void refresh() {
    unsigned width  = settings.width;
    unsigned height = settings.height;

    XWindowAttributes target;
    XGetWindowAttributes(device.display, device.window, &target);

    //we must ensure that the child window is the same size as the parent window.
    //unfortunately, we cannot hook the parent window resize event notification,
    //as we did not create the parent window, nor have any knowledge of the toolkit used.
    //therefore, query each window size and resize as needed.
    XWindowAttributes parent;
    XGetWindowAttributes(device.display, settings.handle, &parent);
    if(target.width != parent.width || target.height != parent.height) {
      XResizeWindow(device.display, device.window, parent.width, parent.height);
    }

    //update target width and height attributes
    XGetWindowAttributes(device.display, device.window, &target);

    switch(device.format) {
      case XvFormatRGB32: render_rgb32(width, height); break;
      case XvFormatRGB24: render_rgb24(width, height); break;
      case XvFormatRGB16: render_rgb16(width, height); break;
      case XvFormatRGB15: render_rgb15(width, height); break;
      case XvFormatYUY2:  render_yuy2 (width, height); break;
      case XvFormatUYVY:  render_uyvy (width, height); break;
    }

    XvShmPutImage(device.display, device.port, device.window, device.gc, device.image,
      0, 0, width, height,
      0, 0, target.width, target.height,
      true);
  }

  bool init() {
    device.display = XOpenDisplay(0);

    if(!XShmQueryExtension(device.display)) {
      fprintf(stderr, "VideoXv: XShm extension not found.\n");
      return false;
    }

    //find an appropriate Xv port
    device.port = -1;
    XvAdaptorInfo *adaptor_info;
    unsigned adaptor_count;
    XvQueryAdaptors(device.display, DefaultRootWindow(device.display), &adaptor_count, &adaptor_info);
    for(unsigned i = 0; i < adaptor_count; i++) {
      //find adaptor that supports both input (memory->drawable) and image (drawable->screen) masks
      if(adaptor_info[i].num_formats < 1) continue;
      if(!(adaptor_info[i].type & XvInputMask)) continue;
      if(!(adaptor_info[i].type & XvImageMask)) continue;

      device.port     = adaptor_info[i].base_id;
      device.depth    = adaptor_info[i].formats->depth;
      device.visualid = adaptor_info[i].formats->visual_id;
      break;
    }
    XvFreeAdaptorInfo(adaptor_info);
    if(device.port < 0) {
      fprintf(stderr, "VideoXv: failed to find valid XvPort.\n");
      return false;
    }

    //create child window to attach to parent window.
    //this is so that even if parent window visual depth doesn't match Xv visual
    //(common with composited windows), Xv can still render to child window.
    XWindowAttributes window_attributes;
    XGetWindowAttributes(device.display, settings.handle, &window_attributes);

    XVisualInfo visualtemplate;
    visualtemplate.visualid = device.visualid;
    visualtemplate.screen   = DefaultScreen(device.display);
    visualtemplate.depth    = device.depth;
    visualtemplate.visual   = 0;
    int visualmatches       = 0;
    XVisualInfo *visualinfo = XGetVisualInfo(device.display, VisualIDMask | VisualScreenMask | VisualDepthMask, &visualtemplate, &visualmatches);
    if(visualmatches < 1 || !visualinfo->visual) {
      if(visualinfo) XFree(visualinfo);
      fprintf(stderr, "VideoXv: unable to find Xv-compatible visual.\n");
      return false;
    }

    device.colormap = XCreateColormap(device.display, settings.handle, visualinfo->visual, AllocNone);
    XSetWindowAttributes attributes;
    attributes.colormap = device.colormap;
    attributes.border_pixel = 0;
    attributes.event_mask = StructureNotifyMask;
    device.window = XCreateWindow(device.display, /* parent = */ settings.handle,
      /* x = */ 0, /* y = */ 0, window_attributes.width, window_attributes.height,
      /* border_width = */ 0, device.depth, InputOutput, visualinfo->visual,
      CWColormap | CWBorderPixel | CWEventMask, &attributes);
    XFree(visualinfo);
    XSetWindowBackground(device.display, device.window, /* color = */ 0);
    XMapWindow(device.display, device.window);

    device.gc = XCreateGC(device.display, device.window, 0, 0);

    //set colorkey to auto paint, so that Xv video output is always visible
    Atom atom = XInternAtom(device.display, "XV_AUTOPAINT_COLORKEY", true);
    if(atom != None) XvSetPortAttribute(device.display, device.port, atom, 1);

    //find optimal rendering format
    device.format = XvFormatUnknown;
    signed format_count;
    XvImageFormatValues *format = XvListImageFormats(device.display, device.port, &format_count);

    if(device.format == XvFormatUnknown) for(signed i = 0; i < format_count; i++) {
      if(format[i].type == XvRGB && format[i].bits_per_pixel == 32) {
        device.format = XvFormatRGB32;
        device.fourcc = format[i].id;
        break;
      }
    }

    if(device.format == XvFormatUnknown) for(signed i = 0; i < format_count; i++) {
      if(format[i].type == XvRGB && format[i].bits_per_pixel == 24) {
        device.format = XvFormatRGB24;
        device.fourcc = format[i].id;
        break;
      }
    }

    if(device.format == XvFormatUnknown) for(signed i = 0; i < format_count; i++) {
      if(format[i].type == XvRGB && format[i].bits_per_pixel <= 16 && format[i].red_mask == 0xf800) {
        device.format = XvFormatRGB16;
        device.fourcc = format[i].id;
        break;
      }
    }

    if(device.format == XvFormatUnknown) for(signed i = 0; i < format_count; i++) {
      if(format[i].type == XvRGB && format[i].bits_per_pixel <= 16 && format[i].red_mask == 0x7c00) {
        device.format = XvFormatRGB15;
        device.fourcc = format[i].id;
        break;
      }
    }

    if(device.format == XvFormatUnknown) for(signed i = 0; i < format_count; i++) {
      if(format[i].type == XvYUV && format[i].bits_per_pixel == 16 && format[i].format == XvPacked) {
        if(format[i].component_order[0] == 'Y' && format[i].component_order[1] == 'U'
        && format[i].component_order[2] == 'Y' && format[i].component_order[3] == 'V'
        ) {
          device.format = XvFormatYUY2;
          device.fourcc = format[i].id;
          break;
        }
      }
    }

    if(device.format == XvFormatUnknown) for(signed i = 0; i < format_count; i++) {
      if(format[i].type == XvYUV && format[i].bits_per_pixel == 16 && format[i].format == XvPacked) {
        if(format[i].component_order[0] == 'U' && format[i].component_order[1] == 'Y'
        && format[i].component_order[2] == 'V' && format[i].component_order[3] == 'Y'
        ) {
          device.format = XvFormatUYVY;
          device.fourcc = format[i].id;
          break;
        }
      }
    }

    free(format);
    if(device.format == XvFormatUnknown) {
      fprintf(stderr, "VideoXv: unable to find a supported image format.\n");
      return false;
    }

    device.image = XvShmCreateImage(device.display, device.port, device.fourcc, 0, 1024, 1024, &device.shminfo);
    if(!device.image) {
      fprintf(stderr, "VideoXv: XShmCreateImage failed.\n");
      return false;
    }

    device.shminfo.shmid    = shmget(IPC_PRIVATE, device.image->data_size, IPC_CREAT | 0777);
    device.shminfo.shmaddr  = device.image->data = (char*)shmat(device.shminfo.shmid, 0, 0);
    device.shminfo.readOnly = false;
    if(!XShmAttach(device.display, &device.shminfo)) {
      fprintf(stderr, "VideoXv: XShmAttach failed.\n");
      return false;
    }

    buffer = new uint32_t[1024 * 1024];
    settings.width  = 256;
    settings.height = 256;
    init_yuv_tables();
    clear();
    return true;
  }

  void term() {
    XShmDetach(device.display, &device.shminfo);
    shmdt(device.shminfo.shmaddr);
    shmctl(device.shminfo.shmid, IPC_RMID, NULL);
    XFree(device.image);

    if(device.window) {
      XUnmapWindow(device.display, device.window);
      device.window = 0;
    }

    if(device.colormap) {
      XFreeColormap(device.display, device.colormap);
      device.colormap = 0;
    }

    if(buffer) { delete[] buffer; buffer = 0; }
    if(ytable) { delete[] ytable; ytable = 0; }
    if(utable) { delete[] utable; utable = 0; }
    if(vtable) { delete[] vtable; vtable = 0; }
  }

  void render_rgb32(unsigned width, unsigned height) {
    uint32_t *input  = (uint32_t*)buffer;
    uint32_t *output = (uint32_t*)device.image->data;

    for(unsigned y = 0; y < height; y++) {
      memcpy(output, input, width * 4);
      input  += 1024;
      output += 1024;
    }
  }

  void render_rgb24(unsigned width, unsigned height) {
    uint32_t *input  = (uint32_t*)buffer;
    uint8_t  *output = (uint8_t*)device.image->data;

    for(unsigned y = 0; y < height; y++) {
      for(unsigned x = 0; x < width; x++) {
        uint32_t p = *input++;
        *output++ = p;
        *output++ = p >> 8;
        *output++ = p >> 16;
      }

      input  += (1024 - width);
      output += (1024 - width) * 3;
    }
  }

  void render_rgb16(unsigned width, unsigned height) {
    uint32_t *input  = (uint32_t*)buffer;
    uint16_t *output = (uint16_t*)device.image->data;

    for(unsigned y = 0; y < height; y++) {
      for(unsigned x = 0; x < width; x++) {
        uint32_t p = *input++;
        *output++ = ((p >> 8) & 0xf800) | ((p >> 5) & 0x07e0) | ((p >> 3) & 0x001f);  //RGB32->RGB16
      }

      input  += 1024 - width;
      output += 1024 - width;
    }
  }

  void render_rgb15(unsigned width, unsigned height) {
    uint32_t *input  = (uint32_t*)buffer;
    uint16_t *output = (uint16_t*)device.image->data;

    for(unsigned y = 0; y < height; y++) {
      for(unsigned x = 0; x < width; x++) {
        uint32_t p = *input++;
        *output++ = ((p >> 9) & 0x7c00) | ((p >> 6) & 0x03e0) | ((p >> 3) & 0x001f);  //RGB32->RGB15
      }

      input  += 1024 - width;
      output += 1024 - width;
    }
  }

  void render_yuy2(unsigned width, unsigned height) {
    uint32_t *input  = (uint32_t*)buffer;
    uint16_t *output = (uint16_t*)device.image->data;

    for(unsigned y = 0; y < height; y++) {
      for(unsigned x = 0; x < width >> 1; x++) {
        uint32_t p0 = *input++;
        uint32_t p1 = *input++;
        p0 = ((p0 >> 8) & 0xf800) + ((p0 >> 5) & 0x07e0) + ((p0 >> 3) & 0x001f);  //RGB32->RGB16
        p1 = ((p1 >> 8) & 0xf800) + ((p1 >> 5) & 0x07e0) + ((p1 >> 3) & 0x001f);  //RGB32->RGB16

        uint8_t u = (utable[p0] + utable[p1]) >> 1;
        uint8_t v = (vtable[p0] + vtable[p1]) >> 1;

        *output++ = (u << 8) | ytable[p0];
        *output++ = (v << 8) | ytable[p1];
      }

      input  += 1024 - width;
      output += 1024 - width;
    }
  }

  void render_uyvy(unsigned width, unsigned height) {
    uint32_t *input  = (uint32_t*)buffer;
    uint16_t *output = (uint16_t*)device.image->data;

    for(unsigned y = 0; y < height; y++) {
      for(unsigned x = 0; x < width >> 1; x++) {
        uint32_t p0 = *input++;
        uint32_t p1 = *input++;
        p0 = ((p0 >> 8) & 0xf800) + ((p0 >> 5) & 0x07e0) + ((p0 >> 3) & 0x001f);
        p1 = ((p1 >> 8) & 0xf800) + ((p1 >> 5) & 0x07e0) + ((p1 >> 3) & 0x001f);

        uint8_t u = (utable[p0] + utable[p1]) >> 1;
        uint8_t v = (vtable[p0] + vtable[p1]) >> 1;

        *output++ = (ytable[p0] << 8) | u;
        *output++ = (ytable[p1] << 8) | v;
      }

      input  += 1024 - width;
      output += 1024 - width;
    }
  }

  void init_yuv_tables() {
    ytable = new uint8_t[65536];
    utable = new uint8_t[65536];
    vtable = new uint8_t[65536];

    for(unsigned i = 0; i < 65536; i++) {
      //extract RGB565 color data from i
      uint8_t r = (i >> 11) & 31, g = (i >> 5) & 63, b = (i) & 31;
      r = (r << 3) | (r >> 2);  //R5->R8
      g = (g << 2) | (g >> 4);  //G6->G8
      b = (b << 3) | (b >> 2);  //B5->B8

      //ITU-R Recommendation BT.601
      //double lr = 0.299, lg = 0.587, lb = 0.114;
      int y = int( +(double(r) * 0.257) + (double(g) * 0.504) + (double(b) * 0.098) +  16.0 );
      int u = int( -(double(r) * 0.148) - (double(g) * 0.291) + (double(b) * 0.439) + 128.0 );
      int v = int( +(double(r) * 0.439) - (double(g) * 0.368) - (double(b) * 0.071) + 128.0 );

      //ITU-R Recommendation BT.709
      //double lr = 0.2126, lg = 0.7152, lb = 0.0722;
      //int y = int( double(r) * lr + double(g) * lg + double(b) * lb );
      //int u = int( (double(b) - y) / (2.0 - 2.0 * lb) + 128.0 );
      //int v = int( (double(r) - y) / (2.0 - 2.0 * lr) + 128.0 );

      ytable[i] = y < 0 ? 0 : y > 255 ? 255 : y;
      utable[i] = u < 0 ? 0 : u > 255 ? 255 : u;
      vtable[i] = v < 0 ? 0 : v > 255 ? 255 : v;
    }
  }

  pVideoXv() {
    device.window   = 0;
    device.colormap = 0;
    device.port     = -1;

    ytable = 0;
    utable = 0;
    vtable = 0;

    settings.handle      = 0;
    settings.synchronize = false;
  }

  ~pVideoXv() {
    term();
  }
};

DeclareVideo(Xv)

};
