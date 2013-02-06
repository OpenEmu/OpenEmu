/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2012 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#include <SDL_surface.h>

typedef struct
{
    Uint8 *src;
    int src_w, src_h;
    int src_pitch;
    int src_skip;
    Uint8 *dst;
    int dst_w, dst_h;
    int dst_pitch;
    int dst_skip;
    SDL_PixelFormat *src_fmt;
    SDL_PixelFormat *dst_fmt;
    Uint8 *table;
    int flags;
    Uint32 colorkey;
    Uint8 r, g, b, a;
} SDL_BlitInfo;

/* Blit mapping definition */
typedef struct SDL_BlitMap
{
    SDL_Surface *dst;
    int identity;
    SDL_blit blit;
    void *data;
    SDL_BlitInfo info;

    /* the version count matches the destination; mismatch indicates
       an invalid mapping */
    Uint32 dst_palette_version;
    Uint32 src_palette_version;
} SDL_BlitMap;

typedef struct SDL_VideoInfo
{
    Uint32 hw_available:1;
    Uint32 wm_available:1;
    Uint32 UnusedBits1:6;
    Uint32 UnusedBits2:1;
    Uint32 blit_hw:1;
    Uint32 blit_hw_CC:1;
    Uint32 blit_hw_A:1;
    Uint32 blit_sw:1;
    Uint32 blit_sw_CC:1;
    Uint32 blit_sw_A:1;
    Uint32 blit_fill:1;
    Uint32 UnusedBits3:16;
    Uint32 video_mem;

    SDL_PixelFormat *vfmt;

    int current_w;
    int current_h;
} SDL_VideoInfo;

#define SDL_ANYFORMAT       0x00100000
#define SDL_HWPALETTE       0x00200000
#define SDL_FULLSCREEN      0x00800000
#define SDL_RESIZABLE       0x01000000
#define SDL_NOFRAME         0x02000000
#define SDL_OPENGL          0x04000000
#define SDL_HWSURFACE       0x08000001  /**< \note Not used */

#define SDL_BUTTON_WHEELUP      4
#define SDL_BUTTON_WHEELDOWN    5

int initialized_video = 0;

static SDL_Window *SDL_VideoWindow = NULL;
static SDL_Surface *SDL_WindowSurface = NULL;
static SDL_Surface *SDL_VideoSurface = NULL;
static SDL_Surface *SDL_ShadowSurface = NULL;
static SDL_Surface *SDL_PublicSurface = NULL;
static SDL_Rect SDL_VideoViewport;
static char *wm_title = NULL;
static Uint32 SDL_VideoFlags = 0;
static SDL_GLContext *SDL_VideoContext = NULL;
static SDL_Surface *SDL_VideoIcon;

static void
SDL_WM_SetCaption(const char *title, const char *icon)
{
    if (wm_title) {
        SDL_free(wm_title);
    }
    if (title) {
        wm_title = SDL_strdup(title);
    } else {
        wm_title = NULL;
    }
    SDL_SetWindowTitle(SDL_VideoWindow, wm_title);
}

static int
GetVideoDisplay()
{
    const char *variable = SDL_getenv("SDL_VIDEO_FULLSCREEN_DISPLAY");
    if ( !variable ) {
        variable = SDL_getenv("SDL_VIDEO_FULLSCREEN_HEAD");
    }
    if ( variable ) {
        return SDL_atoi(variable);
    } else {
        return 0;
    }
}

static const SDL_VideoInfo *
SDL_GetVideoInfo(void)
{
    static SDL_VideoInfo info;
    SDL_DisplayMode mode;

    /* Memory leak, compatibility code, who cares? */
    if (!info.vfmt && SDL_GetDesktopDisplayMode(GetVideoDisplay(), &mode) == 0) {
        info.vfmt = SDL_AllocFormat(mode.format);
        info.current_w = mode.w;
        info.current_h = mode.h;
    }
    return &info;
}

