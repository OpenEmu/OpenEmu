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
 *
 */
/*
 * This code is based on original Tony Tough source code
 *
 * Copyright (c) 1997-2003 Nayma Software
 */

/****************************************************************************\
*       General Introduction
\****************************************************************************/

/*
 * MPAL (MultiPurpose Adventure Language) is a high level language
 * for the definition of adventure. Through the use of MPAL you can describe
 * storyboard the adventure, and then use it with any user interface.
 * In fact, unlike many other similar products, MPAL is not programmed through
 * the whole adventure, but are defined only the locations, objects, as they may
 * interact with each other, etc.. thus making MPAL useful for any type of adventure.
 */

/****************************************************************************\
*       Structure
\****************************************************************************/

/*
 * MPAL consists of two main files: MPAL.DLL and MPAL.H
 * The first is the DLL that contains the code to interface with MPAL
 * adventures, the second is the header that defines the prototypes
 * functions. MPAL is compiled for Win32, and it can therefore be used with
 * any compiler that supports Win32 DLL (Watcom C++, Visual C++,
 * Delphi, etc.), and therefore compatible with both Windows 95 and Windows NT.
 *
 * To use the DLL, and 'obviously need to create a library for symbols to export.
 *
 */

/****************************************************************************\
*       Custom Functions
\****************************************************************************/

/*
 * A custom function and a function specified by the program that uses the
 * library, to perform the particular code. The custom functions are
 * retrieved from the library as specified in the source MPAL, and in particular
 * in defining the behavior of an item with some action.
 *
 * To use the custom functions, you need to prepare an array of
 * pointers to functions (such as using the type casting LPCUSTOMFUNCTION,
 * (defined below), and pass it as second parameter to mpalInit (). Note you
 * must specify the size of the array, as elements of pointers and which do not
 * contain the same: the library will call it only those functions specified in
 * the source MPAL. It can be useful, for debugging reasons, do not bet
 * the shares of arrays used to debugging function, to avoid unpleasant crash,
 * if it has been made an error in source and / or some oversight in the code.
 *
 */

#ifndef TONY_MPAL_H
#define TONY_MPAL_H

#include "common/scummsys.h"
#include "common/coroutines.h"
#include "common/rect.h"
#include "common/str.h"
#include "tony/mpal/memory.h"

namespace Tony {

namespace MPAL {

/****************************************************************************\
*       Macro definitions and structures
\****************************************************************************/

// OK value for the error codes
#define OK 0

#define MAXFRAMES 400 // frame animation of an object
#define MAXPATTERN 40 // pattern of animation of an object
#define MAXPOLLINGLOCATIONS 64

#define GETARG(type)   va_arg(v, type)

/**
 * Macro for use with queries that may refer to X and Y co-ordinates
 */
enum QueryCoordinates {
	MPQ_X,
	MPQ_Y
};

/**
 * Query can be used with mpalQuery methods. In practice corresponds all claims
 * that can do at the library
 */
enum QueryTypes {
	// General Query
	MPQ_VERSION = 10,

	MPQ_GLOBAL_VAR = 50,
	MPQ_RESOURCE,
	MPQ_MESSAGE,

	// Query on leases
	MPQ_LOCATION_IMAGE = 100,
	MPQ_LOCATION_SIZE,

	// Queries about items
	MPQ_ITEM_LIST = 200,
	MPQ_ITEM_DATA,
	MPQ_ITEM_PATTERN,
	MPQ_ITEM_NAME,
	MPQ_ITEM_IS_ACTIVE,

	// Query dialog
	MPQ_DIALOG_PERIOD = 300,
	MPQ_DIALOG_WAITFORCHOICE,
	MPQ_DIALOG_SELECTLIST,
	MPQ_DIALOG_SELECTION,

