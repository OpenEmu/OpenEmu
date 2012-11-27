/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef CREATE_HUGO_H
#define CREATE_HUGO_H

#define ARRAYSIZE(x) ((int)(sizeof(x) / sizeof(x[0])))

#define DATAALIGNMENT 4

#define HUGO_DAT_VER_MAJ 0  // 1 byte
#define HUGO_DAT_VER_MIN 42 // 1 byte

typedef unsigned char  uint8;
typedef unsigned char  byte;
typedef unsigned short uint16;
typedef signed   short int16;

// Structure to define an EXIT or other collision-activated hotspot
struct hotspot_t {
	int        screenIndex;                         // Screen in which hotspot appears
	int        x1, y1, x2, y2;                      // Bounding box of hotspot
	uint16     actIndex;                            // Index of the action list to carry out if a 'hit'
	int16      viewx, viewy, direction;             // Used in auto-route mode
};

struct target_t {                                   // Secondary target for action
	uint16 nounIndex;                               // Index of the noun
	uint16 verbIndex;                               // Index of the verb
};

#define MAX_TARGET     12                           // Max # secondary "MakeUseOf" targets

struct uses_t {                                     // Define uses of certain objects
	int16    objid;                                 // Primary object
	uint16   dataIndex;                             // Index of the string if no secondary object matches
	target_t targets[MAX_TARGET];                   // List of secondary targets
};

// Following is structure of verbs and nouns for 'background' objects
// These are objects that appear in the various screens, but nothing
//  interesting ever happens with them.  Rather than just be dumb and say
// "don't understand" we produce an interesting msg to keep user sane.
struct background_t {
	uint16 verbIndex;                               // Index of the verb
	uint16 nounIndex;                               // Index of the noun
	int    commentIndex;                            // Index of comment produced on match
	bool   matchFl;                                 // TRUE if noun must match when present
	byte   roomState;                               // "State" of room. Comments might differ.
	byte   bonusIndex;                              // Index of bonus score (0 = no bonus)
};

typedef background_t *objectList_t;

struct cmd {
	uint16 verbIndex;                               // Index of the verb
	uint16 reqIndex;                                // Index of the list of required objects
	uint16 textDataNoCarryIndex;                    // Index of the string if any of above not carried
	byte   reqstate;                                // required state for verb to be done
	byte   newstate;                                // new states if verb done
	uint16 textDataWrongIndex;                      // Index of the string if wrong state
	uint16 textDataDoneIndex;                       // Index of the string if verb done
	uint16 actIndex;                                // Index of the action list if verb done
};

struct seq_t {                                      // Linked list of images
	byte   *imagePtr;                               // ptr to image
	uint16  bytesPerLine8;                          // bytes per line (8 bits)
	uint16  lines;                                  // lines
	uint16  x1, x2, y1, y2;                         // Offsets from x,y: data bounding box
	seq_t  *nextSeqPtr;                             // ptr to next record
};

struct seqList_t {
	uint16 imageNbr;                                // Number of images in sequence
	seq_t *seqPtr;                                  // Ptr to sequence structure
};

#define MAX_SEQUENCES  4                            // Number of sequences of images in object
struct object_t {
	uint16     nounIndex;                           // String identifying object
	uint16     dataIndex;                           // String describing the object
	uint16     *stateDataIndex;                     // Added by Strangerke to handle the LOOK_S state-dependant descriptions
	path_t     pathType;                            // Describe path object follows
	int        vxPath, vyPath;                      // Velocity (e.g. for CHASE)
	uint16     actIndex;                            // Action list to do on collision with hero
	byte       seqNumb;                             // Number of sequences in list
	seq_t     *currImagePtr;                        // Sequence image currently in use
	seqList_t  seqList[MAX_SEQUENCES];              // Array of sequence structure ptrs and lengths
	cycle_t    cycling;                             // Whether cycling, forward or backward
	byte       cycleNumb;                           // No. of times to cycle
	byte       frameInterval;                       // Interval (in ticks) between frames
	byte       frameTimer;                          // Decrementing timer for above
	char       radius;                              // Defines sphere of influence by hero
	byte       screenIndex;                         // Screen in which object resides
	int        x, y;                                // Current coordinates of object
	int        oldx, oldy;                          // Previous coordinates of object
	char       vx, vy;                              // Velocity
	byte       objValue;                            // Value of object
	int        genericCmd;                          // Bit mask of 'generic' commands for object
	uint16     cmdIndex;                            // ptr to list of cmd structures for verbs
	bool       carriedFl;                           // TRUE if object being carried
	byte       state;                               // state referenced in cmd list
	bool       verbOnlyFl;                          // TRUE if verb-only cmds allowed e.g. sit,look
	byte       priority;                            // Whether object fore, background or floating
	int16      viewx, viewy;                        // Position to view object from (or 0 or -1)
	int16      direction;                           // Direction to view object from
	byte       curSeqNumb;                          // Save which seq number currently in use
	byte       curImageNumb;                        // Save which image of sequence currently in use
	char       oldvx;                               // Previous vx (used in wandering)
	char       oldvy;                               // Previous vy
};

