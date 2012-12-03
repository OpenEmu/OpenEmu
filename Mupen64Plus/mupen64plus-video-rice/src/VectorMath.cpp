/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - VectorMath.cpp                                          *
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

#include <string.h>

#include "VectorMath.h"

//---------- XMATRIX

XMATRIX::XMATRIX()
{
}

XMATRIX::XMATRIX( const float *pIn )
{
   memcpy(m, pIn, 16*4);
}

XMATRIX::XMATRIX( const MATRIX &pIn )
{
   memcpy(m, pIn.m, 16*4);
}

XMATRIX::XMATRIX( float _11, float _12, float _13, float _14,
              float _21, float _22, float _23, float _24,
              float _31, float _32, float _33, float _34,
              float _41, float _42, float _43, float _44 )
{
   this->_11 = _11;
   this->_12 = _12;
   this->_13 = _13;
   this->_14 = _14;
   this->_21 = _21;
   this->_22 = _22;
   this->_23 = _23;
   this->_24 = _24;
   this->_31 = _31;
   this->_32 = _32;
   this->_33 = _33;
   this->_34 = _34;
   this->_41 = _41;
   this->_42 = _42;
   this->_43 = _43;
   this->_44 = _44;
}

float& XMATRIX::operator () ( unsigned int Row, unsigned int Col )
{
   return m[Row][Col];
}

float  XMATRIX::operator () ( unsigned int Row, unsigned int Col ) const
{
   return m[Row][Col];
}

XMATRIX::operator float* ()
{
   return (float*)m;
}

XMATRIX::operator const float* () const
{
   return (float*)m;
}

XMATRIX& XMATRIX::operator *= ( const XMATRIX &pIn )
{
   XMATRIX mTemp(*this);
   *this = mTemp*pIn;
   return *this;
}

XMATRIX& XMATRIX::operator += ( const XMATRIX &pIn )
{
   XMATRIX mTemp(*this);
   *this = mTemp+pIn;
   return *this;
}

XMATRIX& XMATRIX::operator -= ( const XMATRIX &pIn )
{
   XMATRIX mTemp(*this);
   *this = mTemp-pIn;
   return *this;
}

XMATRIX& XMATRIX::operator *= ( float f)
{
   for (int i=0; i<16; i++) ((float*)m)[i] *= f;
   return *this;
}

XMATRIX& XMATRIX::operator /= ( float f)
{
   for (int i=0; i<16; i++) ((float*)m)[i] /= f;
   return *this;
}

XMATRIX XMATRIX::operator + () const
{
   return *this;
}

XMATRIX XMATRIX::operator - () const
{
   XMATRIX mTemp;
   for (int i=0; i<16; i++) ((float*)mTemp.m)[i] = -((float*)m)[i];
   return mTemp;
}

XMATRIX XMATRIX::operator * ( const XMATRIX &pIn ) const
{
   XMATRIX mTemp;
   for (int i=0; i<4; i++)
     for (int j=0; j<4; j++)
       mTemp.m[i][j] = m[i][0]*pIn.m[0][j] +
                       m[i][1]*pIn.m[1][j] +
                       m[i][2]*pIn.m[2][j] +
                       m[i][3]*pIn.m[3][j];
   return mTemp;
}

XMATRIX XMATRIX::operator + ( const XMATRIX &pIn ) const
{
   XMATRIX mTemp;
   for (int i=0; i<16; i++)
     ((float*)mTemp.m)[i] = ((float*)m)[i] + ((float*)pIn.m)[i];
   return mTemp;
}

XMATRIX XMATRIX::operator - ( const XMATRIX &pIn ) const
{
   XMATRIX mTemp;
   for (int i=0; i<16; i++)
     ((float*)mTemp.m)[i] = ((float*)m)[i] - ((float*)pIn.m)[i];
   return mTemp;
}

/*
    XMATRIX operator * ( float ) const;
    XMATRIX operator / ( float ) const;
    friend XMATRIX operator * ( float, const XMATRIX & );
    bool operator == ( const XMATRIX & ) const;
    bool operator != ( const XMATRIX & ) const;
*/

//---------- VECTOR3

