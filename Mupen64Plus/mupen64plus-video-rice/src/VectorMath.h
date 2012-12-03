/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - VectorMath.h                                            *
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

#ifndef VECTORMATH_H
#define VECTORMATH_H

/******************************************************************************
 * 4x4 matrix
 ******************************************************************************/

typedef struct _MATRIX {
    union {
        struct {
            float        _11, _12, _13, _14;
            float        _21, _22, _23, _24;
            float        _31, _32, _33, _34;
            float        _41, _42, _43, _44;
        };
        float m[4][4];
    };
} MATRIX;

typedef struct XMATRIX : public MATRIX {
public:
    XMATRIX();
    XMATRIX( const float * );
    XMATRIX( const MATRIX & );
    XMATRIX( float _11, float _12, float _13, float _14,
                float _21, float _22, float _23, float _24,
                float _31, float _32, float _33, float _34,
                float _41, float _42, float _43, float _44 );

    float& operator () ( unsigned int Row, unsigned int Col );
    float  operator () ( unsigned int Row, unsigned int Col ) const;

    operator float* ();
    operator const float* () const;

    // assignment operators
    XMATRIX& operator *= ( const XMATRIX & );
    XMATRIX& operator += ( const XMATRIX & );
    XMATRIX& operator -= ( const XMATRIX & );
    XMATRIX& operator *= ( float );
    XMATRIX& operator /= ( float );

    // unary operators
    XMATRIX operator + () const;
    XMATRIX operator - () const;

    // binary operators
    XMATRIX operator * ( const XMATRIX & ) const;
    XMATRIX operator + ( const XMATRIX & ) const;
    XMATRIX operator - ( const XMATRIX & ) const;
    XMATRIX operator * ( float ) const;
    XMATRIX operator / ( float ) const;
    friend XMATRIX operator * ( float, const XMATRIX & );
    bool operator == ( const XMATRIX & ) const;
    bool operator != ( const XMATRIX & ) const;
} XMATRIX, *LPXMATRIX;

/******************************************************************************
 * 3d vector
 ******************************************************************************/

typedef struct _VECTOR3
{
    float x;
    float y;
    float z;
} VECTOR3;

class XVECTOR3 : public VECTOR3
{
public:
    XVECTOR3();
    XVECTOR3( const float *f );
    XVECTOR3( const VECTOR3 &v );
    XVECTOR3( float _x, float _y, float _z );

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
};

/******************************************************************************
 * 4d vector
 ******************************************************************************/

typedef struct _VECTOR4
{
    float x;
    float y;
    float z;
    float w;
} VECTOR4;

class XVECTOR4 : public VECTOR4
{
public:
    XVECTOR4();
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
};

XVECTOR4 Vec3Transform(XVECTOR4 *pOut, const XVECTOR3 *pV, const XMATRIX *pM);

XMATRIX* MatrixTranspose(XMATRIX* pOut, const XMATRIX* pM);

#endif

