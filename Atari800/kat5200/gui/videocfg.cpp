/******************************************************************************
*
* FILENAME: videocfg.cpp
*
* DESCRIPTION:  This class will show the video configuration
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.5.0   10/24/06  bberlin      Creation
******************************************************************************/
#include "videocfg.hpp"
#include "guimain.hpp"

extern "C" {
	#include "../interface/kconfig.h"
	#include "../interface/db_if.h"

	extern t_atari_video g_video;
	extern struct gtia gtia;
}

VideoCfg::VideoCfg ( GuiMain *gMain ) : GuiObject(gMain)
{
	setVisible ( false );
	setCaption ("Video Settings");

	/*
	 * Setup all of our widgets
	 */
	mProfile.setListModel ( &mProfileList );
	mDefault.setCaption ("Use as Default Group");
	mRename.setCaption ("Rename");
	mSaveAs.setCaption ("SaveAs");
	mDescriptionLabel.setCaption ("Description");
	mProfileDec.setCaption ( "Profile" );

	mOk.setCaption ("Ok");
	mCancel.setCaption ("Cancel");

	mNTSCPalette.setListModel ( &mPaletteList );
	mPALPalette.setListModel ( &mPaletteList );
	mPaletteDec.setCaption ( "Palette" );
	mNTSCLabel.setCaption ( "NTSC Palette" );
	mPALLabel.setCaption ( "PAL Palette" );

	mVideoDec.setCaption ( "" );
	mFullScreen.setCaption ("Fullscreen");
	mWideScreen.setCaption ("Widescreen");
	mSpecify.setCaption ("Specify Window");
	mColorDepthLabel.setCaption ("Depth");
	mColorDepth.setListModel (&mColorDepthList);
	mModeLabel.setCaption ("Avail FS Modes");
	mMode.setListModel (&mModeList);
	mZoomLabel.setCaption ("Zoom");
	mZoom.setListModel (&mZoomList);
	mHeightLabel.setCaption ("x");

	mPresetDec.setCaption ( "Presets" );
	mRfPreset.setCaption ("RF");
	mCompositePreset.setCaption ("Composite");
	mSVideoPreset.setCaption ("S-Video");
	mRGBPreset.setCaption ("RGB");

	mFilterDec.setCaption ( "NTSC Filter" );
	mNTSCFilter.setCaption ("Enabled");
	mHue.setScale (-1,1);
	mSaturation.setScale (-1,1);
	mContrast.setScale (-1,1);
	mBrightness.setScale (-1,1);
	mSharpness.setScale (-1,1);
	mGamma.setScale (-1,1);
	mResolution.setScale (-1,1);
	mArtifacts.setScale (-1,1);
	mFringing.setScale (-1,1);
	mBleed.setScale (-1,1);
	mHueWarping.setScale (-1,1);
	mMergeFields.setCaption ("Merge Fields");

	mHueLabel.setCaption ("Hue");
	mSaturationLabel.setCaption ("Saturation");
	mContrastLabel.setCaption ("Contrast");
	mBrightnessLabel.setCaption ("Brightness");
	mSharpnessLabel.setCaption ("Sharpness");
	mGammaLabel.setCaption ("Gamma");
	mResolutionLabel.setCaption ("Resolution");
	mArtifactsLabel.setCaption ("Artifacts");
	mFringingLabel.setCaption ("Fringing");
	mBleedLabel.setCaption ("Bleed");
	mHueWarpingLabel.setCaption ("Hue Warping");

	mHueValue.setCaption ("0");
	mSaturationValue.setCaption ("0");
	mContrastValue.setCaption ("0");
	mBrightnessValue.setCaption ("0");
	mSharpnessValue.setCaption ("0");
	mGammaValue.setCaption ("0");
	mResolutionValue.setCaption ("0");
	mArtifactsValue.setCaption ("0");
	mFringingValue.setCaption ("0");
	mBleedValue.setCaption ("0");
	mHueWarpingValue.setCaption ("0");

	mDisplaySurface = new gcn::SDLImage ( SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 200, 
	                          16, 0, 0, 0, 0), true);
	//mDisplaySurface->convertToDisplayFormat();
	mDisplayImage = new gcn::Icon ( mDisplaySurface );

	/*
	 * Create Callbacks and assign
	 */
	addActionListener ( this );

	mProfile.addActionListener ( this );
	mNTSCPalette.addActionListener ( this );
	mPALPalette.addActionListener ( this );
	mDescriptionBox.addMouseListener ( this );
	mRename.addActionListener ( this );
	mSaveAs.addActionListener ( this );
	mOk.addActionListener ( this );
	mCancel.addActionListener ( this );
	mDefault.addActionListener ( this );
	mFullScreen.addActionListener ( this );
	mWideScreen.addActionListener ( this );
	mSpecify.addActionListener ( this );
	mColorDepth.addActionListener ( this );
	mMode.addActionListener ( this );
	mZoom.addActionListener ( this );
	mWidth.addMouseListener ( this );
	mHeight.addMouseListener ( this );

	mProfile.setActionEventId ( "mProfile" );
	mRename.setActionEventId ( "mRename" );
	mSaveAs.setActionEventId ( "mSaveAs" );
	mNTSCPalette.setActionEventId ( "mPalette" );
	mPALPalette.setActionEventId ( "mPalette" );
	mOk.setActionEventId ( "mOk" );
	mCancel.setActionEventId ( "mCancel" );
	mDefault.setActionEventId ( "mDirty" );
	mFullScreen.setActionEventId ( "mFullScreen" );
	mSpecify.setActionEventId ( "mSpecify" );
	mWideScreen.setActionEventId ( "mDirty" );
	mColorDepth.setActionEventId ( "mColorDepth" );
	mMode.setActionEventId ( "mMode" );
	mZoom.setActionEventId ( "mDirty" );

	mRfPreset.addActionListener ( this );
	mCompositePreset.addActionListener ( this );
	mSVideoPreset.addActionListener ( this );
	mRGBPreset.addActionListener ( this );

	mRfPreset.setActionEventId ( "mRfPreset" );
	mCompositePreset.setActionEventId ( "mCompositePreset" );
	mSVideoPreset.setActionEventId ( "mSVideoPreset" );
	mRGBPreset.setActionEventId ( "mRGBPreset" );

	mNTSCFilter.addActionListener ( this );
	mHue.addActionListener ( this );
	mSaturation.addActionListener ( this );
	mContrast.addActionListener ( this );
	mBrightness.addActionListener ( this );
	mSharpness.addActionListener ( this );
	mGamma.addActionListener ( this );
	mResolution.addActionListener ( this );
	mArtifacts.addActionListener ( this );
	mFringing.addActionListener ( this );
	mBleed.addActionListener ( this );
	mHueWarping.addActionListener ( this );
	mMergeFields.addActionListener ( this );

	mNTSCFilter.setActionEventId ( "mNTSCFilter" );
	mHue.setActionEventId ( "mCaptions" );
	mSaturation.setActionEventId ( "mCaptions" );
	mContrast.setActionEventId ( "mCaptions" );
	mBrightness.setActionEventId ( "mCaptions" );
	mSharpness.setActionEventId ( "mCaptions" );
	mGamma.setActionEventId ( "mCaptions" );
	mResolution.setActionEventId ( "mCaptions" );
	mArtifacts.setActionEventId ( "mCaptions" );
	mFringing.setActionEventId ( "mCaptions" );
	mBleed.setActionEventId ( "mCaptions" );
	mHueWarping.setActionEventId ( "mCaptions" );
	mMergeFields.setActionEventId ( "mDirty" );

	/*
	 * Size and place our widgets
	 */
	add ( &mProfileDec );
	add ( &mProfile );
	add ( &mDefault );
	add ( &mRename );
	add ( &mSaveAs );
	add ( &mDescriptionBox );
	add ( &mDescriptionLabel );

	add ( &mOk );
	add ( &mCancel );

	//add ( &mPaletteDec );
	add ( &mNTSCPalette );
	add ( &mPALPalette );
	add ( &mNTSCLabel );
	add ( &mPALLabel );

	add ( &mVideoDec );
	add ( &mFullScreen );
	add ( &mWideScreen );
	add ( &mSpecify );
	add ( &mColorDepth );
	add ( &mColorDepthLabel );
	add ( &mMode );
	add ( &mModeLabel );
	add ( &mZoom );
	add ( &mZoomLabel );
	add ( &mHeightLabel );
	add ( &mWidth );
	add ( &mHeight );

	add ( &mPresetDec );
	add ( &mRfPreset );
	add ( &mCompositePreset );
	add ( &mSVideoPreset );
	add ( &mRGBPreset );

	add ( &mFilterDec );
	add ( &mNTSCFilter );
	add ( &mHue );
	add ( &mSaturation );
	add ( &mContrast );
	add ( &mBrightness );
	add ( &mSharpness );
	add ( &mGamma );
	add ( &mResolution );
	add ( &mArtifacts );
	add ( &mFringing );
	add ( &mBleed );
	add ( &mHueWarping );
	add ( &mMergeFields );

	add ( &mHueLabel );
	add ( &mSaturationLabel );
	add ( &mContrastLabel );
	add ( &mBrightnessLabel );
	add ( &mSharpnessLabel );
	add ( &mGammaLabel );
	add ( &mResolutionLabel );
	add ( &mArtifactsLabel );
	add ( &mFringingLabel );
	add ( &mBleedLabel );
	add ( &mHueWarpingLabel );

	add ( &mHueValue );
	add ( &mSaturationValue );
	add ( &mContrastValue );
	add ( &mBrightnessValue );
	add ( &mSharpnessValue );
	add ( &mGammaValue );
	add ( &mResolutionValue );
	add ( &mArtifactsValue );
	add ( &mFringingValue );
	add ( &mBleedValue );
	add ( &mHueWarpingValue );

	add ( mDisplayImage );

	setDimension ( mGuiMain->getRectangle(2,2,74,26) );

	mProfile.setDimension ( mGuiMain->getRectangle(1,1.2,54,2) );
	mDescriptionLabel.setDimension ( mGuiMain->getRectangle(1,2.5,54,2) );
	mDescriptionBox.setDimension ( mGuiMain->getRectangle(1,3.5,54,2) );
	mDefault.setDimension ( mGuiMain->getRectangle(57,3.5,14,1) );
	mRename.setDimension ( mGuiMain->getRectangle(57,1.2,3,1) );
	mSaveAs.setDimension ( mGuiMain->getRectangle(65.5,1.2,3,1) );
	mProfileDec.setDimension ( mGuiMain->getRectangle(0.5,0,72.5,5) );

	mOk.setDimension ( mGuiMain->getRectangle(24.9,21.1,3,1) );
	mCancel.setDimension ( mGuiMain->getRectangle(24.9,23.0,6,1) );

	mNTSCLabel.setDimension ( mGuiMain->getRectangle(1,5.3,16,1) );
	mNTSCPalette.setDimension ( mGuiMain->getRectangle(14,5.3,54,1) );
	mPALLabel.setDimension ( mGuiMain->getRectangle(1,6.8,16,1) );
	mPALPalette.setDimension ( mGuiMain->getRectangle(14,6.8,54,1) );
	//mPaletteDec.setDimension ( mGuiMain->getRectangle(0.5,5.0,72.5,3) );

	mVideoDec.setDimension ( mGuiMain->getRectangle(35,8,38,6) );
	mFullScreen.setDimension ( mGuiMain->getRectangle(36.5,8.5,6,1) );
	mWideScreen.setDimension ( mGuiMain->getRectangle(36.5,9.8,6,1) );
	mZoomLabel.setDimension ( mGuiMain->getRectangle(36.5,11.1,5,1) );
	mZoom.setDimension ( mGuiMain->getRectangle(42.5,11.1,5,1) );
	mColorDepthLabel.setDimension ( mGuiMain->getRectangle(36.5,12.4,6,1) );
	mColorDepth.setDimension ( mGuiMain->getRectangle(42.5,12.4,8,1) );

	mModeLabel.setDimension ( mGuiMain->getRectangle(55,8.5,5,1) );
	mMode.setDimension ( mGuiMain->getRectangle(55,9.8,11,1) );
	mSpecify.setDimension ( mGuiMain->getRectangle(55,11.1,6,1) );
	mHeightLabel.setDimension ( mGuiMain->getRectangle(60.5,12.4,7,1) );
	mWidth.setDimension ( mGuiMain->getRectangle(55,12.4,5,1) );
	mHeight.setDimension ( mGuiMain->getRectangle(62,12.4,5,1) );

	mFilterDec.setDimension ( mGuiMain->getRectangle(0.5,8.0,22,16.7) );
	mNTSCFilter.setDimension ( mGuiMain->getRectangle(1,9,6,1) );
	mHueLabel.setDimension ( mGuiMain->getRectangle(1,10.2,4,1) );
	mHue.setDimension ( mGuiMain->getRectangle(12,10.2,6,1) );
	mHueValue.setDimension ( mGuiMain->getRectangle(18.5,10.2,4,1) );
	mSaturationLabel.setDimension ( mGuiMain->getRectangle(1,11.4,6,1) );
	mSaturation.setDimension ( mGuiMain->getRectangle(12,11.4,6,1) );
	mSaturationValue.setDimension ( mGuiMain->getRectangle(18.5,11.4,6,1) );
	mContrastLabel.setDimension ( mGuiMain->getRectangle(1,12.6,6,1) );
	mContrast.setDimension ( mGuiMain->getRectangle(12,12.6,6,1) );
	mContrastValue.setDimension ( mGuiMain->getRectangle(18.5,12.6,6,1) );
	mBrightnessLabel.setDimension ( mGuiMain->getRectangle(1,13.8,6,1) );
	mBrightness.setDimension ( mGuiMain->getRectangle(12,13.8,6,1) );
	mBrightnessValue.setDimension ( mGuiMain->getRectangle(18.5,13.8,6,1) );
	mSharpnessLabel.setDimension ( mGuiMain->getRectangle(1,15.0,6,1) );
	mSharpness.setDimension ( mGuiMain->getRectangle(12,15.0,6,1) );
	mSharpnessValue.setDimension ( mGuiMain->getRectangle(18.5,15.0,6,1) );
	mGammaLabel.setDimension ( mGuiMain->getRectangle(1,16.2,6,1) );
	mGamma.setDimension ( mGuiMain->getRectangle(12,16.2,6,1) );
	mGammaValue.setDimension ( mGuiMain->getRectangle(18.5,16.2,6,1) );
	mResolutionLabel.setDimension ( mGuiMain->getRectangle(1,17.4,6,1) );
	mResolution.setDimension ( mGuiMain->getRectangle(12,17.4,6,1) );
	mResolutionValue.setDimension ( mGuiMain->getRectangle(18.5,17.4,6,1) );
	mArtifactsLabel.setDimension ( mGuiMain->getRectangle(1,18.6,6,1) );
	mArtifacts.setDimension ( mGuiMain->getRectangle(12,18.6,6,1) );
	mArtifactsValue.setDimension ( mGuiMain->getRectangle(18.5,18.6,6,1) );
	mFringingLabel.setDimension ( mGuiMain->getRectangle(1,19.8,6,1) );
	mFringing.setDimension ( mGuiMain->getRectangle(12,19.8,6,1) );
	mFringingValue.setDimension ( mGuiMain->getRectangle(18.5,19.8,6,1) );
	mBleedLabel.setDimension ( mGuiMain->getRectangle(1,21.0,6,1) );
	mBleed.setDimension ( mGuiMain->getRectangle(12,21.0,6,1) );
	mBleedValue.setDimension ( mGuiMain->getRectangle(18.5,21.0,6,1) );
	mHueWarpingLabel.setDimension ( mGuiMain->getRectangle(1,22.2,6,1) );
	mHueWarping.setDimension ( mGuiMain->getRectangle(12,22.2,6,1) );
	mHueWarpingValue.setDimension ( mGuiMain->getRectangle(18.5,22.2,6,1) );
	mMergeFields.setDimension ( mGuiMain->getRectangle(1,23.5,6,1) );

	mPresetDec.setDimension ( mGuiMain->getRectangle(23.0,8,11,12.7) );
	mRfPreset.setDimension ( mGuiMain->getRectangle(23.4,11.2,6,1) );
	mCompositePreset.setDimension ( mGuiMain->getRectangle(23.4,13.2,6,1) );
	mSVideoPreset.setDimension ( mGuiMain->getRectangle(23.4,15.2,6,1) );
	mRGBPreset.setDimension ( mGuiMain->getRectangle(23.4,17.2,6,1) );

	mDisplayImage->setDimension ( mGuiMain->getRectangle(35,14.2,38,12.5) );

	mProfile.adjustHeight();
	mDescriptionLabel.adjustSize();
	mDescriptionBox.adjustHeight();
	mRename.adjustSize();
	mSaveAs.adjustSize();
	mOk.adjustSize();
	mCancel.adjustSize();
	mPALPalette.adjustHeight();
	mNTSCPalette.adjustHeight();
	mPALLabel.adjustSize();
	mNTSCLabel.adjustSize();

	mDefault.adjustSize();
	mNTSCFilter.adjustSize();
	mMergeFields.adjustSize();
	mFullScreen.adjustSize();
	mWideScreen.adjustSize();
	mSpecify.adjustSize();
	mColorDepthLabel.adjustSize();
	mModeLabel.adjustSize();
	mZoomLabel.adjustSize();
	mHeightLabel.adjustSize();
	mWidth.adjustHeight();
	mHeight.adjustHeight();

	mHueLabel.adjustSize();
	mSaturationLabel.adjustSize();
	mContrastLabel.adjustSize();
	mBrightnessLabel.adjustSize();
	mSharpnessLabel.adjustSize();
	mGammaLabel.adjustSize();
	mResolutionLabel.adjustSize();
	mArtifactsLabel.adjustSize();
	mFringingLabel.adjustSize();
	mBleedLabel.adjustSize();
	mHueWarpingLabel.adjustSize();

	mHueValue.adjustSize();
	mSaturationValue.adjustSize();
	mContrastValue.adjustSize();
	mBrightnessValue.adjustSize();
	mSharpnessValue.adjustSize();
	mGammaValue.adjustSize();
	mResolutionValue.adjustSize();
	mArtifactsValue.adjustSize();
	mFringingValue.adjustSize();
	mBleedValue.adjustSize();
	mHueWarpingValue.adjustSize();

	mRfPreset.adjustSize();
	mCompositePreset.adjustSize();
	mSVideoPreset.adjustSize();
	mRGBPreset.adjustSize();

	/*
	 * Initialize the list
	 */
	initGroupControls();
	initProfileList();

	/*
	 * Initialize Current Settings
	 */
	mXOffset = 0;
	mYOffset = 0;
	mXOffsetStart = 0;
	mYOffsetStart = 0;
}