	// Query execution
	MPQ_DO_ACTION = 400,
	MPQ_DO_DIALOG
};

/**
 * Framework to manage the animation of an item
 */
typedef struct {
	char *_frames[MAXFRAMES];
	Common::Rect _frameslocations[MAXFRAMES];
	Common::Rect _bbox[MAXFRAMES];
	short _pattern[MAXPATTERN][MAXFRAMES];
	short _speed;
	char _numframe;
	char _numpattern;
	char _curframe;
	char _curpattern;
	short _destX, _destY;
	signed char _destZ;
	short _objectID;
} Item;
typedef Item *LpItem;

/**
 * Define a custom function, to use the language MPAL to perform various controls as a result of an action
 */
typedef void (*LPCUSTOMFUNCTION)(CORO_PARAM, uint32, uint32, uint32, uint32);
typedef LPCUSTOMFUNCTION *LPLPCUSTOMFUNCTION;

/**
 *
 * Define an IRQ of an item that is called when the  pattern changes or the status of an item
 */
typedef void (*LPITEMIRQFUNCTION)(uint32, int, int);
typedef LPITEMIRQFUNCTION* LPLPITEMIRQFUNCTION;

/**
 * @defgroup Macrofunctions query
 *
 * The following are defines used for simplifying calling the mpalQuery variants
 */
//@{

/**
 * Gets the current version of MPAL
 *
 * @returns	Version number (0x1232 = 1.2.3b)
 */
#define mpalQueryVersion()                              \
        (uint16)mpalQueryDWORD(MPQ_VERSION)

/**
 * Gets the numerical value of a global variable
 *
 * @param lpszVarName		Variable name (ASCIIZ)
 * @returns		Global variable value
 * @remarks		This query was implemented for debugging. The program,
 *				if well designed, should not need to access variables from
 *				within the library.
 */
#define mpalQueryGlobalVar(lpszVarName)                 \
        mpalQueryDWORD(MPQ_GLOBAL_VAR, (const char *)(lpszVarName))

/**
 * Provides access to a resource inside the .MPC file
 *
 * @param dwResId			Resource Id
 * @returns		Handle to a memory area containing the resource,  ready for use.
 */
#define mpalQueryResource(dwResId)                      \
        mpalQueryHANDLE(MPQ_RESOURCE, (uint32)(dwResId))

/**
 * Returns a message.
 *
 * @param nMsg				Message number
 * @returns		ASCIIZ message
 * @remarks		The returned pointer must be freed via the memory manager
*				after use. The message will be in ASCIIZ format.
*/
#define mpalQueryMessage(nMsg)                          \
        (char *)mpalQueryHANDLE(MPQ_MESSAGE, (uint32)(nMsg))

/**
 * Provides a location image
 * @return		Returns a picture handle
 */
#define mpalQueryLocationImage(nLoc)                    \
        mpalQueryHANDLE(MPQ_LOCATION_IMAGE, (uint32)(nLoc))

/**
 * Request the x or y size of a location in pixels
 *
 * @param nLoc				Location number
 * @param dwCoord           MPQ_X or MPQ_Y coordinate to retrieve
 * @returns		Size
 */
#define mpalQueryLocationSize(nLoc, dwCoord)             \
        mpalQueryDWORD(MPQ_LOCATION_SIZE, (uint32)(nLoc), (uint32)(dwCoord))

/**
 * Provides the list of objects in a location.
 *
 * @param nLoc              Location number
 * @returns					List of objects (accessible by Item [0], Item [1], etc.)
 */
// TODO: Determine if this is endian safe
#define mpalQueryItemList(nLoc)                         \
        (uint32 *)mpalQueryHANDLE(MPQ_ITEM_LIST, (uint32)(nLoc))

/**
 * Provides information on an item
 *
 * @param nItem             Item number
 * @returns		Structure filled with requested information
 */
#define mpalQueryItemData(nItem)                          \
        (LpItem)mpalQueryHANDLE(MPQ_ITEM_DATA, (uint32)(nItem))

/**
 * Provides the current pattern of an item
 *
 * @param nItem             Item number
 * @returns		Number of animation patterns to be executed.
 * @remarks		By default, the pattern of 0 indicates that we should do nothing.
 */
#define mpalQueryItemPattern(nItem)                  \
        mpalQueryDWORD(MPQ_ITEM_PATTERN, (uint32)(nItem))

/**
 * Returns true if an item is active
 *
 * @param nItem             Item number
 * @returns		TRUE if the item is active, FALSE otherwise
 */
#define mpalQueryItemIsActive(nItem)                  \
        (bool)mpalQueryDWORD(MPQ_ITEM_IS_ACTIVE, (uint32)(nItem))

/**
 * Returns the name of an item
 *
 * @param nItem             Item number
 * @param lpszName          Pointer to a buffer of at least 33 bytes
 *                          that will be filled with the name
 * @remarks		If the item is not active (ie. if its status or number
 *				is less than or equal to 0), the string will be empty.
 */
#define mpalQueryItemName(nItem, lpszName)             \
        mpalQueryHANDLE(MPQ_ITEM_NAME, (uint32)(nItem), (char *)(lpszName))

/**
 * Returns a sentence of dialog.
 *
 * @param nDialog           Dialog number
 * @param nPeriod           Number of words
 * @returns		A pointer to the string of words, or NULL on failure.
 * @remarks		The string must be freed after use using the memory manager.
 * Unlike normal messages, the sentences of dialogue are formed by a single
 * string terminated with 0.
 */
#define mpalQueryDialogPeriod(nPeriod)                  \
        (char *)mpalQueryHANDLE(MPQ_DIALOG_PERIOD, (uint32)(nPeriod))

/**
 * Wait until the moment in which the need is signaled to make a choice by the user.
 * @returns		Number of choice to be made, or -1 if the dialogue is finished.
 */
#define mpalQueryDialogWaitForChoice(dwRet)                  \
        CORO_INVOKE_2(mpalQueryCORO, MPQ_DIALOG_WAITFORCHOICE, dwRet)

/**
 * Requires a list of various options for some choice within the current dialog.
 *
 * @param nChoice			Choice number
 * @returns		A pointer to an array containing the data matched to each option.
 * @remarks		The figure 'a uint32 specified in the source to which MPAL
 * You can assign meaning that the more' suits.
 * The pointer msut be freed after use using the memory memory.
 */
#define mpalQueryDialogSelectList(nChoice)              \
        (uint32 *)mpalQueryHANDLE(MPQ_DIALOG_SELECTLIST, (uint32)(nChoice))

/**
 * Warns the library that the user has selected, in a certain choice of the current dialog,
 * corresponding option at a certain given.
 *
 * @param nChoice			Choice number of the choice that was in progress
 * @param dwData			Option that was selected by the user.
 * @returns		TRUE if all OK, FALSE on failure.
 * @remarks		After execution of this query, MPAL continue
 * Groups according to the execution of the dialogue. And necessary so the game
 * remains on hold again for another chosen by mpalQueryDialogWaitForChoice ().
 */
#define mpalQueryDialogSelection(nChoice, dwData)        \
        (bool)mpalQueryDWORD(MPQ_DIALOG_SELECTION, (uint32)(nChoice), (uint32)(dwData))

#define mpalQueryDialogSelectionDWORD(nChoice, dwData)        \
        mpalQueryDWORD(MPQ_DIALOG_SELECTION, (uint32)(nChoice), (uint32)(dwData))

/**
 * Warns the library an action was performed on a Object.
 * The library will call custom functions, if necessary.
 *
 * @param nAction			Action number
 * @param nItem				Item number
 * @param dwParam			Action parameter
 * @returns		Handle to the thread that is performing the action, or CORO_INVALID_PID_VALUE
 * if the action is not defined for the item, or the item is inactive.
 * @remarks		The parameter is used primarily to implement actions
 * as "U.S." involving two objects together. The action will be executed only
 * if the item is active, ie if its status is a positive number greater than 0.
 */
#define mpalQueryDoAction(nAction, nItem, dwParam)      \
        mpalQueryDWORD(MPQ_DO_ACTION, (uint32)(nAction), (uint32)(nItem), (uint32)(dwParam))

/**
 * Warns the library a dialogue was required.
 *
 * @param nDialog			Dialog number
 * @param nGroup			Group number to use
 * @returns		Handle to the thread that is running the box, or
 * CORO_INVALID_PID_VALUE if the dialogue does not exist.
 */
#define mpalQueryDoDialog(nDialog, nGroup)               \
        mpalQueryDWORD(MPQ_DO_DIALOG, (uint32)(nDialog), (uint32)(nGroup))

/**
 * @defgroup Functions exported to the main game
 */
//@{

/**
 * Initializes the MPAL library, and opens an .MPC file, which will be 'used for all queries
 * @param lpszMpcFileName	Name of the .MPC file, including extension
 * @param lpszMprFileName	Name of the .MPR file, including extension
 * @param lplpcfArray		Array of pointers to custom functions
 * @returns		TRUE if all OK, FALSE on failure
 */
bool mpalInit(const char *lpszFileName, const char *lpszMprFileName,
              LPLPCUSTOMFUNCTION lplpcfArray, Common::String *lpcfStrings);

/**
 * Frees resources allocated by the MPAL subsystem
 */
void mpalFree();

/**
 * This is a general function to communicate with the library, to request information
 * about what is in the .MPC file
 *
 * @param wQueryType		Type of query. The list is in the QueryTypes enum.
 * @returns		4 bytes depending on the type of query
 * @remarks		This is the specialised version of the original single mpalQuery
 * method that returns numeric results.
 */
uint32 mpalQueryDWORD(uint16 wQueryType, ...);

/**
 * This is a general function to communicate with the library, to request information
 * about what is in the .MPC file
 *
 * @param wQueryType		Type of query. The list is in the QueryTypes enum.
 * @returns		4 bytes depending on the type of query
 * @remarks		This is the specialised version of the original single mpalQuery
 * method that returns a pointer or handle.
 */
MpalHandle mpalQueryHANDLE(uint16 wQueryType, ...);

/**
 * This is a general function to communicate with the library, to request information
 * about what is in the .MPC file
 *
 * @param wQueryType		Type of query. The list is in the QueryTypes enum.
 * @returns		4 bytes depending on the type of query
 * @remarks		This is the specialised version of the original single mpalQuery
 * method that needs to run within a co-routine context.
 */
void mpalQueryCORO(CORO_PARAM, uint16 wQueryType, uint32 *dwRet, ...);

/**
 * Execute a script. The script runs on multitasking by a thread.
 *
 * @param nScript			Script number to run
 * @returns		TRUE if the script 'was launched, FALSE on failure
 */
bool mpalExecuteScript(int nScript);

/**
 * Returns the current MPAL error code
 *
 * @returns		Error code
 */
uint32 mpalGetError();

/**
 * Install a custom routine That will be called by MPAL every time the pattern
 * of an item has been changed.
 *
 * @param lpiifCustom		Custom function to install
 */
void mpalInstallItemIrq(LPITEMIRQFUNCTION lpiifCustom);

/**
 * Process the idle actions of the items on one location.
 *
 * @param nLoc				Number of the location whose items must be processed
 * for idle actions.
 * @returns		TRUE if all OK, and FALSE if it exceeded the maximum limit.
 * @remarks		The maximum number of locations that can be polled
 * simultaneously is defined defined by MAXPOLLINGFUNCIONS
 */
bool mpalStartIdlePoll(int nLoc);

/**
 * Stop processing the idle actions of the items on one location.
 *
 * @param nLo				Number of the location
 * @returns		TRUE if all OK, FALSE if the specified location was not
 * in the process of polling
 */
void mpalEndIdlePoll(CORO_PARAM, int nLoc, bool *result);

/**
 * Load a save state from a buffer.
 *
 * @param buf				Buffer where to store the state
 * @returns		Length of the state buffer in bytes
 */
int mpalLoadState(byte *buf);

/**
 * Store the save state into a buffer. The buffer must be
 * length at least the size specified with mpalGetSaveStateSize
 *
 * @param buf				Buffer where to store the state
 */
void mpalSaveState(byte *buf);

/**
 * Retrieve the length of a save state
 *
 * @returns		Length in bytes
 */
int mpalGetSaveStateSize();

/**
 * Locks the variables for access
 */
void lockVar();

/**
 * Unlocks variables after use
 */
void unlockVar();

} // end of namespace MPAL

} // end of namespace Tony

#endif

