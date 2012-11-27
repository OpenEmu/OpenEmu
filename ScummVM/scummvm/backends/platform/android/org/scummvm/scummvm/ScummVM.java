package org.scummvm.scummvm;

import android.util.Log;
import android.content.res.AssetManager;
import android.view.SurfaceHolder;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;

import javax.microedition.khronos.opengles.GL10;
import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.egl.EGLSurface;

import java.io.File;
import java.util.LinkedHashMap;

public abstract class ScummVM implements SurfaceHolder.Callback, Runnable {
	final protected static String LOG_TAG = "ScummVM";
	final private AssetManager _asset_manager;
	final private Object _sem_surface;

	private EGL10 _egl;
	private EGLDisplay _egl_display = EGL10.EGL_NO_DISPLAY;
	private EGLConfig _egl_config;
	private EGLContext _egl_context = EGL10.EGL_NO_CONTEXT;
	private EGLSurface _egl_surface = EGL10.EGL_NO_SURFACE;

	private SurfaceHolder _surface_holder;
	private AudioTrack _audio_track;
	private int _sample_rate = 0;
	private int _buffer_size = 0;

	private String[] _args;

	final private native void create(AssetManager asset_manager,
										EGL10 egl, EGLDisplay egl_display,
										AudioTrack audio_track,
										int sample_rate, int buffer_size);
	final private native void destroy();
	final private native void setSurface(int width, int height);
	final private native int main(String[] args);

	// pause the engine and all native threads
	final public native void setPause(boolean pause);
	final public native void enableZoning(boolean enable);
	// Feed an event to ScummVM.  Safe to call from other threads.
	final public native void pushEvent(int type, int arg1, int arg2, int arg3,
										int arg4, int arg5);

	// Callbacks from C++ peer instance
	abstract protected void getDPI(float[] values);
	abstract protected void displayMessageOnOSD(String msg);
	abstract protected void setWindowCaption(String caption);
	abstract protected String[] getPluginDirectories();
	abstract protected void showVirtualKeyboard(boolean enable);
	abstract protected String[] getSysArchives();

	public ScummVM(AssetManager asset_manager, SurfaceHolder holder) {
		_asset_manager = asset_manager;
		_sem_surface = new Object();

		holder.addCallback(this);
	}

	// SurfaceHolder callback
	final public void surfaceCreated(SurfaceHolder holder) {
		Log.d(LOG_TAG, "surfaceCreated");

		// no need to do anything, surfaceChanged() will be called in any case
	}

	// SurfaceHolder callback
	final public void surfaceChanged(SurfaceHolder holder, int format,
										int width, int height) {
		// the orientation may reset on standby mode and the theme manager
		// could assert when using a portrait resolution. so lets not do that.
		if (height > width) {
			Log.d(LOG_TAG, String.format("Ignoring surfaceChanged: %dx%d (%d)",
											width, height, format));
			return;
		}

		Log.d(LOG_TAG, String.format("surfaceChanged: %dx%d (%d)",
										width, height, format));

		synchronized(_sem_surface) {
			_surface_holder = holder;
			_sem_surface.notifyAll();
		}

		// store values for the native code
		setSurface(width, height);
	}

	// SurfaceHolder callback
	final public void surfaceDestroyed(SurfaceHolder holder) {
		Log.d(LOG_TAG, "surfaceDestroyed");

		synchronized(_sem_surface) {
			_surface_holder = null;
			_sem_surface.notifyAll();
		}

		// clear values for the native code
		setSurface(0, 0);
	}

	final public void setArgs(String[] args) {
		_args = args;
	}

	final public void run() {
		try {
			initAudio();
			initEGL();

			// wait for the surfaceChanged callback
			synchronized(_sem_surface) {
				while (_surface_holder == null)
					_sem_surface.wait();
			}
		} catch (Exception e) {
			deinitEGL();
			deinitAudio();

			throw new RuntimeException("Error preparing the ScummVM thread", e);
		}

		create(_asset_manager, _egl, _egl_display,
				_audio_track, _sample_rate, _buffer_size);

		int res = main(_args);

		destroy();

		deinitEGL();
		deinitAudio();

		// On exit, tear everything down for a fresh restart next time.
		System.exit(res);
	}

