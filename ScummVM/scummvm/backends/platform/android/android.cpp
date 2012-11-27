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

#if defined(__ANDROID__)

// Allow use of stuff in <time.h>
#define FORBIDDEN_SYMBOL_EXCEPTION_time_h

// Disable printf override in common/forbidden.h to avoid
// clashes with log.h from the Android SDK.
// That header file uses
//   __attribute__ ((format(printf, 3, 4)))
// which gets messed up by our override mechanism; this could
// be avoided by either changing the Android SDK to use the equally
// legal and valid
//   __attribute__ ((format(printf, 3, 4)))
// or by refining our printf override to use a varadic macro
// (which then wouldn't be portable, though).
// Anyway, for now we just disable the printf override globally
// for the Android port
#define FORBIDDEN_SYMBOL_EXCEPTION_printf

#include <sys/time.h>
#include <sys/resource.h>
#include <sys/system_properties.h>
#include <time.h>
#include <unistd.h>

#include "common/util.h"
#include "common/textconsole.h"
#include "common/rect.h"
#include "common/queue.h"
#include "common/mutex.h"
#include "common/events.h"
#include "common/config-manager.h"

#include "backends/keymapper/keymapper.h"
#include "backends/saves/default/default-saves.h"
#include "backends/timer/default/default-timer.h"

#include "backends/platform/android/jni.h"
#include "backends/platform/android/android.h"

const char *android_log_tag = "ScummVM";

// This replaces the bionic libc assert functions with something that
// actually prints the assertion failure before aborting.
extern "C" {
	void __assert(const char *file, int line, const char *expr) {
		__android_log_assert(expr, android_log_tag,
								"Assertion failure: '%s' in %s:%d",
								 expr, file, line);
	}

	void __assert2(const char *file, int line, const char *func,
					const char *expr) {
		__android_log_assert(expr, android_log_tag,
								"Assertion failure: '%s' in %s:%d (%s)",
								 expr, file, line, func);
	}
}

#ifdef ANDROID_DEBUG_GL
static const char *getGlErrStr(GLenum error) {
	switch (error) {
	case GL_INVALID_ENUM:
		return "GL_INVALID_ENUM";
	case GL_INVALID_VALUE:
		return "GL_INVALID_VALUE";
	case GL_INVALID_OPERATION:
		return "GL_INVALID_OPERATION";
	case GL_STACK_OVERFLOW:
		return "GL_STACK_OVERFLOW";
	case GL_STACK_UNDERFLOW:
		return "GL_STACK_UNDERFLOW";
	case GL_OUT_OF_MEMORY:
		return "GL_OUT_OF_MEMORY";
	}

	static char buf[40];
	snprintf(buf, sizeof(buf), "(Unknown GL error code 0x%x)", error);

	return buf;
}

void checkGlError(const char *expr, const char *file, int line) {
	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
		LOGE("GL ERROR: %s on %s (%s:%d)", getGlErrStr(error), expr, file, line);
}
#endif

OSystem_Android::OSystem_Android(int audio_sample_rate, int audio_buffer_size) :
	_audio_sample_rate(audio_sample_rate),
	_audio_buffer_size(audio_buffer_size),
	_screen_changeid(0),
	_egl_surface_width(0),
	_egl_surface_height(0),
	_htc_fail(false),
	_force_redraw(false),
	_game_texture(0),
	_overlay_texture(0),
	_mouse_texture(0),
	_mouse_texture_palette(0),
	_mouse_texture_rgb(0),
	_mouse_hotspot(),
	_mouse_keycolor(0),
	_use_mouse_palette(false),
	_graphicsMode(0),
	_fullscreen(true),
	_ar_correction(true),
	_show_mouse(false),
	_show_overlay(false),
	_enable_zoning(false),
	_mixer(0),
	_shake_offset(0),
	_queuedEventTime(0),
	_event_queue_lock(createMutex()),
	_touch_pt_down(),
	_touch_pt_scroll(),
	_touch_pt_dt(),
	_eventScaleX(100),
	_eventScaleY(100),
	// TODO put these values in some option dlg?
	_touchpad_mode(true),
	_touchpad_scale(66),
	_dpad_scale(4),
	_fingersDown(0),
	_trackball_scale(2) {

	_fsFactory = new POSIXFilesystemFactory();

	Common::String mf = getSystemProperty("ro.product.manufacturer");

	LOGI("Running on: [%s] [%s] [%s] [%s] [%s] SDK:%s ABI:%s",
			mf.c_str(),
			getSystemProperty("ro.product.model").c_str(),
			getSystemProperty("ro.product.brand").c_str(),
			getSystemProperty("ro.build.fingerprint").c_str(),
			getSystemProperty("ro.build.display.id").c_str(),
			getSystemProperty("ro.build.version.sdk").c_str(),
			getSystemProperty("ro.product.cpu.abi").c_str());

	mf.toLowercase();
	_htc_fail = mf.contains("htc");

	if (_htc_fail)
		LOGI("Enabling HTC workaround");
}