VideoCfg::~VideoCfg ()
{
	delete mDisplayImage;
	delete mDisplaySurface;
}

void VideoCfg::show ( )
{
	t_config *p_config = config_get_ptr();
	std::string profile_name;

	mSettingsDirty = false;

	if ( (p_config->machine_type == media_get_ptr()->machine_type) ||
	     (p_config->machine_type != MACHINE_TYPE_5200 && 
	     media_get_ptr()->machine_type != MACHINE_TYPE_5200) )
		video_set_profile ( media_get_ptr()->video_profile );

	mVideo = g_video;

	profile_name = mVideo.name;

	initPaletteList();

	if ( getParent () )
		getParent()->moveToTop (this);

	changeGroup ( profile_name );
	setVisible ( true );
}

void VideoCfg::hide ( )
{
	setVisible ( false );
	if ( getParent () )
		getParent()->moveToBottom (this);
}

void VideoCfg::initProfileList ( )
{
	struct generic_node *nodeptr = 0;

	mProfileList.deleteAll();
	db_if_get_profile_names ( "SELECT Name FROM Video", &nodeptr );
	mProfileList.setList ( nodeptr );
}

void VideoCfg::initPaletteList ( )
{
	struct generic_node *nodeptr = 0;

	mPaletteList.deleteAll();
	db_if_get_profile_names ( "SELECT Name FROM Palette", &nodeptr );
	mPaletteList.setList ( nodeptr );
}
void VideoCfg::setModesVisible ( bool setting )
{
	if ( mSpecify.isSelected() )
	{
		mModeLabel.setVisible ( setting );
		mMode.setVisible ( setting );
	}
	else
	{
		mModeLabel.setVisible ( false );
		mMode.setVisible ( false );
	}
}

