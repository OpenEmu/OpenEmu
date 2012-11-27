package org.scummvm.scummvm;

import android.os.Handler;
import android.os.Message;
import android.content.Context;
import android.view.KeyEvent;
import android.view.KeyCharacterMap;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewConfiguration;
import android.view.GestureDetector;
import android.view.inputmethod.InputMethodManager;

public class ScummVMEvents implements
		android.view.View.OnKeyListener,
		android.view.View.OnTouchListener,
		android.view.GestureDetector.OnGestureListener,
		android.view.GestureDetector.OnDoubleTapListener {

	public static final int JE_SYS_KEY = 0;
	public static final int JE_KEY = 1;
	public static final int JE_DPAD = 2;
	public static final int JE_DOWN = 3;
	public static final int JE_SCROLL = 4;
	public static final int JE_TAP = 5;
	public static final int JE_DOUBLE_TAP = 6;
	public static final int JE_MULTI = 7;
	public static final int JE_BALL = 8;
	public static final int JE_LMB_DOWN = 9;
	public static final int JE_LMB_UP = 10;
	public static final int JE_RMB_DOWN = 11;
	public static final int JE_RMB_UP = 12;
	public static final int JE_MOUSE_MOVE = 13;
	public static final int JE_QUIT = 0x1000;

	final protected Context _context;
	final protected ScummVM _scummvm;
	final protected GestureDetector _gd;
	final protected int _longPress;
	final protected MouseHelper _mouseHelper;

	public ScummVMEvents(Context context, ScummVM scummvm, MouseHelper mouseHelper) {
		_context = context;
		_scummvm = scummvm;
		_mouseHelper = mouseHelper;

		_gd = new GestureDetector(context, this);
		_gd.setOnDoubleTapListener(this);
		_gd.setIsLongpressEnabled(false);

		_longPress = ViewConfiguration.getLongPressTimeout();
	}

	final public void sendQuitEvent() {
		_scummvm.pushEvent(JE_QUIT, 0, 0, 0, 0, 0);
	}

	public boolean onTrackballEvent(MotionEvent e) {
		_scummvm.pushEvent(JE_BALL, e.getAction(),
							(int)(e.getX() * e.getXPrecision() * 100),
							(int)(e.getY() * e.getYPrecision() * 100),
							0, 0);
		return true;
	}

	final static int MSG_MENU_LONG_PRESS = 1;

	final private Handler keyHandler = new Handler() {
		@Override
		public void handleMessage(Message msg) {
			if (msg.what == MSG_MENU_LONG_PRESS) {
				InputMethodManager imm = (InputMethodManager)
					_context.getSystemService(Context.INPUT_METHOD_SERVICE);

				if (imm != null)
					imm.toggleSoftInput(InputMethodManager.SHOW_FORCED, 0);
			}
		}
	};

	// OnKeyListener
	@Override
	final public boolean onKey(View v, int keyCode, KeyEvent e) {
		final int action = e.getAction();

		if (keyCode == 238) {
			// this (undocumented) event is sent when ACTION_HOVER_ENTER or ACTION_HOVER_EXIT occurs
			return false;
		}

		if (keyCode == KeyEvent.KEYCODE_BACK) {
			if (action != KeyEvent.ACTION_UP) {
				// only send event from back button on up event, since down event is sent on right mouse click and
				// cannot be caught (thus rmb click would send escape key first)
				return true;
			}

			if (_mouseHelper != null) {
				if (_mouseHelper.getRmbGuard()) {
					// right mouse button was just clicked which sends an extra back button press
					return true;
				}
			}
		}

		if (e.isSystem()) {
			// filter what we handle
			switch (keyCode) {
			case KeyEvent.KEYCODE_BACK:
			case KeyEvent.KEYCODE_MENU:
			case KeyEvent.KEYCODE_CAMERA:
			case KeyEvent.KEYCODE_SEARCH:
				break;

			default:
				return false;
			}

			// no repeats for system keys
			if (e.getRepeatCount() > 0)
				return false;

			// Have to reimplement hold-down-menu-brings-up-softkeybd
			// ourselves, since we are otherwise hijacking the menu key :(
			// See com.android.internal.policy.impl.PhoneWindow.onKeyDownPanel()
			// for the usual Android implementation of this feature.
			if (keyCode == KeyEvent.KEYCODE_MENU) {
				final boolean fired =
					!keyHandler.hasMessages(MSG_MENU_LONG_PRESS);

				keyHandler.removeMessages(MSG_MENU_LONG_PRESS);

				if (action == KeyEvent.ACTION_DOWN) {
					keyHandler.sendMessageDelayed(keyHandler.obtainMessage(
									MSG_MENU_LONG_PRESS), _longPress);
					return true;
				}

				if (fired)
					return true;

				// only send up events of the menu button to the native side
				if (action != KeyEvent.ACTION_UP)
					return true;
			}

			_scummvm.pushEvent(JE_SYS_KEY, action, keyCode, 0, 0, 0);

			return true;
		}

		// sequence of characters
		if (action == KeyEvent.ACTION_MULTIPLE &&
				keyCode == KeyEvent.KEYCODE_UNKNOWN) {
			final KeyCharacterMap m = KeyCharacterMap.load(e.getDeviceId());
			final KeyEvent[] es = m.getEvents(e.getCharacters().toCharArray());

			if (es == null)
				return true;

			for (KeyEvent s : es) {
				_scummvm.pushEvent(JE_KEY, s.getAction(), s.getKeyCode(),
					s.getUnicodeChar() & KeyCharacterMap.COMBINING_ACCENT_MASK,
					s.getMetaState(), s.getRepeatCount());
			}

			return true;
		}

		switch (keyCode) {
		case KeyEvent.KEYCODE_DPAD_UP:
		case KeyEvent.KEYCODE_DPAD_DOWN:
		case KeyEvent.KEYCODE_DPAD_LEFT:
		case KeyEvent.KEYCODE_DPAD_RIGHT:
		case KeyEvent.KEYCODE_DPAD_CENTER:
			_scummvm.pushEvent(JE_DPAD, action, keyCode,
								(int)(e.getEventTime() - e.getDownTime()),
								e.getRepeatCount(), 0);
			return true;
		}

		_scummvm.pushEvent(JE_KEY, action, keyCode,
					e.getUnicodeChar() & KeyCharacterMap.COMBINING_ACCENT_MASK,
					e.getMetaState(), e.getRepeatCount());

		return true;
	}

	// OnTouchListener
	@Override
	final public boolean onTouch(View v, MotionEvent e) {
		if (_mouseHelper != null) {
			boolean isMouse = MouseHelper.isMouse(e);
			if (isMouse) {
				// mouse button is pressed
				return _mouseHelper.onMouseEvent(e, false);
			}
		}

		final int action = e.getAction();

		// constants from APIv5:
		// (action & ACTION_POINTER_INDEX_MASK) >> ACTION_POINTER_INDEX_SHIFT
		final int pointer = (action & 0xff00) >> 8;

		if (pointer > 0) {
			_scummvm.pushEvent(JE_MULTI, pointer, action & 0xff, // ACTION_MASK
								(int)e.getX(), (int)e.getY(), 0);
			return true;
		}

		return _gd.onTouchEvent(e);
	}

	// OnGestureListener
	@Override
	final public boolean onDown(MotionEvent e) {
		_scummvm.pushEvent(JE_DOWN, (int)e.getX(), (int)e.getY(), 0, 0, 0);
		return true;
	}

	@Override
	final public boolean onFling(MotionEvent e1, MotionEvent e2,
									float velocityX, float velocityY) {
		//Log.d(ScummVM.LOG_TAG, String.format("onFling: %s -> %s (%.3f %.3f)",
		//										e1.toString(), e2.toString(),
		//										velocityX, velocityY));

		return true;
	}

	@Override
	final public void onLongPress(MotionEvent e) {
		// disabled, interferes with drag&drop
	}

	@Override
	final public boolean onScroll(MotionEvent e1, MotionEvent e2,
									float distanceX, float distanceY) {
		_scummvm.pushEvent(JE_SCROLL, (int)e1.getX(), (int)e1.getY(),
							(int)e2.getX(), (int)e2.getY(), 0);

		return true;
	}

	@Override
	final public void onShowPress(MotionEvent e) {
	}

	@Override
	final public boolean onSingleTapUp(MotionEvent e) {
		_scummvm.pushEvent(JE_TAP, (int)e.getX(), (int)e.getY(),
							(int)(e.getEventTime() - e.getDownTime()), 0, 0);

		return true;
	}

	// OnDoubleTapListener
	@Override
	final public boolean onDoubleTap(MotionEvent e) {
		return true;
	}

	@Override
	final public boolean onDoubleTapEvent(MotionEvent e) {
		_scummvm.pushEvent(JE_DOUBLE_TAP, (int)e.getX(), (int)e.getY(),
							e.getAction(), 0, 0);

		return true;
	}

	@Override
	final public boolean onSingleTapConfirmed(MotionEvent e) {
		return true;
	}
}
