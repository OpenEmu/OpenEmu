//============================================================================
//
//   SSSS    tt          lll  lll       
//  SS  SS   tt           ll   ll        
//  SS     tttttt  eeee   ll   ll   aaaa 
//   SSSS    tt   ee  ee  ll   ll      aa
//      SS   tt   eeeeee  ll   ll   aaaaa  --  "An Atari 2600 VCS Emulator"
//  SS  SS   tt   ee      ll   ll  aa  aa
//   SSSS     ttt  eeeee llll llll  aaaaa
//
// Copyright (c) 1995-2011 by Bradford W. Mott, Stephen Anthony
// and the Stella Team
//
// See the file "License.txt" for information on usage and redistribution of
// this file, and for a DISCLAIMER OF ALL WARRANTIES.
//
// $Id: TIA.hxx 2231 2011-05-10 15:04:19Z stephena $
//============================================================================

#ifndef TIA_HXX
#define TIA_HXX

class Console;
class Settings;
class Sound;

#include "bspf.hxx"
#include "Device.hxx"
#include "System.hxx"
#include "TIATables.hxx"

/**
  This class is a device that emulates the Television Interface Adapator 
  found in the Atari 2600 and 7800 consoles.  The Television Interface 
  Adapator is an integrated circuit designed to interface between an 
  eight bit microprocessor and a television video modulator. It converts 
  eight bit parallel data into serial outputs for the color, luminosity, 
  and composite sync required by a video modulator.  

  This class outputs the serial data into a frame buffer which can then
  be displayed on screen.

  @author  Bradford W. Mott
  @version $Id: TIA.hxx 2231 2011-05-10 15:04:19Z stephena $
*/
class TIA : public Device
{
  public:
    friend class TIADebug;

    /**
      Create a new TIA for the specified console

      @param console  The console the TIA is associated with
      @param sound    The sound object the TIA is associated with
      @param settings The settings object for this TIA device
    */
    TIA(Console& console, Sound& sound, Settings& settings);
 
    /**
      Destructor
    */
    virtual ~TIA();

  public:
    /**
      Reset device to its power-on state
    */
    void reset();

    /**
      Reset frame to current YStart/Height properties
    */
    void frameReset();

    /**
      Notification method invoked by the system right before the
      system resets its cycle counter to zero.  It may be necessary
      to override this method for devices that remember cycle counts.
    */
    void systemCyclesReset();

    /**
      Install TIA in the specified system.  Invoked by the system
      when the TIA is attached to it.

      @param system The system the device should install itself in
    */
    void install(System& system);

    /**
      Install TIA in the specified system and device.  Invoked by
      the system when the TIA is attached to it.  All devices
      which invoke this method take responsibility for chaining
      requests back to *this* device.

      @param system The system the device should install itself in
      @param device The device responsible for this address space
    */
    void install(System& system, Device& device);

    /**
      Save the current state of this device to the given Serializer.

      @param out  The Serializer object to use
      @return  False on any errors, else true
    */
    bool save(Serializer& out) const;

    /**
      Load the current state of this device from the given Serializer.

      @param in  The Serializer object to use
      @return  False on any errors, else true
    */
    bool load(Serializer& in);

    /**
      The following are very similar to save() and load(), except they
      do a 'deeper' save of the display data itself.

      Normally, the internal framebuffer doesn't need to be saved to
      a state file, since the file already contains all the information
      needed to re-create it, starting from scanline 0.  In effect, when a
      state is loaded, the framebuffer is empty, and the next call to
      update() generates valid framebuffer data.

      However, state files saved from the debugger need more information,
      such as the exact state of the internal framebuffer itself *before*
      we call update(), including if the display was in partial frame mode.

      Essentially, a normal state save has 'frame resolution', whereas
      the debugger state save has 'cycle resolution', and hence needs
      more information.  The methods below save/load this extra info,
      and eliminate having to save approx. 50K to normal state files.
    */
    bool saveDisplay(Serializer& out) const;
    bool loadDisplay(Serializer& in);

