/******************************************************************************
*
* FILENAME: sdlguiinput.hpp
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
#ifndef GCN_SDLGUIINPUT_HPP
#define GCN_SDLGUIINPUT_HPP

#include <queue>

#include <SDL/SDL.h>

#include <guichan/sdl/sdlinput.hpp>
#include "joystickinput.hpp"

namespace gcn
{
    /**
     * SDL implementation of Input.
     */
    class SDLGuiInput : public SDLInput
    {
    public:

        /**
         * Constructor.
         */
        SDLGuiInput();

        /**
         * Pushes an SDL event. It should be called at least once per frame to
         * update input with user input.
         *
         * @param event an event from SDL.
         */
        virtual void pushInput(SDL_Event event);

        // Inherited from Input

        virtual bool isJoystickQueueEmpty();

        virtual JoystickInput dequeueJoystickInput();

    protected:
        /**
         * Converts a mouse button from SDL to a Guichan mouse button
         * representation.
         *
         * @param button an SDL mouse button.
         * @return a Guichan mouse button.
         */

        std::queue<JoystickInput> mJoystickInputQueue;
    };
}

#endif // end GCN_SDLGUIINPUT_HPP
