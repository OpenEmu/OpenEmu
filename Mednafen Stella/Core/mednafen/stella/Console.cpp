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
// $Id: Console.cxx 2232 2011-05-24 16:04:48Z stephena $
//============================================================================

#include <cassert>
#include <iostream>
#include <sstream>
#include <fstream>

#include "AtariVox.hxx"
#include "Booster.hxx"
#include "Cart.hxx"
#include "Control.hxx"
#include "Cart.hxx"
#include "Driving.hxx"
#include "Event.hxx"
//ROBO: No EventHandler
//#include "EventHandler.hxx"
#include "Joystick.hxx"
#include "Keyboard.hxx"
#include "KidVid.hxx"
#include "Genesis.hxx"
#include "M6502.hxx"
#include "M6532.hxx"
#include "Paddles.hxx"
#include "Props.hxx"
#include "PropsSet.hxx"
#include "SaveKey.hxx"
#include "Settings.hxx" 
#include "Sound.hxx"
#include "Switches.hxx"
#include "System.hxx"
#include "TIA.hxx"
#include "TrackBall.hxx"
//ROBO: No FrameBuffer or OSystem or Menu or CommandMenu
//#include "FrameBuffer.hxx"
//#include "OSystem.hxx"
//#include "Menu.hxx"
//#include "CommandMenu.hxx"
#include "Serializable.hxx"
#include "Version.hxx"

#ifdef DEBUGGER_SUPPORT
  #include "Debugger.hxx"
#endif

#ifdef CHEATCODE_SUPPORT
  #include "CheatManager.hxx"
#endif

#include "Console.hxx"