void VideoCfg::setSpecifyVisible ( bool setting )
{
	mWidth.setVisible ( setting );
	mHeight.setVisible ( setting );
	mHeightLabel.setVisible ( setting );
	if ( mFullScreen.isSelected() )
		setModesVisible ( setting );
	else
		setModesVisible ( false );
}

void VideoCfg::setNtscVisible ( bool setting )
{
	mHue.setVisible( setting );
	mSaturation.setVisible( setting );
	mContrast.setVisible( setting );
	mBrightness.setVisible( setting );
	mSharpness.setVisible( setting );
	mArtifacts.setVisible( setting );
	mHueWarping.setVisible( setting );

	mGamma.setVisible( false );
	mResolution.setVisible( false );
	mFringing.setVisible( false );
	mBleed.setVisible( false );
	mMergeFields.setVisible( false );

	mHueLabel.setVisible( setting );
	mSaturationLabel.setVisible( setting );
	mContrastLabel.setVisible( setting );
	mBrightnessLabel.setVisible( setting );
	mSharpnessLabel.setVisible( setting );
	mArtifactsLabel.setVisible( setting );
	mHueWarpingLabel.setVisible( setting );

	mGammaLabel.setVisible( false );
	mResolutionLabel.setVisible( false );
	mFringingLabel.setVisible( false );
	mBleedLabel.setVisible( false );

	mHueValue.setVisible( setting );
	mSaturationValue.setVisible( setting );
	mContrastValue.setVisible( setting );
	mBrightnessValue.setVisible( setting );
	mSharpnessValue.setVisible( setting );
	mArtifactsValue.setVisible( setting );
	mHueWarpingValue.setVisible( setting );

	mGammaValue.setVisible( false );
	mResolutionValue.setVisible( false );
	mFringingValue.setVisible( false );
	mBleedValue.setVisible( false );

	mRfPreset.setVisible( setting );
	mCompositePreset.setVisible( setting );
	mSVideoPreset.setVisible( setting );
	mRGBPreset.setVisible( setting );

	mPALLabel.setVisible ( !setting );
	mPALPalette.setVisible ( !setting );
	mNTSCLabel.setVisible ( !setting );
	mNTSCPalette.setVisible ( !setting );
	mZoomLabel.setVisible ( !setting );
	mZoom.setVisible ( !setting );
	mColorDepthLabel.setVisible ( !setting );
	mColorDepth.setVisible ( !setting );
}

