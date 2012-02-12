/* Extended string routines
 *
 * Copyright notice for this file:
 *  Copyright (C) 2004 Jason Oster (Parasyte)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <string>
#include <string.h>
#include <stdlib.h>

#include "../types.h"
#include "../emufile.h"

#ifndef __GNUC__
#define strcasecmp strcmp
#endif

void splitpath(const char* path, char* drv, char* dir, char* name, char* ext);

std::string mass_replace(const std::string &source, const std::string &victim, const std::string &replacement);

//TODO - dont we already have another  function that can do this
std::string getExtension(const char* input);
