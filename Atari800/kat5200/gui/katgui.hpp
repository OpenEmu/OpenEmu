/******************************************************************************
*
* FILENAME: katgui.hpp
*
* DESCRIPTION:  My own version of Guichan's Gui class to handle joysticks
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.5.0   09/20/06  bberlin      Creation
******************************************************************************/
#ifndef GCN_KATGUI_HPP
#define GCN_KATGUI_HPP

#include <list>
#include <SDL/SDL.h>
#include <guichan/gui.hpp>

#include "window.hpp"
#include "sdlguiinput.hpp"

typedef std::list< gcn::GuiWindow * > GuiWindowList;
typedef GuiWindowList::iterator GuiWindowListIterator;
typedef GuiWindowList::reverse_iterator GuiWindowListReverseIterator;

namespace gcn
{
    class SDLGuiInput;
	class JoystickListener;
    class Gui;

    class KatGui: public Gui
    {
    public:

		KatGui ();

        /**
         * Sets the Input object to use for input handling.
         *
         * @param input the Input object to use for input handling.
         * @see SDLInput, AllegroInput
         */
        virtual void setInput(SDLGuiInput* input);
		virtual void setJoystickEnable(bool enable);

        /**
         * Gets the Input object being used for input handling.
         *
         *  @return the Input object used for handling input. NULL if no
         *          Input object has been set.
         */
        virtual Input* getInput() const;

        /**
         * Performs the Gui logic. By calling this function all logic
         * functions down in the Gui heirarchy will be called.
         * What performs in Logic can be just about anything like
         * adjusting a Widgets size or doing some calculations.
         *
         * NOTE: Logic also deals with user input (Mouse and Keyboard)
         *       for Widgets.
         */
        virtual void logic();

		/*
		 * Since I don't want to re-write the whole guichan library,
		 *   here is a cheap way to pass joystick input where I want it.
		 */
        virtual void addJoystickWindow( GuiWindow *window );
		virtual void removeJoystickWindow(GuiWindow *window);
        /**
         * Adds a global JoystickListener to the Gui.
         *
         * @param joystickListener a JoystickListener to add.
         */
        virtual void addGlobalJoystickListener(JoystickListener* joystickListener);

        /**
         * Remove global JoystickListener from the Gui.
         *
         * @param joystickListener a JoystickListener to remove.
         * @throws Exception if the JoystickListener hasn't been added.
         */
        virtual void removeGlobalJoystickListener(JoystickListener* joystickListener);

		void setJoystickAxisSpeed ( int speed );
		void enableJoystickMovement ( bool joymove );

    protected:

        /**
         * Handles joystick input.
         *
         * @since 0.6.0
         */
        virtual void handleJoystickInput();

        /**
         * Distributes a joystick event.
         *
         * @param joystickEvent the joystick event to distribute.
         * @since 0.6.0
         */
        virtual void distributeJoystickEvent(JoystickEvent& joystickEvent);

        /**
         * Distributes a key event to the global joystick listeners.
         *
         * @param joystickEvent the joystick event to distribute.
         *
         * @since 0.6.0
         */
        virtual void distributeJoystickEventToGlobalJoystickListeners(JoystickEvent& joystickEvent);

        /**
         * Gets the source of the joystick event.
         *
         * @return the source widget of the joystick event.
         * @since 0.6.0
         */
        virtual GuiWindow* getJoystickEventSource();

        SDLGuiInput* mInput;
		unsigned long mTimeStamp;
		int xAxisActive;
		int yAxisActive;
		bool joystickEnable;
		int joystickAxisSpeed;

        typedef std::list<JoystickListener*> JoystickListenerList;
        JoystickListenerList mJoystickListeners;
        typedef JoystickListenerList::iterator JoystickListenerListIterator;
		std::list<GuiWindow*> mWindows;
    };
}

#endif // end GCN_GUI_HPP
