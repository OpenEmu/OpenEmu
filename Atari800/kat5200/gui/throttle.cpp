/******************************************************************************
*
* FILENAME: throttle.cpp
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
#include "throttle.hpp"
#include "guimain.hpp"

Throttle::Throttle ( GuiMain *gMain ) : GuiObject(gMain)
{
	setVisible ( false );
	setCaption ( "Throttle Adjust" );

	/*
	 * Setup all of our widgets
	 */
	mThrottleValue.setCaption ("100");
	mThrottleSlider.setScale(1,200);
	mThrottleSlider.setStepLength ( 1.0 );

	mMaxSpeed.setCaption ("Run as fast as possible");

	mOk.setCaption ("Ok");
	mCancel.setCaption ("Cancel");

	/*
	 * Create Callbacks and assign
	 */
	addActionListener ( this );

	mOk.addActionListener ( this );
	mOk.setActionEventId ( "mOk" );

	mCancel.addActionListener ( this );
	mCancel.setActionEventId ( "mCancel" );

	mThrottleSlider.addActionListener ( this );
	mThrottleSlider.setActionEventId ( "mThrottleSlider" );

	mMaxSpeed.addActionListener ( this );
	mMaxSpeed.setActionEventId ( "mMaxSpeed" );
	/*
	 * Size and place our widgets
	 */
	add ( &mThrottleSlider );
	add ( &mThrottleValue );
	add ( &mMaxSpeed );
	add ( &mOk );
	add ( &mCancel );

	setDimension ( mGuiMain->getRectangle(20,10,37,8) );

	mThrottleSlider.setDimension ( mGuiMain->getRectangle(1,1,30,1) );
	mThrottleValue.setDimension ( mGuiMain->getRectangle(32,1,6,1) );
	mMaxSpeed.setDimension ( mGuiMain->getRectangle(5,3,30,1) );
	mOk.setDimension ( mGuiMain->getRectangle(12.5,5,3,1) );
	mCancel.setDimension ( mGuiMain->getRectangle(17.5,5,3,1) );

	mOk.adjustSize();
	mCancel.adjustSize();
	mThrottleValue.adjustSize();
	mMaxSpeed.adjustSize();
}

Throttle::~Throttle ()
{
}

void Throttle::show ( )
{
	t_config *p_config = config_get_ptr();
	char tmp_string[10];

	mSettingsDirty = false;

	if ( p_config->throttle )
	{
		mMaxSpeed.setSelected ( false );
		mThrottleSlider.setValue( p_config->throttle );
	}
	else
	{
		mMaxSpeed.setSelected ( true );
		mThrottleSlider.setValue( 100 );
	}
	sprintf ( tmp_string, "%d%c", (int)mThrottleSlider.getValue(), '%' );
	mThrottleValue.setCaption ( tmp_string );
	mThrottleValue.adjustSize();

	setVisible ( true );
	if ( getParent () )
		getParent()->moveToTop (this);
}

void Throttle::hide ( )
{
	setVisible ( false );
	if ( getParent () )
		getParent()->moveToBottom (this);
}

void Throttle::action(const gcn::ActionEvent &actionEvent)
{
	t_config *p_config = config_get_ptr();
	char tmp_string[10];

	if ( actionEvent.getId() == "mOk" ) {
		if ( mSettingsDirty == true )
		{
			if ( mMaxSpeed.isSelected() )
				p_config->throttle = 0;
			else
				p_config->throttle = (int)mThrottleSlider.getValue();

			config_save();
		}
		hide();
	}

	if ( actionEvent.getId() == "mCancel" ) {
		hide();
	}

	if ( actionEvent.getId() == "mThrottleSlider" ) {
		mSettingsDirty = true;
		sprintf ( tmp_string, "%d%c", (int)mThrottleSlider.getValue(),'%' );
		mThrottleValue.setCaption ( tmp_string );
		mThrottleValue.adjustSize();
	}

	if ( actionEvent.getId() == "mMaxSpeed" ) {
		mSettingsDirty = true;
	}
}