struct act0 {                                       // Type 0 - Schedule
	byte     actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	uint16   actIndex;                              // Index of an action list
};

struct act1 {                                       // Type 1 - Start an object
	byte     actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objNumb;                               // The object number
	int      cycleNumb;                             // Number of times to cycle
	cycle_t  cycle;                                 // Direction to start cycling
};

struct act2 {                                       // Type 2 - Initialize an object coords
	byte     actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objNumb;                               // The object number
	int      x, y;                                  // Coordinates
};

struct act3 {                                       // Type 3 - Prompt user for text
	byte     actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	uint16   promptIndex;                           // index of prompt string
	int     *responsePtr;                           // Array of indexes to valid response
	// string(s) (terminate list with -1)
	uint16   actPassIndex;                          // Index of the action list if success
	uint16   actFailIndex;                          // Index of the action list if failure
	bool     encoded;                               // (HUGO 1 DOS ONLY) Whether response is encoded or not
};

struct act4 {                                       // Type 4 - Set new background color
	byte     actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	long     newBkgColor;                           // New color
};

struct act5 {                                       // Type 5 - Initialize an object velocity
	byte     actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objNumb;                               // The object number
	int      vx, vy;                                // velocity
};

struct act6 {                                       // Type 6 - Initialize an object carrying
	byte     actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objNumb;                               // The object number
	bool     carriedFl;                             // carrying
};

struct act7 {                                       // Type 7 - Initialize an object to hero's coords
	byte     actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objNumb;                               // The object number
};

struct act8 {                                       // Type 8 - switch to new screen
	byte     actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      screenIndex;                           // The new screen number
};

struct act9 {                                       // Type 9 - Initialize an object state
	byte     actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objNumb;                               // The object number
	byte     newState;                              // New state
};

struct act10 {                                      // Type 10 - Initialize an object path type
	byte     actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objNumb;                               // The object number
	int      newPathType;                           // New path type
	char     vxPath, vyPath;                        // Max delta velocities e.g. for CHASE
};

struct act11 {                                      // Type 11 - Conditional on object's state
	byte     actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objNumb;                               // The object number
	byte     stateReq;                              // Required state
	uint16   actPassIndex;                          // Index of the action list if success
	uint16   actFailIndex;                          // Index of the action list if failure
};

struct act12 {                                      // Type 12 - Simple text box
	byte     actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      stringIndex;                           // Index (enum) of string in strings.dat
};

struct act13 {                                      // Type 13 - Swap first object image with second
	byte     actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      obj1;                                  // Index of first object
	int      obj2;                                  // 2nd
};

struct act14 {                                      // Type 14 - Conditional on current screen
	byte     actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objNumb;                               // The required object
	int      screenReq;                             // The required screen number
	uint16   actPassIndex;                          // Index of the action list if success
	uint16   actFailIndex;                          // Index of the action list if failure
};

struct act15 {                                      // Type 15 - Home in on an object
	byte     actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      obj1;                                  // The object number homing in
	int      obj2;                                  // The object number to home in on
	char     vx, vy;                                // Max delta velocities
};
// Note: Don't set a sequence at time 0 of a new screen, it causes
// problems clearing the boundary bits of the object!  t>0 is safe
struct act16 {                                      // Type 16 - Set curr_seq_p to seq
	byte     actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objNumb;                               // The object number
	int      seqIndex;                              // The index of seq array to set to
};

