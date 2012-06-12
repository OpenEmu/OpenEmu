/******************************************************************************
*
* FILENAME: guiinputbutton.hpp
*
* DESCRIPTION:  Button inherited from Radio Button and looks like a Button
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.5.0   11/28/06  bberlin      Creation
******************************************************************************/
#ifndef guiinputbutton_hpp
#define guiinputbutton_hpp

#include <guichan.hpp>

extern "C" {
	#include "../interface/input.h"
}

class GuiInputButton : public gcn::RadioButton
{

public:
	GuiInputButton ( const std::string &caption="", const std::string &group="", bool marked=false );
	void draw(gcn::Graphics *graphics);
	void drawFrame(gcn::Graphics *graphics);
	void adjustSize();
	GuiInputButton *getMarkedButton();

	t_atari_common *mCtrl;
};

#endif
