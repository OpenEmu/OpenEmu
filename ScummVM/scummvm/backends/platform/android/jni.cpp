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

// Allow use of stuff in <time.h> and abort()
#define FORBIDDEN_SYMBOL_EXCEPTION_time_h
#define FORBIDDEN_SYMBOL_EXCEPTION_abort

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

#include "base/main.h"
#include "base/version.h"
#include "common/config-manager.h"
#include "common/error.h"
#include "common/textconsole.h"
#include "engines/engine.h"

#include "backends/platform/android/android.h"
#include "backends/platform/android/asset-archive.h"
#include "backends/platform/android/jni.h"

__attribute__ ((visibility("default")))
jint JNICALL JNI_OnLoad(JavaVM *vm, void *) {
	return JNI::onLoad(vm);
}

JavaVM *JNI::_vm = 0;
jobject JNI::_jobj = 0;
jobject JNI::_jobj_audio_track = 0;
jobject JNI::_jobj_egl = 0;
jobject JNI::_jobj_egl_display = 0;
jobject JNI::_jobj_egl_surface = 0;

Common::Archive *JNI::_asset_archive = 0;
OSystem_Android *JNI::_system = 0;

bool JNI::pause = false;
sem_t JNI::pause_sem = { 0 };

int JNI::surface_changeid = 0;
int JNI::egl_surface_width = 0;
int JNI::egl_surface_height = 0;
bool JNI::_ready_for_events = 0;

jmethodID JNI::_MID_getDPI = 0;
jmethodID JNI::_MID_displayMessageOnOSD = 0;
jmethodID JNI::_MID_setWindowCaption = 0;
jmethodID JNI::_MID_showVirtualKeyboard = 0;
jmethodID JNI::_MID_getSysArchives = 0;
jmethodID JNI::_MID_getPluginDirectories = 0;
jmethodID JNI::_MID_initSurface = 0;
jmethodID JNI::_MID_deinitSurface = 0;

jmethodID JNI::_MID_EGL10_eglSwapBuffers = 0;

jmethodID JNI::_MID_AudioTrack_flush = 0;
jmethodID JNI::_MID_AudioTrack_pause = 0;
jmethodID JNI::_MID_AudioTrack_play = 0;
jmethodID JNI::_MID_AudioTrack_stop = 0;
jmethodID JNI::_MID_AudioTrack_write = 0;

const JNINativeMethod JNI::_natives[] = {
	{ "create", "(Landroid/content/res/AssetManager;"
				"Ljavax/microedition/khronos/egl/EGL10;"
				"Ljavax/microedition/khronos/egl/EGLDisplay;"
				"Landroid/media/AudioTrack;II)V",
		(void *)JNI::create },
	{ "destroy", "()V",
		(void *)JNI::destroy },
	{ "setSurface", "(II)V",
		(void *)JNI::setSurface },
	{ "main", "([Ljava/lang/String;)I",
		(void *)JNI::main },
	{ "pushEvent", "(IIIIII)V",
		(void *)JNI::pushEvent },
	{ "enableZoning", "(Z)V",
		(void *)JNI::enableZoning },
	{ "setPause", "(Z)V",
		(void *)JNI::setPause }
};

JNI::JNI() {
}

JNI::~JNI() {
}

jint JNI::onLoad(JavaVM *vm) {
	_vm = vm;

	JNIEnv *env;

	if (_vm->GetEnv((void **)&env, JNI_VERSION_1_2))
		return JNI_ERR;

	jclass cls = env->FindClass("org/scummvm/scummvm/ScummVM");
	if (cls == 0)
		return JNI_ERR;

	if (env->RegisterNatives(cls, _natives, ARRAYSIZE(_natives)) < 0)
		return JNI_ERR;

	return JNI_VERSION_1_2;
}

JNIEnv *JNI::getEnv() {
	JNIEnv *env = 0;

	jint res = _vm->GetEnv((void **)&env, JNI_VERSION_1_2);

	if (res != JNI_OK) {
		LOGE("GetEnv() failed: %d", res);
		abort();
	}

	return env;
}

void JNI::attachThread() {
	JNIEnv *env = 0;

	jint res = _vm->AttachCurrentThread(&env, 0);

	if (res != JNI_OK) {
		LOGE("AttachCurrentThread() failed: %d", res);
		abort();
	}
}