struct act17 {                                      // Type 17 - SET obj individual state bits
	byte     actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objNumb;                               // The object number
	int      stateMask;                             // The mask to OR with current obj state
};

struct act18 {                                      // Type 18 - CLEAR obj individual state bits
	byte     actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objNumb;                               // The object number
	int      stateMask;                             // The mask to ~AND with current obj state
};

struct act19 {                                      // Type 19 - TEST obj individual state bits
	byte     actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objNumb;                               // The object number
	int      stateMask;                             // The mask to AND with current obj state
	uint16   actPassIndex;                          // Index of the action list (all bits set)
	uint16   actFailIndex;                          // Index of the action list (not all set)
};

struct act20 {                                      // Type 20 - Remove all events with this type of action
	byte     actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	byte     actTypeDel;                            // The action type to remove
};

struct act21 {                                      // Type 21 - Gameover.  Disable hero & commands
	byte     actType;                               // The type of action
	int      timer;                                 // Time to set off the action
};

struct act22 {                                      // Type 22 - Initialize an object to hero's coords
	byte     actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objNumb;                               // The object number
};

struct act23 {                                      // Type 23 - Exit game back to DOS
	byte     actType;                               // The type of action
	int      timer;                                 // Time to set off the action
};

struct act24 {                                      // Type 24 - Get bonus score
	byte     actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      pointIndex;                            // Index into points array
};

struct act25 {                                      // Type 25 - Conditional on bounding box
	byte     actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objNumb;                               // The required object number
	int      x1, y1, x2, y2;                        // The bounding box
	uint16   actPassIndex;                          // Index of the action list if success
	uint16   actFailIndex;                          // Index of the action list if failure
};

struct act26 {                                      // Type 26 - Play a sound
	byte     actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int16    soundIndex;                            // Sound index in data file
};

struct act27 {                                      // Type 27 - Add object's value to score
	byte     actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objNumb;                               // object number
};

struct act28 {                                      // Type 28 - Subtract object's value from score
	byte     actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objNumb;                               // object number
};

struct act29 {                                      // Type 29 - Conditional on object carried
	byte     actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objNumb;                               // The required object number
	uint16   actPassIndex;                          // Index of the action list if success
	uint16   actFailIndex;                          // Index of the action list if failure
};

struct act30 {                                      // Type 30 - Start special maze processing
	byte     actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	byte     mazeSize;                              // Size of (square) maze
	int      x1, y1, x2, y2;                        // Bounding box of maze
	int      x3, x4;                                // Extra x points for perspective correction
	byte     firstScreenIndex;                      // First (top left) screen of maze
};

struct act31 {                                      // Type 31 - Exit special maze processing
	byte     actType;                               // The type of action
	int      timer;                                 // Time to set off the action
};

struct act32 {                                      // Type 32 - Init fbg field of object
	byte     actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objNumb;                               // The object number
	byte     priority;                              // Value of foreground/background field
};

struct act33 {                                      // Type 33 - Init screen field of object
	byte     actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objNumb;                               // The object number
	int      screenIndex;                           // Screen number
};

struct act34 {                                      // Type 34 - Global Schedule
	byte     actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	uint16   actIndex;                              // Index of an action list
};

struct act35 {                                      // Type 35 - Remappe palette
	byte     actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int16    oldColorIndex;                         // Old color index, 0..15
	int16    newColorIndex;                         // New color index, 0..15
};

struct act36 {                                      // Type 36 - Conditional on noun mentioned
	byte     actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	uint16   nounIndex;                             // The required noun (list)
	uint16   actPassIndex;                          // Index of the action list if success
	uint16   actFailIndex;                          // Index of the action list if failure
};

struct act37 {                                      // Type 37 - Set new screen state
	byte     actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      screenIndex;                           // The screen number
	byte     newState;                              // The new state
};

struct act38 {                                      // Type 38 - Position lips
	byte     actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      lipsObjNumb;                           // The LIPS object
	int      objNumb;                               // The object to speak
	byte     dxLips;                                // Relative offset of x
	byte     dyLips;                                // Relative offset of y
};