void VideoCfg::setNtscCaptions ( )
{
	char tmp_string[100];

	sprintf ( tmp_string, "%+#03.1lf", mHue.getValue() );
	mHueValue.setCaption( tmp_string );
	sprintf ( tmp_string, "%+#03.1lf", mSaturation.getValue() );
	mSaturationValue.setCaption( tmp_string );
	sprintf ( tmp_string, "%+#03.1lf", mContrast.getValue() );
	mContrastValue.setCaption( tmp_string );
	sprintf ( tmp_string, "%+#03.1lf", mBrightness.getValue() );
	mBrightnessValue.setCaption( tmp_string );
	sprintf ( tmp_string, "%+#03.1lf", mSharpness.getValue() );
	mSharpnessValue.setCaption( tmp_string );
	sprintf ( tmp_string, "%+#03.1lf", mGamma.getValue() );
	mGammaValue.setCaption( tmp_string );
	sprintf ( tmp_string, "%+#03.1lf", mResolution.getValue() );
	mResolutionValue.setCaption( tmp_string );
	sprintf ( tmp_string, "%+#03.1lf", mArtifacts.getValue() );
	mArtifactsValue.setCaption( tmp_string );
	sprintf ( tmp_string, "%+#03.1lf", mFringing.getValue() );
	mFringingValue.setCaption( tmp_string );
	sprintf ( tmp_string, "%+#03.1lf", mBleed.getValue() );
	mBleedValue.setCaption( tmp_string );
	sprintf ( tmp_string, "%+#03.1lf", mHueWarping.getValue() );
	mHueWarpingValue.setCaption( tmp_string );

	mHueValue.adjustSize();
	mSaturationValue.adjustSize();
	mContrastValue.adjustSize();
	mBrightnessValue.adjustSize();
	mSharpnessValue.adjustSize();
	mGammaValue.adjustSize();
	mResolutionValue.adjustSize();
	mArtifactsValue.adjustSize();
	mFringingValue.adjustSize();
	mBleedValue.adjustSize();
	mHueWarpingValue.adjustSize();
}