void JNI::detachThread() {
	jint res = _vm->DetachCurrentThread();

	if (res != JNI_OK) {
		LOGE("DetachCurrentThread() failed: %d", res);
		abort();
	}
}

void JNI::setReadyForEvents(bool ready) {
	_ready_for_events = ready;
}

void JNI::throwByName(JNIEnv *env, const char *name, const char *msg) {
	jclass cls = env->FindClass(name);

	// if cls is 0, an exception has already been thrown
	if (cls != 0)
		env->ThrowNew(cls, msg);

	env->DeleteLocalRef(cls);
}

void JNI::throwRuntimeException(JNIEnv *env, const char *msg) {
	throwByName(env, "java/lang/RuntimeException", msg);
}

// calls to the dark side

void JNI::getDPI(float *values) {
	values[0] = 0.0;
	values[1] = 0.0;

	JNIEnv *env = JNI::getEnv();

	jfloatArray array = env->NewFloatArray(2);

	env->CallVoidMethod(_jobj, _MID_getDPI, array);

	if (env->ExceptionCheck()) {
		LOGE("Failed to get DPIs");

		env->ExceptionDescribe();
		env->ExceptionClear();
	} else {
		jfloat *res = env->GetFloatArrayElements(array, 0);

		if (res) {
			values[0] = res[0];
			values[1] = res[1];

			env->ReleaseFloatArrayElements(array, res, 0);
		}
	}

	env->DeleteLocalRef(array);
}

void JNI::displayMessageOnOSD(const char *msg) {
	JNIEnv *env = JNI::getEnv();
	jstring java_msg = env->NewStringUTF(msg);

	env->CallVoidMethod(_jobj, _MID_displayMessageOnOSD, java_msg);

	if (env->ExceptionCheck()) {
		LOGE("Failed to display OSD message");

		env->ExceptionDescribe();
		env->ExceptionClear();
	}

	env->DeleteLocalRef(java_msg);
}

void JNI::setWindowCaption(const char *caption) {
	JNIEnv *env = JNI::getEnv();
	jstring java_caption = env->NewStringUTF(caption);

	env->CallVoidMethod(_jobj, _MID_setWindowCaption, java_caption);

	if (env->ExceptionCheck()) {
		LOGE("Failed to set window caption");

		env->ExceptionDescribe();
		env->ExceptionClear();
	}

	env->DeleteLocalRef(java_caption);
}

void JNI::showVirtualKeyboard(bool enable) {
	JNIEnv *env = JNI::getEnv();

	env->CallVoidMethod(_jobj, _MID_showVirtualKeyboard, enable);

	if (env->ExceptionCheck()) {
		LOGE("Error trying to show virtual keyboard");

		env->ExceptionDescribe();
		env->ExceptionClear();
	}
}

void JNI::addSysArchivesToSearchSet(Common::SearchSet &s, int priority) {
	JNIEnv *env = JNI::getEnv();

	s.add("ASSET", _asset_archive, priority, false);

	jobjectArray array =
		(jobjectArray)env->CallObjectMethod(_jobj, _MID_getSysArchives);

	if (env->ExceptionCheck()) {
		LOGE("Error finding system archive path");

		env->ExceptionDescribe();
		env->ExceptionClear();

		return;
	}

	jsize size = env->GetArrayLength(array);
	for (jsize i = 0; i < size; ++i) {
		jstring path_obj = (jstring)env->GetObjectArrayElement(array, i);
		const char *path = env->GetStringUTFChars(path_obj, 0);

		if (path != 0) {
			s.addDirectory(path, path, priority);
			env->ReleaseStringUTFChars(path_obj, path);
		}

		env->DeleteLocalRef(path_obj);
	}
}

void JNI::getPluginDirectories(Common::FSList &dirs) {
	JNIEnv *env = JNI::getEnv();

	jobjectArray array =
		(jobjectArray)env->CallObjectMethod(_jobj, _MID_getPluginDirectories);

	if (env->ExceptionCheck()) {
		LOGE("Error finding plugin directories");

		env->ExceptionDescribe();
		env->ExceptionClear();

		return;
	}

	jsize size = env->GetArrayLength(array);
	for (jsize i = 0; i < size; ++i) {
		jstring path_obj = (jstring)env->GetObjectArrayElement(array, i);

		if (path_obj == 0)
			continue;

		const char *path = env->GetStringUTFChars(path_obj, 0);

		if (path == 0) {
			LOGE("Error getting string characters from plugin directory");

			env->ExceptionClear();
			env->DeleteLocalRef(path_obj);

			continue;
		}

		dirs.push_back(Common::FSNode(path));

		env->ReleaseStringUTFChars(path_obj, path);
		env->DeleteLocalRef(path_obj);
	}
}