static SDL_Rect **
SDL_ListModes(const SDL_PixelFormat * format, Uint32 flags)
{
    int i, nmodes;
    SDL_Rect **modes;

    if (!initialized_video) {
        return NULL;
    }

    if (!(flags & SDL_FULLSCREEN)) {
        return (SDL_Rect **) (-1);
    }

    if (!format) {
        format = SDL_GetVideoInfo()->vfmt;
    }

    /* Memory leak, but this is a compatibility function, who cares? */
    nmodes = 0;
    modes = NULL;
    for (i = 0; i < SDL_GetNumDisplayModes(GetVideoDisplay()); ++i) {
        SDL_DisplayMode mode;
        int bpp;

        SDL_GetDisplayMode(GetVideoDisplay(), i, &mode);
        if (!mode.w || !mode.h) {
            return (SDL_Rect **) (-1);
        }

        /* Copied from src/video/SDL_pixels.c:SDL_PixelFormatEnumToMasks */
        if (SDL_BYTESPERPIXEL(mode.format) <= 2) {
            bpp = SDL_BITSPERPIXEL(mode.format);
        } else {
            bpp = SDL_BYTESPERPIXEL(mode.format) * 8;
        }

        if (bpp != format->BitsPerPixel) {
            continue;
        }
        if (nmodes > 0 && modes[nmodes - 1]->w == mode.w
            && modes[nmodes - 1]->h == mode.h) {
            continue;
        }

        modes = SDL_realloc(modes, (nmodes + 2) * sizeof(*modes));
        if (!modes) {
            return NULL;
        }
        modes[nmodes] = (SDL_Rect *) SDL_malloc(sizeof(SDL_Rect));
        if (!modes[nmodes]) {
            return NULL;
        }
        modes[nmodes]->x = 0;
        modes[nmodes]->y = 0;
        modes[nmodes]->w = mode.w;
        modes[nmodes]->h = mode.h;
        ++nmodes;
    }
    if (modes) {
        modes[nmodes] = NULL;
    }
    return modes;
}

static void
SDL_UpdateRects(SDL_Surface * screen, int numrects, SDL_Rect * rects)
{
    int i;

    if (screen == SDL_ShadowSurface) {
        for (i = 0; i < numrects; ++i) {
            SDL_BlitSurface(SDL_ShadowSurface, &rects[i], SDL_VideoSurface,
                            &rects[i]);
        }

        /* Fall through to video surface update */
        screen = SDL_VideoSurface;
    }
    if (screen == SDL_VideoSurface) {
        if (SDL_VideoViewport.x || SDL_VideoViewport.y) {
            SDL_Rect *stackrects = SDL_stack_alloc(SDL_Rect, numrects);
            SDL_Rect *stackrect;
            const SDL_Rect *rect;

            /* Offset all the rectangles before updating */
            for (i = 0; i < numrects; ++i) {
                rect = &rects[i];
                stackrect = &stackrects[i];
                stackrect->x = SDL_VideoViewport.x + rect->x;
                stackrect->y = SDL_VideoViewport.y + rect->y;
                stackrect->w = rect->w;
                stackrect->h = rect->h;
            }
            SDL_UpdateWindowSurfaceRects(SDL_VideoWindow, stackrects, numrects);
            SDL_stack_free(stackrects);
        } else {
            SDL_UpdateWindowSurfaceRects(SDL_VideoWindow, rects, numrects);
        }
    }
}

static void
SDL_UpdateRect(SDL_Surface * screen, Sint32 x, Sint32 y, Uint32 w, Uint32 h)
{
    if (screen) {
        SDL_Rect rect;

        /* Fill the rectangle */
        rect.x = (int) x;
        rect.y = (int) y;
        rect.w = (int) (w ? w : screen->w);
        rect.h = (int) (h ? h : screen->h);
        SDL_UpdateRects(screen, 1, &rect);
    }
}

static int
SDL_Flip(SDL_Surface * screen)
{
    SDL_UpdateRect(screen, 0, 0, 0, 0);
    return 0;
}