struct act39 {                                      // Type 39 - Init story mode
	byte     actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	bool     storyModeFl;                           // New state of story_mode flag
};

struct act40 {                                      // Type 40 - Unsolicited text box
	byte     actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      stringIndex;                           // Index (enum) of string in strings.dat
};

struct act41 {                                      // Type 41 - Conditional on bonus scored
	byte     actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      BonusIndex;                            // Index into bonus list
	uint16   actPassIndex;                          // Index of the action list if scored for the first time
	uint16   actFailIndex;                          // Index of the action list if already scored
};

struct act42 {                                      // Type 42 - Text box with "take" string
	byte     actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objNumb;                               // The object taken
};

struct act43 {                                      // Type 43 - Prompt user for Yes or No
	byte     actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      prompt;                                // Index of prompt string
	uint16   actYesIndex;                           // Index of the action list if YES
	uint16   actNoIndex;                            // Index of the action list if NO
};

struct act44 {                                      // Type 44 - Stop any route in progress
	byte     actType;                               // The type of action
	int      timer;                                 // Time to set off the action
};

struct act45 {                                      // Type 45 - Conditional on route in progress
	byte     actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      routeIndex;                            // Must be >= current status.rindex
	uint16   actPassIndex;                          // Index of the action list if en-route
	uint16   actFailIndex;                          // Index of the action list if not
};

struct act46 {                                      // Type 46 - Init status.jumpexit
	byte     actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	bool     jumpExitFl;                            // New state of jumpexit flag
};

struct act47 {                                      // Type 47 - Init viewx,viewy,dir
	byte     actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objNumb;                               // The object
	int16    viewx;                                 // object.viewx
	int16    viewy;                                 // object.viewy
	int16    direction;                             // object.dir
};

struct act48 {                                      // Type 48 - Set curr_seq_p to frame n
	byte     actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objNumb;                               // The object number
	int      seqIndex;                              // The index of seq array to set to
	int      frameIndex;                            // The index of frame to set to
};

struct act49 {                                      // Added by Strangerke - Type 79 - Play a sound (DOS way)
	byte     actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	uint16   songIndex;                             // Song index in string array
};

union act {
	act0     a0;
	act1     a1;
	act2     a2;
	act3     a3;
	act4     a4;
	act5     a5;
	act6     a6;
	act7     a7;
	act8     a8;
	act9     a9;
	act10    a10;
	act11    a11;
	act12    a12;
	act13    a13;
	act14    a14;
	act15    a15;
	act16    a16;
	act17    a17;
	act18    a18;
	act19    a19;
	act20    a20;
	act21    a21;
	act22    a22;
	act23    a23;
	act24    a24;
	act25    a25;
	act26    a26;
	act27    a27;
	act28    a28;
	act29    a29;
	act30    a30;
	act31    a31;
	act32    a32;
	act33    a33;
	act34    a34;
	act35    a35;
	act36    a36;
	act37    a37;
	act38    a38;
	act39    a39;
	act40    a40;
	act41    a41;
	act42    a42;
	act43    a43;
	act44    a44;
	act45    a45;
	act46    a46;
	act47    a47;
	act48    a48;
	act49    a49;
};

typedef void       *actListPtr;                     // Ptr to a list of actions
typedef actListPtr *actList;                        // A list of actions

void writeTextArray(FILE *outFile, const char *textData[], int nbrText);
void writeUint16Array(FILE *outFile, const uint16 *uint16Array[], int nbrElem);
void writeHotspot(FILE *outFile, const hotspot_t hotspots[], int nbrElem);
void writeUseArray(FILE *outFile, const uses_t uses[], int nbrElem);
void writeBackgroundArray(FILE *outFile, const background_t background[], int nbrElem);
void writeCmdArray(FILE *outFile, const cmd *cmdList[], int nbrElem);
void writeScreenActs(FILE *outFile, const uint16 *screenActs[], int nbrElem);
void writeObjectArray(FILE *outFile, const object_t objects[], int nbrElem);
void writeActListArray(FILE *outFile, const actList actListArr[], int nbrElem);

#endif // CREATE_HUGO_H
