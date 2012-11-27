/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef _ANDROID_H_
#define _ANDROID_H_

#if defined(__ANDROID__)

#include "common/fs.h"
#include "common/archive.h"
#include "audio/mixer_intern.h"
#include "graphics/palette.h"
#include "graphics/surface.h"
#include "backends/base-backend.h"
#include "backends/plugins/posix/posix-provider.h"
#include "backends/fs/posix/posix-fs-factory.h"

#include "backends/platform/android/texture.h"

#include <pthread.h>

#include <android/log.h>

#include <GLES/gl.h>
#include <GLES/glext.h>

// toggles start
//#define ANDROID_DEBUG_ENTER
//#define ANDROID_DEBUG_GL
//#define ANDROID_DEBUG_GL_CALLS
// toggles end

extern const char *android_log_tag;

#define _ANDROID_LOG(prio, fmt, args...) __android_log_print(prio, android_log_tag, fmt, ## args)
#define LOGD(fmt, args...) _ANDROID_LOG(ANDROID_LOG_DEBUG, fmt, ##args)
#define LOGI(fmt, args...) _ANDROID_LOG(ANDROID_LOG_INFO, fmt, ##args)
#define LOGW(fmt, args...) _ANDROID_LOG(ANDROID_LOG_WARN, fmt, ##args)
#define LOGE(fmt, args...) _ANDROID_LOG(ANDROID_LOG_ERROR, fmt, ##args)

#ifdef ANDROID_DEBUG_ENTER
#define ENTER(fmt, args...) LOGD("%s(" fmt ")", __FUNCTION__, ##args)
#else
#define ENTER(fmt, args...) do {  } while (false)
#endif

#ifdef ANDROID_DEBUG_GL
extern void checkGlError(const char *expr, const char *file, int line);

#ifdef ANDROID_DEBUG_GL_CALLS
#define GLCALLLOG(x, before) \
	do { \
		if (before) \
			LOGD("calling '%s' (%s:%d)", x, __FILE__, __LINE__); \
		else \
			LOGD("returned from '%s' (%s:%d)", x, __FILE__, __LINE__); \
	} while (false)
#else
#define GLCALLLOG(x, before) do {  } while (false)
#endif