void VideoCfg::setNtscPreset ( const atari_ntsc_setup_t *setup )
{
	mHue.setValue( setup->hue );
	mSaturation.setValue( setup->saturation );
	mContrast.setValue( setup->contrast );
	mBrightness.setValue( setup->brightness );
	mSharpness.setValue( setup->sharpness );
	mGamma.setValue( setup->gamma );
	mResolution.setValue( setup->resolution );
	mArtifacts.setValue( setup->burst_phase );
	mFringing.setValue( setup->fringing );
	mBleed.setValue( setup->bleed );
	mHueWarping.setValue( setup->hue_warping );
	if ( setup->merge_fields == 1 )
		mMergeFields.setSelected( true );
	else
		mMergeFields.setSelected( false );

	setNtscCaptions();
}

void VideoCfg::initGroupControls ( )
{
	mColorDepthList.deleteAll();
	mColorDepthList.addElement ( "8-bit", 8, 0 );
	mColorDepthList.addElement ( "16-bit", 16, 0 );
	mColorDepthList.addElement ( "32-bit", 32, 0 );

	mZoomList.deleteAll();
	mZoomList.addElement ( "1x", 0, 0 );
	mZoomList.addElement ( "2x", 1, 0 );
	mZoomList.addElement ( "3x", 2, 0 );
}

void VideoCfg::setupModeList ( int pixel_depth )
{
	SDL_Rect **p_modes;
	Uint32 flags;
	int status=1,i;
	char temp[50];

	flags = SDL_HWPALETTE | SDL_FULLSCREEN;

	status = pc_get_modes ( pixel_depth, flags, &p_modes );

	mModeList.deleteAll();

	if ( !status ) {
		for ( i = 0; p_modes[i]; i++ ) {
			sprintf ( temp, "%dx%d", p_modes[i]->w, p_modes[i]->h );
			mModeList.addElement ( temp, p_modes[i]->w, p_modes[i]->h );
		}
	}

	mMode.setSelected ( 0 );
}

