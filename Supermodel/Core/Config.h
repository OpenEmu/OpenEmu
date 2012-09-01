/**
 ** Supermodel
 ** A Sega Model 3 Arcade Emulator.
 ** Copyright 2011 Bart Trzynadlowski, Nik Henson 
 **
 ** This file is part of Supermodel.
 **
 ** Supermodel is free software: you can redistribute it and/or modify it under
 ** the terms of the GNU General Public License as published by the Free 
 ** Software Foundation, either version 3 of the License, or (at your option)
 ** any later version.
 **
 ** Supermodel is distributed in the hope that it will be useful, but WITHOUT
 ** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 ** FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 ** more details.
 **
 ** You should have received a copy of the GNU General Public License along
 ** with Supermodel.  If not, see <http://www.gnu.org/licenses/>.
 **/

/*
 * Config.h
 * 
 * Header file for program-wide configuration settings.
 */

#ifndef INCLUDED_CONFIG_H
#define INCLUDED_CONFIG_H


#include "Supermodel.h"


/*
 * CConfig:
 *
 * Class defining all configuration settings. Inherits settings from classes
 * defined throughout the program. 
 *
 * Conventions
 * -----------
 * - Modules and classes should only use the members of their own configuration
 *   class, so long as this is practical. This is left to programmer
 *   discretion. The intent is that one class, CReal3D for example, should not
 *   need information from another unrelated class, CModel3, unless explicitly
 *   passed to it. On the other hand, the OSD layer may have a legitimate
 *   need to use the renderer's video settings rather than maintaining its own.
 * - Member variables that have a limited range of allowable values must be
 *   private and accessed through accessor members (SetVar(), GetVar()). The 
 *   accessors must ensure that a value outside the allowable range is never
 *   returned and must clamp or reset to a default when an invalid setting
 *   is passed. Warnings may be printed.
 * - Strings should be copied and retained locally.
 * - Constructors must be defined and must initialize to the program default.
 * - User-tunable settings should be stored here, not necessarily every 
 *   concievable parameter a class initializer might take.
 */
class CConfig: public COSDConfig, public CRender3DConfig, public CModel3Config, public CSoundBoardConfig, public CDSBConfig, public CDriveBoardConfig
{
};

/*
 * g_Config:
 *
 * Program-wide configuration settings object.
 */
extern CConfig g_Config;


#endif	// INCLUDED_CONFIG_H