/******************************************************************************
*
* FILENAME: joysticklistener.cpp
*
* DESCRIPTION:  My own version of Guichan's Listener class to handle joysticks
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.5.0   09/20/06  bberlin      Creation
******************************************************************************/
#ifndef GCN_JOYSTICKLISTENER_HPP
#define GCN_JOYSTICKLISTENER_HPP

#include "guichan/platform.hpp"
#include "joystickevent.hpp"

namespace gcn
{
    /**
     * Joystick listeners base class. Inorder to use this class you must inherit
     * from it and implements it's functions. JoystickListeners listen for joystick
     * events on a Widgets. When a Widget recives a joystick event, the
     * corresponding function in all it's joy listeners will be
     *
     * @see Widget::addJoystickListener
     */
    class GCN_CORE_DECLSPEC JoystickListener
    {
    public:

        /**
         * Destructor.
         */
        virtual ~JoystickListener() { }

        /**
         * Called when joystick event happens
         *
         * @param ji the event
         */
        virtual void joyEvent(JoystickEvent& joystickEvent) { }

    protected:
        /**
         * Constructor.
         *
         * You should not be able to make an instance of MouseListener,
         * therefore its constructor is protected. To use MouseListener
         * you must inherit from this class and implement it's
         * functions.
         */
        JoystickListener() { }
    };
}

#endif // end GCN_JOYSTICKLISTENER_HPP