void VideoCfg::blitScreenImage ( )
{
	SDL_Color color_palette[0x100];
	unsigned int color_rgb[0x100];
	SDL_Surface *tmp_surface;
	SDL_Rect s_rect, d_rect;
	int x,y,i,adder;
	unsigned char *sp, *dp;
	atari_ntsc_t *ntsc_emu;

	updateCurrentValues();

	SDL_Surface *surface = ((gcn::SDLImage *)mDisplaySurface)->getSurface();

	pc_set_palette ( &mVideo, color_palette, color_rgb );

	/*
	 * We must use an 8-bit surface to transfer
	 */
	tmp_surface=SDL_CreateRGBSurface(SDL_SWSURFACE, 448, 
	                     (config_get_ptr()->system_type==PAL ? 312 : 240), 
                         8, 0, 0, 0, 0);

	SDL_SetColors(tmp_surface, color_palette, 0, 256);

	if ( gtia.d_surface.bytes_per_pixel != 1 ) 
	{
		adder = gtia.d_surface.bytes_per_pixel;
		for ( y = 0; y < (config_get_ptr()->system_type==PAL ? 312 : 240); ++y )
		{
			sp = (unsigned char *)gtia.d_surface.start + (y * gtia.d_surface.width);
			dp = (unsigned char *)tmp_surface->pixels + (y * tmp_surface->pitch);
			for ( x = 0; x < 448; ++x ) {
				for ( i = 0; i < 256; ++i )
				{
					if ( (adder == 2 && *((Uint16 *)(sp)) == gtia.color_array[i]) ||
					     (adder == 4 && *((Uint32 *)(sp)) == gtia.color_array[i]))
					{
						*dp = i;
						break;
					}
				}
				sp+=adder;
				dp++;
			}
		}
	}
	else {
		for ( y = 0; y < (config_get_ptr()->system_type==PAL ? 312 : 240); ++y ) 
		{
			sp = (unsigned char *)gtia.d_surface.start + (y * gtia.d_surface.width);
			dp = (unsigned char *)tmp_surface->pixels + (y * tmp_surface->pitch);
			for ( x = 0; x < 448; ++x ) {
				*dp = *sp;
				sp++;
				dp++;
			}
		}
	}

	if ( mYOffset < 0 )
		mYOffset = 0;
	if ( mXOffset < 0 )
		mXOffset = 0;

	s_rect.x = 32 + mXOffset;
	s_rect.y = mYOffset;
	
	if ( s_rect.y > (tmp_surface->h - 200))
		s_rect.y = tmp_surface->h - 200;

	d_rect.x = 0;
	d_rect.y = 0;
	d_rect.w = 320;
	d_rect.h = 200;

	/*
	 * Load up palette and transfer colors
	 */
	if ( mVideo.ntsc_filter_on )
	{
		/*
		s_rect.x = 32 + ATARI_NTSC_IN_WIDTH(mXOffset);
		s_rect.y = mYOffset;
		s_rect.w = ATARI_NTSC_IN_WIDTH( 320 );
		s_rect.h = 100;
		*/
		s_rect.x = 32 + (mXOffset);
		s_rect.y = mYOffset;
		s_rect.w = 320;
		s_rect.h = 100;

		/*
		if ( mXOffset > (ATARI_NTSC_OUT_WIDTH(320) - 320 ) )
		{
			mXOffset = (ATARI_NTSC_OUT_WIDTH(320) - 320 ); 
		}
		*/
		if ( mXOffset > 320-32 )
			mXOffset = 320-32;

		if ( s_rect.y > ((tmp_surface->h * 2) - 200) / 2)
			s_rect.y = mYOffset = (tmp_surface->h*2 - 200) / 2;

		ntsc_emu = (atari_ntsc_t*) malloc( sizeof (atari_ntsc_t) );
		atari_ntsc_init( ntsc_emu, &mVideo.ntsc_setup );
		pc_blit_ntsc_filtered_screen_with_inputs ( ntsc_emu,tmp_surface, 
		                                           s_rect,
		                                           surface, d_rect);
		free ( ntsc_emu );
	}
	else
	{
		mXOffset = 0;
		s_rect.x = 32;
		s_rect.y = mYOffset;
		s_rect.w = 320;
		s_rect.h = 200;

		if ( s_rect.y > (tmp_surface->h - 200))
			s_rect.y = mYOffset = tmp_surface->h - 200;

		SDL_BlitSurface(tmp_surface, &s_rect, surface, &d_rect);
	}

	SDL_FreeSurface ( tmp_surface );
}

void VideoCfg::updateCurrentValues ( )
{
	t_config *p_config = config_get_ptr();
	t_atari_video *p_video = &mVideo;
	char tmp_string[257];

	strcpy ( p_video->description, mDescriptionBox.getText().c_str() );

	strcpy ( tmp_string, mPaletteList.getLabelFromIndex(mNTSCPalette.getSelected()) );
	strcpy ( p_video->ntsc_palette, tmp_string );
	strcpy ( tmp_string, mPaletteList.getLabelFromIndex(mPALPalette.getSelected()) );
	strcpy ( p_video->pal_palette, tmp_string );

	p_video->fullscreen = ( mFullScreen.isSelected() == true ) ? 1 : 0;
	p_video->widescreen = ( mWideScreen.isSelected() == true ) ? 1 : 0;

	p_video->pixel_depth = mColorDepthList.getValueFromIndex( mColorDepth.getSelected() );
	p_video->zoom = mZoomList.getValueFromIndex( mZoom.getSelected() );
	if ( mSpecify.isSelected() )
	{
		p_video->width = atoi ( mWidth.getText().c_str() );
		p_video->height = atoi ( mHeight.getText().c_str() );
	}
	else
	{
		p_video->width = 0;
		p_video->height = 0;
	}

	p_video->ntsc_filter_on = ( mNTSCFilter.isSelected() == true ) ? 1 : 0;

	p_video->ntsc_setup.hue = (float)mHue.getValue();
	p_video->ntsc_setup.saturation = (float)mSaturation.getValue();
	p_video->ntsc_setup.contrast = (float)mContrast.getValue();
	p_video->ntsc_setup.brightness = (float)mBrightness.getValue();
	p_video->ntsc_setup.sharpness = (float)mSharpness.getValue();

	p_video->ntsc_setup.gamma = mGamma.getValue();
	p_video->ntsc_setup.resolution = mResolution.getValue();
	p_video->ntsc_setup.burst_phase = (float)mArtifacts.getValue();
	p_video->ntsc_setup.fringing = mFringing.getValue();
	p_video->ntsc_setup.bleed = mBleed.getValue();
	p_video->ntsc_setup.hue_warping = (float)mHueWarping.getValue();
	p_video->ntsc_setup.merge_fields = ( mMergeFields.isSelected() == true ) ? 1 : 0;
}