//ROBO: External global settings
extern Settings& stellaMDFNSettings();

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//ROBO: No OSystem, just sound
//Console::Console(OSystem* osystem, Cartridge* cart, const Properties& props)
Console::Console(Sound& sound, Cartridge* cart, const Properties& props)
//: //myOSystem(osystem),
  : 
    myProperties(props),
    myDisplayFormat("NTSC"),
    myFramerate(60.0),
    myUserPaletteDefined(false)
{
  myControllers[0] = 0;
  myControllers[1] = 0;
  myTIA = 0;
  mySwitches = 0;
  mySystem = 0;
  myEvent = 0;

  // Attach the event subsystem to the current console
//ROBO: No OSystem, create own event
//  myEvent = myOSystem->eventHandler().event();
  myEvent = new Event();

  // Load user-defined palette for this ROM
  loadUserPalette();

  // Create switches for the console
  mySwitches = new Switches(*myEvent, myProperties);

  // Construct the system and components
  mySystem = new System(13, 6);

  // The real controllers for this console will be added later
  // For now, we just add dummy joystick controllers, since autodetection
  // runs the emulation for a while, and this may interfere with 'smart'
  // controllers such as the AVox and SaveKey
  // Note that the controllers must be added directly after the system
  // has been created, and before any other device is added
  // (particularly the M6532)
  myControllers[0] = new Joystick(Controller::Left, *myEvent, *mySystem);
  myControllers[1] = new Joystick(Controller::Right, *myEvent, *mySystem);

  M6502* m6502 = new M6502(1);
#ifdef DEBUGGER_SUPPORT
  m6502->attach(myOSystem->debugger());
#endif

  myCart = cart;
//ROBO: External global settings
//myRiot = new M6532(*this, myOSystem->settings());
//myTIA  = new TIA(*this, myOSystem->sound(), myOSystem->settings());
  myRiot = new M6532(*this, stellaMDFNSettings());
  myTIA  = new TIA(*this, sound, stellaMDFNSettings());


  mySystem->attach(m6502);
  mySystem->attach(myRiot);
  mySystem->attach(myTIA);
  mySystem->attach(myCart);

#ifdef DEBUGGER_SUPPORT
  // The debugger must be added before we run the console for the first time
  myOSystem->debugger().setConsole(this);
  myOSystem->debugger().initialize();
#endif

  // Auto-detect NTSC/PAL mode if it's requested
  string autodetected = "";
  myDisplayFormat = myProperties.get(Display_Format);
  if(myDisplayFormat == "AUTO-DETECT" ||
//ROBO: External global settings
//     myOSystem->settings().getBool("rominfo"))
     stellaMDFNSettings().getBool("rominfo"))
  {
    // Run the system for 60 frames, looking for PAL scanline patterns
    // We turn off the SuperCharger progress bars, otherwise the SC BIOS
    // will take over 250 frames!
    // The 'fastscbios' option must be changed before the system is reset
//ROBO: External global settings
//    bool fastscbios = myOSystem->settings().getBool("fastscbios");
//    myOSystem->settings().setBool("fastscbios", true);
    bool fastscbios = stellaMDFNSettings().getBool("fastscbios");
    stellaMDFNSettings().setBool("fastscbios", true);

    mySystem->reset(true);  // autodetect in reset enabled
    int palCount = 0;
    for(int i = 0; i < 60; ++i)
    {
      myTIA->update();
      if(myTIA->scanlines() > 285)
        ++palCount;
    }
    myDisplayFormat = (palCount >= 20) ? "PAL" : "NTSC";
    if(myProperties.get(Display_Format) == "AUTO-DETECT")
      autodetected = "*";

    // Don't forget to reset the SC progress bars again
//ROBO: External global settings
//    myOSystem->settings().setBool("fastscbios", fastscbios);
    stellaMDFNSettings().setBool("fastscbios", fastscbios);
  }
  myConsoleInfo.DisplayFormat = myDisplayFormat + autodetected;

  // Set up the correct properties used when toggling format
  // Note that this can be overridden if a format is forced
  //   For example, if a PAL ROM is forced to be NTSC, it will use NTSC-like
  //   properties (60Hz, 262 scanlines, etc) and cycle between NTSC-like modes
  // The TIA will self-adjust the framerate if necessary

  // TODO - query these values directly from the TIA if value is 'AUTO'
  uInt32 ystart = atoi(myProperties.get(Display_YStart).c_str());
  if(ystart < 0)       ystart = 0;
  else if(ystart > 64) ystart = 64;
  uInt32 height = atoi(myProperties.get(Display_Height).c_str());
  if(height < 210)      height = 210;
  else if(height > 256) height = 256;

  if(myDisplayFormat == "NTSC" || myDisplayFormat == "PAL60" ||
     myDisplayFormat == "SECAM60")
  {
    // Assume we've got ~262 scanlines (NTSC-like format)
    myFramerate = 60.0;
    myConsoleInfo.InitialFrameRate = "60";
  }
  else
  {
    // Assume we've got ~312 scanlines (PAL-like format)
    myFramerate = 50.0;
    myConsoleInfo.InitialFrameRate = "50";

    // PAL ROMs normally need at least 250 lines
    height = BSPF_max(height, 250u);
  }

  // Make sure these values fit within the bounds of the desktop
  // If not, attempt to center vertically
//ROBO: Fixed size
/*  if(height <= myOSystem->desktopHeight())
  {
    myTIA->setYStart(ystart);
    myTIA->setHeight(height);
  }
  else
  {
    ystart += height - myOSystem->desktopHeight();
    ystart = BSPF_min(ystart, 64u);
    height = myOSystem->desktopHeight();
    myTIA->setYStart(ystart);
    myTIA->setHeight(height);
  }*/

  myTIA->setYStart(ystart);
  myTIA->setHeight(height);


//ROBO: Send a palette, whoever was supposed to do this is gone now
  setPalette(stellaMDFNSettings().getString("palette"));

  const string& md5 = myProperties.get(Cartridge_MD5);

  // Add the real controllers for this system
  setControllers(md5);

  // Bumper Bash always requires all 4 directions
  // Other ROMs can use it if the setting is enabled
//ROBO: TODO: No EventHandler...
/*  bool joyallow4 = md5 == "aa1c41f86ec44c0a44eb64c332ce08af" ||
                   md5 == "1bf503c724001b09be79c515ecfcbd03" ||
                  myOSystem->settings().getBool("joyallow4");
  myOSystem->eventHandler().allowAllDirections(joyallow4);*/

  // Reset the system to its power-on state
  mySystem->reset();

  // Finally, add remaining info about the console
  myConsoleInfo.CartName   = myProperties.get(Cartridge_Name);
  myConsoleInfo.CartMD5    = myProperties.get(Cartridge_MD5);
  myConsoleInfo.Control0   = myControllers[0]->about();
  myConsoleInfo.Control1   = myControllers[1]->about();
  myConsoleInfo.BankSwitch = cart->about();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Console::~Console()
{
//ROBO: Delete myEvent
  delete myEvent;

  delete mySystem;
  delete mySwitches;
  delete myControllers[0];
  delete myControllers[1];
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool Console::save(Serializer& out) const
{
  try
  {
    // First save state for the system
    if(!mySystem->save(out))
      return false;

    // Now save the console switches
    if(!mySwitches->save(out))
      return false;
  }
  catch(const char* msg)
  {
    cerr << "ERROR: Console::save" << endl << "  " << msg << endl;
    return false;
  }

  return true;  // success
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool Console::load(Serializer& in)
{
  try
  {
    // First load state for the system
    if(!mySystem->load(in))
      return false;

    // Then load the console switches
    if(!mySwitches->load(in))
      return false;
  }
  catch(const char* msg)
  {
    cerr << "ERROR: Console::load" << endl << "  " << msg << endl;
    return false;
  }

  return true;  // success
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Console::toggleFormat()
{
  string format, message;

  if(myDisplayFormat.compare(0, 4, "NTSC") == 0)
  {
    if(myFramerate > 55.0)
    {
      format  = "PAL60";
      message = "PAL palette (PAL60)";
    }
    else
    {
      format  = "PAL";
      message = "PAL palette (PAL)";
    }
  }
  else if(myDisplayFormat.compare(0, 3, "PAL") == 0)
  {
    if(myFramerate > 55.0)
    {
      format  = "SECAM";
      message = "SECAM palette (SECAM60)";
    }
    else
    {
      format  = "SECAM";
      message = "SECAM palette (SECAM)";
    }
  }
  else if(myDisplayFormat.compare(0, 5, "SECAM") == 0)
  {
    if(myFramerate > 55.0)
    {
      format  = "NTSC";
      message = "NTSC palette (NTSC)";
    }
    else
    {
      format  = "NTSC50";
      message = "NTSC palette (NTSC50)";
    }
  }

  myDisplayFormat = format;
  myProperties.set(Display_Format, myDisplayFormat);
//ROBO: No FrameBuffer Messages
//  myOSystem->frameBuffer().showMessage(message);
//ROBO: External global settings
//  setPalette(myOSystem->settings().getString("palette"));
  setPalette(stellaMDFNSettings().getString("palette"));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Console::toggleColorLoss()
{
//ROBO: Global settings
//  bool colorloss = !myOSystem->settings().getBool("colorloss");
//  myOSystem->settings().setBool("colorloss", colorloss);
  bool colorloss = !stellaMDFNSettings().getBool("colorloss");
  stellaMDFNSettings().setBool("colorloss", colorloss);
  myTIA->enableColorLoss(colorloss);

//ROBO: No Framebuffer Messages
//  string message = string("PAL color-loss ") +
//                   (colorloss ? "enabled" : "disabled");
//  myOSystem->frameBuffer().showMessage(message);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Console::togglePalette()
{
  string palette, message;
//ROBO: External settings
//  palette = myOSystem->settings().getString("palette");
  palette = stellaMDFNSettings().getString("palette");
 
  if(palette == "standard")       // switch to z26
  {
    palette = "z26";
    message = "Z26 palette";
  }
  else if(palette == "z26")       // switch to user or standard
  {
    // If we have a user-defined palette, it will come next in
    // the sequence; otherwise loop back to the standard one
    if(myUserPaletteDefined)
    {
      palette = "user";
      message = "User-defined palette";
    }
    else
    {
      palette = "standard";
      message = "Standard Stella palette";
    }
  }
  else if(palette == "user")  // switch to standard
  {
    palette = "standard";
    message = "Standard Stella palette";
  }
  else  // switch to standard mode if we get this far
  {
    palette = "standard";
    message = "Standard Stella palette";
  }

//ROBO: External settings
//  myOSystem->settings().setString("palette", palette);
  stellaMDFNSettings().setString("palette", palette);
//ROBO: No FrameBuffer Messages
//  myOSystem->frameBuffer().showMessage(message);

  setPalette(palette);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//ROBO: Function to receive palette
extern void stellaMDFNSetPalette(const uInt32* palette);

void Console::setPalette(const string& type)
{
  // Look at all the palettes, since we don't know which one is
  // currently active
  uInt32* palettes[3][3] = {
    { &ourNTSCPalette[0],    &ourPALPalette[0],    &ourSECAMPalette[0]    },
    { &ourNTSCPaletteZ26[0], &ourPALPaletteZ26[0], &ourSECAMPaletteZ26[0] },
    { 0, 0, 0 }
  };
  if(myUserPaletteDefined)
  {
    palettes[2][0] = &ourUserNTSCPalette[0];
    palettes[2][1] = &ourUserPALPalette[0];
    palettes[2][2] = &ourUserSECAMPalette[0];
  }

  // See which format we should be using
  int paletteNum = 0;
  if(type == "standard")
    paletteNum = 0;
  else if(type == "z26")
    paletteNum = 1;
  else if(type == "user" && myUserPaletteDefined)
    paletteNum = 2;

  // Now consider the current display format
  const uInt32* palette =
    (myDisplayFormat.compare(0, 3, "PAL") == 0)   ? palettes[paletteNum][1] :
    (myDisplayFormat.compare(0, 5, "SECAM") == 0) ? palettes[paletteNum][2] :
     palettes[paletteNum][0];

//ROBO: Send palette to custom function
//  myOSystem->frameBuffer().setTIAPalette(palette);
  stellaMDFNSetPalette(palette);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Console::togglePhosphor()
{
  const string& phosphor = myProperties.get(Display_Phosphor);
  int blend = atoi(myProperties.get(Display_PPBlend).c_str());
  bool enable;
  if(phosphor == "YES")
  {
    myProperties.set(Display_Phosphor, "No");
    enable = false;
//ROBO: No FrameBuffer Messages
//    myOSystem->frameBuffer().showMessage("Phosphor effect disabled");
  }
  else
  {
    myProperties.set(Display_Phosphor, "Yes");
    enable = true;
//ROBO: No FrameBuffer Messages
//    myOSystem->frameBuffer().showMessage("Phosphor effect enabled");
  }

//ROBO TODO: No framebuffer
//  myOSystem->frameBuffer().enablePhosphor(enable, blend);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Console::setProperties(const Properties& props)
{
  myProperties = props;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//ROBO TODO: Function not needed?
/*FBInitStatus Console::initializeVideo(bool full)
{
  FBInitStatus fbstatus = kSuccess;

  if(full)
  {
    const string& title = string("Stella ") + STELLA_VERSION +
                   ": \"" + myProperties.get(Cartridge_Name) + "\"";
    fbstatus = myOSystem->frameBuffer().initialize(title,
                 myTIA->width() << 1, myTIA->height());
    if(fbstatus != kSuccess)
      return fbstatus;

    myOSystem->frameBuffer().showFrameStats(myOSystem->settings().getBool("stats"));
    setColorLossPalette();
  }

  bool enable = myProperties.get(Display_Phosphor) == "YES";
  int blend = atoi(myProperties.get(Display_PPBlend).c_str());
  myOSystem->frameBuffer().enablePhosphor(enable, blend);
  setPalette(myOSystem->settings().getString("palette"));

  // Set the correct framerate based on the format of the ROM
  // This can be overridden by changing the framerate in the
  // VideoDialog box or on the commandline, but it can't be saved
  // (ie, framerate is now determined based on number of scanlines).
  int framerate = myOSystem->settings().getInt("framerate");
  if(framerate > 0) myFramerate = float(framerate);
  myOSystem->setFramerate(myFramerate);

  // Make sure auto-frame calculation is only enabled when necessary
  myTIA->enableAutoFrame(framerate <= 0);

  return fbstatus;
}*/

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Console::initializeAudio()
{
//ROBO TODO: Dummied function
/*  // Initialize the sound interface.
  // The # of channels can be overridden in the AudioDialog box or on
  // the commandline, but it can't be saved.
  int framerate = myOSystem->settings().getInt("framerate");
  if(framerate > 0) myFramerate = float(framerate);
  const string& sound = myProperties.get(Cartridge_Sound);
  uInt32 channels = (sound == "STEREO" ? 2 : 1);

  myOSystem->sound().close();
  myOSystem->sound().setChannels(channels);
  myOSystem->sound().setFrameRate(myFramerate);
  myOSystem->sound().open();

  // Make sure auto-frame calculation is only enabled when necessary
  myTIA->enableAutoFrame(framerate <= 0);*/
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
/* Original frying research and code by Fred Quimby.
   I've tried the following variations on this code:
   - Both OR and Exclusive OR instead of AND. This generally crashes the game
     without ever giving us realistic "fried" effects.
   - Loop only over the RIOT RAM. This still gave us frying-like effects, but
     it seemed harder to duplicate most effects. I have no idea why, but
     munging the TIA regs seems to have some effect (I'd think it wouldn't).

   Fred says he also tried mangling the PC and registers, but usually it'd just
   crash the game (e.g. black screen, no way out of it).

   It's definitely easier to get some effects (e.g. 255 lives in Battlezone)
   with this code than it is on a real console. My guess is that most "good"
   frying effects come from a RIOT location getting cleared to 0. Fred's
   code is more likely to accomplish this than frying a real console is...

   Until someone comes up with a more accurate way to emulate frying, I'm
   leaving this as Fred posted it.   -- B.
*/
void Console::fry() const
{
  for (int ZPmem=0; ZPmem<0x100; ZPmem += rand() % 4)
    mySystem->poke(ZPmem, mySystem->peek(ZPmem) & (uInt8)rand() % 256);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Console::changeYStart(int direction)
{
  uInt32 ystart = myTIA->ystart();

  if(direction == +1)       // increase YStart
  {
    ystart++;
    if(ystart > 64)
    {
//ROBO: No FrameBuffer Messages
//      myOSystem->frameBuffer().showMessage("YStart at maximum");
      return;
    }
  }
  else if(direction == -1)  // decrease YStart
  {
    ystart--;
    if(ystart < 0)
    {
//ROBO: No FrameBuffer Messages
//      myOSystem->frameBuffer().showMessage("YStart at minimum");
      return;
    }
  }
  else
    return;

  myTIA->setYStart(ystart);
  myTIA->frameReset();
//ROBO TODO: No Framebuffer
//  myOSystem->frameBuffer().refresh();

  ostringstream val;
  val << ystart;
//ROBO: No FrameBuffer Messages
//  myOSystem->frameBuffer().showMessage("YStart " + val.str());
  myProperties.set(Display_YStart, val.str());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Console::changeHeight(int direction)
{
  uInt32 height = myTIA->height();

  if(direction == +1)       // increase Height
  {
    height++;
//ROBO: No OSystem
//    if(height > 256 || height > myOSystem->desktopHeight())
    if(height > 256)
    {
//ROBO: No FrameBuffer Messages
//      myOSystem->frameBuffer().showMessage("Height at maximum");
      return;
    }
  }
  else if(direction == -1)  // decrease Height
  {
    height--;
    if(height < 210)
    {
//ROBO: No FrameBuffer Messages
//      myOSystem->frameBuffer().showMessage("Height at minimum");
      return;
    }
  }
  else
    return;

  myTIA->setHeight(height);
  myTIA->frameReset();
//ROBO TODO: video change
//  initializeVideo();  // takes care of refreshing the screen

  ostringstream val;
  val << height;
//ROBO: No FrameBuffer Messages
//  myOSystem->frameBuffer().showMessage("Height " + val.str());
  myProperties.set(Display_Height, val.str());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Console::setControllers(const string& rommd5)
{
  delete myControllers[0];
  delete myControllers[1];

  // Setup the controllers based on properties
  const string& left  = myProperties.get(Controller_Left);
  const string& right = myProperties.get(Controller_Right);

  // Swap the ports if necessary
  int leftPort, rightPort;
  if(myProperties.get(Console_SwapPorts) == "NO")
  {
    leftPort = 0; rightPort = 1;
  }
  else
  {
    leftPort = 1; rightPort = 0;
  }

  // Also check if we should swap the paddles plugged into a jack
  bool swapPaddles = myProperties.get(Controller_SwapPaddles) == "YES";
  // Set default controller for mouse
  Controller::setMouseIsController(0);

  // Construct left controller
  if(left == "BOOSTERGRIP")
  {
    myControllers[leftPort] = new BoosterGrip(Controller::Left, *myEvent, *mySystem);
  }
  else if(left == "DRIVING")
  {
    myControllers[leftPort] = new Driving(Controller::Left, *myEvent, *mySystem);
  }
  else if((left == "KEYBOARD") || (left == "KEYPAD"))
  {
    myControllers[leftPort] = new Keyboard(Controller::Left, *myEvent, *mySystem);
  }
  else if(BSPF_startsWithIgnoreCase(left, "PADDLES"))
  {
    Controller::setMouseIsController(swapPaddles ? 1 : 0);
    bool swapAxis = false, swapDir = false;
    if(left == "PADDLES_IAXIS")
      swapAxis = true;
    else if(left == "PADDLES_IDIR")
      swapDir = true;
    else if(left == "PADDLES_IAXDR")
      swapAxis = swapDir = true;
    myControllers[leftPort] =
      new Paddles(Controller::Left, *myEvent, *mySystem,
                  swapPaddles, swapAxis, swapDir);
  }
  else if(left == "TRACKBALL22")
  {
    myControllers[leftPort] = new TrackBall(Controller::Left, *myEvent, *mySystem,
                                            Controller::TrackBall22);
  }
  else if(left == "TRACKBALL80")
  {
    myControllers[leftPort] = new TrackBall(Controller::Left, *myEvent, *mySystem,
                                            Controller::TrackBall80);
  }
  else if(left == "AMIGAMOUSE")
  {
    myControllers[leftPort] = new TrackBall(Controller::Left, *myEvent, *mySystem,
                                            Controller::AmigaMouse);
  }
  else if(left == "GENESIS")
  {
    myControllers[leftPort] = new Genesis(Controller::Left, *myEvent, *mySystem);
  }
  else
  {
    myControllers[leftPort] = new Joystick(Controller::Left, *myEvent, *mySystem);
  }
 
  // Construct right controller
  if(right == "BOOSTERGRIP")
  {
    myControllers[rightPort] = new BoosterGrip(Controller::Right, *myEvent, *mySystem);
  }
  else if(right == "DRIVING")
  {
    myControllers[rightPort] = new Driving(Controller::Right, *myEvent, *mySystem);
  }
  else if((right == "KEYBOARD") || (right == "KEYPAD"))
  {
    myControllers[rightPort] = new Keyboard(Controller::Right, *myEvent, *mySystem);
  }
  else if(BSPF_startsWithIgnoreCase(right, "PADDLES"))
  {
    bool swapAxis = false, swapDir = false;
    if(right == "PADDLES_IAXIS")
      swapAxis = true;
    else if(right == "PADDLES_IDIR")
      swapDir = true;
    else if(right == "PADDLES_IAXDR")
      swapAxis = swapDir = true;
    myControllers[rightPort] =
      new Paddles(Controller::Right, *myEvent, *mySystem,
                  swapPaddles, swapAxis, swapDir);
  }
  else if(right == "TRACKBALL22")
  {
    myControllers[rightPort] = new TrackBall(Controller::Left, *myEvent, *mySystem,
                                             Controller::TrackBall22);
  }
  else if(right == "TRACKBALL80")
  {
    myControllers[rightPort] = new TrackBall(Controller::Left, *myEvent, *mySystem,
                                             Controller::TrackBall80);
  }
  else if(right == "AMIGAMOUSE")
  {
    myControllers[rightPort] = new TrackBall(Controller::Left, *myEvent, *mySystem,
                                             Controller::AmigaMouse);
  }
  else if(right == "ATARIVOX")
  {
//ROBO TODO: No OSystem, use medanfen paths
/*    const string& eepromfile = myOSystem->eepromDir() + "atarivox_eeprom.dat";
    myControllers[rightPort] = new AtariVox(Controller::Right, *myEvent,
                   *mySystem, myOSystem->serialPort(),
                   myOSystem->settings().getString("avoxport"), eepromfile);*/
  }
  else if(right == "SAVEKEY")
  {
//ROBO TODO: No OSystem, use medanfen paths
/*    const string& eepromfile = myOSystem->eepromDir() + "savekey_eeprom.dat";
    myControllers[rightPort] = new SaveKey(Controller::Right, *myEvent, *mySystem,
                                           eepromfile);*/
  }
  else if(right == "KIDVID")
  {
    myControllers[rightPort] = new KidVid(Controller::Right, *myEvent, *mySystem, rommd5);
  }
  else if(right == "GENESIS")
  {
    myControllers[rightPort] = new Genesis(Controller::Right, *myEvent, *mySystem);
  }
  else
  {
    myControllers[rightPort] = new Joystick(Controller::Right, *myEvent, *mySystem);
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Console::loadUserPalette()
{
//ROBO TODO: Allow definintion thru mednafen setting
//  const string& palette = myOSystem->paletteFile();
  const string& palette = "ERRORS";
  ifstream in(palette.c_str(), ios::binary);
  if(!in)
    return;

  // Make sure the contains enough data for the NTSC, PAL and SECAM palettes
  // This means 128 colours each for NTSC and PAL, at 3 bytes per pixel
  // and 8 colours for SECAM at 3 bytes per pixel
  in.seekg(0, ios::end);
  streampos length = in.tellg();
  in.seekg(0, ios::beg);
  if(length < 128 * 3 * 2 + 8 * 3)
  {
    in.close();
    cerr << "ERROR: invalid palette file " << palette << endl;
    return;
  }

  // Now that we have valid data, create the user-defined palettes
  uInt8 pixbuf[3];  // Temporary buffer for one 24-bit pixel

  for(int i = 0; i < 128; i++)  // NTSC palette
  {
    in.read((char*)pixbuf, 3);
    uInt32 pixel = ((int)pixbuf[0] << 16) + ((int)pixbuf[1] << 8) + (int)pixbuf[2];
    ourUserNTSCPalette[(i<<1)] = pixel;
  }
  for(int i = 0; i < 128; i++)  // PAL palette
  {
    in.read((char*)pixbuf, 3);
    uInt32 pixel = ((int)pixbuf[0] << 16) + ((int)pixbuf[1] << 8) + (int)pixbuf[2];
    ourUserPALPalette[(i<<1)] = pixel;
  }

  uInt32 secam[16];  // All 8 24-bit pixels, plus 8 colorloss pixels
  for(int i = 0; i < 8; i++)    // SECAM palette
  {
    in.read((char*)pixbuf, 3);
    uInt32 pixel = ((int)pixbuf[0] << 16) + ((int)pixbuf[1] << 8) + (int)pixbuf[2];
    secam[(i<<1)]   = pixel;
    secam[(i<<1)+1] = 0;
  }
  uInt32* ptr = ourUserSECAMPalette;
  for(int i = 0; i < 16; ++i)
  {
    uInt32* s = secam;
    for(int j = 0; j < 16; ++j)
      *ptr++ = *s++;
  }

  in.close();
  myUserPaletteDefined = true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Console::setColorLossPalette()
{
  // Look at all the palettes, since we don't know which one is
  // currently active
  uInt32* palette[9] = {
    &ourNTSCPalette[0],    &ourPALPalette[0],    &ourSECAMPalette[0],
    &ourNTSCPaletteZ26[0], &ourPALPaletteZ26[0], &ourSECAMPaletteZ26[0],
    0, 0, 0
  };
  if(myUserPaletteDefined)
  {
    palette[6] = &ourUserNTSCPalette[0];
    palette[7] = &ourUserPALPalette[0];
    palette[8] = &ourUserSECAMPalette[0];
  }

  for(int i = 0; i < 9; ++i)
  {
    if(palette[i] == 0)
      continue;

    // Fill the odd numbered palette entries with gray values (calculated
    // using the standard RGB -> grayscale conversion formula)
    for(int j = 0; j < 128; ++j)
    {
      uInt32 pixel = palette[i][(j<<1)];
      uInt8 r = (pixel >> 16) & 0xff;
      uInt8 g = (pixel >> 8)  & 0xff;
      uInt8 b = (pixel >> 0)  & 0xff;
      uInt8 sum = (uInt8) (((float)r * 0.2989) +
                           ((float)g * 0.5870) +
                           ((float)b * 0.1140));
      palette[i][(j<<1)+1] = (sum << 16) + (sum << 8) + sum;
    }
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Console::setFramerate(float framerate)
{
  myFramerate = framerate;
//ROBO TODO: No OSYSTEM
//  myOSystem->setFramerate(framerate);
//  myOSystem->sound().setFrameRate(framerate);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Console::toggleTIABit(TIABit bit, const string& bitname, bool show) const
{
  bool result = myTIA->toggleBit(bit);
//ROBO: No FrameBuffer Messages
//  string message = bitname + (result ? " enabled" : " disabled");
//  myOSystem->frameBuffer().showMessage(message);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Console::toggleTIACollision(TIABit bit, const string& bitname, bool show) const
{
  bool result = myTIA->toggleCollision(bit);
//ROBO: No FrameBuffer Messages
//  string message = bitname + (result ? " collision enabled" : " collision disabled");
//  myOSystem->frameBuffer().showMessage(message);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Console::toggleHMOVE() const
{
//ROBO: No FrameBuffer Messages
//  if(myTIA->toggleHMOVEBlank())
//    myOSystem->frameBuffer().showMessage("HMOVE blanking enabled");
//  else
//    myOSystem->frameBuffer().showMessage("HMOVE blanking disabled");
  myTIA->toggleHMOVEBlank();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Console::enableBits(bool enable) const
{
  myTIA->enableBits(enable);
//ROBO: No FrameBuffer Messages
//  string message = string("TIA bits") + (enable ? " enabled" : " disabled");
//  myOSystem->frameBuffer().showMessage(message);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Console::enableCollisions(bool enable) const
{
  myTIA->enableCollisions(enable);
//ROBO: No FrameBuffer Messages
//  string message = string("TIA collisions") + (enable ? " enabled" : " disabled");
//  myOSystem->frameBuffer().showMessage(message);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Console::toggleFixedColors() const
{
//ROBO: No FrameBuffer Messages
//  if(myTIA->toggleFixedColors())
//    myOSystem->frameBuffer().showMessage("Fixed debug colors enabled");
//  else
//    myOSystem->frameBuffer().showMessage("Fixed debug colors disabled");
  myTIA->toggleFixedColors();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt32 Console::ourNTSCPalette[256] = {
  0x000000, 0, 0x4a4a4a, 0, 0x6f6f6f, 0, 0x8e8e8e, 0,
  0xaaaaaa, 0, 0xc0c0c0, 0, 0xd6d6d6, 0, 0xececec, 0,
  0x484800, 0, 0x69690f, 0, 0x86861d, 0, 0xa2a22a, 0,
  0xbbbb35, 0, 0xd2d240, 0, 0xe8e84a, 0, 0xfcfc54, 0,
  0x7c2c00, 0, 0x904811, 0, 0xa26221, 0, 0xb47a30, 0,
  0xc3903d, 0, 0xd2a44a, 0, 0xdfb755, 0, 0xecc860, 0,
  0x901c00, 0, 0xa33915, 0, 0xb55328, 0, 0xc66c3a, 0,
  0xd5824a, 0, 0xe39759, 0, 0xf0aa67, 0, 0xfcbc74, 0,
  0x940000, 0, 0xa71a1a, 0, 0xb83232, 0, 0xc84848, 0,
  0xd65c5c, 0, 0xe46f6f, 0, 0xf08080, 0, 0xfc9090, 0,
  0x840064, 0, 0x97197a, 0, 0xa8308f, 0, 0xb846a2, 0,
  0xc659b3, 0, 0xd46cc3, 0, 0xe07cd2, 0, 0xec8ce0, 0,
  0x500084, 0, 0x68199a, 0, 0x7d30ad, 0, 0x9246c0, 0,
  0xa459d0, 0, 0xb56ce0, 0, 0xc57cee, 0, 0xd48cfc, 0,
  0x140090, 0, 0x331aa3, 0, 0x4e32b5, 0, 0x6848c6, 0,
  0x7f5cd5, 0, 0x956fe3, 0, 0xa980f0, 0, 0xbc90fc, 0,
  0x000094, 0, 0x181aa7, 0, 0x2d32b8, 0, 0x4248c8, 0,
  0x545cd6, 0, 0x656fe4, 0, 0x7580f0, 0, 0x8490fc, 0,
  0x001c88, 0, 0x183b9d, 0, 0x2d57b0, 0, 0x4272c2, 0,
  0x548ad2, 0, 0x65a0e1, 0, 0x75b5ef, 0, 0x84c8fc, 0,
  0x003064, 0, 0x185080, 0, 0x2d6d98, 0, 0x4288b0, 0,
  0x54a0c5, 0, 0x65b7d9, 0, 0x75cceb, 0, 0x84e0fc, 0,
  0x004030, 0, 0x18624e, 0, 0x2d8169, 0, 0x429e82, 0,
  0x54b899, 0, 0x65d1ae, 0, 0x75e7c2, 0, 0x84fcd4, 0,
  0x004400, 0, 0x1a661a, 0, 0x328432, 0, 0x48a048, 0,
  0x5cba5c, 0, 0x6fd26f, 0, 0x80e880, 0, 0x90fc90, 0,
  0x143c00, 0, 0x355f18, 0, 0x527e2d, 0, 0x6e9c42, 0,
  0x87b754, 0, 0x9ed065, 0, 0xb4e775, 0, 0xc8fc84, 0,
  0x303800, 0, 0x505916, 0, 0x6d762b, 0, 0x88923e, 0,
  0xa0ab4f, 0, 0xb7c25f, 0, 0xccd86e, 0, 0xe0ec7c, 0,
  0x482c00, 0, 0x694d14, 0, 0x866a26, 0, 0xa28638, 0,
  0xbb9f47, 0, 0xd2b656, 0, 0xe8cc63, 0, 0xfce070, 0
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt32 Console::ourPALPalette[256] = {
  0x000000, 0, 0x2b2b2b, 0, 0x525252, 0, 0x767676, 0,
  0x979797, 0, 0xb6b6b6, 0, 0xd2d2d2, 0, 0xececec, 0,
  0x000000, 0, 0x2b2b2b, 0, 0x525252, 0, 0x767676, 0,
  0x979797, 0, 0xb6b6b6, 0, 0xd2d2d2, 0, 0xececec, 0,
  0x805800, 0, 0x96711a, 0, 0xab8732, 0, 0xbe9c48, 0,
  0xcfaf5c, 0, 0xdfc06f, 0, 0xeed180, 0, 0xfce090, 0,
  0x445c00, 0, 0x5e791a, 0, 0x769332, 0, 0x8cac48, 0,
  0xa0c25c, 0, 0xb3d76f, 0, 0xc4ea80, 0, 0xd4fc90, 0,
  0x703400, 0, 0x89511a, 0, 0xa06b32, 0, 0xb68448, 0,
  0xc99a5c, 0, 0xdcaf6f, 0, 0xecc280, 0, 0xfcd490, 0,
  0x006414, 0, 0x1a8035, 0, 0x329852, 0, 0x48b06e, 0,
  0x5cc587, 0, 0x6fd99e, 0, 0x80ebb4, 0, 0x90fcc8, 0,
  0x700014, 0, 0x891a35, 0, 0xa03252, 0, 0xb6486e, 0,
  0xc95c87, 0, 0xdc6f9e, 0, 0xec80b4, 0, 0xfc90c8, 0,
  0x005c5c, 0, 0x1a7676, 0, 0x328e8e, 0, 0x48a4a4, 0,
  0x5cb8b8, 0, 0x6fcbcb, 0, 0x80dcdc, 0, 0x90ecec, 0,
  0x70005c, 0, 0x841a74, 0, 0x963289, 0, 0xa8489e, 0,
  0xb75cb0, 0, 0xc66fc1, 0, 0xd380d1, 0, 0xe090e0, 0,
  0x003c70, 0, 0x195a89, 0, 0x2f75a0, 0, 0x448eb6, 0,
  0x57a5c9, 0, 0x68badc, 0, 0x79ceec, 0, 0x88e0fc, 0,
  0x580070, 0, 0x6e1a89, 0, 0x8332a0, 0, 0x9648b6, 0,
  0xa75cc9, 0, 0xb76fdc, 0, 0xc680ec, 0, 0xd490fc, 0,
  0x002070, 0, 0x193f89, 0, 0x2f5aa0, 0, 0x4474b6, 0,
  0x578bc9, 0, 0x68a1dc, 0, 0x79b5ec, 0, 0x88c8fc, 0,
  0x340080, 0, 0x4a1a96, 0, 0x5f32ab, 0, 0x7248be, 0,
  0x835ccf, 0, 0x936fdf, 0, 0xa280ee, 0, 0xb090fc, 0,
  0x000088, 0, 0x1a1a9d, 0, 0x3232b0, 0, 0x4848c2, 0,
  0x5c5cd2, 0, 0x6f6fe1, 0, 0x8080ef, 0, 0x9090fc, 0,
  0x000000, 0, 0x2b2b2b, 0, 0x525252, 0, 0x767676, 0,
  0x979797, 0, 0xb6b6b6, 0, 0xd2d2d2, 0, 0xececec, 0,
  0x000000, 0, 0x2b2b2b, 0, 0x525252, 0, 0x767676, 0,
  0x979797, 0, 0xb6b6b6, 0, 0xd2d2d2, 0, 0xececec, 0
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt32 Console::ourSECAMPalette[256] = {
  0x000000, 0, 0x2121ff, 0, 0xf03c79, 0, 0xff50ff, 0, 
  0x7fff00, 0, 0x7fffff, 0, 0xffff3f, 0, 0xffffff, 0, 
  0x000000, 0, 0x2121ff, 0, 0xf03c79, 0, 0xff50ff, 0, 
  0x7fff00, 0, 0x7fffff, 0, 0xffff3f, 0, 0xffffff, 0, 
  0x000000, 0, 0x2121ff, 0, 0xf03c79, 0, 0xff50ff, 0, 
  0x7fff00, 0, 0x7fffff, 0, 0xffff3f, 0, 0xffffff, 0, 
  0x000000, 0, 0x2121ff, 0, 0xf03c79, 0, 0xff50ff, 0, 
  0x7fff00, 0, 0x7fffff, 0, 0xffff3f, 0, 0xffffff, 0, 
  0x000000, 0, 0x2121ff, 0, 0xf03c79, 0, 0xff50ff, 0, 
  0x7fff00, 0, 0x7fffff, 0, 0xffff3f, 0, 0xffffff, 0, 
  0x000000, 0, 0x2121ff, 0, 0xf03c79, 0, 0xff50ff, 0, 
  0x7fff00, 0, 0x7fffff, 0, 0xffff3f, 0, 0xffffff, 0, 
  0x000000, 0, 0x2121ff, 0, 0xf03c79, 0, 0xff50ff, 0, 
  0x7fff00, 0, 0x7fffff, 0, 0xffff3f, 0, 0xffffff, 0, 
  0x000000, 0, 0x2121ff, 0, 0xf03c79, 0, 0xff50ff, 0, 
  0x7fff00, 0, 0x7fffff, 0, 0xffff3f, 0, 0xffffff, 0, 
  0x000000, 0, 0x2121ff, 0, 0xf03c79, 0, 0xff50ff, 0, 
  0x7fff00, 0, 0x7fffff, 0, 0xffff3f, 0, 0xffffff, 0, 
  0x000000, 0, 0x2121ff, 0, 0xf03c79, 0, 0xff50ff, 0, 
  0x7fff00, 0, 0x7fffff, 0, 0xffff3f, 0, 0xffffff, 0, 
  0x000000, 0, 0x2121ff, 0, 0xf03c79, 0, 0xff50ff, 0, 
  0x7fff00, 0, 0x7fffff, 0, 0xffff3f, 0, 0xffffff, 0, 
  0x000000, 0, 0x2121ff, 0, 0xf03c79, 0, 0xff50ff, 0, 
  0x7fff00, 0, 0x7fffff, 0, 0xffff3f, 0, 0xffffff, 0, 
  0x000000, 0, 0x2121ff, 0, 0xf03c79, 0, 0xff50ff, 0, 
  0x7fff00, 0, 0x7fffff, 0, 0xffff3f, 0, 0xffffff, 0, 
  0x000000, 0, 0x2121ff, 0, 0xf03c79, 0, 0xff50ff, 0, 
  0x7fff00, 0, 0x7fffff, 0, 0xffff3f, 0, 0xffffff, 0, 
  0x000000, 0, 0x2121ff, 0, 0xf03c79, 0, 0xff50ff, 0, 
  0x7fff00, 0, 0x7fffff, 0, 0xffff3f, 0, 0xffffff, 0, 
  0x000000, 0, 0x2121ff, 0, 0xf03c79, 0, 0xff50ff, 0, 
  0x7fff00, 0, 0x7fffff, 0, 0xffff3f, 0, 0xffffff, 0
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt32 Console::ourNTSCPaletteZ26[256] = {
  0x000000, 0, 0x505050, 0, 0x646464, 0, 0x787878, 0,
  0x8c8c8c, 0, 0xa0a0a0, 0, 0xb4b4b4, 0, 0xc8c8c8, 0,
  0x445400, 0, 0x586800, 0, 0x6c7c00, 0, 0x809000, 0,
  0x94a414, 0, 0xa8b828, 0, 0xbccc3c, 0, 0xd0e050, 0,
  0x673900, 0, 0x7b4d00, 0, 0x8f6100, 0, 0xa37513, 0,
  0xb78927, 0, 0xcb9d3b, 0, 0xdfb14f, 0, 0xf3c563, 0,
  0x7b2504, 0, 0x8f3918, 0, 0xa34d2c, 0, 0xb76140, 0,
  0xcb7554, 0, 0xdf8968, 0, 0xf39d7c, 0, 0xffb190, 0,
  0x7d122c, 0, 0x912640, 0, 0xa53a54, 0, 0xb94e68, 0,
  0xcd627c, 0, 0xe17690, 0, 0xf58aa4, 0, 0xff9eb8, 0,
  0x730871, 0, 0x871c85, 0, 0x9b3099, 0, 0xaf44ad, 0,
  0xc358c1, 0, 0xd76cd5, 0, 0xeb80e9, 0, 0xff94fd, 0,
  0x5d0b92, 0, 0x711fa6, 0, 0x8533ba, 0, 0x9947ce, 0,
  0xad5be2, 0, 0xc16ff6, 0, 0xd583ff, 0, 0xe997ff, 0,
  0x401599, 0, 0x5429ad, 0, 0x683dc1, 0, 0x7c51d5, 0,
  0x9065e9, 0, 0xa479fd, 0, 0xb88dff, 0, 0xcca1ff, 0,
  0x252593, 0, 0x3939a7, 0, 0x4d4dbb, 0, 0x6161cf, 0,
  0x7575e3, 0, 0x8989f7, 0, 0x9d9dff, 0, 0xb1b1ff, 0,
  0x0f3480, 0, 0x234894, 0, 0x375ca8, 0, 0x4b70bc, 0,
  0x5f84d0, 0, 0x7398e4, 0, 0x87acf8, 0, 0x9bc0ff, 0,
  0x04425a, 0, 0x18566e, 0, 0x2c6a82, 0, 0x407e96, 0,
  0x5492aa, 0, 0x68a6be, 0, 0x7cbad2, 0, 0x90cee6, 0,
  0x044f30, 0, 0x186344, 0, 0x2c7758, 0, 0x408b6c, 0,
  0x549f80, 0, 0x68b394, 0, 0x7cc7a8, 0, 0x90dbbc, 0,
  0x0f550a, 0, 0x23691e, 0, 0x377d32, 0, 0x4b9146, 0,
  0x5fa55a, 0, 0x73b96e, 0, 0x87cd82, 0, 0x9be196, 0,
  0x1f5100, 0, 0x336505, 0, 0x477919, 0, 0x5b8d2d, 0,
  0x6fa141, 0, 0x83b555, 0, 0x97c969, 0, 0xabdd7d, 0,
  0x344600, 0, 0x485a00, 0, 0x5c6e14, 0, 0x708228, 0,
  0x84963c, 0, 0x98aa50, 0, 0xacbe64, 0, 0xc0d278, 0,
  0x463e00, 0, 0x5a5205, 0, 0x6e6619, 0, 0x827a2d, 0,
  0x968e41, 0, 0xaaa255, 0, 0xbeb669, 0, 0xd2ca7d, 0
}; 
  
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt32 Console::ourPALPaletteZ26[256] = {
  0x000000, 0, 0x4c4c4c, 0, 0x606060, 0, 0x747474, 0,
  0x888888, 0, 0x9c9c9c, 0, 0xb0b0b0, 0, 0xc4c4c4, 0,
  0x000000, 0, 0x4c4c4c, 0, 0x606060, 0, 0x747474, 0,
  0x888888, 0, 0x9c9c9c, 0, 0xb0b0b0, 0, 0xc4c4c4, 0,
  0x533a00, 0, 0x674e00, 0, 0x7b6203, 0, 0x8f7617, 0,
  0xa38a2b, 0, 0xb79e3f, 0, 0xcbb253, 0, 0xdfc667, 0,
  0x1b5800, 0, 0x2f6c00, 0, 0x438001, 0, 0x579415, 0,
  0x6ba829, 0, 0x7fbc3d, 0, 0x93d051, 0, 0xa7e465, 0,
  0x6a2900, 0, 0x7e3d12, 0, 0x925126, 0, 0xa6653a, 0,
  0xba794e, 0, 0xce8d62, 0, 0xe2a176, 0, 0xf6b58a, 0,
  0x075b00, 0, 0x1b6f11, 0, 0x2f8325, 0, 0x439739, 0,
  0x57ab4d, 0, 0x6bbf61, 0, 0x7fd375, 0, 0x93e789, 0,
  0x741b2f, 0, 0x882f43, 0, 0x9c4357, 0, 0xb0576b, 0,
  0xc46b7f, 0, 0xd87f93, 0, 0xec93a7, 0, 0xffa7bb, 0,
  0x00572e, 0, 0x106b42, 0, 0x247f56, 0, 0x38936a, 0,
  0x4ca77e, 0, 0x60bb92, 0, 0x74cfa6, 0, 0x88e3ba, 0,
  0x6d165f, 0, 0x812a73, 0, 0x953e87, 0, 0xa9529b, 0,
  0xbd66af, 0, 0xd17ac3, 0, 0xe58ed7, 0, 0xf9a2eb, 0,
  0x014c5e, 0, 0x156072, 0, 0x297486, 0, 0x3d889a, 0,
  0x519cae, 0, 0x65b0c2, 0, 0x79c4d6, 0, 0x8dd8ea, 0,
  0x5f1588, 0, 0x73299c, 0, 0x873db0, 0, 0x9b51c4, 0,
  0xaf65d8, 0, 0xc379ec, 0, 0xd78dff, 0, 0xeba1ff, 0,
  0x123b87, 0, 0x264f9b, 0, 0x3a63af, 0, 0x4e77c3, 0,
  0x628bd7, 0, 0x769feb, 0, 0x8ab3ff, 0, 0x9ec7ff, 0,
  0x451e9d, 0, 0x5932b1, 0, 0x6d46c5, 0, 0x815ad9, 0,
  0x956eed, 0, 0xa982ff, 0, 0xbd96ff, 0, 0xd1aaff, 0,
  0x2a2b9e, 0, 0x3e3fb2, 0, 0x5253c6, 0, 0x6667da, 0,
  0x7a7bee, 0, 0x8e8fff, 0, 0xa2a3ff, 0, 0xb6b7ff, 0,
  0x000000, 0, 0x4c4c4c, 0, 0x606060, 0, 0x747474, 0,
  0x888888, 0, 0x9c9c9c, 0, 0xb0b0b0, 0, 0xc4c4c4, 0,
  0x000000, 0, 0x4c4c4c, 0, 0x606060, 0, 0x747474, 0,
  0x888888, 0, 0x9c9c9c, 0, 0xb0b0b0, 0, 0xc4c4c4, 0
}; 

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt32 Console::ourSECAMPaletteZ26[256] = {
  0x000000, 0, 0x2121ff, 0, 0xf03c79, 0, 0xff3cff, 0, 
  0x7fff00, 0, 0x7fffff, 0, 0xffff3f, 0, 0xffffff, 0, 
  0x000000, 0, 0x2121ff, 0, 0xf03c79, 0, 0xff3cff, 0, 
  0x7fff00, 0, 0x7fffff, 0, 0xffff3f, 0, 0xffffff, 0, 
  0x000000, 0, 0x2121ff, 0, 0xf03c79, 0, 0xff3cff, 0, 
  0x7fff00, 0, 0x7fffff, 0, 0xffff3f, 0, 0xffffff, 0, 
  0x000000, 0, 0x2121ff, 0, 0xf03c79, 0, 0xff3cff, 0, 
  0x7fff00, 0, 0x7fffff, 0, 0xffff3f, 0, 0xffffff, 0, 
  0x000000, 0, 0x2121ff, 0, 0xf03c79, 0, 0xff3cff, 0, 
  0x7fff00, 0, 0x7fffff, 0, 0xffff3f, 0, 0xffffff, 0, 
  0x000000, 0, 0x2121ff, 0, 0xf03c79, 0, 0xff3cff, 0, 
  0x7fff00, 0, 0x7fffff, 0, 0xffff3f, 0, 0xffffff, 0, 
  0x000000, 0, 0x2121ff, 0, 0xf03c79, 0, 0xff3cff, 0, 
  0x7fff00, 0, 0x7fffff, 0, 0xffff3f, 0, 0xffffff, 0, 
  0x000000, 0, 0x2121ff, 0, 0xf03c79, 0, 0xff3cff, 0, 
  0x7fff00, 0, 0x7fffff, 0, 0xffff3f, 0, 0xffffff, 0, 
  0x000000, 0, 0x2121ff, 0, 0xf03c79, 0, 0xff3cff, 0, 
  0x7fff00, 0, 0x7fffff, 0, 0xffff3f, 0, 0xffffff, 0, 
  0x000000, 0, 0x2121ff, 0, 0xf03c79, 0, 0xff3cff, 0, 
  0x7fff00, 0, 0x7fffff, 0, 0xffff3f, 0, 0xffffff, 0, 
  0x000000, 0, 0x2121ff, 0, 0xf03c79, 0, 0xff3cff, 0, 
  0x7fff00, 0, 0x7fffff, 0, 0xffff3f, 0, 0xffffff, 0, 
  0x000000, 0, 0x2121ff, 0, 0xf03c79, 0, 0xff3cff, 0, 
  0x7fff00, 0, 0x7fffff, 0, 0xffff3f, 0, 0xffffff, 0, 
  0x000000, 0, 0x2121ff, 0, 0xf03c79, 0, 0xff3cff, 0, 
  0x7fff00, 0, 0x7fffff, 0, 0xffff3f, 0, 0xffffff, 0, 
  0x000000, 0, 0x2121ff, 0, 0xf03c79, 0, 0xff3cff, 0, 
  0x7fff00, 0, 0x7fffff, 0, 0xffff3f, 0, 0xffffff, 0, 
  0x000000, 0, 0x2121ff, 0, 0xf03c79, 0, 0xff3cff, 0, 
  0x7fff00, 0, 0x7fffff, 0, 0xffff3f, 0, 0xffffff, 0, 
  0x000000, 0, 0x2121ff, 0, 0xf03c79, 0, 0xff3cff, 0, 
  0x7fff00, 0, 0x7fffff, 0, 0xffff3f, 0, 0xffffff, 0
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt32 Console::ourUserNTSCPalette[256]  = { 0 }; // filled from external file

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt32 Console::ourUserPALPalette[256]   = { 0 }; // filled from external file

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt32 Console::ourUserSECAMPalette[256] = { 0 }; // filled from external file

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Console::Console(const Console& console)
//ROBO: No OSystem
//  : myOSystem(console.myOSystem)
{
  assert(false);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Console& Console::operator = (const Console&)
{
  assert(false);

  return *this;
}