    /**
      Get a descriptor for the device name (used in error checking).

      @return The name of the object
    */
    string name() const { return "TIA"; }

    /**
      Get the byte at the specified address

      @return The byte at the specified address
    */
    uInt8 peek(uInt16 address);

    /**
      Change the byte at the specified address to the given value

      @param address The address where the value should be stored
      @param value The value to be stored at the address

      @return  True if the poke changed the device address space, else false
    */
    bool poke(uInt16 address, uInt8 value);

    /**
      This method should be called at an interval corresponding to the 
      desired frame rate to update the TIA.  Invoking this method will update
      the graphics buffer and generate the corresponding audio samples.
    */
    void update();

    /**
      Answers the current frame buffer

      @return Pointer to the current frame buffer
    */
    uInt8* currentFrameBuffer() const
      { return myCurrentFrameBuffer + myFramePointerOffset; }

    /**
      Answers the previous frame buffer

      @return Pointer to the previous frame buffer
    */
    uInt8* previousFrameBuffer() const
      { return myPreviousFrameBuffer + myFramePointerOffset; }

    /**
      Answers the width and height of the frame buffer
    */
    uInt32 width() const  { return myFrameWidth;  }
    uInt32 height() const { return myFrameHeight; }
    uInt32 ystart() const { return myFrameYStart; }

    /**
      Changes the current Height/YStart properties.
      Note that calls to these method(s) must be eventually followed by
      ::frameReset() for the changes to take effect.
    */
    void setHeight(uInt32 height) { myFrameHeight = height; }
    void setYStart(uInt32 ystart) { myFrameYStart = ystart; }

    /**
      Enables/disables auto-frame calculation.  If enabled, the TIA
      re-adjusts the framerate at regular intervals.

      @param mode  Whether to enable or disable all auto-frame calculation
    */
    void enableAutoFrame(bool mode) { myAutoFrameEnabled = mode; }

    /**
      Enables/disables color-loss for PAL modes only.

      @param mode  Whether to enable or disable PAL color-loss mode
    */
    void enableColorLoss(bool mode)
      { myColorLossEnabled = myFramerate <= 55 ? mode : false; }

    /**
      Answers the current color clock we've gotten to on this scanline.

      @return The current color clock
    */
    uInt32 clocksThisLine() const
      { return ((mySystem->cycles() * 3) - myClockWhenFrameStarted) % 228; }

    /**
      Answers the total number of scanlines the TIA generated in producing
      the current frame buffer. For partial frames, this will be the
      current scanline.

      @return The total number of scanlines generated
    */
    uInt32 scanlines() const
      { return ((mySystem->cycles() * 3) - myClockWhenFrameStarted) / 228; }

    /**
      Answers whether the TIA is currently in 'partial frame' mode
      (we're in between a call of startFrame and endFrame).

      @return If we're in partial frame mode
    */
    bool partialFrame() const { return myPartialFrameFlag; }

    /**
      Answers the first scanline at which drawing occured in the last frame.

      @return The starting scanline
    */
    uInt32 startScanline() const { return myStartScanline; }

    /**
      Answers the current position of the virtual 'electron beam' used to
      draw the TIA image.  If not in partial frame mode, the position is
      defined to be in the lower right corner (@ width/height of the screen).
      Note that the coordinates are with respect to currentFrameBuffer(),
      taking any YStart values into account.

      @return The x/y coordinates of the scanline electron beam, and whether
              it is in the visible/viewable area of the screen
    */
    bool scanlinePos(uInt16& x, uInt16& y) const;

    /**
      Enables/disables all TIABit bits.  Note that disabling a graphical
      object also disables its collisions.

      @param mode  Whether to enable or disable all bits
    */
    void enableBits(bool mode);