void VideoCfg::saveSettings ( std::string profile_name )
{
	t_config *p_config = config_get_ptr();
	t_atari_video *p_video = &mVideo;

	updateCurrentValues();

	if ( !video_save_profile ( (char *)profile_name.c_str(), p_video ) ) {

		strcpy ( p_video->name, profile_name.c_str() );

		initProfileList();
		mProfile.setSelected ( mProfileList.getIndexFromLabel(profile_name.c_str()) );

		if ( mDefault.isSelected() == true ) {
			strcpy ( p_config->default_video_profile, p_video->name );
			config_save();
		}
	}
}

void VideoCfg::changeGroup ( std::string profile_name )
{
	char temp_string[10];
	t_config *p_config = config_get_ptr();
	t_atari_video *p_video = &mVideo;
	std::string actualfile;

	/*
	 * Save current settings, if necessary
	 */
	if ( mSettingsDirty == true ) {
		saveSettings( mProfileList.getLabelFromIndex(mProfile.getSelected()) );
	}

	mSettingsDirty = false;

	/*
	 * Get values for this Group
	 */
	if ( video_load_profile ( profile_name.c_str(), p_video ) ) {
		mGuiMain->showError(ERR_PROFILE_LOAD);

		video_load_profile ( p_config->default_video_profile, &mVideo );
		profile_name = p_config->default_video_profile;
	}
	mProfile.setSelected ( mProfileList.getIndexFromLabel(profile_name.c_str()) );

	/*
	 * Set values on the GUI
	 */
	mDescriptionBox.setText ( p_video->description );

	mPALPalette.setSelected ( mPaletteList.getIndexFromLabel (p_video->pal_palette) );
	mNTSCPalette.setSelected ( mPaletteList.getIndexFromLabel (p_video->ntsc_palette) );

	if ( !strcmp(profile_name.c_str(), p_config->default_video_profile) )
		mDefault.setSelected ( true );
	else
		mDefault.setSelected ( false );

	if ( p_video->fullscreen )
	{
		mFullScreen.setSelected ( true );
		setModesVisible ( true );
	}
	else
	{
		mFullScreen.setSelected ( false );
		setModesVisible ( false );
	}

	if ( p_video->widescreen )
		mWideScreen.setSelected ( true );
	else
		mWideScreen.setSelected ( false );

	if ( p_video->width && p_video->height )
	{
		mSpecify.setSelected ( true );
		setSpecifyVisible ( true );
	}
	else
	{
		mSpecify.setSelected ( false);
		setSpecifyVisible ( false );
	}
	sprintf ( temp_string, "%d", p_video->width );
	mWidth.setText ( temp_string );
	sprintf ( temp_string, "%d", p_video->height );
	mHeight.setText ( temp_string );
	mColorDepth.setSelected ( mColorDepthList.getIndexFromValue(p_video->pixel_depth) );
	mZoom.setSelected ( mZoomList.getIndexFromValue(p_video->zoom) );
	setupModeList ( p_video->pixel_depth );

	if ( p_video->ntsc_filter_on )
		mNTSCFilter.setSelected ( true );
	else
		mNTSCFilter.setSelected ( false );

	mHue.setValue(p_video->ntsc_setup.hue);
	mSaturation.setValue(p_video->ntsc_setup.saturation);
	mContrast.setValue(p_video->ntsc_setup.contrast);
	mBrightness.setValue(p_video->ntsc_setup.brightness);
	mSharpness.setValue(p_video->ntsc_setup.sharpness);
	mGamma.setValue(p_video->ntsc_setup.gamma);
	mResolution.setValue(p_video->ntsc_setup.resolution);
	mArtifacts.setValue(p_video->ntsc_setup.burst_phase);
	mFringing.setValue(p_video->ntsc_setup.fringing);
	mBleed.setValue(p_video->ntsc_setup.bleed);
	mHueWarping.setValue(p_video->ntsc_setup.hue_warping);

	if ( p_video->ntsc_setup.merge_fields )
		mMergeFields.setSelected ( true );
	else
		mMergeFields.setSelected ( false );

	setNtscVisible ( mNTSCFilter.isSelected() );
	setNtscCaptions ( );

	blitScreenImage();
}

