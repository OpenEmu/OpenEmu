/******************************************************************************
*
* FILENAME: inputautolistener.hpp
*
* DESCRIPTION:  Contains action functions for the auto input window
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.5.0   12/18/06  bberlin      Creation
******************************************************************************/
#ifndef inputautolistener_hpp
#define inputautolistener_hpp

#include <SDL/SDL.h>
#include <guichan.hpp>
#include "joysticklistener.hpp"

extern "C" {
	#include "../interface/input.h"
}

class InputAutoListener : public gcn::MouseListener,
                          public gcn::KeyListener,
						  public gcn::JoystickListener
{

public:

	InputAutoListener ( );
	~InputAutoListener ( );

	void setWindow( gcn::Window *window );

	void keyPressed (gcn::KeyEvent &keyEvent);

	void mousePressed (gcn::MouseEvent& mouseEvent);
	void mouseMoved(gcn::MouseEvent& mouseEvent);

	void joyEvent(gcn::JoystickEvent& ji);

	void getInput( int player, e_dev_type *device, 
	               int *device_num, e_part_type *part_type, 
				   int *part_num, e_direction *direction );

	int convertMouseButton(int button);
	int convertKeyCharacter(gcn::KeyEvent &keyEvent);
	void setMousePosition ( int x, int y );

	bool mCountModKey;
	int mouseX;
	int mouseY;

	gcn::Window *mWindow;

	e_dev_type mDevice;
	int mDeviceNum;
	e_part_type mPartType;
	int mPartNum;
	e_direction mDirection;
};

#endif