    /**
      Enables/disable/toggle the specified TIA bit.  Note that disabling a
      graphical object also disables its collisions.

      @param mode  1/0 indicates on/off, and values greater than 1 mean
                   flip the bit from its current state

      @return  Whether the bit was enabled or disabled
    */
    bool toggleBit(TIABit b, uInt8 mode = 2);

    /**
      Enables/disables all TIABit collisions.

      @param mode  Whether to enable or disable all collisions
    */
    void enableCollisions(bool mode);

    /**
      Enables/disable/toggle the specified TIA bit collision.

      @param mode  1/0 indicates on/off, and values greater than 1 mean
                   flip the collision from its current state

      @return  Whether the collision was enabled or disabled
    */
    bool toggleCollision(TIABit b, uInt8 mode = 2);

    /**
      Toggle the display of HMOVE blanks.

      @return  Whether the HMOVE blanking was enabled or disabled
    */
    bool toggleHMOVEBlank();

    /**
      Enables/disable/toggle 'fixed debug colors' mode.

      @param mode  1/0 indicates on/off, otherwise flip from
                   its current state

      @return  Whether the mode was enabled or disabled
    */
    bool toggleFixedColors(uInt8 mode = 2);

#ifdef DEBUGGER_SUPPORT
    /**
      This method should be called to update the TIA with a new scanline.
    */
    void updateScanline();

    /**
      This method should be called to update the TIA with a new partial
      scanline by stepping one CPU instruction.
    */
    void updateScanlineByStep();

    /**
      This method should be called to update the TIA with a new partial
      scanline by tracing to target address.
    */
    void updateScanlineByTrace(int target);
#endif

  private:
    // Update the current frame buffer to the specified color clock
    void updateFrame(Int32 clock);

    // Waste cycles until the current scanline is finished
    void waitHorizontalSync();

    // Clear both internal TIA buffers to black (palette color 0)
    void clearBuffers();

    // Set up bookkeeping for the next frame
    void startFrame();

    // Update bookkeeping at end of frame
    void endFrame();

    // Convert resistance from ports to dumped value
    uInt8 dumpedInputPort(int resistance);

    // Write the specified value to the HMOVE registers at the given clock
    void pokeHMP0(uInt8 value, Int32 clock);
    void pokeHMP1(uInt8 value, Int32 clock);
    void pokeHMM0(uInt8 value, Int32 clock);
    void pokeHMM1(uInt8 value, Int32 clock);
    void pokeHMBL(uInt8 value, Int32 clock);

    // Apply motion to registers when HMOVE is currently active
    void applyActiveHMOVEMotion(int hpos, Int16& pos, Int32 motionClock);

    // Apply motion to registers when HMOVE was previously active
    void applyPreviousHMOVEMotion(int hpos, Int16& pos, uInt8 motion);

  private:
    // Console the TIA is associated with
    Console& myConsole;

    // Sound object the TIA is associated with
    Sound& mySound;

    // Settings object the TIA is associated with
    Settings& mySettings;

    // Pointer to the current frame buffer
    uInt8* myCurrentFrameBuffer;

    // Pointer to the previous frame buffer
    uInt8* myPreviousFrameBuffer;

    // Pointer to the next pixel that will be drawn in the current frame buffer
    uInt8* myFramePointer;

    // Indicates offset used by the exported frame buffer
    // (the exported frame buffer is a vertical 'sliding window' of the actual buffer)
    uInt32 myFramePointerOffset;

    // Indicates the number of 'colour clocks' offset from the base
    // frame buffer pointer
    // (this is used when loading state files with a 'partial' frame)
    uInt32 myFramePointerClocks;

    // Indicates the width of the visible scanline
    uInt32 myFrameWidth;

    // Indicated what scanline the frame should start being drawn at
    uInt32 myFrameYStart;

    // Indicates the height of the frame in scanlines
    uInt32 myFrameHeight;

    // Indicates offset in color clocks when display should begin
    uInt32 myStartDisplayOffset;

    // Indicates offset in color clocks when display should stop
    uInt32 myStopDisplayOffset;

