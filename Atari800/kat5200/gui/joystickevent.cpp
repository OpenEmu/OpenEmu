/******************************************************************************
*
* FILENAME: joystickevent.cpp
*
* DESCRIPTION:  My own version of Guichan's Event class to handle joysticks
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.6.0   12/15/07  bberlin      Creation
******************************************************************************/
#include "joystickevent.hpp"

namespace gcn
{
    JoystickEvent::JoystickEvent(Widget* source,
                           bool isShiftPressed,
                           bool isControlPressed,
                           bool isAltPressed,
                           bool isMetaPressed,
                           unsigned int type,
						   int x,
						   int y,
                           int device,
                           int part,
                           int part_type,
                           int value)
            :InputEvent(source,
                        isShiftPressed,
                        isControlPressed,
                        isAltPressed,
                        isMetaPressed),
             mType(type),
			 mX(x),
			 mY(y),
             mDevice(device),
             mPart(part),
             mPartType(part_type),
             mValue(value)
    {

    }

	JoystickEvent::~JoystickEvent()
	{
	}

    int JoystickEvent::getDevice() const
    {
        return mDevice;
    }

    int JoystickEvent::getPart() const
    {
        return mPart;
    }

    int JoystickEvent::getType() const
    {
        return mType;
    }

    int JoystickEvent::getPartType() const
    {
        return mPartType;
    }

    int JoystickEvent::getValue() const
    {
        return mValue;
    }

    int JoystickEvent::getX() const
    {
        return mX;
    }

    int JoystickEvent::getY() const
    {
        return mY;
    }
}
