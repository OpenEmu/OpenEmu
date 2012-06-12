/******************************************************************************
*
* FILENAME: inputautolistener.cpp
*
* DESCRIPTION:  Contains action functions for the auto input window
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.5.0   12/18/06  bberlin      Creation
******************************************************************************/
#include "inputautolistener.hpp"
#include "window.hpp"

InputAutoListener::InputAutoListener ()
{
	mCountModKey = true;
	mWindow = 0;
}

InputAutoListener::~InputAutoListener ()
{
}

void InputAutoListener::setWindow( gcn::Window *window )
{
	mWindow = window;
	mouseX = -999;
	mouseY = -999;
}

void InputAutoListener::keyPressed (gcn::KeyEvent &keyEvent)
{
	gcn::GuiWindow *window = (gcn::GuiWindow *)mWindow;

	if ( mCountModKey == false )
	{
		switch ( keyEvent.getKey().getValue() )
		{
			case gcn::Key::LEFT_ALT:
			case gcn::Key::RIGHT_ALT:
			case gcn::Key::LEFT_CONTROL:
			case gcn::Key::RIGHT_CONTROL:
			case gcn::Key::LEFT_SHIFT:
			case gcn::Key::RIGHT_SHIFT:
			case gcn::Key::LEFT_META:
			case gcn::Key::RIGHT_META:
				return;
				break;
			default:
				break;
		}
	}

	mDevice = DEV_KEYBOARD;
	mPartType = PART_TYPE_KEY;
	mPartNum = convertKeyCharacter ( keyEvent );
	mDirection = DIR_PRESS;

	if ( keyEvent.isAltPressed() == true )
		mDeviceNum = KMOD_ALT;
	else if ( keyEvent.isControlPressed() == true )
		mDeviceNum = KMOD_CTRL;
	else if ( keyEvent.isShiftPressed() == true )
		mDeviceNum = KMOD_SHIFT;
	else if ( keyEvent.isMetaPressed() == true )
		mDeviceNum = KMOD_META;
	else
		mDeviceNum = 0;

	window->generateInputAction();
}

void InputAutoListener::mousePressed(gcn::MouseEvent& mouseEvent)
{
	gcn::GuiWindow *window = (gcn::GuiWindow *)mWindow;

	mDevice = DEV_MOUSE;
	mDeviceNum = 0;
	mPartType = PART_TYPE_BUTTON;
	mPartNum = convertMouseButton ( mouseEvent.getButton() );
	mDirection = DIR_PRESS;

	window->generateInputAction();
}

void InputAutoListener::mouseMoved(gcn::MouseEvent& mouseEvent)
{
	int delta_x, delta_y;
	gcn::GuiWindow *window = (gcn::GuiWindow *)mWindow;

	delta_x = mouseEvent.getX()-mouseX;
	delta_y = mouseEvent.getY()-mouseY;

	if ( mouseX == -999 ) {
		delta_x = 0;
		delta_y = 0;
	}

	mouseX = mouseEvent.getX();
	mouseY = mouseEvent.getY();

	if ( (abs(delta_x) < 4) && (abs(delta_y) < 4) )
		return;

	mDevice = DEV_MOUSE;
	mDeviceNum = 0;
	mPartType = PART_TYPE_AXIS;
	if ( abs(delta_x) > abs(delta_y) )
	{
		mPartNum = 0;
		if ( delta_x < 0 )
			mDirection = DIR_MINUS;
		else
			mDirection = DIR_PLUS;
	}
	else
	{
		mPartNum = 1;
		if ( delta_y < 0 )
			mDirection = DIR_MINUS;
		else
			mDirection = DIR_PLUS;
	}

	window->generateInputAction();
}