XVECTOR3::XVECTOR3()
{
}

XVECTOR3::XVECTOR3( const float *f )
{
   x = f[0];
   y = f[1];
   z = f[2];
}

XVECTOR3::XVECTOR3( const VECTOR3 &v )
{
   x = v.x;
   y = v.y;
   z = v.z;
}

XVECTOR3::XVECTOR3( float _x, float _y, float _z )
{
   x = _x;
   y = _y;
   z = _z;
}

/*
    // casting
    inline operator float* ();
    inline operator const float* () const;

    // assignment operators
    inline XVECTOR3& operator += ( const XVECTOR3 &op );
    inline XVECTOR3& operator -= ( const XVECTOR3 &op );
    inline XVECTOR3& operator *= ( float op );
        inline XVECTOR3& operator /= ( float op );

    // unary operators
    inline XVECTOR3 operator + () const;
    inline XVECTOR3 operator - () const;

    // binary operators
        inline XVECTOR3 operator + ( const XVECTOR3 &op ) const;
    inline XVECTOR3 operator - ( const XVECTOR3 &op ) const;
    inline XVECTOR3 operator * ( float op ) const;
    inline XVECTOR3 operator / ( float op ) const;


    friend XVECTOR3 operator * ( float, const XVECTOR3& );

    inline bool operator == ( const XVECTOR3 &op ) const;
    inline bool operator != ( const XVECTOR3 &op ) const;
*/

//---------- XVECTOR4

XVECTOR4::XVECTOR4()
{
}

/*
    XVECTOR4( const float *f );
    XVECTOR4( const VECTOR4 &v );
    XVECTOR4( float _x, float _y, float _z, float _w );

    // casting
        inline operator float* ();
    inline operator const float* () const;

    // assignment operators
    inline XVECTOR4& operator += ( const XVECTOR4 &op );
    inline XVECTOR4& operator -= ( const XVECTOR4 &op );
    inline XVECTOR4& operator *= ( float op );
    inline XVECTOR4& operator /= ( float op );

    // unary operators
    inline XVECTOR4 operator + () const;
    inline XVECTOR4 operator - () const;

    // binary operators
    inline XVECTOR4 operator + ( const XVECTOR4 &op ) const;
    inline XVECTOR4 operator - ( const XVECTOR4 &op ) const;
    inline XVECTOR4 operator * ( float op ) const;
    inline XVECTOR4 operator / ( float op ) const;

    friend XVECTOR4 operator * ( float, const XVECTOR4& );

    inline bool operator == ( const XVECTOR4 &op ) const;
    inline bool operator != ( const XVECTOR4 &op ) const;
*/

//---------- OTHER

XMATRIX* MatrixTranspose(
                XMATRIX* pOut,
                const XMATRIX* pM
                )
{
   pOut->_11 = pM->_11;
   pOut->_12 = pM->_21;
   pOut->_13 = pM->_31;
   pOut->_14 = pM->_41;
   pOut->_21 = pM->_12;
   pOut->_22 = pM->_22;
   pOut->_23 = pM->_32;
   pOut->_24 = pM->_42;
   pOut->_31 = pM->_13;
   pOut->_32 = pM->_23;
   pOut->_33 = pM->_33;
   pOut->_34 = pM->_43;
   pOut->_41 = pM->_14;
   pOut->_42 = pM->_24;
   pOut->_43 = pM->_34;
   pOut->_44 = pM->_44;
   return pOut;
}

XVECTOR4 Vec3Transform(XVECTOR4 *pOut, const XVECTOR3 *pV, const XMATRIX *pM)
{
   pOut->x = pV->x*pM->_11 + pV->y*pM->_21 + pV->z*pM->_31 + pM->_41;
   pOut->y = pV->x*pM->_12 + pV->y*pM->_22 + pV->z*pM->_32 + pM->_42;
   pOut->z = pV->x*pM->_13 + pV->y*pM->_23 + pV->z*pM->_33 + pM->_43;
   pOut->w = pV->x*pM->_14 + pV->y*pM->_24 + pV->z*pM->_34 + pM->_44;
   return *pOut;
}

