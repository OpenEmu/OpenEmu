/***************************************************************************
 * Gens: Old INI file handler functions. [BROKEN]                          *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008 by David Korth                                       *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU General Public License as published by the   *
 * Free Software Foundation; either version 2 of the License, or (at your  *
 * option) any later version.                                              *
 *                                                                         *
 * This program is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License along *
 * with this program; if not, write to the Free Software Foundation, Inc., *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.           *
 ***************************************************************************/

#ifndef GENS_INI_OLD_H
#define GENS_INI_OLD_H

#ifdef __cplusplus
extern "C" {
#endif

int GetPrivateProfileInt(const char *section, const char *var,
			 const int def, const char *filename);

void GetPrivateProfileString(const char *section, const char *var,
			     const char *def, char *get, const int length,
			     const char *filename);

void WritePrivateProfileString(const char *section, const char *var,
			       const char *var_name, const char *filename);

#ifdef __cplusplus
}
#endif

#endif /* GENS_INI_OLD_H */
