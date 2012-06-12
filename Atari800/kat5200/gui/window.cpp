/******************************************************************************
*
* FILENAME: window.cpp
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
#include "window.hpp"

#include "guichan/exception.hpp"
#include "guichan/font.hpp"
#include "guichan/graphics.hpp"
#include "guichan/mouseinput.hpp"
#include "guichan/actionlistener.hpp"

#include "joysticklistener.hpp"

namespace gcn
{
    GuiWindow::GuiWindow()
    {
        addJoystickListener(this);
    }

    GuiWindow::GuiWindow(const std::string& caption)
    {
        addJoystickListener(this);
    }

	void GuiWindow::joyEvent (JoystickEvent &joystickEvent)
	{

	}

	void GuiWindow::generateInputAction ()
	{
		distributeActionEvent();
	}

	void GuiWindow::deleteAllListeners ()
	{
		mJoystickListeners.clear();
		mActionListeners.clear();
		mMouseListeners.clear();
		mKeyListeners.clear();
	}

	void GuiWindow::addJoystickListener(JoystickListener* joystickListener)
	{
		mJoystickListeners.push_back(joystickListener);
	}
 
	void GuiWindow::removeJoystickListener(JoystickListener* joystickListener)
	{
		mJoystickListeners.remove(joystickListener);
	}

	const std::list<JoystickListener*> & GuiWindow::_getJoystickListeners()
	{
		return mJoystickListeners;
	}

    void GuiWindow::drawFrame(Graphics* graphics)
    {
        Color faceColor = getBaseColor();
        Color highlightColor, shadowColor;
        int alpha = getBaseColor().a;
        int width = getWidth() + getFrameSize() * 2 - 1;
        int height = getHeight() + getFrameSize() * 2 - 1;
        highlightColor = faceColor + 0x303030;
        highlightColor.a = alpha;
        shadowColor = faceColor - 0x303030;
        shadowColor.a = alpha;

        unsigned int i;
        for (i = 0; i < getFrameSize(); ++i)
        {
            graphics->setColor(highlightColor);
            graphics->drawLine(i,i, width - i, i);
            graphics->drawLine(i,i + 1, i, height - i - 1);
            graphics->setColor(shadowColor);
            graphics->drawLine(width - i,i + 1, width - i, height - i);
            graphics->drawLine(i,height - i, width - i - 1, height - i);
        }
    }
}