/*
 * Calculate the pad-aligned scanline width of a surface
 */
static int
SDL_CalculatePitch(SDL_Surface * surface)
{
    int pitch;

    /* Surface should be 4-byte aligned for speed */
    pitch = surface->w * surface->format->BytesPerPixel;
    switch (surface->format->BitsPerPixel) {
    case 1:
        pitch = (pitch + 7) / 8;
        break;
    case 4:
        pitch = (pitch + 1) / 2;
        break;
    default:
        break;
    }
    pitch = (pitch + 3) & ~3;   /* 4-byte aligning */
    return (pitch);
}

static void
SDL_InvalidateMap(SDL_BlitMap * map)
{
    if (!map) {
        return;
    }
    if (map->dst) {
        /* Release our reference to the surface - see the note below */
        if (--map->dst->refcount <= 0) {
            SDL_FreeSurface(map->dst);
        }
    }
    map->dst = NULL;
    map->src_palette_version = 0;
    map->dst_palette_version = 0;
    if (map->info.table) {
        SDL_free(map->info.table);
        map->info.table = NULL;
    }
}

static void
SDL_GL_SwapBuffers(void)
{
    SDL_GL_SwapWindow(SDL_VideoWindow);
}

static int
SDL_WM_ToggleFullScreen(SDL_Surface * surface)
{
    int length;
    void *pixels;
    Uint8 *src, *dst;
    int row;
    int window_w;
    int window_h;

    if (!SDL_PublicSurface) {
        SDL_SetError("SDL_SetVideoMode() hasn't been called");
        return 0;
    }

    /* Copy the old bits out */
    length = SDL_PublicSurface->w * SDL_PublicSurface->format->BytesPerPixel;
    pixels = SDL_malloc(SDL_PublicSurface->h * length);
    if (pixels && SDL_PublicSurface->pixels) {
        src = (Uint8*)SDL_PublicSurface->pixels;
        dst = (Uint8*)pixels;
        for (row = 0; row < SDL_PublicSurface->h; ++row) {
            SDL_memcpy(dst, src, length);
            src += SDL_PublicSurface->pitch;
            dst += length;
        }
    }

    /* Do the physical mode switch */
    if (SDL_GetWindowFlags(SDL_VideoWindow) & SDL_WINDOW_FULLSCREEN) {
        if (SDL_SetWindowFullscreen(SDL_VideoWindow, 0) < 0) {
            return 0;
        }
        SDL_PublicSurface->flags &= ~SDL_FULLSCREEN;
    } else {
        if (SDL_SetWindowFullscreen(SDL_VideoWindow, 1) < 0) {
            return 0;
        }
        SDL_PublicSurface->flags |= SDL_FULLSCREEN;
    }

    /* Recreate the screen surface */
    SDL_WindowSurface = SDL_GetWindowSurface(SDL_VideoWindow);
    if (!SDL_WindowSurface) {
        /* We're totally hosed... */
        return 0;
    }

    /* Center the public surface in the window surface */
    SDL_GetWindowSize(SDL_VideoWindow, &window_w, &window_h);
    SDL_VideoViewport.x = (window_w - SDL_VideoSurface->w)/2;
    SDL_VideoViewport.y = (window_h - SDL_VideoSurface->h)/2;
    SDL_VideoViewport.w = SDL_VideoSurface->w;
    SDL_VideoViewport.h = SDL_VideoSurface->h;

    /* Do some shuffling behind the application's back if format changes */
    if (SDL_VideoSurface->format->format != SDL_WindowSurface->format->format) {
        if (SDL_ShadowSurface) {
            if (SDL_ShadowSurface->format->format == SDL_WindowSurface->format->format) {
                /* Whee!  We don't need a shadow surface anymore! */
                SDL_VideoSurface->flags &= ~SDL_DONTFREE;
                SDL_FreeSurface(SDL_VideoSurface);
                SDL_free(SDL_ShadowSurface->pixels);
                SDL_VideoSurface = SDL_ShadowSurface;
                SDL_VideoSurface->flags |= SDL_PREALLOC;
                SDL_ShadowSurface = NULL;
            } else {
                /* No problem, just change the video surface format */
                SDL_FreeFormat(SDL_VideoSurface->format);
                SDL_VideoSurface->format = SDL_WindowSurface->format;
                SDL_VideoSurface->format->refcount++;
                SDL_InvalidateMap(SDL_ShadowSurface->map);
            }
        } else {
            /* We can make the video surface the shadow surface */
            SDL_ShadowSurface = SDL_VideoSurface;
            SDL_ShadowSurface->pitch = SDL_CalculatePitch(SDL_ShadowSurface);
            SDL_ShadowSurface->pixels = SDL_malloc(SDL_ShadowSurface->h * SDL_ShadowSurface->pitch);
            if (!SDL_ShadowSurface->pixels) {
                /* Uh oh, we're hosed */
                SDL_ShadowSurface = NULL;
                return 0;
            }
            SDL_ShadowSurface->flags &= ~SDL_PREALLOC;

            SDL_VideoSurface = SDL_CreateRGBSurfaceFrom(NULL, 0, 0, 32, 0, 0, 0, 0, 0);
            SDL_VideoSurface->flags = SDL_ShadowSurface->flags;
            SDL_VideoSurface->flags |= SDL_PREALLOC;
            SDL_FreeFormat(SDL_VideoSurface->format);
            SDL_VideoSurface->format = SDL_WindowSurface->format;
            SDL_VideoSurface->format->refcount++;
            SDL_VideoSurface->w = SDL_ShadowSurface->w;
            SDL_VideoSurface->h = SDL_ShadowSurface->h;
        }
    }

    /* Update the video surface */
    SDL_VideoSurface->pitch = SDL_WindowSurface->pitch;
    SDL_VideoSurface->pixels = (void *)((Uint8 *)SDL_WindowSurface->pixels +
        SDL_VideoViewport.y * SDL_VideoSurface->pitch +
        SDL_VideoViewport.x  * SDL_VideoSurface->format->BytesPerPixel);
    SDL_SetClipRect(SDL_VideoSurface, NULL);

    /* Copy the old bits back */
    if (pixels) {
        src = (Uint8*)pixels;
        dst = (Uint8*)SDL_PublicSurface->pixels;
        for (row = 0; row < SDL_PublicSurface->h; ++row) {
            SDL_memcpy(dst, src, length);
            src += length;
            dst += SDL_PublicSurface->pitch;
        }
        SDL_Flip(SDL_PublicSurface);
        SDL_free(pixels);
    }

    /* We're done! */
    return 1;
}