#define GLCALL(x) \
	do { \
		GLCALLLOG(#x, true); \
		(x); \
		GLCALLLOG(#x, false); \
		checkGlError(#x, __FILE__, __LINE__); \
	} while (false)

#define GLTHREADCHECK \
	do { \
		assert(pthread_self() == _main_thread); \
	} while (false)

#else
#define GLCALL(x) do { (x); } while (false)
#define GLTHREADCHECK do {  } while (false)
#endif

#ifdef DYNAMIC_MODULES
class AndroidPluginProvider : public POSIXPluginProvider {
protected:
	virtual void addCustomDirectories(Common::FSList &dirs) const;
};
#endif

class OSystem_Android : public EventsBaseBackend, public PaletteManager {
private:
	// passed from the dark side
	int _audio_sample_rate;
	int _audio_buffer_size;

	int _screen_changeid;
	int _egl_surface_width;
	int _egl_surface_height;
	bool _htc_fail;

	bool _force_redraw;

	// Game layer
	GLESBaseTexture *_game_texture;
	int _shake_offset;
	Common::Rect _focus_rect;

	// Overlay layer
	GLES4444Texture *_overlay_texture;
	bool _show_overlay;

	// Mouse layer
	GLESBaseTexture *_mouse_texture;
	GLESBaseTexture *_mouse_texture_palette;
	GLES5551Texture *_mouse_texture_rgb;
	Common::Point _mouse_hotspot;
	uint32 _mouse_keycolor;
	int _mouse_targetscale;
	bool _show_mouse;
	bool _use_mouse_palette;

	int _graphicsMode;
	bool _fullscreen;
	bool _ar_correction;

	pthread_t _main_thread;

	bool _timer_thread_exit;
	pthread_t _timer_thread;
	static void *timerThreadFunc(void *arg);

	bool _audio_thread_exit;
	pthread_t _audio_thread;
	static void *audioThreadFunc(void *arg);

	bool _enable_zoning;
	bool _virtkeybd_on;

	Audio::MixerImpl *_mixer;
	timeval _startTime;

	Common::String getSystemProperty(const char *name) const;

	void initSurface();
	void deinitSurface();
	void initViewport();

	void initOverlay();

#ifdef USE_RGB_COLOR
	Common::String getPixelFormatName(const Graphics::PixelFormat &format) const;
	void initTexture(GLESBaseTexture **texture, uint width, uint height,
						const Graphics::PixelFormat *format);
#endif

	void setupKeymapper();
	void setCursorPaletteInternal(const byte *colors, uint start, uint num);

public:
	OSystem_Android(int audio_sample_rate, int audio_buffer_size);
	virtual ~OSystem_Android();

	virtual void initBackend();
	void addPluginDirectories(Common::FSList &dirs) const;
	void enableZoning(bool enable) { _enable_zoning = enable; }

	virtual bool hasFeature(Feature f);
	virtual void setFeatureState(Feature f, bool enable);
	virtual bool getFeatureState(Feature f);

	virtual const GraphicsMode *getSupportedGraphicsModes() const;
	virtual int getDefaultGraphicsMode() const;
	virtual bool setGraphicsMode(int mode);
	virtual int getGraphicsMode() const;

#ifdef USE_RGB_COLOR
	virtual Graphics::PixelFormat getScreenFormat() const;
	virtual Common::List<Graphics::PixelFormat> getSupportedFormats() const;
#endif

	virtual void initSize(uint width, uint height,
							const Graphics::PixelFormat *format);

	enum FixupType {
		kClear = 0,		// glClear
		kClearSwap,		// glClear + swapBuffers
		kClearUpdate	// glClear + updateScreen
	};

	void clearScreen(FixupType type, byte count = 1);

	void updateScreenRect();
	virtual int getScreenChangeID() const;

	virtual int16 getHeight();
	virtual int16 getWidth();

	virtual PaletteManager *getPaletteManager() {
		return this;
	}

public:
	void pushEvent(int type, int arg1, int arg2, int arg3, int arg4, int arg5);

private:
	Common::Queue<Common::Event> _event_queue;
	Common::Event _queuedEvent;
	uint32 _queuedEventTime;
	MutexRef _event_queue_lock;

	Common::Point _touch_pt_down, _touch_pt_scroll, _touch_pt_dt;
	int _eventScaleX;
	int _eventScaleY;
	bool _touchpad_mode;
	int _touchpad_scale;
	int _trackball_scale;
	int _dpad_scale;
	int _fingersDown;

	void clipMouse(Common::Point &p);
	void scaleMouse(Common::Point &p, int x, int y, bool deductDrawRect = true, bool touchpadMode = false);
	void updateEventScale();
	void disableCursorPalette();

protected:
	// PaletteManager API
	virtual void setPalette(const byte *colors, uint start, uint num);
	virtual void grabPalette(byte *colors, uint start, uint num);

public:
	virtual void copyRectToScreen(const void *buf, int pitch, int x, int y,
									int w, int h);
	virtual void updateScreen();
	virtual Graphics::Surface *lockScreen();
	virtual void unlockScreen();
	virtual void setShakePos(int shakeOffset);
	virtual void fillScreen(uint32 col);
	virtual void setFocusRectangle(const Common::Rect& rect);
	virtual void clearFocusRectangle();

	virtual void showOverlay();
	virtual void hideOverlay();
	virtual void clearOverlay();
	virtual void grabOverlay(void *buf, int pitch);
	virtual void copyRectToOverlay(const void *buf, int pitch,
									int x, int y, int w, int h);
	virtual int16 getOverlayHeight();
	virtual int16 getOverlayWidth();
	virtual Graphics::PixelFormat getOverlayFormat() const;

	virtual bool showMouse(bool visible);

	virtual void warpMouse(int x, int y);
	virtual void setMouseCursor(const void *buf, uint w, uint h, int hotspotX,
								int hotspotY, uint32 keycolor,
								bool dontScale,
								const Graphics::PixelFormat *format);
	virtual void setCursorPalette(const byte *colors, uint start, uint num);

	virtual bool pollEvent(Common::Event &event);
	virtual uint32 getMillis();
	virtual void delayMillis(uint msecs);

	virtual MutexRef createMutex(void);
	virtual void lockMutex(MutexRef mutex);
	virtual void unlockMutex(MutexRef mutex);
	virtual void deleteMutex(MutexRef mutex);

	virtual void quit();

	virtual void setWindowCaption(const char *caption);
	virtual void displayMessageOnOSD(const char *msg);
	virtual void showVirtualKeyboard(bool enable);

	virtual Audio::Mixer *getMixer();
	virtual void getTimeAndDate(TimeDate &t) const;
	virtual void logMessage(LogMessageType::Type type, const char *message);
	virtual void addSysArchivesToSearchSet(Common::SearchSet &s,
											int priority = 0);
	virtual Common::String getSystemLanguage() const;
};

#endif
#endif