    // Indicates color clocks when the current frame began
    Int32 myClockWhenFrameStarted;

    // Indicates color clocks when frame should begin to be drawn
    Int32 myClockStartDisplay;

    // Indicates color clocks when frame should stop being drawn
    Int32 myClockStopDisplay;

    // Indicates color clocks when the frame was last updated
    Int32 myClockAtLastUpdate;

    // Indicates how many color clocks remain until the end of 
    // current scanline.  This value is valid during the 
    // displayed portion of the frame.
    Int32 myClocksToEndOfScanLine;

    // Indicates the total number of scanlines generated by the last frame
    uInt32 myScanlineCountForLastFrame;

    // Indicates the maximum number of scanlines to be generated for a frame
    uInt32 myMaximumNumberOfScanlines;

    // Indicates potentially the first scanline at which drawing occurs
    uInt32 myStartScanline;

    // Color clock when VSYNC ending causes a new frame to be started
    Int32 myVSYNCFinishClock; 

    uInt8 myVSYNC;        // Holds the VSYNC register value
    uInt8 myVBLANK;       // Holds the VBLANK register value

    uInt8 myNUSIZ0;       // Number and size of player 0 and missle 0
    uInt8 myNUSIZ1;       // Number and size of player 1 and missle 1

    uInt8 myPlayfieldPriorityAndScore;
    uInt8 myPriorityEncoder[2][256];
    uInt32 myColor[8];
    uInt32 myFixedColor[8];
    uInt32* myColorPtr;
    enum { _BK, _PF, _P0, _P1, _M0, _M1, _BL, _HBLANK };

    uInt8 myCTRLPF;       // Playfield control register

    bool myREFP0;         // Indicates if player 0 is being reflected
    bool myREFP1;         // Indicates if player 1 is being reflected

    uInt32 myPF;          // Playfield graphics (19-12:PF2 11-4:PF1 3-0:PF0)

    uInt8 myGRP0;         // Player 0 graphics register
    uInt8 myGRP1;         // Player 1 graphics register
    
    uInt8 myDGRP0;        // Player 0 delayed graphics register
    uInt8 myDGRP1;        // Player 1 delayed graphics register

    bool myENAM0;         // Indicates if missle 0 is enabled
    bool myENAM1;         // Indicates if missle 1 is enabled

    bool myENABL;         // Indicates if the ball is enabled
    bool myDENABL;        // Indicates if the vertically delayed ball is enabled

    uInt8 myHMP0;         // Player 0 horizontal motion register
    uInt8 myHMP1;         // Player 1 horizontal motion register
    uInt8 myHMM0;         // Missle 0 horizontal motion register
    uInt8 myHMM1;         // Missle 1 horizontal motion register
    uInt8 myHMBL;         // Ball horizontal motion register

    bool myVDELP0;        // Indicates if player 0 is being vertically delayed
    bool myVDELP1;        // Indicates if player 1 is being vertically delayed
    bool myVDELBL;        // Indicates if the ball is being vertically delayed

    bool myRESMP0;        // Indicates if missle 0 is reset to player 0
    bool myRESMP1;        // Indicates if missle 1 is reset to player 1

    uInt16 myCollision;     // Collision register

    // Determines whether specified collisions are enabled or disabled
    // The lower 16 bits are and'ed with the collision register to mask out
    // any collisions we don't want to be processed
    // The upper 16 bits are used to store which objects is currently
    // enabled or disabled
    // This is necessary since there are 15 collision combinations which
    // are controlled by 6 objects
    uInt32 myCollisionEnabledMask;

    // Note that these position registers contain the color clock 
    // on which the object's serial output should begin (0 to 159)
    Int16 myPOSP0;        // Player 0 position register
    Int16 myPOSP1;        // Player 1 position register
    Int16 myPOSM0;        // Missle 0 position register
    Int16 myPOSM1;        // Missle 1 position register
    Int16 myPOSBL;        // Ball position register

