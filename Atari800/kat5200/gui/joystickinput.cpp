/******************************************************************************
*
* FILENAME: joystickinput.cpp
*
* DESCRIPTION:  My own version of Guichan's Input class to handle joysticks
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.5.0   09/20/06  bberlin      Creation
******************************************************************************/
#include "joystickinput.hpp"

namespace gcn
{
    JoystickInput::JoystickInput(int device, int part, int type, int value, int timeStamp)
    {
        mDevice = device;
        mPart = part;
        mType = type;
        mValue = value;
        mTimeStamp = timeStamp;
    }

    void JoystickInput::setDevice(int device)
    {
        mDevice = device;
    }

    int JoystickInput::getDevice() const
    {
        return mDevice;
    }

    void JoystickInput::setPart(int part)
    {
        mPart = part;
    }

    int JoystickInput::getPart() const
    {
        return mPart;
    }

    void JoystickInput::setType(int type)
    {
        mType = type;
    }

    int JoystickInput::getType() const
    {
        return mType;
    }

    void JoystickInput::setValue(int value)
    {
        mValue = value;
    }

    int JoystickInput::getValue() const
    {
        return mValue;
    }

    void JoystickInput::setX(int value)
    {
        mxValue = value;
    }

    int JoystickInput::getX() const
    {
        return mxValue;
    }

    void JoystickInput::setY(int value)
    {
        myValue = value;
    }

    int JoystickInput::getY() const
    {
        return myValue;
    }

    int JoystickInput::getTimeStamp() const
    {
        return mTimeStamp;
    }

    void JoystickInput::setTimeStamp(int timeStamp)
    {
        mTimeStamp = timeStamp;
    }
}
