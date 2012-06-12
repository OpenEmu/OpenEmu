/******************************************************************************
*
* FILENAME: throttle.hpp
*
* DESCRIPTION:  This class will show the Throttle adjustment
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.5.0   12/21/06  bberlin      Creation
******************************************************************************/
#ifndef throttle_hpp
#define throttle_hpp

#include <guichan.hpp>
#include "guiobject.hpp"
#include "window.hpp"

class Throttle : public gcn::GuiWindow,
                 public GuiObject,
                 public gcn::ActionListener
{
	public:
		Throttle( GuiMain *gMain );
		~Throttle();

		void show (); 
		void hide ();

		void action(const gcn::ActionEvent &actionEvent);

	private:
		gcn::Slider mThrottleSlider;
		gcn::Label mThrottleValue;
		gcn::CheckBox mMaxSpeed;

		gcn::Button mOk;
		gcn::Button mCancel;

		bool mSettingsDirty;
};

#endif
