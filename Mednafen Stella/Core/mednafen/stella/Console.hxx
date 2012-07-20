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
// $Id: Console.hxx 2231 2011-05-10 15:04:19Z stephena $
//============================================================================

#ifndef CONSOLE_HXX
#define CONSOLE_HXX

class Controller;
class Event;
class Switches;
class System;
class TIA;
class M6532;
class Cartridge;

#include "bspf.hxx"
#include "Control.hxx"
#include "Props.hxx"
#include "TIATables.hxx"
//ROBO: No FrameBuffer support
//#include "FrameBuffer.hxx"
#include "Serializable.hxx"

//ROBO: For sound
#include "Sound.hxx"

/**
  Contains detailed info about a console.
*/
struct ConsoleInfo
{
  string BankSwitch;
  string CartName;
  string CartMD5;
  string Control0;
  string Control1;
  string DisplayFormat;
  string InitialFrameRate;
};

/**
  This class represents the entire game console.

  @author  Bradford W. Mott
  @version $Id: Console.hxx 2231 2011-05-10 15:04:19Z stephena $
*/
class Console : public Serializable
{
  public:
    /**
      Create a new console for emulating the specified game using the
      given game image and operating system.

      @param osystem  The OSystem object to use
      @param cart     The cartridge to use with this console
      @param props    The properties for the cartridge  
    */
//ROBO: No OSystem support, just Sound
    Console(Sound& osystem, Cartridge* cart, const Properties& props);

    /**
      Create a new console object by copying another one

      @param console The object to copy
    */
    Console(const Console& console);
 
    /**
      Destructor
    */
    virtual ~Console();

  public:
    /**
      Get the controller plugged into the specified jack

      @return The specified controller
    */
    Controller& controller(Controller::Jack jack) const
    {
      return (jack == Controller::Left) ? *myControllers[0] : *myControllers[1];
    }

    /**
      Get the TIA for this console

      @return The TIA
    */
    TIA& tia() const { return *myTIA; }

    /**
      Get the properties being used by the game

      @return The properties being used by the game
    */
    const Properties& properties() const { return myProperties; }

    /**
      Get the console switches

      @return The console switches
    */
    Switches& switches() const { return *mySwitches; }

    /**
      Get the 6502 based system used by the console to emulate the game

      @return The 6502 based system
    */
    System& system() const { return *mySystem; }

    /**
      Get the cartridge used by the console which contains the ROM code

      @return The cartridge for this console
    */
    Cartridge& cartridge() const { return *myCart; }

    /**
      Get the 6532 used by the console

      @return The 6532 for this console
    */
    M6532& riot() const { return *myRiot; }

    /**
      Saves the current state of this console class to the given Serializer.

      @param out The serializer device to save to.
      @return The result of the save.  True on success, false on failure.
    */
    bool save(Serializer& out) const;

    /**
      Loads the current state of this console class from the given Serializer.

      @param in The Serializer device to load from.
      @return The result of the load.  True on success, false on failure.
    */
    bool load(Serializer& in);

    /**
      Get a descriptor for this console class (used in error checking).

      @return The name of the object
    */
    string name() const { return "Console"; }

    /**
      Set the properties to those given

      @param The properties to use for the current game
    */
    void setProperties(const Properties& props);

    /**
      Query detailed information about this console.
    */
    const ConsoleInfo& about() const { return myConsoleInfo; }

  public:
    /**
      Overloaded assignment operator

      @param console The console object to set myself equal to
      @return Myself after assignment has taken place
    */
    Console& operator = (const Console& console);

  public:
    /**
      Toggle between NTSC/PAL/SECAM (and variants) display format.
    */
    void toggleFormat();

    /**
      Toggle between the available palettes.
    */
    void togglePalette();

    /**
      Sets the palette according to the given palette name.

      @param palette  The palette to switch to.
    */
    void setPalette(const string& palette);

    /**
      Toggles phosphor effect.
    */
    void togglePhosphor();

    /**
      Toggles the PAL color-loss effect.
    */
    void toggleColorLoss();

    /**
      Initialize the video subsystem wrt this class.
      This is required for changing window size, title, etc.

      @param full  Whether we want a full initialization,
                   or only reset certain attributes.

      @return  The results from FrameBuffer::initialize()
    */
//ROBO: Function isn't needed
//    FBInitStatus initializeVideo(bool full = true);

    /**
      Initialize the audio subsystem wrt this class.
      This is required any time the sound settings change.
    */
    void initializeAudio();

    /**
      "Fry" the Atari (mangle memory/TIA contents)
    */
    void fry() const;