static void
ClearVideoSurface()
{
    if (SDL_ShadowSurface) {
        SDL_FillRect(SDL_ShadowSurface, NULL,
            SDL_MapRGB(SDL_ShadowSurface->format, 0, 0, 0));
    }
    SDL_FillRect(SDL_WindowSurface, NULL, 0);
    SDL_UpdateWindowSurface(SDL_VideoWindow);
}

static int
SDL_ResizeVideoMode(int width, int height, int bpp, Uint32 flags)
{
    int w, h;

    /* We can't resize something we don't have... */
    if (!SDL_VideoSurface) {
        return -1;
    }

    /* We probably have to recreate the window in fullscreen mode */
    if (flags & SDL_FULLSCREEN) {
        return -1;
    }

    /* I don't think there's any change we can gracefully make in flags */
    if (flags != SDL_VideoFlags) {
        return -1;
    }
    if (bpp != SDL_VideoSurface->format->BitsPerPixel) {
        return -1;
    }

    /* Resize the window */
    SDL_GetWindowSize(SDL_VideoWindow, &w, &h);
    if (w != width || h != height) {
        SDL_SetWindowSize(SDL_VideoWindow, width, height);
    }

    /* If we're in OpenGL mode, just resize the stub surface and we're done! */
    if (flags & SDL_OPENGL) {
        SDL_VideoSurface->w = width;
        SDL_VideoSurface->h = height;
        return 0;
    }

    SDL_WindowSurface = SDL_GetWindowSurface(SDL_VideoWindow);
    if (!SDL_WindowSurface) {
        return -1;
    }
    if (SDL_VideoSurface->format != SDL_WindowSurface->format) {
        return -1;
    }
    SDL_VideoSurface->w = width;
    SDL_VideoSurface->h = height;
    SDL_VideoSurface->pixels = SDL_WindowSurface->pixels;
    SDL_VideoSurface->pitch = SDL_WindowSurface->pitch;
    SDL_SetClipRect(SDL_VideoSurface, NULL);

    if (SDL_ShadowSurface) {
        SDL_ShadowSurface->w = width;
        SDL_ShadowSurface->h = height;
        SDL_ShadowSurface->pitch = SDL_CalculatePitch(SDL_ShadowSurface);
        SDL_ShadowSurface->pixels =
            SDL_realloc(SDL_ShadowSurface->pixels,
                        SDL_ShadowSurface->h * SDL_ShadowSurface->pitch);
        SDL_SetClipRect(SDL_ShadowSurface, NULL);
        SDL_InvalidateMap(SDL_ShadowSurface->map);
    } else {
        SDL_PublicSurface = SDL_VideoSurface;
    }

    ClearVideoSurface();

    return 0;
}

