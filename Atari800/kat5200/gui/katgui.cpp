/******************************************************************************
*
* FILENAME: katgui.cpp
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
#include "katgui.hpp"

#include "guichan/exception.hpp"
#include "guichan/focushandler.hpp"
#include "guichan/graphics.hpp"
#include "guichan/input.hpp"
#include "guichan/keyinput.hpp"
#include "guichan/keylistener.hpp"
#include "guichan/mouseinput.hpp"
#include "guichan/widget.hpp"

namespace gcn
{
	KatGui::KatGui ( ) : Gui()
	{
		xAxisActive = 0;
		yAxisActive = 0;
		joystickEnable = false;
		joystickAxisSpeed = 3;
		mTimeStamp = SDL_GetTicks();
	}

    void KatGui::setInput(SDLGuiInput* input)
    {
        mInput = input;

		Gui::setInput(input);
    }

    void KatGui::setJoystickEnable(bool enable)
    {
        joystickEnable = enable;
    }

	/*
	 * This is so wrong, but I don't want to inheirit from every
	 *   widget at this point
	 */
    void KatGui::addJoystickWindow(GuiWindow *window)
    {
        mWindows.push_back(window);
    }

    void KatGui::removeJoystickWindow(GuiWindow *window)
    {
		GuiWindowListIterator iter;
		for (iter = mWindows.begin(); iter != mWindows.end(); iter++)
		{
			if (*iter == window)
		    {
				mWindows.erase(iter);
				return;
			}
		}

		throw GCN_EXCEPTION("There is no such widget in this container.");
    }

    Input* KatGui::getInput() const
    {
        return mInput;
    }

    void KatGui::logic()
    {
        if (mTop == NULL)
        {
            throw GCN_EXCEPTION("No top widget set");
        }

		/*
		 * Check if joystick movement is active
		 */
		int x,y;
		if ( (joystickEnable == true) && ((SDL_GetTicks()-mTimeStamp) > 16) ) {
			mTimeStamp = SDL_GetTicks();
			SDL_GetMouseState ( &x, &y );	
			x += xAxisActive * joystickAxisSpeed;
			y += yAxisActive * joystickAxisSpeed;
			SDL_WarpMouse ( x, y );
		}

        handleModalFocus();
        handleModalMouseInputFocus();

        if (mInput != NULL)
        {
            mInput->_pollInput();

            handleKeyInput();
            handleMouseInput();
            handleJoystickInput();
 
        } // end if

        mTop->logic();
    }

    void KatGui::addGlobalJoystickListener(JoystickListener* joystickListener)
    {
        mJoystickListeners.push_back(joystickListener);
    }

    void KatGui::removeGlobalJoystickListener(JoystickListener* joystickListener)
    {
        mJoystickListeners.remove(joystickListener);
    }

    void KatGui::handleJoystickInput()
    {
        while (!mInput->isJoystickQueueEmpty())
         {
             JoystickInput ji = mInput->dequeueJoystickInput();

             switch (ji.getType())
             {
               case JoystickInput::AXIS:
					if ( ji.getPart() == 0 ) {
						if ( abs(ji.getValue()) > 16000 )
							xAxisActive = ji.getValue() / abs(ji.getValue());
						else
							xAxisActive = 0;
					}
					if ( ji.getPart() == 1 ) {
						if ( abs(ji.getValue()) > 16000 )
							yAxisActive = ji.getValue() / abs(ji.getValue());
						else
							yAxisActive = 0;
					}

                   break;
               case JoystickInput::HAT:
					if ( ji.getValue() & SDL_HAT_LEFT )
						xAxisActive = -1;
					else if ( ji.getValue() & SDL_HAT_RIGHT )
						xAxisActive = 1;
					else
						xAxisActive = 0;

					if ( ji.getValue() & SDL_HAT_UP )
						yAxisActive = -1;
					else if ( ji.getValue() & SDL_HAT_DOWN )
						yAxisActive = 1;
					else
						yAxisActive = 0;

                   break;
               case JoystickInput::BUTTON:
					if ( joystickEnable )
					{
					    SDL_Event event;

						SDL_GetMouseState ( (int *)&event.button.x, (int *)&event.button.y );
						if ( ji.getPart() == 0 )
							event.button.button = SDL_BUTTON_LEFT;
						if ( ji.getPart() == 1 )
							event.button.button = SDL_BUTTON_RIGHT;
						if ( ji.getValue() == JoystickInput::PRESS )
							event.type = SDL_MOUSEBUTTONDOWN;
						if ( ji.getValue() == JoystickInput::RELEASE )
							event.type = SDL_MOUSEBUTTONUP;
						mInput->pushInput(event);
					}
                   break;
               case JoystickInput::BALL:
                   break;
               default:
                   throw GCN_EXCEPTION("Unknown joystick input type.");
                   break;
             }

			 JoystickEvent joystickEvent ( getJoystickEventSource(),
                                           mShiftPressed,
                                           mControlPressed,
                                           mAltPressed,
                                           mMetaPressed,
                                           ji.getType(),
                                           ji.getX(),
                                           ji.getY(),
                                           ji.getDevice(),
                                           ji.getPart(),
                                           ji.getType(),
                                           ji.getValue() );

			 distributeJoystickEvent ( joystickEvent );
			 distributeJoystickEventToGlobalJoystickListeners ( joystickEvent );
         }
    }

    void KatGui::distributeJoystickEvent(JoystickEvent& joystickEvent)
    {
        Widget* parent = joystickEvent.getSource();
        Widget* widget = joystickEvent.getSource();

		if ( !widget )
			return;

        if (mFocusHandler->getModalFocused() != NULL
            && !widget->isModalFocused())
        {
            return;
        }

        if (mFocusHandler->getModalMouseInputFocused() != NULL
            && !widget->isModalMouseInputFocused())
        {
            return;
        }

        while (parent != NULL)
        {
            // If the widget has been removed due to input
            // cancel the distribution.
            if (!Widget::widgetExists(widget))
            {
                break;
            }

            parent = (Widget*)widget->getParent();

            if (widget->isEnabled())
            {
                std::list<JoystickListener*> joystickListeners = ((GuiWindow *)widget)->_getJoystickListeners();
            
                // Send the event to all joystick listeners of the source widget.
                for (std::list<JoystickListener*>::iterator it = joystickListeners.begin();
                     it != joystickListeners.end();
                     ++it)
                {

					(*it)->joyEvent(joystickEvent);
                }
            }

            Widget* swap = widget;
            widget = parent;
            parent = (Widget*)swap->getParent();

			// For our Joystick stuff, do not allow a loop
			break;

            // If a non modal focused widget has been reach
            // and we have modal focus cancel the distribution.
            if (mFocusHandler->getModalFocused() != NULL
                && !widget->isModalFocused())
            {
                break;
            }
        }
    }

    void KatGui::distributeJoystickEventToGlobalJoystickListeners(JoystickEvent& joystickEvent)
    {
        JoystickListenerListIterator it;

        for (it = mJoystickListeners.begin(); it != mJoystickListeners.end(); it++)
        {
			(*it)->joyEvent(joystickEvent);
        }
    }

    GuiWindow* KatGui::getJoystickEventSource()
    {
		// Must return a Gui Window
        Widget* widget = mFocusHandler->getFocused();

		if ( !widget )
			return NULL;

		GuiWindowListIterator iter;

		for (iter = mWindows.begin(); iter != mWindows.end(); iter++)
		{
			if (*iter == widget || *iter == widget->getParent())
		    {
				return *iter;
			}
		}

        return NULL;
    }
}
