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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef BADA_FORM_H
#define BADA_FORM_H

#include <FApp.h>
#include <FUi.h>
#include <FSystem.h>
#include <FBase.h>
#include <FUiITouchEventListener.h>
#include <FUiITextEventListener.h>

#include "config.h"
#include "common/scummsys.h"
#include "common/events.h"
#include "common/queue.h"
#include "common/mutex.h"

//
// BadaAppForm
//
class BadaAppForm : public Osp::Ui::Controls::Form,
										public Osp::Ui::IOrientationEventListener,
										public Osp::Ui::ITouchEventListener,
										public Osp::Ui::IKeyEventListener,
										public Osp::Base::Runtime::IRunnable {
public:
	BadaAppForm();
	~BadaAppForm();

	result Construct();
	bool pollEvent(Common::Event &event);
	bool isClosing() { return _state == kClosingState; }
	void pushKey(Common::KeyCode keycode);
	void exitSystem();

private:
	Object *Run();
	result OnInitializing(void);
	result OnDraw(void);
	void OnOrientationChanged(const Osp::Ui::Control &source,
														Osp::Ui::OrientationStatus orientationStatus);
	void OnTouchDoublePressed(const Osp::Ui::Control &source,
														const Osp::Graphics::Point &currentPosition,
														const Osp::Ui::TouchEventInfo &touchInfo);
	void OnTouchFocusIn(const Osp::Ui::Control &source,
											const Osp::Graphics::Point &currentPosition,
											const Osp::Ui::TouchEventInfo &touchInfo);
	void OnTouchFocusOut(const Osp::Ui::Control &source,
											 const Osp::Graphics::Point &currentPosition,
											 const Osp::Ui::TouchEventInfo &touchInfo);
	void OnTouchLongPressed(const Osp::Ui::Control &source,
													const Osp::Graphics::Point &currentPosition,
													const Osp::Ui::TouchEventInfo &touchInfo);
	void OnTouchMoved(const Osp::Ui::Control &source,
										const Osp::Graphics::Point &currentPosition,
										const Osp::Ui::TouchEventInfo &touchInfo);
	void OnTouchPressed(const Osp::Ui::Control &source,
											const Osp::Graphics::Point &currentPosition,
											const Osp::Ui::TouchEventInfo &touchInfo);
	void OnTouchReleased(const Osp::Ui::Control &source,
											 const Osp::Graphics::Point &currentPosition,
											 const Osp::Ui::TouchEventInfo &touchInfo);
	void OnKeyLongPressed(const Osp::Ui::Control &source,
												Osp::Ui::KeyCode keyCode);
	void OnKeyPressed(const Osp::Ui::Control &source,
										Osp::Ui::KeyCode keyCode);
	void OnKeyReleased(const Osp::Ui::Control &source,
										 Osp::Ui::KeyCode keyCode);

	void pushEvent(Common::EventType type,
								 const Osp::Graphics::Point &currentPosition);
	void terminate();
	void setButtonShortcut();
	void setShortcut();
	void setVolume(bool up, bool minMax);
	void showKeypad();

	// event handling
	Osp::Base::Runtime::Thread *_gameThread;
	Osp::Base::Runtime::Mutex *_eventQueueLock;
	Common::Queue<Common::Event> _eventQueue;
	enum { kInitState, kActiveState, kClosingState, kDoneState, kErrorState } _state;
	enum { kLeftButton, kRightButtonOnce, kRightButton, kMoveOnly } _buttonState;
	enum { kControlMouse, kEscapeKey, kGameMenu, kShowKeypad, kSetVolume } _shortcut;
};

#endif