static int
SDL_CompatEventFilter(void *userdata, SDL_Event * event)
{
    SDL_Event fake;

    switch (event->type) {
    case SDL_WINDOWEVENT:
        switch (event->window.event) {
        case SDL_WINDOWEVENT_CLOSE:
            fake.type = SDL_QUIT;
            SDL_PushEvent(&fake);
            break;
        }
    case SDL_KEYDOWN:
    case SDL_KEYUP:
        {
            Uint32 unicode = 0;
            if (event->key.type == SDL_KEYDOWN && event->key.keysym.sym < 256) {
                unicode = event->key.keysym.sym;
                if (unicode >= 'a' && unicode <= 'z') {
                    int shifted = !!(event->key.keysym.mod & KMOD_SHIFT);
                    int capslock = !!(event->key.keysym.mod & KMOD_CAPS);
                    if ((shifted ^ capslock) != 0) {
                        unicode = SDL_toupper(unicode);
                    }
                }
            }
            if (unicode) {
                event->key.keysym.unicode = unicode;
            }
            break;
        }
    case SDL_TEXTINPUT:
        {
            /* FIXME: Generate an old style key repeat event if needed */
            //printf("TEXTINPUT: '%s'\n", event->text.text);
            break;
        }
    case SDL_MOUSEMOTION:
        {
            event->motion.x -= SDL_VideoViewport.x;
            event->motion.y -= SDL_VideoViewport.y;
            break;
        }
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
        {
            event->button.x -= SDL_VideoViewport.x;
            event->button.y -= SDL_VideoViewport.y;
            break;
        }
    case SDL_MOUSEWHEEL:
        {
            Uint8 button;
            int x, y;

            if (event->wheel.y == 0) {
                break;
            }

            SDL_GetMouseState(&x, &y);

            if (event->wheel.y > 0) {
                button = SDL_BUTTON_WHEELUP;
            } else {
                button = SDL_BUTTON_WHEELDOWN;
            }

            fake.button.button = button;
            fake.button.x = x;
            fake.button.y = y;
            fake.button.windowID = event->wheel.windowID;

            fake.type = SDL_MOUSEBUTTONDOWN;
            fake.button.state = SDL_PRESSED;
            SDL_PushEvent(&fake);

            fake.type = SDL_MOUSEBUTTONUP;
            fake.button.state = SDL_RELEASED;
            SDL_PushEvent(&fake);
            break;
        }

    }
    return 1;
}

