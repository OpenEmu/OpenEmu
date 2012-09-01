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
 * Types.h
 * 
 * Fundamental data types. This file is used by both C++ and C modules, so it
 * must NOT include any C++-specific constructs. Some modules may elect to 
 * include it directly rather than through Supermodel.h.
 *
 * All ports must define this file.
 */
 
#ifndef INCLUDED_TYPES_H
#define INCLUDED_TYPES_H


// Boolean return codes (must be 0 or 1 only)
#define OKAY	0
#define FAIL	1

// Types
typedef unsigned long long	UINT64;
typedef signed long long	INT64;
typedef unsigned int		UINT32;
typedef signed int			INT32;
typedef unsigned short int	UINT16;
typedef signed short int	INT16;
typedef unsigned char		UINT8;
typedef signed char			INT8;
typedef float				FLOAT32;
typedef double				FLOAT64;


#endif	// INCLUDED_TYPES_H