OSystem_Android::~OSystem_Android() {
	ENTER();

	delete _mixer;
	_mixer = 0;
	delete _fsFactory;
	_fsFactory = 0;
	delete _timerManager;
	_timerManager = 0;

	deleteMutex(_event_queue_lock);
}

void *OSystem_Android::timerThreadFunc(void *arg) {
	OSystem_Android *system = (OSystem_Android *)arg;
	DefaultTimerManager *timer = (DefaultTimerManager *)(system->_timerManager);

	// renice this thread to boost the audio thread
	if (setpriority(PRIO_PROCESS, 0, 19) < 0)
		LOGW("couldn't renice the timer thread");

	JNI::attachThread();

	struct timespec tv;
	tv.tv_sec = 0;
	tv.tv_nsec = 10 * 1000 * 1000; // 10ms

	while (!system->_timer_thread_exit) {
		if (JNI::pause) {
			LOGD("timer thread going to sleep");
			sem_wait(&JNI::pause_sem);
			LOGD("timer thread woke up");
		}

		timer->handler();
		nanosleep(&tv, 0);
	}

	JNI::detachThread();

	return 0;
}

void *OSystem_Android::audioThreadFunc(void *arg) {
	JNI::attachThread();

	OSystem_Android *system = (OSystem_Android *)arg;
	Audio::MixerImpl *mixer = system->_mixer;

	uint buf_size = system->_audio_buffer_size;

	JNIEnv *env = JNI::getEnv();

	jbyteArray bufa = env->NewByteArray(buf_size);

	bool paused = true;

	byte *buf;
	int offset, left, written;
	int samples, i;

	struct timespec tv_delay;
	tv_delay.tv_sec = 0;
	tv_delay.tv_nsec = 20 * 1000 * 1000;

	uint msecs_full = buf_size * 1000 / (mixer->getOutputRate() * 2 * 2);

	struct timespec tv_full;
	tv_full.tv_sec = 0;
	tv_full.tv_nsec = msecs_full * 1000 * 1000;

	bool silence;
	uint silence_count = 33;

	while (!system->_audio_thread_exit) {
		if (JNI::pause) {
			JNI::setAudioStop();

			paused = true;
			silence_count = 33;

			LOGD("audio thread going to sleep");
			sem_wait(&JNI::pause_sem);
			LOGD("audio thread woke up");
		}

		buf = (byte *)env->GetPrimitiveArrayCritical(bufa, 0);
		assert(buf);

		samples = mixer->mixCallback(buf, buf_size);

		silence = samples < 1;

		// looks stupid, and it is, but currently there's no way to detect
		// silence-only buffers from the mixer
		if (!silence) {
			silence = true;

			for (i = 0; i < samples; i += 2)
				// SID streams constant crap
				if (READ_UINT16(buf + i) > 32) {
					silence = false;
					break;
				}
		}

		env->ReleasePrimitiveArrayCritical(bufa, buf, 0);

		if (silence) {
			if (!paused)
				silence_count++;

			// only pause after a while to prevent toggle mania
			if (silence_count > 32) {
				if (!paused) {
					LOGD("AudioTrack pause");

					JNI::setAudioPause();
					paused = true;
				}

				nanosleep(&tv_full, 0);

				continue;
			}
		}

		if (paused) {
			LOGD("AudioTrack play");

			JNI::setAudioPlay();
			paused = false;

			silence_count = 0;
		}

		offset = 0;
		left = buf_size;
		written = 0;

		while (left > 0) {
			written = JNI::writeAudio(env, bufa, offset, left);

			if (written < 0) {
				LOGE("AudioTrack error: %d", written);
				break;
			}

			// buffer full
			if (written < left)
				nanosleep(&tv_delay, 0);

			offset += written;
			left -= written;
		}

		if (written < 0)
			break;

		// prepare the next buffer, and run into the blocking AudioTrack.write
	}

	JNI::setAudioStop();

	env->DeleteLocalRef(bufa);

	JNI::detachThread();

	return 0;
}

