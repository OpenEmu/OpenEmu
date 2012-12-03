/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - COLOR.h                                                 *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2002 Rice1964                                           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef XCOLOR_H
#define XCOLOR_H

typedef struct _COLORVALUE 
{
   
       float r;
       float g;
       float b;
       float a;
} COLORVALUE;


typedef struct XCOLOR {
   float r, g, b, a;
#ifdef __cplusplus
 public:
   XCOLOR() 
     {
     }
   
   XCOLOR( unsigned int argb );
   XCOLOR( const float * );
   XCOLOR( const COLORVALUE& );
   XCOLOR( float r, float g, float b, float a );
   
   // casting
   operator unsigned int () const;
   
   operator float* ();
   operator const float* () const;
   
   operator COLORVALUE* ();
   operator const COLORVALUE* () const;
    
   operator COLORVALUE& ();
   operator const COLORVALUE& () const;
   
   // assignment operators
   XCOLOR& operator += ( const XCOLOR& );
   XCOLOR& operator -= ( const XCOLOR& );
   XCOLOR& operator *= ( float );
   XCOLOR& operator /= ( float );
   
   // unary operators
   XCOLOR operator + () const;
   XCOLOR operator - () const;
   
   // binary operators
   XCOLOR operator + ( const XCOLOR& ) const;
   XCOLOR operator - ( const XCOLOR& ) const;
   XCOLOR operator * ( float ) const;
   XCOLOR operator / ( float ) const;
   
   friend XCOLOR operator * (float, const XCOLOR& );
    
   bool operator == ( const XCOLOR& ) const;
   bool operator != ( const XCOLOR& ) const;
   
#endif //__cplusplus
} XCOLOR;

#endif