	final private void initEGL() throws Exception {
		_egl = (EGL10)EGLContext.getEGL();
		_egl_display = _egl.eglGetDisplay(EGL10.EGL_DEFAULT_DISPLAY);

		int[] version = new int[2];
		_egl.eglInitialize(_egl_display, version);

		int[] num_config = new int[1];
		_egl.eglGetConfigs(_egl_display, null, 0, num_config);

		final int numConfigs = num_config[0];

		if (numConfigs <= 0)
			throw new IllegalArgumentException("No EGL configs");

		EGLConfig[] configs = new EGLConfig[numConfigs];
		_egl.eglGetConfigs(_egl_display, configs, numConfigs, num_config);

		// Android's eglChooseConfig is busted in several versions and
		// devices so we have to filter/rank the configs ourselves.
		_egl_config = chooseEglConfig(configs);

		_egl_context = _egl.eglCreateContext(_egl_display, _egl_config,
											EGL10.EGL_NO_CONTEXT, null);

		if (_egl_context == EGL10.EGL_NO_CONTEXT)
			throw new Exception(String.format("Failed to create context: 0x%x",
												_egl.eglGetError()));
	}

	// Callback from C++ peer instance
	final protected EGLSurface initSurface() throws Exception {
		_egl_surface = _egl.eglCreateWindowSurface(_egl_display, _egl_config,
													_surface_holder, null);

		if (_egl_surface == EGL10.EGL_NO_SURFACE)
			throw new Exception(String.format(
					"eglCreateWindowSurface failed: 0x%x", _egl.eglGetError()));

		_egl.eglMakeCurrent(_egl_display, _egl_surface, _egl_surface,
							_egl_context);

		GL10 gl = (GL10)_egl_context.getGL();

		Log.i(LOG_TAG, String.format("Using EGL %s (%s); GL %s/%s (%s)",
						_egl.eglQueryString(_egl_display, EGL10.EGL_VERSION),
						_egl.eglQueryString(_egl_display, EGL10.EGL_VENDOR),
						gl.glGetString(GL10.GL_VERSION),
						gl.glGetString(GL10.GL_RENDERER),
						gl.glGetString(GL10.GL_VENDOR)));

		return _egl_surface;
	}

	// Callback from C++ peer instance
	final protected void deinitSurface() {
		if (_egl_display != EGL10.EGL_NO_DISPLAY) {
			_egl.eglMakeCurrent(_egl_display, EGL10.EGL_NO_SURFACE,
								EGL10.EGL_NO_SURFACE, EGL10.EGL_NO_CONTEXT);

			if (_egl_surface != EGL10.EGL_NO_SURFACE)
				_egl.eglDestroySurface(_egl_display, _egl_surface);
		}

		_egl_surface = EGL10.EGL_NO_SURFACE;
	}

	final private void deinitEGL() {
		if (_egl_display != EGL10.EGL_NO_DISPLAY) {
			_egl.eglMakeCurrent(_egl_display, EGL10.EGL_NO_SURFACE,
								EGL10.EGL_NO_SURFACE, EGL10.EGL_NO_CONTEXT);

			if (_egl_surface != EGL10.EGL_NO_SURFACE)
				_egl.eglDestroySurface(_egl_display, _egl_surface);

			if (_egl_context != EGL10.EGL_NO_CONTEXT)
				_egl.eglDestroyContext(_egl_display, _egl_context);

			_egl.eglTerminate(_egl_display);
		}

		_egl_surface = EGL10.EGL_NO_SURFACE;
		_egl_context = EGL10.EGL_NO_CONTEXT;
		_egl_config = null;
		_egl_display = EGL10.EGL_NO_DISPLAY;
		_egl = null;
	}

