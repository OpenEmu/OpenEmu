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
#ifndef GCN_JOYSTICKINPUT_HPP
#define GCN_JOYSTICKINPUT_HPP

#include "guichan/platform.hpp"

namespace gcn
{

    /**
     * Internal class representing joystick input. Generally you won't have to
     * bother using this class.
     */
    class JoystickInput
    {
    public:

        /**
         * Constructor.
         */
        JoystickInput(){};

        /**
         * Constructor.
         *
         */
        JoystickInput(int device, int part, int type, int value, int timeStamp);

        void setDevice(int device);
        int getDevice() const;

        void setPart(int part);
        int getPart() const;

        void setType(int type);
        int getType() const;

        void setValue(int type);
        int getValue() const;

        void setX(int type);
        int getX() const;

        void setY(int type);
        int getY() const;

        /**
         * Sets the timestamp for the input.
         *
         * @param timeStamp the timestamp of the input.
         */
        void setTimeStamp(int timeStamp);

        /**
         * Gets the time stamp of the input.
         *
         * @return the time stamp of the input.
         */
        int getTimeStamp() const;

        /**
         * Enum with joystick input values.
         */
        enum
        {
            EMPTY = 0,
            PRESS,
            RELEASE,
            LEFT,
            RIGHT,
            UP,
            DOWN,
            MOTION,
            HAT,
            BALL,
            AXIS,
            BUTTON
        };

    protected:
		int mDevice;
		int mPart;
        int mType;
        int mValue;
        int mxValue;
        int myValue;
        int mTimeStamp;
    };
}

#endif // end GCN_JOYSTICKINPUT_HPP
