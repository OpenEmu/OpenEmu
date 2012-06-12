/******************************************************************************
*
* FILENAME: sdlguiinput.cpp
*
* DESCRIPTION:  My own version of Guichan's SDLInput to handle joysticks
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.5.0   09/20/06  bberlin      Creation
******************************************************************************/
#include "sdlguiinput.hpp"
#include "guichan/exception.hpp"

namespace gcn
{
    SDLGuiInput::SDLGuiInput()
    {

    }

    bool SDLGuiInput::isJoystickQueueEmpty()
    {
        return mJoystickInputQueue.empty();
    }

    JoystickInput SDLGuiInput::dequeueJoystickInput()
    {
        JoystickInput joystickInput;

        if (mJoystickInputQueue.empty())
        {
            throw GCN_EXCEPTION("The queue is empty.");
        }

        joystickInput = mJoystickInputQueue.front();
        mJoystickInputQueue.pop();

        return joystickInput;
    }

    void SDLGuiInput::pushInput(SDL_Event event)
    {
        KeyInput keyInput;
        MouseInput mouseInput;
        JoystickInput joystickInput;

        switch (event.type)
        {
          case SDL_JOYBALLMOTION:
			  joystickInput.setDevice(event.jball.which);
			  joystickInput.setType(JoystickInput::BALL);
			  joystickInput.setPart(event.jball.ball);
			  joystickInput.setX(event.jball.xrel);
			  joystickInput.setY(event.jball.yrel);
			  mJoystickInputQueue.push(joystickInput);
              break;
          case SDL_JOYHATMOTION:
			  joystickInput.setDevice(event.jhat.which);
			  joystickInput.setType(JoystickInput::HAT);
			  joystickInput.setPart(event.jhat.hat);
			  joystickInput.setValue(event.jhat.value);
			  mJoystickInputQueue.push(joystickInput);
              break;
          case SDL_JOYAXISMOTION:
			  joystickInput.setDevice(event.jaxis.which);
			  joystickInput.setType(JoystickInput::AXIS);
			  joystickInput.setPart(event.jaxis.axis);
			  joystickInput.setValue(event.jaxis.value);
			  mJoystickInputQueue.push(joystickInput);
              break;
          case SDL_JOYBUTTONDOWN:
			  joystickInput.setDevice(event.jbutton.which);
			  joystickInput.setType(JoystickInput::BUTTON);
			  joystickInput.setPart(event.jbutton.button);
			  joystickInput.setValue(JoystickInput::PRESS);
			  mJoystickInputQueue.push(joystickInput);
              break;
          case SDL_JOYBUTTONUP:
			  joystickInput.setDevice(event.jbutton.which);
			  joystickInput.setType(JoystickInput::BUTTON);
			  joystickInput.setPart(event.jbutton.button);
			  joystickInput.setValue(JoystickInput::RELEASE);
			  mJoystickInputQueue.push(joystickInput);
              break;

        } // end switch

		SDLInput::pushInput(event);
    }
}
