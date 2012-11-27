package org.scummvm.scummvm;

import android.view.InputDevice;
import android.view.MotionEvent;
import android.view.SurfaceView;
import android.view.View;

/**
 * Contains helper methods for mouse/hover events that were introduced in Android 4.0.
 */
public class MouseHelper {
	private View.OnHoverListener _listener;
	private ScummVM _scummvm;
	private long _rmbGuardTime;
	private boolean _rmbPressed;
	private boolean _lmbPressed;

	/**
	 * Class initialization fails when this throws an exception.
	 * Checking hover availability is done on static class initialization for Android 1.6 compatibility.
	 */
	static {
		try {
			Class.forName("android.view.View$OnHoverListener");
		} catch (Exception ex) {
			throw new RuntimeException(ex);
		}
	}

	/**
	 * Calling this forces class initialization
	 */
	public static void checkHoverAvailable() {}

	public MouseHelper(ScummVM scummvm) {
		_scummvm = scummvm;
		_listener = createListener();
	}

	private View.OnHoverListener createListener() {
		return new View.OnHoverListener() {
			@Override
			public boolean onHover(View view, MotionEvent e) {
				return onMouseEvent(e, true);
			}
		};
	}

	public void attach(SurfaceView main_surface) {
		main_surface.setOnHoverListener(_listener);
	}

	public static boolean isMouse(MotionEvent e) {
		if (e == null) {
			return false;
		}

		InputDevice device = e.getDevice();

		if (device == null) {
			return false;
		}

		int sources = device.getSources();

		return ((sources & InputDevice.SOURCE_MOUSE) == InputDevice.SOURCE_MOUSE) ||
		       ((sources & InputDevice.SOURCE_STYLUS) == InputDevice.SOURCE_STYLUS) ||
		       ((sources & InputDevice.SOURCE_TOUCHPAD) == InputDevice.SOURCE_TOUCHPAD);
	}

	public boolean onMouseEvent(MotionEvent e, boolean hover) {
		_scummvm.pushEvent(ScummVMEvents.JE_MOUSE_MOVE, (int)e.getX(), (int)e.getY(), 0, 0, 0);
		
		int buttonState = e.getButtonState();

		boolean lmbDown = (buttonState & MotionEvent.BUTTON_PRIMARY) == MotionEvent.BUTTON_PRIMARY;

		if (!hover && e.getAction() != MotionEvent.ACTION_UP && buttonState == 0) {
			// On some device types, ButtonState is 0 even when tapping on the touchpad or using the stylus on the screen etc.
			lmbDown = true;
		}

		if (lmbDown) {
			if (!_lmbPressed) {
				// left mouse button was pressed just now
				_scummvm.pushEvent(ScummVMEvents.JE_LMB_DOWN, (int)e.getX(), (int)e.getY(), e.getButtonState(), 0, 0);
			}

			_lmbPressed = true;
		} else {
			if (_lmbPressed) {
				// left mouse button was released just now
				_scummvm.pushEvent(ScummVMEvents.JE_LMB_UP, (int)e.getX(), (int)e.getY(), e.getButtonState(), 0, 0);
			}

			_lmbPressed = false;
		}

		boolean rmbDown = (buttonState & MotionEvent.BUTTON_SECONDARY) == MotionEvent.BUTTON_SECONDARY;
		if (rmbDown) {
			if (!_rmbPressed) {
				// right mouse button was pressed just now
				_scummvm.pushEvent(ScummVMEvents.JE_RMB_DOWN, (int)e.getX(), (int)e.getY(), e.getButtonState(), 0, 0);
			}

			_rmbPressed = true;
		} else {
			if (_rmbPressed) {
				// right mouse button was released just now
				_scummvm.pushEvent(ScummVMEvents.JE_RMB_UP, (int)e.getX(), (int)e.getY(), e.getButtonState(), 0, 0);
				_rmbGuardTime = System.currentTimeMillis();
			}

			_rmbPressed = false;
		}

		return true;
	}

	/**
	 * Checks whether right mouse button is pressed or was pressed just previously. This is used to prevent sending
	 * extra back key on right mouse click which is the default behaviour in some platforms.
	 *
	 * @return true if right mouse button is (or was in the last 200ms) pressed
	 */
	public boolean getRmbGuard() {
		return _rmbPressed || _rmbGuardTime + 200 > System.currentTimeMillis();
	}
}