    // The color clocks elapsed so far for each of the graphical objects,
    // as denoted by 'MOTCK' line described in A. Towers TIA Hardware Notes
    Int32 myMotionClockP0;
    Int32 myMotionClockP1;
    Int32 myMotionClockM0;
    Int32 myMotionClockM1;
    Int32 myMotionClockBL;

    // Indicates 'start' signal for each of the graphical objects as
    // described in A. Towers TIA Hardware Notes
    Int32 myStartP0;
    Int32 myStartP1;
    Int32 myStartM0;
    Int32 myStartM1;

    // Index into the player mask arrays indicating whether display
    // of the first copy should be suppressed
    uInt8 mySuppressP0;
    uInt8 mySuppressP1;

    // Latches for 'more motion required' as described in A. Towers TIA
    // Hardware Notes
    bool myHMP0mmr;
    bool myHMP1mmr;
    bool myHMM0mmr;
    bool myHMM1mmr;
    bool myHMBLmmr;

    // Graphics for Player 0 that should be displayed.  This will be
    // reflected if the player is being reflected.
    uInt8 myCurrentGRP0;

    // Graphics for Player 1 that should be displayed.  This will be
    // reflected if the player is being reflected.
    uInt8 myCurrentGRP1;

    // It's VERY important that the BL, M0, M1, P0 and P1 current
    // mask pointers are always on a uInt32 boundary.  Otherwise,
    // the TIA code will fail on a good number of CPUs.

    // Pointer to the currently active mask array for the ball
    uInt8* myCurrentBLMask;

    // Pointer to the currently active mask array for missle 0
    uInt8* myCurrentM0Mask;

    // Pointer to the currently active mask array for missle 1
    uInt8* myCurrentM1Mask;

    // Pointer to the currently active mask array for player 0
    uInt8* myCurrentP0Mask;

    // Pointer to the currently active mask array for player 1
    uInt8* myCurrentP1Mask;

    // Pointer to the currently active mask array for the playfield
    uInt32* myCurrentPFMask;

    // Audio values; only used by TIADebug
    uInt8 myAUDV0, myAUDV1, myAUDC0, myAUDC1, myAUDF0, myAUDF1;

    // Indicates when the dump for paddles was last set
    Int32 myDumpDisabledCycle;

    // Indicates if the dump is current enabled for the paddles
    bool myDumpEnabled;

    // Latches for INPT4 and INPT5
    uInt8 myINPT4, myINPT5;

    // Indicates if HMOVE blanks are currently or previously enabled,
    // and at which horizontal position the HMOVE was initiated
    Int32 myCurrentHMOVEPos;
    Int32 myPreviousHMOVEPos;
    bool myHMOVEBlankEnabled;
    bool myAllowHMOVEBlanks;

    // Indicates if unused TIA pins are randomly driven high or low
    // Otherwise, they take on the value previously on the databus
    bool myTIAPinsDriven;

    // Bitmap of the objects that should be considered while drawing
    uInt8 myEnabledObjects;

    // Determines whether specified bits (from TIABit) are enabled or disabled
    // This is and'ed with the enabled objects each scanline to mask out any
    // objects we don't want to be processed
    uInt8 myDisabledObjects;

    // Indicates if color loss should be enabled or disabled.  Color loss
    // occurs on PAL (and maybe SECAM) systems when the previous frame
    // contains an odd number of scanlines.
    bool myColorLossEnabled;

    // Indicates whether we're done with the current frame. poke() clears this
    // when VSYNC is strobed or the max scanlines/frame limit is hit.
    bool myPartialFrameFlag;

    // Automatic framerate correction based on number of scanlines
    bool myAutoFrameEnabled;

    // Number of frames displayed by this TIA
    int myFrameCounter;

    // The framerate currently in use by the Console
    float myFramerate;

  private:
    // Copy constructor isn't supported by this class so make it private
    TIA(const TIA&);

    // Assignment operator isn't supported by this class so make it private
    TIA& operator = (const TIA&);
};

#endif