void OSystem_Android::initBackend() {
	ENTER();

	_main_thread = pthread_self();

	ConfMan.registerDefault("fullscreen", true);
	ConfMan.registerDefault("aspect_ratio", true);

	ConfMan.setInt("autosave_period", 0);
	ConfMan.setBool("FM_high_quality", false);
	ConfMan.setBool("FM_medium_quality", true);

	// TODO hackity hack
	if (ConfMan.hasKey("multi_midi"))
		_touchpad_mode = !ConfMan.getBool("multi_midi");

	// must happen before creating TimerManager to avoid race in
	// creating EventManager
	setupKeymapper();

	// BUG: "transient" ConfMan settings get nuked by the options
	// screen. Passing the savepath in this way makes it stick
	// (via ConfMan.registerDefault)
	_savefileManager = new DefaultSaveFileManager(ConfMan.get("savepath"));
	_timerManager = new DefaultTimerManager();

	gettimeofday(&_startTime, 0);

	_mixer = new Audio::MixerImpl(this, _audio_sample_rate);
	_mixer->setReady(true);

	_timer_thread_exit = false;
	pthread_create(&_timer_thread, 0, timerThreadFunc, this);

	_audio_thread_exit = false;
	pthread_create(&_audio_thread, 0, audioThreadFunc, this);

	initSurface();
	initViewport();

	_game_texture = new GLESFakePalette565Texture();
	_overlay_texture = new GLES4444Texture();
	_mouse_texture_palette = new GLESFakePalette5551Texture();
	_mouse_texture = _mouse_texture_palette;

	initOverlay();

	// renice this thread to boost the audio thread
	if (setpriority(PRIO_PROCESS, 0, 19) < 0)
		warning("couldn't renice the main thread");

	JNI::setReadyForEvents(true);

	EventsBaseBackend::initBackend();
}

void OSystem_Android::addPluginDirectories(Common::FSList &dirs) const {
	ENTER();

	JNI::getPluginDirectories(dirs);
}

bool OSystem_Android::hasFeature(Feature f) {
	return (f == kFeatureFullscreenMode ||
			f == kFeatureAspectRatioCorrection ||
			f == kFeatureCursorPalette ||
			f == kFeatureVirtualKeyboard ||
			f == kFeatureOverlaySupportsAlpha);
}

void OSystem_Android::setFeatureState(Feature f, bool enable) {
	ENTER("%d, %d", f, enable);

	switch (f) {
	case kFeatureFullscreenMode:
		_fullscreen = enable;
		updateScreenRect();
		break;
	case kFeatureAspectRatioCorrection:
		_ar_correction = enable;
		updateScreenRect();
		break;
	case kFeatureVirtualKeyboard:
		_virtkeybd_on = enable;
		showVirtualKeyboard(enable);
		break;
	case kFeatureCursorPalette:
		_use_mouse_palette = enable;
		if (!enable)
			disableCursorPalette();
		break;
	default:
		break;
	}
}

bool OSystem_Android::getFeatureState(Feature f) {
	switch (f) {
	case kFeatureFullscreenMode:
		return _fullscreen;
	case kFeatureAspectRatioCorrection:
		return _ar_correction;
	case kFeatureVirtualKeyboard:
		return _virtkeybd_on;
	case kFeatureCursorPalette:
		return _use_mouse_palette;
	default:
		return false;
	}
}

uint32 OSystem_Android::getMillis() {
	timeval curTime;

	gettimeofday(&curTime, 0);

	return (uint32)(((curTime.tv_sec - _startTime.tv_sec) * 1000) +
			((curTime.tv_usec - _startTime.tv_usec) / 1000));
}