bool JNI::initSurface() {
	JNIEnv *env = JNI::getEnv();

	jobject obj = env->CallObjectMethod(_jobj, _MID_initSurface);

	if (!obj || env->ExceptionCheck()) {
		LOGE("initSurface failed");

		env->ExceptionDescribe();
		env->ExceptionClear();

		return false;
	}

	_jobj_egl_surface = env->NewGlobalRef(obj);

	return true;
}

void JNI::deinitSurface() {
	JNIEnv *env = JNI::getEnv();

	env->CallVoidMethod(_jobj, _MID_deinitSurface);

	if (env->ExceptionCheck()) {
		LOGE("deinitSurface failed");

		env->ExceptionDescribe();
		env->ExceptionClear();
	}

	env->DeleteGlobalRef(_jobj_egl_surface);
	_jobj_egl_surface = 0;
}

void JNI::setAudioPause() {
	JNIEnv *env = JNI::getEnv();

	env->CallVoidMethod(_jobj_audio_track, _MID_AudioTrack_flush);

	if (env->ExceptionCheck()) {
		LOGE("Error flushing AudioTrack");

		env->ExceptionDescribe();
		env->ExceptionClear();
	}

	env->CallVoidMethod(_jobj_audio_track, _MID_AudioTrack_pause);

	if (env->ExceptionCheck()) {
		LOGE("Error setting AudioTrack: pause");

		env->ExceptionDescribe();
		env->ExceptionClear();
	}
}

void JNI::setAudioPlay() {
	JNIEnv *env = JNI::getEnv();

	env->CallVoidMethod(_jobj_audio_track, _MID_AudioTrack_play);

	if (env->ExceptionCheck()) {
		LOGE("Error setting AudioTrack: play");

		env->ExceptionDescribe();
		env->ExceptionClear();
	}
}

void JNI::setAudioStop() {
	JNIEnv *env = JNI::getEnv();

	env->CallVoidMethod(_jobj_audio_track, _MID_AudioTrack_stop);

	if (env->ExceptionCheck()) {
		LOGE("Error setting AudioTrack: stop");

		env->ExceptionDescribe();
		env->ExceptionClear();
	}
}

// natives for the dark side

void JNI::create(JNIEnv *env, jobject self, jobject asset_manager,
				jobject egl, jobject egl_display,
				jobject at, jint audio_sample_rate, jint audio_buffer_size) {
	LOGI(gScummVMFullVersion);

	assert(!_system);

	pause = false;
	// initial value of zero!
	sem_init(&pause_sem, 0, 0);

	_asset_archive = new AndroidAssetArchive(asset_manager);
	assert(_asset_archive);

	_system = new OSystem_Android(audio_sample_rate, audio_buffer_size);
	assert(_system);

	// weak global ref to allow class to be unloaded
	// ... except dalvik implements NewWeakGlobalRef only on froyo
	//_jobj = env->NewWeakGlobalRef(self);

	_jobj = env->NewGlobalRef(self);

	jclass cls = env->GetObjectClass(_jobj);

#define FIND_METHOD(prefix, name, signature) do {							\
		_MID_ ## prefix ## name = env->GetMethodID(cls, #name, signature);	\
		if (_MID_ ## prefix ## name == 0)									\
			return;															\
	} while (0)

	FIND_METHOD(, setWindowCaption, "(Ljava/lang/String;)V");
	FIND_METHOD(, getDPI, "([F)V");
	FIND_METHOD(, displayMessageOnOSD, "(Ljava/lang/String;)V");
	FIND_METHOD(, showVirtualKeyboard, "(Z)V");
	FIND_METHOD(, getSysArchives, "()[Ljava/lang/String;");
	FIND_METHOD(, getPluginDirectories, "()[Ljava/lang/String;");
	FIND_METHOD(, initSurface, "()Ljavax/microedition/khronos/egl/EGLSurface;");
	FIND_METHOD(, deinitSurface, "()V");

	_jobj_egl = env->NewGlobalRef(egl);
	_jobj_egl_display = env->NewGlobalRef(egl_display);

	cls = env->GetObjectClass(_jobj_egl);

	FIND_METHOD(EGL10_, eglSwapBuffers,
				"(Ljavax/microedition/khronos/egl/EGLDisplay;"
				"Ljavax/microedition/khronos/egl/EGLSurface;)Z");

	_jobj_audio_track = env->NewGlobalRef(at);

	cls = env->GetObjectClass(_jobj_audio_track);

	FIND_METHOD(AudioTrack_, flush, "()V");
	FIND_METHOD(AudioTrack_, pause, "()V");
	FIND_METHOD(AudioTrack_, play, "()V");
	FIND_METHOD(AudioTrack_, stop, "()V");
	FIND_METHOD(AudioTrack_, write, "([BII)I");

#undef FIND_METHOD

	g_system = _system;
}