    /**
      Change the "Display.YStart" variable.

      @param direction +1 indicates increase, -1 indicates decrease.
    */
    void changeYStart(int direction);

    /**
      Change the "Display.Height" variable.

      @param direction +1 indicates increase, -1 indicates decrease.
    */
    void changeHeight(int direction);

    /**
      Sets the framerate of the console, which in turn communicates
      this to all applicable subsystems.
    */
    void setFramerate(float framerate);

    /**
      Returns the framerate based on a number of factors
      (whether 'framerate' is set, what display format is in use, etc)
    */
    float getFramerate() const { return myFramerate; }

    /**
      Toggles the TIA bit specified in the method name.
    */
    void toggleP0Bit() const { toggleTIABit(P0Bit, "P0"); }
    void toggleP1Bit() const { toggleTIABit(P1Bit, "P1"); }
    void toggleM0Bit() const { toggleTIABit(M0Bit, "M0"); }
    void toggleM1Bit() const { toggleTIABit(M1Bit, "M1"); }
    void toggleBLBit() const { toggleTIABit(BLBit, "BL"); }
    void togglePFBit() const { toggleTIABit(PFBit, "PF"); }
    void toggleHMOVE() const;
    void enableBits(bool enable) const;

    /**
      Toggles the TIA collisions specified in the method name.
    */
    void toggleP0Collision() const { toggleTIACollision(P0Bit, "P0"); }
    void toggleP1Collision() const { toggleTIACollision(P1Bit, "P1"); }
    void toggleM0Collision() const { toggleTIACollision(M0Bit, "M0"); }
    void toggleM1Collision() const { toggleTIACollision(M1Bit, "M1"); }
    void toggleBLCollision() const { toggleTIACollision(BLBit, "BL"); }
    void togglePFCollision() const { toggleTIACollision(PFBit, "PF"); }
    void enableCollisions(bool enable) const;

    /**
      Toggles the TIA 'fixed debug colors' mode.
    */
    void toggleFixedColors() const;

	//ROBO: Get the event
	Event& event() {return *myEvent;}

  private:
    /**
      Adds the left and right controllers to the console.
    */
    void setControllers(const string& rommd5);

    /**
      Loads a user-defined palette file (from OSystem::paletteFile), filling the
      appropriate user-defined palette arrays.
    */
    void loadUserPalette();

    /**
      Loads all defined palettes with PAL color-loss data, even those that
      normally can't have it enabled (NTSC), since it's also used for
      'greying out' the frame in the debugger.
    */
    void setColorLossPalette();

    /**
      Returns a pointer to the palette data for the palette currently defined
      by the ROM properties.
    */
    const uInt32* getPalette(int direction) const;

    void toggleTIABit(TIABit bit, const string& bitname, bool show = true) const;
    void toggleTIACollision(TIABit bit, const string& bitname, bool show = true) const;

  private:
    // Pointer to the osystem object
//ROBO: No OSystem
//    OSystem* myOSystem;

    // Pointers to the left and right controllers
    Controller* myControllers[2];

    // Pointer to the event object to use
    Event* myEvent;

    // Pointer to the TIA object 
    TIA* myTIA;

    // Properties for the game
    Properties myProperties;

    // Pointer to the switches on the front of the console
    Switches* mySwitches;
 
    // Pointer to the 6502 based system being emulated 
    System* mySystem;

    // Pointer to the Cartridge (the debugger needs it)
    Cartridge *myCart;

    // Pointer to the 6532 (aka RIOT) (the debugger needs it)
    // A RIOT of my own! (...with apologies to The Clash...)
    M6532 *myRiot;

    // The currently defined display format (NTSC/PAL/SECAM)
    string myDisplayFormat;

    // The currently defined display framerate
    float myFramerate;

    // Indicates whether an external palette was found and
    // successfully loaded
    bool myUserPaletteDefined;

    // Contains detailed info about this console
    ConsoleInfo myConsoleInfo;

    // Table of RGB values for NTSC, PAL and SECAM
    static uInt32 ourNTSCPalette[256];
    static uInt32 ourPALPalette[256];
    static uInt32 ourSECAMPalette[256];

    // Table of RGB values for NTSC, PAL and SECAM - Z26 version
    static uInt32 ourNTSCPaletteZ26[256];
    static uInt32 ourPALPaletteZ26[256];
    static uInt32 ourSECAMPaletteZ26[256];

    // Table of RGB values for NTSC, PAL and SECAM - user-defined
    static uInt32 ourUserNTSCPalette[256];
    static uInt32 ourUserPALPalette[256];
    static uInt32 ourUserSECAMPalette[256];
};

#endif