void InputAutoListener::joyEvent(gcn::JoystickEvent& ji)
{
	gcn::GuiWindow *window = (gcn::GuiWindow *)mWindow;

	switch ( ji.getType() ) {
		case gcn::JoystickEvent::AXIS:
			if ( abs(ji.getValue()) < 16000 )
				return;
			if ( ji.getValue() < 0 )
				mDirection = DIR_MINUS;
			else
				mDirection = DIR_PLUS;
			mPartType = PART_TYPE_AXIS;
		break;
		case gcn::JoystickEvent::BALL:
			if ( (abs(ji.getValue()) < 5) )
				return;
			mPartType = PART_TYPE_BALL;
		break;
		case gcn::JoystickEvent::BUTTON:
			mDirection = DIR_PRESS;
			mPartType = PART_TYPE_BUTTON;
		break;
		case gcn::JoystickEvent::HAT:
			if ( ji.getValue() & SDL_HAT_LEFT )
				mDirection = DIR_LEFT;
			else if ( ji.getValue() & SDL_HAT_RIGHT )
				mDirection = DIR_RIGHT;
			else if ( ji.getValue() & SDL_HAT_UP )
				mDirection = DIR_UP;
			else if ( ji.getValue() & SDL_HAT_DOWN )
				mDirection = DIR_DOWN;
			mPartType = PART_TYPE_HAT;
		break;
	}

	mDevice = DEV_JOYSTICK;
	mDeviceNum = ji.getDevice();
	mPartNum = ji.getPart();

	window->generateInputAction();
}

void InputAutoListener::getInput( int player, e_dev_type *device, 
                                  int *device_num, e_part_type *part_type, 
                                  int *part_num, e_direction *direction )
{
	*device = mDevice;
	*device_num = mDeviceNum;
	*part_type = mPartType;
	*part_num = mPartNum;
	*direction = mDirection;
}

int InputAutoListener::convertMouseButton(int button)
{
    switch (button)
    {
	  case gcn::MouseEvent::LEFT:
          return SDL_BUTTON_LEFT;
          break;
	  case gcn::MouseEvent::RIGHT:
          return SDL_BUTTON_RIGHT;
          break;
	  case gcn::MouseEvent::MIDDLE:
          return SDL_BUTTON_MIDDLE;
          break;
    }

    throw GCN_EXCEPTION("Unknown GuiChan mouse type.");

    return 0;
}

void InputAutoListener::setMousePosition(int x, int y)
{
	mouseX = x;
	mouseY = y;
}