void JNI::destroy(JNIEnv *env, jobject self) {
	delete _asset_archive;
	_asset_archive = 0;

	delete _system;
	g_system = 0;
	_system = 0;

	sem_destroy(&pause_sem);

	// see above
	//JNI::getEnv()->DeleteWeakGlobalRef(_jobj);

	JNI::getEnv()->DeleteGlobalRef(_jobj_egl_display);
	JNI::getEnv()->DeleteGlobalRef(_jobj_egl);
	JNI::getEnv()->DeleteGlobalRef(_jobj_audio_track);
	JNI::getEnv()->DeleteGlobalRef(_jobj);
}

void JNI::setSurface(JNIEnv *env, jobject self, jint width, jint height) {
	egl_surface_width = width;
	egl_surface_height = height;
	surface_changeid++;
}

jint JNI::main(JNIEnv *env, jobject self, jobjectArray args) {
	assert(_system);

	const int MAX_NARGS = 32;
	int res = -1;

	int argc = env->GetArrayLength(args);
	if (argc > MAX_NARGS) {
		throwByName(env, "java/lang/IllegalArgumentException",
					"too many arguments");
		return 0;
	}

	char *argv[MAX_NARGS];

	// note use in cleanup loop below
	int nargs;

	for (nargs = 0; nargs < argc; ++nargs) {
		jstring arg = (jstring)env->GetObjectArrayElement(args, nargs);

		if (arg == 0) {
			argv[nargs] = 0;
		} else {
			const char *cstr = env->GetStringUTFChars(arg, 0);

			argv[nargs] = const_cast<char *>(cstr);

			// exception already thrown?
			if (cstr == 0)
				goto cleanup;
		}

		env->DeleteLocalRef(arg);
	}

#ifdef DYNAMIC_MODULES
	PluginManager::instance().addPluginProvider(new AndroidPluginProvider());
#endif

	LOGI("Entering scummvm_main with %d args", argc);

	res = scummvm_main(argc, argv);

	LOGI("scummvm_main exited with code %d", res);

	_system->quit();

cleanup:
	nargs--;

	for (int i = 0; i < nargs; ++i) {
		if (argv[i] == 0)
			continue;

		jstring arg = (jstring)env->GetObjectArrayElement(args, nargs);

		// Exception already thrown?
		if (arg == 0)
			return res;

		env->ReleaseStringUTFChars(arg, argv[i]);
		env->DeleteLocalRef(arg);
	}

	return res;
}

void JNI::pushEvent(JNIEnv *env, jobject self, int type, int arg1, int arg2,
					int arg3, int arg4, int arg5) {
	// drop events until we're ready and after we quit
	if (!_ready_for_events) {
		LOGW("dropping event");
		return;
	}

	assert(_system);

	_system->pushEvent(type, arg1, arg2, arg3, arg4, arg5);
}

void JNI::enableZoning(JNIEnv *env, jobject self, jboolean enable) {
	assert(_system);

	_system->enableZoning(enable);
}

void JNI::setPause(JNIEnv *env, jobject self, jboolean value) {
	if (!_system)
		return;

	if (g_engine) {
		LOGD("pauseEngine: %d", value);

		g_engine->pauseEngine(value);

		if (value &&
				g_engine->hasFeature(Engine::kSupportsSavingDuringRuntime) &&
				g_engine->canSaveGameStateCurrently())
			g_engine->saveGameState(0, "Android parachute");
	}

	pause = value;

	if (!pause) {
		// wake up all threads
		for (uint i = 0; i < 3; ++i)
			sem_post(&pause_sem);
	}
}

#endif
