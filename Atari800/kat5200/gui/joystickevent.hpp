/******************************************************************************
*
* FILENAME: joystickevent.hpp
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
#ifndef GCN_JOYSTICKEVENT_HPP
#define GCN_JOYSTICKEVENT_HPP

#include "guichan/inputevent.hpp"
#include "guichan/platform.hpp"

namespace gcn
{

	class Widget;

    /**
     * Joystick event
     * 
     */
    class JoystickEvent: public InputEvent
    {
    public:

        /**
         * Constructor.
         *
         */
       JoystickEvent(Widget* source,
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
                 int value);

        /**
         * Destructor.
         */
        virtual ~JoystickEvent();

        int getType() const;

        int getDevice() const;
        int getPart() const;
        int getPartType() const;
        int getValue() const;
        int getX() const;
        int getY() const;

        /**
         * Enum with joystick event values. Equal to joystick input
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
        int mPartType;
        int mValue;
		int mX;
		int mY;

		unsigned int mType;
    };
}

#endif // end GCN_JOYSTICKEVENT_HPP