	final private void initAudio() throws Exception {
		_sample_rate = AudioTrack.getNativeOutputSampleRate(
									AudioManager.STREAM_MUSIC);
		_buffer_size = AudioTrack.getMinBufferSize(_sample_rate,
									AudioFormat.CHANNEL_CONFIGURATION_STEREO,
									AudioFormat.ENCODING_PCM_16BIT);

		// ~50ms
		int buffer_size_want = (_sample_rate * 2 * 2 / 20) & ~1023;

		if (_buffer_size < buffer_size_want) {
			Log.w(LOG_TAG, String.format(
				"adjusting audio buffer size (was: %d)", _buffer_size));

			_buffer_size = buffer_size_want;
		}

		Log.i(LOG_TAG, String.format("Using %d bytes buffer for %dHz audio",
										_buffer_size, _sample_rate));

		_audio_track = new AudioTrack(AudioManager.STREAM_MUSIC,
									_sample_rate,
									AudioFormat.CHANNEL_CONFIGURATION_STEREO,
									AudioFormat.ENCODING_PCM_16BIT,
									_buffer_size,
									AudioTrack.MODE_STREAM);

		if (_audio_track.getState() != AudioTrack.STATE_INITIALIZED)
			throw new Exception(
				String.format("Error initializing AudioTrack: %d",
								_audio_track.getState()));
	}

	final private void deinitAudio() {
		if (_audio_track != null)
			_audio_track.stop();

		_audio_track = null;
		_buffer_size = 0;
		_sample_rate = 0;
	}

	private static final int[] s_eglAttribs = {
		EGL10.EGL_CONFIG_ID,
		EGL10.EGL_BUFFER_SIZE,
		EGL10.EGL_RED_SIZE,
		EGL10.EGL_GREEN_SIZE,
		EGL10.EGL_BLUE_SIZE,
		EGL10.EGL_ALPHA_SIZE,
		EGL10.EGL_CONFIG_CAVEAT,
		EGL10.EGL_DEPTH_SIZE,
		EGL10.EGL_LEVEL,
		EGL10.EGL_MAX_PBUFFER_WIDTH,
		EGL10.EGL_MAX_PBUFFER_HEIGHT,
		EGL10.EGL_MAX_PBUFFER_PIXELS,
		EGL10.EGL_NATIVE_RENDERABLE,
		EGL10.EGL_NATIVE_VISUAL_ID,
		EGL10.EGL_NATIVE_VISUAL_TYPE,
		EGL10.EGL_SAMPLE_BUFFERS,
		EGL10.EGL_SAMPLES,
		EGL10.EGL_STENCIL_SIZE,
		EGL10.EGL_SURFACE_TYPE,
		EGL10.EGL_TRANSPARENT_TYPE,
		EGL10.EGL_TRANSPARENT_RED_VALUE,
		EGL10.EGL_TRANSPARENT_GREEN_VALUE,
		EGL10.EGL_TRANSPARENT_BLUE_VALUE
	};

	final private class EglAttribs extends LinkedHashMap<Integer, Integer> {
		public EglAttribs(EGLConfig config) {
			super(s_eglAttribs.length);

			int[] value = new int[1];

			for (int i : s_eglAttribs) {
				_egl.eglGetConfigAttrib(_egl_display, config, i, value);

				put(i, value[0]);
			}
		}

		private int weightBits(int attr, int size) {
			final int value = get(attr);

			int score = 0;

			if (value == size || (size > 0 && value > size))
				score += 10;

			// penalize for wasted bits
			score -= value - size;

			return score;
		}