int InputAutoListener::convertKeyCharacter(gcn::KeyEvent &keyEvent)
{
    int value = 0;
	gcn::Key key = keyEvent.getKey();

    if ( keyEvent.isNumericPad() == true )
    {
		if ( (key.getValue() >= '1') && (key.getValue() <= '9') )
			return SDLK_KP0 + (key.getValue() - '1');
		if ( key.getValue() == '.' )
			return SDLK_KP_PERIOD;
		if ( key.getValue() == '/' )
			return SDLK_KP_DIVIDE;
		if ( key.getValue() == '*' )
			return SDLK_KP_MULTIPLY;
		if ( key.getValue() == '-' )
			return SDLK_KP_MINUS;
		if ( key.getValue() == '+' )
			return SDLK_KP_PLUS;
		if ( key.getValue() == '=' )
			return SDLK_KP_EQUALS;
		if ( key.getValue() == gcn::Key::ENTER )
			return SDLK_KP_ENTER;

		switch ( key.getValue() )
		{
              case gcn::Key::INSERT:
                  return SDLK_KP0;
                  break;
              case gcn::Key::END:
                  return SDLK_KP1;
                  break;
              case gcn::Key::DOWN:
                  return SDLK_KP2;
                  break;
              case gcn::Key::PAGE_DOWN:
                  return SDLK_KP3;
                  break;
              case gcn::Key::LEFT:
                  return SDLK_KP4;
                  break;
              case 0:
                  return SDLK_KP5;
                  break;
              case gcn::Key::RIGHT:
                  return SDLK_KP6;
                  break;
              case gcn::Key::HOME:
                  return SDLK_KP7;
                  break;
              case gcn::Key::UP:
                  return SDLK_KP8;
                  break;
              case gcn::Key::PAGE_UP:
                  return SDLK_KP9;
                  break;
		}
    }

    switch (key.getValue())
    {
	  case gcn::Key::TAB:
          return SDLK_TAB;
          break;
	  case gcn::Key::LEFT_ALT:
      	  return SDLK_LALT;
          break;
	  case gcn::Key::RIGHT_ALT:
          return SDLK_RALT;
          break;
      case gcn::Key::LEFT_SHIFT:
          return SDLK_LSHIFT;
          break;
      case gcn::Key::RIGHT_SHIFT:
          return SDLK_RSHIFT;
          break;
      case gcn::Key::LEFT_CONTROL:
          return SDLK_LCTRL;
          break;
      case gcn::Key::RIGHT_CONTROL:
          return SDLK_RCTRL;
          break;
      case gcn::Key::BACKSPACE:
          return SDLK_BACKSPACE;
          break;
      case gcn::Key::PAUSE:
          return SDLK_PAUSE;
          break;
      case gcn::Key::SPACE:
          return SDLK_SPACE;
          break;
      case gcn::Key::ESCAPE:
          return SDLK_ESCAPE;
          break;
      case gcn::Key::DELETE:
          return SDLK_DELETE;
          break;
      case gcn::Key::INSERT:
          return SDLK_INSERT;
          break;
      case gcn::Key::HOME:
          return SDLK_HOME;
          break;
      case gcn::Key::END:
          return SDLK_END;
          break;
      case gcn::Key::PAGE_UP:
          return SDLK_PAGEUP;
          break;
      case gcn::Key::PRINT_SCREEN:
          return SDLK_PRINT;
          break;
      case gcn::Key::PAGE_DOWN:
          return SDLK_PAGEDOWN;
          break;
      case gcn::Key::F1:
          return SDLK_F1;
          break;
      case gcn::Key::F2:
          return SDLK_F2;
          break;
      case gcn::Key::F3:
          return SDLK_F3;
          break;
      case gcn::Key::F4:
          return SDLK_F4;
          break;
      case gcn::Key::F5:
          return SDLK_F5;
          break;
      case gcn::Key::F6:
          return SDLK_F6;
          break;
      case gcn::Key::F7:
          return SDLK_F7;
          break;
      case gcn::Key::F8:
          return SDLK_F8;
          break;
      case gcn::Key::F9:
          return SDLK_F9;
          break;
      case gcn::Key::F10:
          return SDLK_F10;
          break;
      case gcn::Key::F11:
          return SDLK_F11;
          break;
      case gcn::Key::F12:
          return SDLK_F12;
          break;
      case gcn::Key::F13:
          return SDLK_F13;
          break;
      case gcn::Key::F14:
          return SDLK_F14;
          break;
      case gcn::Key::F15:
          return SDLK_F15;
          break;
      case gcn::Key::NUM_LOCK:
          return SDLK_NUMLOCK;
          break;
      case gcn::Key::CAPS_LOCK:
          return SDLK_CAPSLOCK;
          break;
      case gcn::Key::SCROLL_LOCK:
          return SDLK_SCROLLOCK;
          break;
      case gcn::Key::RIGHT_META:
          return SDLK_RMETA;
          break;
      case gcn::Key::LEFT_META:
          return SDLK_LMETA;
          break;
      case gcn::Key::LEFT_SUPER:
          return SDLK_LSUPER;
          break;
      case gcn::Key::RIGHT_SUPER:
          return SDLK_RSUPER;
          break;
      case gcn::Key::ALT_GR:
          return SDLK_MODE;
          break;
      case gcn::Key::UP:
          return SDLK_UP;
          break;
      case gcn::Key::DOWN:
          return SDLK_DOWN;
          break;
      case gcn::Key::LEFT:
          return SDLK_LEFT;
          break;
      case gcn::Key::RIGHT:
          return SDLK_RIGHT;
          break;
      case gcn::Key::ENTER:
          return SDLK_RETURN;
          break;
    }

	return key.getValue();
}