static void
GetEnvironmentWindowPosition(int w, int h, int *x, int *y)
{
    int display = GetVideoDisplay();
    const char *window = SDL_getenv("SDL_VIDEO_WINDOW_POS");
    const char *center = SDL_getenv("SDL_VIDEO_CENTERED");
    if (window) {
        if (SDL_sscanf(window, "%d,%d", x, y) == 2) {
            return;
        }
        if (SDL_strcmp(window, "center") == 0) {
            center = window;
        }
    }
    if (center) {
        *x = SDL_WINDOWPOS_CENTERED_DISPLAY(display);
        *y = SDL_WINDOWPOS_CENTERED_DISPLAY(display);
    }
}

static SDL_Surface *
SDL_SetVideoMode(int width, int height, int bpp, Uint32 flags)
{
    SDL_DisplayMode desktop_mode;
    int display = GetVideoDisplay();
    int window_x = SDL_WINDOWPOS_UNDEFINED_DISPLAY(display);
    int window_y = SDL_WINDOWPOS_UNDEFINED_DISPLAY(display);
    int window_w;
    int window_h;
    Uint32 window_flags;
    Uint32 surface_flags;

    if (!initialized_video) {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE) < 0) {
            return NULL;
        }
        initialized_video = 1;
    }

    SDL_GetDesktopDisplayMode(display, &desktop_mode);

    if (width == 0) {
        width = desktop_mode.w;
    }
    if (height == 0) {
        height = desktop_mode.h;
    }
    if (bpp == 0) {
        bpp = SDL_BITSPERPIXEL(desktop_mode.format);
    }

    /* See if we can simply resize the existing window and surface */
    if (SDL_ResizeVideoMode(width, height, bpp, flags) == 0) {
        return SDL_PublicSurface;
    }

    /* Destroy existing window */
    SDL_PublicSurface = NULL;
    if (SDL_ShadowSurface) {
        SDL_ShadowSurface->flags &= ~SDL_DONTFREE;
        SDL_FreeSurface(SDL_ShadowSurface);
        SDL_ShadowSurface = NULL;
    }
    if (SDL_VideoSurface) {
        SDL_VideoSurface->flags &= ~SDL_DONTFREE;
        SDL_FreeSurface(SDL_VideoSurface);
        SDL_VideoSurface = NULL;
    }
    if (SDL_VideoContext) {
        /* SDL_GL_MakeCurrent(0, NULL); *//* Doesn't do anything */
        SDL_GL_DeleteContext(SDL_VideoContext);
        SDL_VideoContext = NULL;
    }
    if (SDL_VideoWindow) {
        SDL_GetWindowPosition(SDL_VideoWindow, &window_x, &window_y);
        SDL_DestroyWindow(SDL_VideoWindow);
    }

    /* Set up the event filter */
    if (!SDL_GetEventFilter(NULL, NULL)) {
        SDL_SetEventFilter(SDL_CompatEventFilter, NULL);
    }

    /* Create a new window */
    window_flags = SDL_WINDOW_SHOWN;
    if (flags & SDL_FULLSCREEN) {
        window_flags |= SDL_WINDOW_FULLSCREEN;
    }
    if (flags & SDL_OPENGL) {
        window_flags |= SDL_WINDOW_OPENGL;
    }
    if (flags & SDL_RESIZABLE) {
        window_flags |= SDL_WINDOW_RESIZABLE;
    }
    if (flags & SDL_NOFRAME) {
        window_flags |= SDL_WINDOW_BORDERLESS;
    }
    GetEnvironmentWindowPosition(width, height, &window_x, &window_y);
    SDL_VideoWindow =
        SDL_CreateWindow(wm_title, window_x, window_y, width, height,
                         window_flags);
    if (!SDL_VideoWindow) {
        return NULL;
    }
    SDL_SetWindowIcon(SDL_VideoWindow, SDL_VideoIcon);

    window_flags = SDL_GetWindowFlags(SDL_VideoWindow);
    surface_flags = 0;
    if (window_flags & SDL_WINDOW_FULLSCREEN) {
        surface_flags |= SDL_FULLSCREEN;
    }
    if ((window_flags & SDL_WINDOW_OPENGL) && (flags & SDL_OPENGL)) {
        surface_flags |= SDL_OPENGL;
    }
    if (window_flags & SDL_WINDOW_RESIZABLE) {
        surface_flags |= SDL_RESIZABLE;
    }
    if (window_flags & SDL_WINDOW_BORDERLESS) {
        surface_flags |= SDL_NOFRAME;
    }

    SDL_VideoFlags = flags;

    /* If we're in OpenGL mode, just create a stub surface and we're done! */
    if (flags & SDL_OPENGL) {
        SDL_VideoContext = SDL_GL_CreateContext(SDL_VideoWindow);
        if (!SDL_VideoContext) {
            return NULL;
        }
        if (SDL_GL_MakeCurrent(SDL_VideoWindow, SDL_VideoContext) < 0) {
            return NULL;
        }
        SDL_VideoSurface =
            SDL_CreateRGBSurfaceFrom(NULL, width, height, bpp, 0, 0, 0, 0, 0);
        if (!SDL_VideoSurface) {
            return NULL;
        }
        SDL_VideoSurface->flags |= surface_flags;
        SDL_PublicSurface = SDL_VideoSurface;
        return SDL_PublicSurface;
    }

    /* Create the screen surface */
    SDL_WindowSurface = SDL_GetWindowSurface(SDL_VideoWindow);
    if (!SDL_WindowSurface) {
        return NULL;
    }

    /* Center the public surface in the window surface */
    SDL_GetWindowSize(SDL_VideoWindow, &window_w, &window_h);
    SDL_VideoViewport.x = (window_w - width)/2;
    SDL_VideoViewport.y = (window_h - height)/2;
    SDL_VideoViewport.w = width;
    SDL_VideoViewport.h = height;

    SDL_VideoSurface = SDL_CreateRGBSurfaceFrom(NULL, 0, 0, 32, 0, 0, 0, 0, 0);
    SDL_VideoSurface->flags |= surface_flags;
    SDL_VideoSurface->flags |= SDL_DONTFREE;
    SDL_FreeFormat(SDL_VideoSurface->format);
    SDL_VideoSurface->format = SDL_WindowSurface->format;
    SDL_VideoSurface->format->refcount++;
    SDL_VideoSurface->w = width;
    SDL_VideoSurface->h = height;
    SDL_VideoSurface->pitch = SDL_WindowSurface->pitch;
    SDL_VideoSurface->pixels = (void *)((Uint8 *)SDL_WindowSurface->pixels +
        SDL_VideoViewport.y * SDL_VideoSurface->pitch +
        SDL_VideoViewport.x  * SDL_VideoSurface->format->BytesPerPixel);
    SDL_SetClipRect(SDL_VideoSurface, NULL);

    /* Create a shadow surface if necessary */
    if ((bpp != SDL_VideoSurface->format->BitsPerPixel)
        && !(flags & SDL_ANYFORMAT)) {
        SDL_ShadowSurface =
            SDL_CreateRGBSurface(0, width, height, bpp, 0, 0, 0, 0);
        if (!SDL_ShadowSurface) {
            return NULL;
        }
        SDL_ShadowSurface->flags |= surface_flags;
        SDL_ShadowSurface->flags |= SDL_DONTFREE;

        /* 8-bit SDL_ShadowSurface surfaces report that they have exclusive palette */
        if (SDL_ShadowSurface->format->palette) {
            SDL_ShadowSurface->flags |= SDL_HWPALETTE;
            //TODO SDL_DitherColors(SDL_ShadowSurface->format->palette->colors,
            //                 SDL_ShadowSurface->format->BitsPerPixel);
        }
        SDL_FillRect(SDL_ShadowSurface, NULL,
            SDL_MapRGB(SDL_ShadowSurface->format, 0, 0, 0));
    }
    SDL_PublicSurface =
        (SDL_ShadowSurface ? SDL_ShadowSurface : SDL_VideoSurface);

    ClearVideoSurface();

    /* We're finally done! */
    return SDL_PublicSurface;
}