		public int weight() {
			int score = 10000;

			if (get(EGL10.EGL_CONFIG_CAVEAT) != EGL10.EGL_NONE)
				score -= 1000;

			// less MSAA is better
			score -= get(EGL10.EGL_SAMPLES) * 100;

			// Must be at least 565, but then smaller is better
			score += weightBits(EGL10.EGL_RED_SIZE, 5);
			score += weightBits(EGL10.EGL_GREEN_SIZE, 6);
			score += weightBits(EGL10.EGL_BLUE_SIZE, 5);
			score += weightBits(EGL10.EGL_ALPHA_SIZE, 0);
			score += weightBits(EGL10.EGL_DEPTH_SIZE, 0);
			score += weightBits(EGL10.EGL_STENCIL_SIZE, 0);

			return score;
		}

		public String toString() {
			String s;

			if (get(EGL10.EGL_ALPHA_SIZE) > 0)
				s = String.format("[%d] RGBA%d%d%d%d",
									get(EGL10.EGL_CONFIG_ID),
									get(EGL10.EGL_RED_SIZE),
									get(EGL10.EGL_GREEN_SIZE),
									get(EGL10.EGL_BLUE_SIZE),
									get(EGL10.EGL_ALPHA_SIZE));
			else
				s = String.format("[%d] RGB%d%d%d",
									get(EGL10.EGL_CONFIG_ID),
									get(EGL10.EGL_RED_SIZE),
									get(EGL10.EGL_GREEN_SIZE),
									get(EGL10.EGL_BLUE_SIZE));

			if (get(EGL10.EGL_DEPTH_SIZE) > 0)
				s += String.format(" D%d", get(EGL10.EGL_DEPTH_SIZE));

			if (get(EGL10.EGL_STENCIL_SIZE) > 0)
				s += String.format(" S%d", get(EGL10.EGL_STENCIL_SIZE));

			if (get(EGL10.EGL_SAMPLES) > 0)
				s += String.format(" MSAAx%d", get(EGL10.EGL_SAMPLES));

			if ((get(EGL10.EGL_SURFACE_TYPE) & EGL10.EGL_WINDOW_BIT) > 0)
				s += " W";
			if ((get(EGL10.EGL_SURFACE_TYPE) & EGL10.EGL_PBUFFER_BIT) > 0)
				s += " P";
			if ((get(EGL10.EGL_SURFACE_TYPE) & EGL10.EGL_PIXMAP_BIT) > 0)
				s += " X";

			switch (get(EGL10.EGL_CONFIG_CAVEAT)) {
			case EGL10.EGL_NONE:
				break;

			case EGL10.EGL_SLOW_CONFIG:
				s += " SLOW";
				break;

			case EGL10.EGL_NON_CONFORMANT_CONFIG:
				s += " NON_CONFORMANT";

			default:
				s += String.format(" unknown CAVEAT 0x%x",
									get(EGL10.EGL_CONFIG_CAVEAT));
			}

			return s;
		}
	};

	final private EGLConfig chooseEglConfig(EGLConfig[] configs) {
		EGLConfig res = configs[0];
		int bestScore = -1;

		Log.d(LOG_TAG, "EGL configs:");

		for (EGLConfig config : configs) {
			EglAttribs attr = new EglAttribs(config);

			// must have
			if ((attr.get(EGL10.EGL_SURFACE_TYPE) & EGL10.EGL_WINDOW_BIT) == 0)
				continue;

			int score = attr.weight();

			Log.d(LOG_TAG, String.format("%s (%d)", attr.toString(), score));

			if (score > bestScore) {
				res = config;
				bestScore = score;
			}
		}

		if (bestScore < 0)
			Log.e(LOG_TAG,
					"Unable to find an acceptable EGL config, expect badness.");

		Log.d(LOG_TAG, String.format("Chosen EGL config: %s",
										new EglAttribs(res).toString()));

		return res;
	}

	static {
		// For grabbing with gdb...
		final boolean sleep_for_debugger = false;
		if (sleep_for_debugger) {
			try {
				Thread.sleep(20 * 1000);
			} catch (InterruptedException e) {
			}
		}

		File cache_dir = ScummVMApplication.getLastCacheDir();
		String libname = System.mapLibraryName("scummvm");
		File libpath = new File(cache_dir, libname);

		System.load(libpath.getPath());
	}
}
