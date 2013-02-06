/*
* Glide64 - Glide video plugin for Nintendo 64 emulators.
* Copyright (c) 2010  Jon Ring
* Copyright (c) 2002  Dave2001
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public
* Licence along with this program; if not, write to the Free
* Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
* Boston, MA  02110-1301, USA
*/

#ifndef CONFIG_H
#define CONFIG_H

#include "winlnxdefs.h"
#include "m64p.h"

BOOL Config_Open();
int Config_ReadScreenInt(const char *itemname);
PackedScreenResolution Config_ReadScreenSettings();
BOOL Config_ReadInt(const char *itemname, const char *desc, int def_value, int create=TRUE, BOOL isBoolean=TRUE);


#endif /* CONFIG_H */