void OSystem_Android::delayMillis(uint msecs) {
	usleep(msecs * 1000);
}

OSystem::MutexRef OSystem_Android::createMutex() {
	pthread_mutexattr_t attr;

	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);

	pthread_mutex_t *mutex = new pthread_mutex_t;

	if (pthread_mutex_init(mutex, &attr) != 0) {
		warning("pthread_mutex_init() failed");

		delete mutex;

		return 0;
	}

	return (MutexRef)mutex;
}

void OSystem_Android::lockMutex(MutexRef mutex) {
	if (pthread_mutex_lock((pthread_mutex_t *)mutex) != 0)
		warning("pthread_mutex_lock() failed");
}

void OSystem_Android::unlockMutex(MutexRef mutex) {
	if (pthread_mutex_unlock((pthread_mutex_t *)mutex) != 0)
		warning("pthread_mutex_unlock() failed");
}

void OSystem_Android::deleteMutex(MutexRef mutex) {
	pthread_mutex_t *m = (pthread_mutex_t *)mutex;

	if (pthread_mutex_destroy(m) != 0)
		warning("pthread_mutex_destroy() failed");
	else
		delete m;
}

void OSystem_Android::quit() {
	ENTER();

	JNI::setReadyForEvents(false);

	_audio_thread_exit = true;
	pthread_join(_audio_thread, 0);

	_timer_thread_exit = true;
	pthread_join(_timer_thread, 0);

	delete _game_texture;
	delete _overlay_texture;
	delete _mouse_texture_palette;
	delete _mouse_texture_rgb;

	deinitSurface();
}

void OSystem_Android::setWindowCaption(const char *caption) {
	ENTER("%s", caption);

	JNI::setWindowCaption(caption);
}

void OSystem_Android::displayMessageOnOSD(const char *msg) {
	ENTER("%s", msg);

	JNI::displayMessageOnOSD(msg);
}

void OSystem_Android::showVirtualKeyboard(bool enable) {
	ENTER("%d", enable);

	JNI::showVirtualKeyboard(enable);
}

Audio::Mixer *OSystem_Android::getMixer() {
	assert(_mixer);
	return _mixer;
}

void OSystem_Android::getTimeAndDate(TimeDate &td) const {
	struct tm tm;
	const time_t curTime = time(0);

	localtime_r(&curTime, &tm);
	td.tm_sec = tm.tm_sec;
	td.tm_min = tm.tm_min;
	td.tm_hour = tm.tm_hour;
	td.tm_mday = tm.tm_mday;
	td.tm_mon = tm.tm_mon;
	td.tm_year = tm.tm_year;
	td.tm_wday = tm.tm_wday;
}

void OSystem_Android::addSysArchivesToSearchSet(Common::SearchSet &s,
												int priority) {
	ENTER("");

	JNI::addSysArchivesToSearchSet(s, priority);
}

void OSystem_Android::logMessage(LogMessageType::Type type,
									const char *message) {
	switch (type) {
	case LogMessageType::kInfo:
		__android_log_write(ANDROID_LOG_INFO, android_log_tag, message);
		break;

	case LogMessageType::kDebug:
		__android_log_write(ANDROID_LOG_DEBUG, android_log_tag, message);
		break;

	case LogMessageType::kWarning:
		__android_log_write(ANDROID_LOG_WARN, android_log_tag, message);
		break;

	case LogMessageType::kError:
		__android_log_write(ANDROID_LOG_ERROR, android_log_tag, message);
		break;
	}
}

Common::String OSystem_Android::getSystemLanguage() const {
	return Common::String::format("%s_%s",
							getSystemProperty("persist.sys.language").c_str(),
							getSystemProperty("persist.sys.country").c_str());
}

Common::String OSystem_Android::getSystemProperty(const char *name) const {
	char value[PROP_VALUE_MAX];

	int len = __system_property_get(name, value);

	return Common::String(value, len);
}

#ifdef DYNAMIC_MODULES
void AndroidPluginProvider::addCustomDirectories(Common::FSList &dirs) const {
	((OSystem_Android *)g_system)->addPluginDirectories(dirs);
}
#endif

#endif
