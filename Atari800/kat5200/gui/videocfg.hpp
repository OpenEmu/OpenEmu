/******************************************************************************
*
* FILENAME: videocfg.hpp
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
#ifndef videocfg_hpp
#define videocfg_hpp

#include <guichan.hpp>
#include "genericlist.hpp"
#include "decoration.hpp"
#include "guiobject.hpp"
#include "window.hpp"
#include "tabbedwindow.hpp"

extern "C" {
	#include "../interface/atari_ntsc.h"
	#include "../interface/video.h"
}

class VideoCfg : public gcn::GuiWindow,
                 public GuiObject,
                 public gcn::ActionListener
{
	public:
		VideoCfg( GuiMain *gMain );
		~VideoCfg();

		void show (); 
		void hide ();

		void initProfileList ( );
		void initPaletteList ( );
		void setNtscVisible ( bool setting );
		void setNtscCaptions ();
		void setNtscPreset ( const atari_ntsc_setup_t *setup );
		void setModesVisible ( bool setting );
		void setSpecifyVisible ( bool setting );
		void setupModeList ( int pixel_depth );
		void initGroupControls ();
		void blitScreenImage ();
		void changeGroup ( std::string profile_name );
		void updateCurrentValues ();
		void saveSettings ( std::string profile_name );

		void action(const gcn::ActionEvent &actionEvent);
		void mousePressed(gcn::MouseEvent &mouseEvent);
		void mouseDragged(gcn::MouseEvent &mouseEvent);

	private:
		gcn::DropDown mProfile;
		GenericList mProfileList;
		gcn::CheckBox mDefault;
		gcn::Button mRename;
		gcn::Button mSaveAs;
		gcn::TextField mDescriptionBox;
		gcn::Label mDescriptionLabel;
		Decoration mProfileDec;

		gcn::Button mOk;
		gcn::Button mCancel;

		GenericList mPaletteList;
		Decoration mPaletteDec;

		gcn::Label mNTSCLabel;
		gcn::DropDown mNTSCPalette;

		gcn::Label mPALLabel;
		gcn::DropDown mPALPalette;

		Decoration mVideoDec;
		gcn::CheckBox mFullScreen;
		gcn::CheckBox mWideScreen;
		gcn::CheckBox mSpecify;
		gcn::Label mColorDepthLabel;
		gcn::DropDown mColorDepth;
		GenericList mColorDepthList;
		gcn::Label mModeLabel;
		gcn::DropDown mMode;
		GenericList mModeList;
		gcn::Label mZoomLabel;
		gcn::DropDown mZoom;
		GenericList mZoomList;
		gcn::Label mHeightLabel;
		gcn::TextField mWidth;
		gcn::TextField mHeight;

		Decoration mFilterDec;
		gcn::CheckBox mNTSCFilter;
		gcn::Slider mHue;
		gcn::Slider mSaturation;
		gcn::Slider mContrast;
		gcn::Slider mBrightness;
		gcn::Slider mSharpness;

		gcn::Slider mGamma;
		gcn::Slider mResolution;
		gcn::Slider mArtifacts;
		gcn::Slider mFringing;
		gcn::Slider mBleed;
		gcn::Slider mHueWarping;
		gcn::CheckBox mMergeFields;

		gcn::Label mHueLabel;
		gcn::Label mSaturationLabel;
		gcn::Label mContrastLabel;
		gcn::Label mBrightnessLabel;
		gcn::Label mSharpnessLabel;
		gcn::Label mGammaLabel;
		gcn::Label mResolutionLabel;
		gcn::Label mArtifactsLabel;
		gcn::Label mFringingLabel;
		gcn::Label mBleedLabel;
		gcn::Label mHueWarpingLabel;

		gcn::Label mHueValue;
		gcn::Label mSaturationValue;
		gcn::Label mContrastValue;
		gcn::Label mBrightnessValue;
		gcn::Label mSharpnessValue;
		gcn::Label mGammaValue;
		gcn::Label mResolutionValue;
		gcn::Label mArtifactsValue;
		gcn::Label mFringingValue;
		gcn::Label mBleedValue;
		gcn::Label mHueWarpingValue;

		Decoration mPresetDec;
		gcn::Button mRfPreset;
		gcn::Button mCompositePreset;
		gcn::Button mSVideoPreset;
		gcn::Button mRGBPreset;

		gcn::Icon *mDisplayImage;
		gcn::Image *mDisplaySurface;

		t_atari_video mVideo;

		int mXStart;
		int mYStart;
		int mXOffset;
		int mYOffset;
		int mXOffsetStart;
		int mYOffsetStart;
		bool mSettingsDirty;
		int mCurrentGrp;
};

#endif