void VideoCfg::action(const gcn::ActionEvent &actionEvent)
{
	char tmp_string[150];
	std::string new_name;

	if ( actionEvent.getId() == "mOk" ) {
		if ( mSettingsDirty == true ) {
			saveSettings( mProfileList.getLabelFromIndex(mProfile.getSelected()) );
		}
		hide();
	}
	else if ( actionEvent.getId() == "mCancel" ) {
		hide();
	}
	else if ( actionEvent.getId() == "mProfile" ) {
		if ( mSettingsDirty == true ) {
			saveSettings( mVideo.name );
		}
		changeGroup (mProfileList.getLabelFromIndex(mProfile.getSelected()) );
	}
	else if ( actionEvent.getId() == "mRename" ) {
		if ( !mProfileList.getNumberOfItems() ) 
		{
			mGuiMain->getInfoPopup()->show("No Existing Profiles.  No Action Taken");
			return;
		}
		mGuiMain->getMessagePopup()->deleteActions();
		mGuiMain->getMessagePopup()->addActionListener(this);
		mGuiMain->getMessagePopup()->setActionEventId("Rename");
		mGuiMain->getMessagePopup()->setInput(mProfileList.getLabelFromIndex(mProfile.getSelected()));
		mGuiMain->getMessagePopup()->show("Enter Profile Name", "Ok", true, 
		                                     "Cancel", true, "", false, true);
	}
	else if ( actionEvent.getId() == "Rename" ) {
		mGuiMain->getMessagePopup()->hide();
		if ( mGuiMain->getMessagePopup()->getButton() == 0 ) {
			new_name = mGuiMain->getMessagePopup()->getInput();
			if ( mProfileList.getIndexFromLabel(new_name.c_str()) < mProfileList.getNumberOfItems() ) {
				mGuiMain->getInfoPopup()->show("Profile Exists.  No Action Taken");
			}
			else {
				sprintf ( tmp_string, "UPDATE Video SET Name='%s' WHERE Name='%s'", 
				          new_name.c_str(), mProfileList.getLabelFromIndex(mProfile.getSelected()) );
				db_if_exec_sql ( tmp_string, NULL, NULL );
				initProfileList();
				changeGroup (new_name );
			}
		}
	}
	else if ( actionEvent.getId() == "mSaveAs" ) {
		mGuiMain->getProfilePopup()->deleteActions();
		mGuiMain->getProfilePopup()->addActionListener(this);
		mGuiMain->getProfilePopup()->setActionEventId("SaveBrowser");
		mGuiMain->getProfilePopup()->show( "Choose Profile or Create New", "SELECT Name FROM Video", 
		                     mProfileList.getLabelFromIndex(mProfile.getSelected()), true );
	}
	else if ( actionEvent.getId() == "SaveBrowser" ) {
		if ( mGuiMain->getProfilePopup()->getCancelPressed() == false ) {
			saveSettings ( mGuiMain->getProfilePopup()->getProfile() );
			mSettingsDirty = false;
		}
	}
	else if ( actionEvent.getId() == "mDirty" ) {
		mSettingsDirty = true;
	}
	else if ( actionEvent.getId() == "mPalette" ) {
		mSettingsDirty = true;
		blitScreenImage ();
	}
	else if ( actionEvent.getId() == "mFullScreen" ) {
		mSettingsDirty = true;
		setModesVisible ( mFullScreen.isSelected() );
	}
	else if ( actionEvent.getId() == "mSpecify" ) {
		setSpecifyVisible ( mSpecify.isSelected() );
	}
	else if ( actionEvent.getId() == "mMode" ) {
		mSettingsDirty = true;
		sprintf ( tmp_string, "%d",mModeList.getValueFromIndex( mMode.getSelected() ) );
		mWidth.setText ( tmp_string );
		sprintf ( tmp_string, "%d",mModeList.getDataFromIndex( mMode.getSelected() ) );
		mHeight.setText ( tmp_string );
	}
	else if ( actionEvent.getId() == "mColorDepth" ) {
		mSettingsDirty = true;
		setupModeList ( mColorDepthList.getValueFromIndex( mColorDepth.getSelected() ) );
	}
	else if ( actionEvent.getId() == "mNTSCFilter" ) {
		mSettingsDirty = true;
		setNtscVisible ( mNTSCFilter.isSelected() );
		blitScreenImage ();
	}
	else if ( actionEvent.getId() == "mCaptions" ) {
		mSettingsDirty = true;
		setNtscCaptions();
		blitScreenImage ();
	}
	else if ( actionEvent.getId() == "mRfPreset" ) {
		mSettingsDirty = true;
		setNtscPreset ( &atari_ntsc_rf );
		blitScreenImage ();
	}
	else if ( actionEvent.getId() == "mCompositePreset" ) {
		mSettingsDirty = true;
		setNtscPreset ( &atari_ntsc_composite );
		blitScreenImage ();
	}
	else if ( actionEvent.getId() == "mSVideoPreset" ) {
		mSettingsDirty = true;
		setNtscPreset ( &atari_ntsc_svideo );
		blitScreenImage ();
	}
	else if ( actionEvent.getId() == "mRGBPreset" ) {
		mSettingsDirty = true;
		setNtscPreset ( &atari_ntsc_rgb );
		blitScreenImage ();
	}
}

void VideoCfg::mousePressed(gcn::MouseEvent &mouseEvent)
{
	bool comp_has_mouse = false;

	if ( mouseEvent.getSource() == &mDescriptionBox || 
	     mouseEvent.getSource() == &mWidth || mouseEvent.getSource() == &mHeight )
		comp_has_mouse = true;

    if ((comp_has_mouse == true) && mouseEvent.getButton() == gcn::MouseEvent::LEFT)
    {
		mSettingsDirty = true;
    }

	if ( mouseEvent.getSource() == mDisplayImage )
    {
		mXStart = mouseEvent.getX();
		mYStart = mouseEvent.getY();
		mXOffsetStart = mXOffset;
		mYOffsetStart = mYOffset;
	}

	gcn::Window::mousePressed(mouseEvent);
}

void VideoCfg::mouseDragged(gcn::MouseEvent &mouseEvent)
{
	if ( mouseEvent.getSource() == mDisplayImage )
    {
		mXOffset = mXOffsetStart + mouseEvent.getX() - mXStart;
		mYOffset = mYOffsetStart + mouseEvent.getY() - mYStart;
    }

	blitScreenImage ();

	gcn::Window::mouseDragged(mouseEvent);
}
