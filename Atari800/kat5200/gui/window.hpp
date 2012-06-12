/******************************************************************************
*
* FILENAME: window.hpp
*
* DESCRIPTION:  My own version of Guichan's Window to handle joysticks
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.5.0   09/22/06  bberlin      Creation
******************************************************************************/
#ifndef guiwindow_hpp
#define guiwindow_hpp

#include <string>

#include "joysticklistener.hpp"
#include "guichan/mouselistener.hpp"
#include "guichan/platform.hpp"
#include "guichan/widgets/window.hpp"

namespace gcn
{
    /**
     * A movable window which can conatin another Widget.
     */
    class GuiWindow : public Window, public JoystickListener
    {
    public:
		GuiWindow();
		GuiWindow(const std::string& caption);
		void generateInputAction();
        virtual void joyEvent(JoystickEvent &joystickEvent);
		virtual const std::list<JoystickListener*> & _getJoystickListeners();
		void addJoystickListener(JoystickListener* joystickListener);
		void removeJoystickListener(JoystickListener* joystickListener);
		void deleteAllListeners();
		void drawFrame(Graphics* graphics);
    protected:
		typedef std::list<JoystickListener*> JoystickListenerList;
		JoystickListenerList mJoystickListeners;
		typedef JoystickListenerList::iterator JoystickListenerIterator;
    };
}

#endif // end window_hpp
