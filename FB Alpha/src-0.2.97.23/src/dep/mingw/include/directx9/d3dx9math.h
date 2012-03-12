//////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
//  File:       d3dx9math.h
//  Content:    D3DX math types and functions
//
//////////////////////////////////////////////////////////////////////////////

#include "d3dx9.h"

#ifndef __D3DX9MATH_H__
#define __D3DX9MATH_H__

#include <math.h>
//#if _MSC_VER >= 1200
//#pragma warning(push)
//#endif
//#pragma warning(disable:4201) // anonymous unions warning

#if __GNUC__ >= 4
#pragma GCC diagnostic ignored "-Wshadow"
#endif

//===========================================================================
//
// General purpose utilities
//
//===========================================================================
#define D3DX_PI    ((FLOAT)  3.141592654f)
#define D3DX_1BYPI ((FLOAT)  0.318309886f)

#define D3DXToRadian( degree ) ((degree) * (D3DX_PI / 180.0f))
#define D3DXToDegree( radian ) ((radian) * (180.0f / D3DX_PI))



//===========================================================================
//
// 16 bit floating point numbers
//
//===========================================================================

#define D3DX_16F_DIG          3                // # of decimal digits of precision
#define D3DX_16F_EPSILON      4.8875809e-4f    // smallest such that 1.0 + epsilon != 1.0
#define D3DX_16F_MANT_DIG     11               // # of bits in mantissa
#define D3DX_16F_MAX          6.550400e+004    // max value
#define D3DX_16F_MAX_10_EXP   4                // max decimal exponent
#define D3DX_16F_MAX_EXP      15               // max binary exponent
#define D3DX_16F_MIN          6.1035156e-5f    // min positive value
#define D3DX_16F_MIN_10_EXP   (-4)             // min decimal exponent
#define D3DX_16F_MIN_EXP      (-14)            // min binary exponent
#define D3DX_16F_RADIX        2                // exponent radix
#define D3DX_16F_ROUNDS       1                // addition rounding: near


typedef struct D3DXFLOAT16
{
#ifdef __cplusplus
public:
    D3DXFLOAT16() {};
    D3DXFLOAT16( FLOAT );
    D3DXFLOAT16( CONST D3DXFLOAT16& );

    // casting
    operator FLOAT ();

    // binary operators
    BOOL operator == ( CONST D3DXFLOAT16& ) const;
    BOOL operator != ( CONST D3DXFLOAT16& ) const;

protected:
#endif //__cplusplus
    WORD value;
} D3DXFLOAT16, *LPD3DXFLOAT16;



//===========================================================================
//
// Vectors
//
//===========================================================================


//--------------------------
// 2D Vector
//--------------------------
typedef struct D3DXVECTOR2
{
#ifdef __cplusplus
public:
    D3DXVECTOR2() {};
    D3DXVECTOR2( CONST FLOAT * );
    D3DXVECTOR2( CONST D3DXFLOAT16 * );
    D3DXVECTOR2( FLOAT x, FLOAT y );

    // casting
    operator FLOAT* ();
    operator CONST FLOAT* () const;

    // assignment operators
    D3DXVECTOR2& operator += ( CONST D3DXVECTOR2& );
    D3DXVECTOR2& operator -= ( CONST D3DXVECTOR2& );
    D3DXVECTOR2& operator *= ( FLOAT );
    D3DXVECTOR2& operator /= ( FLOAT );

    // unary operators
    D3DXVECTOR2 operator + () const;
    D3DXVECTOR2 operator - () const;

    // binary operators
    D3DXVECTOR2 operator + ( CONST D3DXVECTOR2& ) const;
    D3DXVECTOR2 operator - ( CONST D3DXVECTOR2& ) const;
    D3DXVECTOR2 operator * ( FLOAT ) const;
    D3DXVECTOR2 operator / ( FLOAT ) const;

    friend D3DXVECTOR2 operator * ( FLOAT, CONST D3DXVECTOR2& );

    BOOL operator == ( CONST D3DXVECTOR2& ) const;
    BOOL operator != ( CONST D3DXVECTOR2& ) const;


public:
#endif //__cplusplus
    FLOAT x, y;
} D3DXVECTOR2, *LPD3DXVECTOR2;



//--------------------------
// 2D Vector (16 bit)
//--------------------------

typedef struct D3DXVECTOR2_16F
{
#ifdef __cplusplus
public:
    D3DXVECTOR2_16F() {};
    D3DXVECTOR2_16F( CONST FLOAT * );
    D3DXVECTOR2_16F( CONST D3DXFLOAT16 * );
    D3DXVECTOR2_16F( CONST D3DXFLOAT16 &x, CONST D3DXFLOAT16 &y );

    // casting
    operator D3DXFLOAT16* ();
    operator CONST D3DXFLOAT16* () const;

    // binary operators
    BOOL operator == ( CONST D3DXVECTOR2_16F& ) const;
    BOOL operator != ( CONST D3DXVECTOR2_16F& ) const;

public:
#endif //__cplusplus
    D3DXFLOAT16 x, y;

} D3DXVECTOR2_16F, *LPD3DXVECTOR2_16F;



//--------------------------
// 3D Vector
//--------------------------
#ifdef __cplusplus
typedef struct D3DXVECTOR3 : public D3DVECTOR
{
public:
    D3DXVECTOR3() {};
    D3DXVECTOR3( CONST FLOAT * );
    D3DXVECTOR3( CONST D3DVECTOR& );
    D3DXVECTOR3( CONST D3DXFLOAT16 * );
    D3DXVECTOR3( FLOAT x, FLOAT y, FLOAT z );

    // casting
    operator FLOAT* ();
    operator CONST FLOAT* () const;

    // assignment operators
    D3DXVECTOR3& operator += ( CONST D3DXVECTOR3& );
    D3DXVECTOR3& operator -= ( CONST D3DXVECTOR3& );
    D3DXVECTOR3& operator *= ( FLOAT );
    D3DXVECTOR3& operator /= ( FLOAT );

    // unary operators
    D3DXVECTOR3 operator + () const;
    D3DXVECTOR3 operator - () const;

    // binary operators
    D3DXVECTOR3 operator + ( CONST D3DXVECTOR3& ) const;
    D3DXVECTOR3 operator - ( CONST D3DXVECTOR3& ) const;
    D3DXVECTOR3 operator * ( FLOAT ) const;
    D3DXVECTOR3 operator / ( FLOAT ) const;

    friend D3DXVECTOR3 operator * ( FLOAT, CONST struct D3DXVECTOR3& );

    BOOL operator == ( CONST D3DXVECTOR3& ) const;
    BOOL operator != ( CONST D3DXVECTOR3& ) const;

} D3DXVECTOR3, *LPD3DXVECTOR3;

#else //!__cplusplus
typedef struct _D3DVECTOR D3DXVECTOR3, *LPD3DXVECTOR3;
#endif //!__cplusplus



//--------------------------
// 3D Vector (16 bit)
//--------------------------
typedef struct D3DXVECTOR3_16F
{
#ifdef __cplusplus
public:
    D3DXVECTOR3_16F() {};
    D3DXVECTOR3_16F( CONST FLOAT * );
    D3DXVECTOR3_16F( CONST D3DVECTOR& );
    D3DXVECTOR3_16F( CONST D3DXFLOAT16 * );
    D3DXVECTOR3_16F( CONST D3DXFLOAT16 &x, CONST D3DXFLOAT16 &y, CONST D3DXFLOAT16 &z );

    // casting
    operator D3DXFLOAT16* ();
    operator CONST D3DXFLOAT16* () const;

    // binary operators
    BOOL operator == ( CONST D3DXVECTOR3_16F& ) const;
    BOOL operator != ( CONST D3DXVECTOR3_16F& ) const;

public:
#endif //__cplusplus
    D3DXFLOAT16 x, y, z;

} D3DXVECTOR3_16F, *LPD3DXVECTOR3_16F;



//--------------------------
// 4D Vector
//--------------------------
typedef struct D3DXVECTOR4
{
#ifdef __cplusplus
public:
    D3DXVECTOR4() {};
    D3DXVECTOR4( CONST FLOAT* );
    D3DXVECTOR4( CONST D3DXFLOAT16* );
    D3DXVECTOR4( CONST D3DVECTOR& xyz, FLOAT w );
    D3DXVECTOR4( FLOAT x, FLOAT y, FLOAT z, FLOAT w );

    // casting
    operator FLOAT* ();
    operator CONST FLOAT* () const;

    // assignment operators
    D3DXVECTOR4& operator += ( CONST D3DXVECTOR4& );
    D3DXVECTOR4& operator -= ( CONST D3DXVECTOR4& );
    D3DXVECTOR4& operator *= ( FLOAT );
    D3DXVECTOR4& operator /= ( FLOAT );

    // unary operators
    D3DXVECTOR4 operator + () const;
    D3DXVECTOR4 operator - () const;

    // binary operators
    D3DXVECTOR4 operator + ( CONST D3DXVECTOR4& ) const;
    D3DXVECTOR4 operator - ( CONST D3DXVECTOR4& ) const;
    D3DXVECTOR4 operator * ( FLOAT ) const;
    D3DXVECTOR4 operator / ( FLOAT ) const;

    friend D3DXVECTOR4 operator * ( FLOAT, CONST D3DXVECTOR4& );

    BOOL operator == ( CONST D3DXVECTOR4& ) const;
    BOOL operator != ( CONST D3DXVECTOR4& ) const;

public:
#endif //__cplusplus
    FLOAT x, y, z, w;
} D3DXVECTOR4, *LPD3DXVECTOR4;


//--------------------------
// 4D Vector (16 bit)
//--------------------------
typedef struct D3DXVECTOR4_16F
{
#ifdef __cplusplus
public:
    D3DXVECTOR4_16F() {};
    D3DXVECTOR4_16F( CONST FLOAT * );
    D3DXVECTOR4_16F( CONST D3DXFLOAT16* );
    D3DXVECTOR4_16F( CONST D3DXVECTOR3_16F& xyz, CONST D3DXFLOAT16& w );
    D3DXVECTOR4_16F( CONST D3DXFLOAT16& x, CONST D3DXFLOAT16& y, CONST D3DXFLOAT16& z, CONST D3DXFLOAT16& w );

    // casting
    operator D3DXFLOAT16* ();
    operator CONST D3DXFLOAT16* () const;

    // binary operators
    BOOL operator == ( CONST D3DXVECTOR4_16F& ) const;
    BOOL operator != ( CONST D3DXVECTOR4_16F& ) const;

public:
#endif //__cplusplus
    D3DXFLOAT16 x, y, z, w;

} D3DXVECTOR4_16F, *LPD3DXVECTOR4_16F;



//===========================================================================
//
// Matrices
//
//===========================================================================
#ifdef __cplusplus
typedef struct D3DXMATRIX : public D3DMATRIX
{
public:
    D3DXMATRIX() {};
    D3DXMATRIX( CONST FLOAT * );
    D3DXMATRIX( CONST D3DMATRIX& );
    D3DXMATRIX( CONST D3DXFLOAT16 * );
    D3DXMATRIX( FLOAT _11, FLOAT _12, FLOAT _13, FLOAT _14,
                FLOAT _21, FLOAT _22, FLOAT _23, FLOAT _24,
                FLOAT _31, FLOAT _32, FLOAT _33, FLOAT _34,
                FLOAT _41, FLOAT _42, FLOAT _43, FLOAT _44 );


    // access grants
    FLOAT& operator () ( UINT Row, UINT Col );
    FLOAT  operator () ( UINT Row, UINT Col ) const;

    // casting operators
    operator FLOAT* ();
    operator CONST FLOAT* () const;

    // assignment operators
    D3DXMATRIX& operator *= ( CONST D3DXMATRIX& );
    D3DXMATRIX& operator += ( CONST D3DXMATRIX& );
    D3DXMATRIX& operator -= ( CONST D3DXMATRIX& );
    D3DXMATRIX& operator *= ( FLOAT );
    D3DXMATRIX& operator /= ( FLOAT );

    // unary operators
    D3DXMATRIX operator + () const;
    D3DXMATRIX operator - () const;

    // binary operators
    D3DXMATRIX operator * ( CONST D3DXMATRIX& ) const;
    D3DXMATRIX operator + ( CONST D3DXMATRIX& ) const;
    D3DXMATRIX operator - ( CONST D3DXMATRIX& ) const;
    D3DXMATRIX operator * ( FLOAT ) const;
    D3DXMATRIX operator / ( FLOAT ) const;

    friend D3DXMATRIX operator * ( FLOAT, CONST D3DXMATRIX& );

    BOOL operator == ( CONST D3DXMATRIX& ) const;
    BOOL operator != ( CONST D3DXMATRIX& ) const;

} D3DXMATRIX, *LPD3DXMATRIX;

#else //!__cplusplus
typedef struct _D3DMATRIX D3DXMATRIX, *LPD3DXMATRIX;
#endif //!__cplusplus


//---------------------------------------------------------------------------
// Aligned Matrices
//
// This class helps keep matrices 16-byte aligned as preferred by P4 cpus.
// It aligns matrices on the stack and on the heap or in global scope.
// It does this using __declspec(align(16)) which works on VC7 and on VC 6
// with the processor pack. Unfortunately there is no way to detect the 
// latter so this is turned on only on VC7. On other compilers this is the
// the same as D3DXMATRIX.
//
// Using this class on a compiler that does not actually do the alignment
// can be dangerous since it will not expose bugs that ignore alignment.
// E.g if an object of this class in inside a struct or class, and some code
// memcopys data in it assuming tight packing. This could break on a compiler
// that eventually start aligning the matrix.
//---------------------------------------------------------------------------
#ifdef __cplusplus
typedef struct _D3DXMATRIXA16 : public D3DXMATRIX
{
    _D3DXMATRIXA16() {}
    _D3DXMATRIXA16( CONST FLOAT * );
    _D3DXMATRIXA16( CONST D3DMATRIX& );
    _D3DXMATRIXA16( CONST D3DXFLOAT16 * );
    _D3DXMATRIXA16( FLOAT _11, FLOAT _12, FLOAT _13, FLOAT _14,
                    FLOAT _21, FLOAT _22, FLOAT _23, FLOAT _24,
                    FLOAT _31, FLOAT _32, FLOAT _33, FLOAT _34,
                    FLOAT _41, FLOAT _42, FLOAT _43, FLOAT _44 );

    // new operators
    void* operator new   ( size_t );
    void* operator new[] ( size_t );

    // delete operators
    void operator delete   ( void* );   // These are NOT virtual; Do not 
    void operator delete[] ( void* );   // cast to D3DXMATRIX and delete.
    
    // assignment operators
    _D3DXMATRIXA16& operator = ( CONST D3DXMATRIX& );

} _D3DXMATRIXA16;

#else //!__cplusplus
typedef D3DXMATRIX  _D3DXMATRIXA16;
#endif //!__cplusplus



//#if _MSC_VER >= 1300  // VC7
//#define D3DX_ALIGN16 __declspec(align(16))
//#else
#define D3DX_ALIGN16  // Earlier compiler may not understand this, do nothing.
//#endif

typedef D3DX_ALIGN16 _D3DXMATRIXA16 D3DXMATRIXA16, *LPD3DXMATRIXA16;



//===========================================================================
//
//    Quaternions
//
//===========================================================================
typedef struct D3DXQUATERNION
{
#ifdef __cplusplus
public:
    D3DXQUATERNION() {}
    D3DXQUATERNION( CONST FLOAT * );
    D3DXQUATERNION( CONST D3DXFLOAT16 * );
    D3DXQUATERNION( FLOAT x, FLOAT y, FLOAT z, FLOAT w );

    // casting
    operator FLOAT* ();
    operator CONST FLOAT* () const;

    // assignment operators
    D3DXQUATERNION& operator += ( CONST D3DXQUATERNION& );
    D3DXQUATERNION& operator -= ( CONST D3DXQUATERNION& );
    D3DXQUATERNION& operator *= ( CONST D3DXQUATERNION& );
    D3DXQUATERNION& operator *= ( FLOAT );
    D3DXQUATERNION& operator /= ( FLOAT );

    // unary operators
    D3DXQUATERNION  operator + () const;
    D3DXQUATERNION  operator - () const;

    // binary operators
    D3DXQUATERNION operator + ( CONST D3DXQUATERNION& ) const;
    D3DXQUATERNION operator - ( CONST D3DXQUATERNION& ) const;
    D3DXQUATERNION operator * ( CONST D3DXQUATERNION& ) const;
    D3DXQUATERNION operator * ( FLOAT ) const;
    D3DXQUATERNION operator / ( FLOAT ) const;

    friend D3DXQUATERNION operator * (FLOAT, CONST D3DXQUATERNION& );

    BOOL operator == ( CONST D3DXQUATERNION& ) const;
    BOOL operator != ( CONST D3DXQUATERNION& ) const;

#endif //__cplusplus
    FLOAT x, y, z, w;
} D3DXQUATERNION, *LPD3DXQUATERNION;


//===========================================================================
//
// Planes
//
//===========================================================================
typedef struct D3DXPLANE
{
#ifdef __cplusplus
public:
    D3DXPLANE() {}
    D3DXPLANE( CONST FLOAT* );
    D3DXPLANE( CONST D3DXFLOAT16* );
    D3DXPLANE( FLOAT a, FLOAT b, FLOAT c, FLOAT d );

    // casting
    operator FLOAT* ();
    operator CONST FLOAT* () const;

    // assignment operators
    D3DXPLANE& operator *= ( FLOAT );
    D3DXPLANE& operator /= ( FLOAT );

    // unary operators
    D3DXPLANE operator + () const;
    D3DXPLANE operator - () const;

    // binary operators
    D3DXPLANE operator * ( FLOAT ) const;
    D3DXPLANE operator / ( FLOAT ) const;

    friend D3DXPLANE operator * ( FLOAT, CONST D3DXPLANE& );

    BOOL operator == ( CONST D3DXPLANE& ) const;
    BOOL operator != ( CONST D3DXPLANE& ) const;

#endif //__cplusplus
    FLOAT a, b, c, d;
} D3DXPLANE, *LPD3DXPLANE;


//===========================================================================
//
// Colors
//
//===========================================================================

typedef struct D3DXCOLOR
{
#ifdef __cplusplus
public:
    D3DXCOLOR() {}
    D3DXCOLOR( DWORD argb );
    D3DXCOLOR( CONST FLOAT * );
    D3DXCOLOR( CONST D3DXFLOAT16 * );
    D3DXCOLOR( CONST D3DCOLORVALUE& );
    D3DXCOLOR( FLOAT r, FLOAT g, FLOAT b, FLOAT a );

    // casting
    operator DWORD () const;

    operator FLOAT* ();
    operator CONST FLOAT* () const;

    operator D3DCOLORVALUE* ();
    operator CONST D3DCOLORVALUE* () const;

    operator D3DCOLORVALUE& ();
    operator CONST D3DCOLORVALUE& () const;

    // assignment operators
    D3DXCOLOR& operator += ( CONST D3DXCOLOR& );
    D3DXCOLOR& operator -= ( CONST D3DXCOLOR& );
    D3DXCOLOR& operator *= ( FLOAT );
    D3DXCOLOR& operator /= ( FLOAT );

    // unary operators
    D3DXCOLOR operator + () const;
    D3DXCOLOR operator - () const;

    // binary operators
    D3DXCOLOR operator + ( CONST D3DXCOLOR& ) const;
    D3DXCOLOR operator - ( CONST D3DXCOLOR& ) const;
    D3DXCOLOR operator * ( FLOAT ) const;
    D3DXCOLOR operator / ( FLOAT ) const;

    friend D3DXCOLOR operator * ( FLOAT, CONST D3DXCOLOR& );

    BOOL operator == ( CONST D3DXCOLOR& ) const;
    BOOL operator != ( CONST D3DXCOLOR& ) const;

#endif //__cplusplus
    FLOAT r, g, b, a;
} D3DXCOLOR, *LPD3DXCOLOR;



//===========================================================================
//
// D3DX math functions:
//
// NOTE:
//  * All these functions can take the same object as in and out parameters.
//
//  * Out parameters are typically also returned as return values, so that
//    the output of one function may be used as a parameter to another.
//
//===========================================================================

//--------------------------
// Float16
//--------------------------

// non-inline
#ifdef __cplusplus
extern "C" {
#endif

// Converts an array 32-bit floats to 16-bit floats
D3DXFLOAT16* WINAPI D3DXFloat32To16Array
    ( D3DXFLOAT16 *pOut, CONST FLOAT *pIn, UINT n );

// Converts an array 16-bit floats to 32-bit floats
FLOAT* WINAPI D3DXFloat16To32Array
    ( FLOAT *pOut, CONST D3DXFLOAT16 *pIn, UINT n );

#ifdef __cplusplus
}
#endif


//--------------------------
// 2D Vector
//--------------------------

// inline

FLOAT D3DXVec2Length
    ( CONST D3DXVECTOR2 *pV );

FLOAT D3DXVec2LengthSq
    ( CONST D3DXVECTOR2 *pV );

FLOAT D3DXVec2Dot
    ( CONST D3DXVECTOR2 *pV1, CONST D3DXVECTOR2 *pV2 );

// Z component of ((x1,y1,0) cross (x2,y2,0))
FLOAT D3DXVec2CCW
    ( CONST D3DXVECTOR2 *pV1, CONST D3DXVECTOR2 *pV2 );

D3DXVECTOR2* D3DXVec2Add
    ( D3DXVECTOR2 *pOut, CONST D3DXVECTOR2 *pV1, CONST D3DXVECTOR2 *pV2 );

D3DXVECTOR2* D3DXVec2Subtract
    ( D3DXVECTOR2 *pOut, CONST D3DXVECTOR2 *pV1, CONST D3DXVECTOR2 *pV2 );

// Minimize each component.  x = min(x1, x2), y = min(y1, y2)
D3DXVECTOR2* D3DXVec2Minimize
    ( D3DXVECTOR2 *pOut, CONST D3DXVECTOR2 *pV1, CONST D3DXVECTOR2 *pV2 );

// Maximize each component.  x = max(x1, x2), y = max(y1, y2)
D3DXVECTOR2* D3DXVec2Maximize
    ( D3DXVECTOR2 *pOut, CONST D3DXVECTOR2 *pV1, CONST D3DXVECTOR2 *pV2 );

D3DXVECTOR2* D3DXVec2Scale
    ( D3DXVECTOR2 *pOut, CONST D3DXVECTOR2 *pV, FLOAT s );

// Linear interpolation. V1 + s(V2-V1)
D3DXVECTOR2* D3DXVec2Lerp
    ( D3DXVECTOR2 *pOut, CONST D3DXVECTOR2 *pV1, CONST D3DXVECTOR2 *pV2,
      FLOAT s );

// non-inline
#ifdef __cplusplus
extern "C" {
#endif

D3DXVECTOR2* WINAPI D3DXVec2Normalize
    ( D3DXVECTOR2 *pOut, CONST D3DXVECTOR2 *pV );

// Hermite interpolation between position V1, tangent T1 (when s == 0)
// and position V2, tangent T2 (when s == 1).
D3DXVECTOR2* WINAPI D3DXVec2Hermite
    ( D3DXVECTOR2 *pOut, CONST D3DXVECTOR2 *pV1, CONST D3DXVECTOR2 *pT1,
      CONST D3DXVECTOR2 *pV2, CONST D3DXVECTOR2 *pT2, FLOAT s );

// CatmullRom interpolation between V1 (when s == 0) and V2 (when s == 1)
D3DXVECTOR2* WINAPI D3DXVec2CatmullRom
    ( D3DXVECTOR2 *pOut, CONST D3DXVECTOR2 *pV0, CONST D3DXVECTOR2 *pV1,
      CONST D3DXVECTOR2 *pV2, CONST D3DXVECTOR2 *pV3, FLOAT s );

// Barycentric coordinates.  V1 + f(V2-V1) + g(V3-V1)
D3DXVECTOR2* WINAPI D3DXVec2BaryCentric
    ( D3DXVECTOR2 *pOut, CONST D3DXVECTOR2 *pV1, CONST D3DXVECTOR2 *pV2,
      CONST D3DXVECTOR2 *pV3, FLOAT f, FLOAT g);

// Transform (x, y, 0, 1) by matrix.
D3DXVECTOR4* WINAPI D3DXVec2Transform
    ( D3DXVECTOR4 *pOut, CONST D3DXVECTOR2 *pV, CONST D3DXMATRIX *pM );

// Transform (x, y, 0, 1) by matrix, project result back into w=1.
D3DXVECTOR2* WINAPI D3DXVec2TransformCoord
    ( D3DXVECTOR2 *pOut, CONST D3DXVECTOR2 *pV, CONST D3DXMATRIX *pM );

// Transform (x, y, 0, 0) by matrix.
D3DXVECTOR2* WINAPI D3DXVec2TransformNormal
    ( D3DXVECTOR2 *pOut, CONST D3DXVECTOR2 *pV, CONST D3DXMATRIX *pM );
     
// Transform Array (x, y, 0, 1) by matrix.
D3DXVECTOR4* WINAPI D3DXVec2TransformArray
    ( D3DXVECTOR4 *pOut, UINT OutStride, CONST D3DXVECTOR2 *pV, UINT VStride, CONST D3DXMATRIX *pM, UINT n);

// Transform Array (x, y, 0, 1) by matrix, project result back into w=1.
D3DXVECTOR2* WINAPI D3DXVec2TransformCoordArray
    ( D3DXVECTOR2 *pOut, UINT OutStride, CONST D3DXVECTOR2 *pV, UINT VStride, CONST D3DXMATRIX *pM, UINT n );

// Transform Array (x, y, 0, 0) by matrix.
D3DXVECTOR2* WINAPI D3DXVec2TransformNormalArray
    ( D3DXVECTOR2 *pOut, UINT OutStride, CONST D3DXVECTOR2 *pV, UINT VStride, CONST D3DXMATRIX *pM, UINT n );
    
    

#ifdef __cplusplus
}
#endif


//--------------------------
// 3D Vector
//--------------------------

// inline

FLOAT D3DXVec3Length
    ( CONST D3DXVECTOR3 *pV );

FLOAT D3DXVec3LengthSq
    ( CONST D3DXVECTOR3 *pV );

FLOAT D3DXVec3Dot
    ( CONST D3DXVECTOR3 *pV1, CONST D3DXVECTOR3 *pV2 );

D3DXVECTOR3* D3DXVec3Cross
    ( D3DXVECTOR3 *pOut, CONST D3DXVECTOR3 *pV1, CONST D3DXVECTOR3 *pV2 );

D3DXVECTOR3* D3DXVec3Add
    ( D3DXVECTOR3 *pOut, CONST D3DXVECTOR3 *pV1, CONST D3DXVECTOR3 *pV2 );

D3DXVECTOR3* D3DXVec3Subtract
    ( D3DXVECTOR3 *pOut, CONST D3DXVECTOR3 *pV1, CONST D3DXVECTOR3 *pV2 );

// Minimize each component.  x = min(x1, x2), y = min(y1, y2), ...
D3DXVECTOR3* D3DXVec3Minimize
    ( D3DXVECTOR3 *pOut, CONST D3DXVECTOR3 *pV1, CONST D3DXVECTOR3 *pV2 );

// Maximize each component.  x = max(x1, x2), y = max(y1, y2), ...
D3DXVECTOR3* D3DXVec3Maximize
    ( D3DXVECTOR3 *pOut, CONST D3DXVECTOR3 *pV1, CONST D3DXVECTOR3 *pV2 );

D3DXVECTOR3* D3DXVec3Scale
    ( D3DXVECTOR3 *pOut, CONST D3DXVECTOR3 *pV, FLOAT s);

// Linear interpolation. V1 + s(V2-V1)
D3DXVECTOR3* D3DXVec3Lerp
    ( D3DXVECTOR3 *pOut, CONST D3DXVECTOR3 *pV1, CONST D3DXVECTOR3 *pV2,
      FLOAT s );

// non-inline
#ifdef __cplusplus
extern "C" {
#endif

D3DXVECTOR3* WINAPI D3DXVec3Normalize
    ( D3DXVECTOR3 *pOut, CONST D3DXVECTOR3 *pV );

// Hermite interpolation between position V1, tangent T1 (when s == 0)
// and position V2, tangent T2 (when s == 1).
D3DXVECTOR3* WINAPI D3DXVec3Hermite
    ( D3DXVECTOR3 *pOut, CONST D3DXVECTOR3 *pV1, CONST D3DXVECTOR3 *pT1,
      CONST D3DXVECTOR3 *pV2, CONST D3DXVECTOR3 *pT2, FLOAT s );

// CatmullRom interpolation between V1 (when s == 0) and V2 (when s == 1)
D3DXVECTOR3* WINAPI D3DXVec3CatmullRom
    ( D3DXVECTOR3 *pOut, CONST D3DXVECTOR3 *pV0, CONST D3DXVECTOR3 *pV1,
      CONST D3DXVECTOR3 *pV2, CONST D3DXVECTOR3 *pV3, FLOAT s );

// Barycentric coordinates.  V1 + f(V2-V1) + g(V3-V1)
D3DXVECTOR3* WINAPI D3DXVec3BaryCentric
    ( D3DXVECTOR3 *pOut, CONST D3DXVECTOR3 *pV1, CONST D3DXVECTOR3 *pV2,
      CONST D3DXVECTOR3 *pV3, FLOAT f, FLOAT g);

// Transform (x, y, z, 1) by matrix.
D3DXVECTOR4* WINAPI D3DXVec3Transform
    ( D3DXVECTOR4 *pOut, CONST D3DXVECTOR3 *pV, CONST D3DXMATRIX *pM );

// Transform (x, y, z, 1) by matrix, project result back into w=1.
D3DXVECTOR3* WINAPI D3DXVec3TransformCoord
    ( D3DXVECTOR3 *pOut, CONST D3DXVECTOR3 *pV, CONST D3DXMATRIX *pM );

// Transform (x, y, z, 0) by matrix.  If you transforming a normal by a 
// non-affine matrix, the matrix you pass to this function should be the 
// transpose of the inverse of the matrix you would use to transform a coord.
D3DXVECTOR3* WINAPI D3DXVec3TransformNormal
    ( D3DXVECTOR3 *pOut, CONST D3DXVECTOR3 *pV, CONST D3DXMATRIX *pM );
    
    
// Transform Array (x, y, z, 1) by matrix. 
D3DXVECTOR4* WINAPI D3DXVec3TransformArray
    ( D3DXVECTOR4 *pOut, UINT OutStride, CONST D3DXVECTOR3 *pV, UINT VStride, CONST D3DXMATRIX *pM, UINT n );

// Transform Array (x, y, z, 1) by matrix, project result back into w=1.
D3DXVECTOR3* WINAPI D3DXVec3TransformCoordArray
    ( D3DXVECTOR3 *pOut, UINT OutStride, CONST D3DXVECTOR3 *pV, UINT VStride, CONST D3DXMATRIX *pM, UINT n );

// Transform (x, y, z, 0) by matrix.  If you transforming a normal by a 
// non-affine matrix, the matrix you pass to this function should be the 
// transpose of the inverse of the matrix you would use to transform a coord.
D3DXVECTOR3* WINAPI D3DXVec3TransformNormalArray
    ( D3DXVECTOR3 *pOut, UINT OutStride, CONST D3DXVECTOR3 *pV, UINT VStride, CONST D3DXMATRIX *pM, UINT n );

// Project vector from object space into screen space
D3DXVECTOR3* WINAPI D3DXVec3Project
    ( D3DXVECTOR3 *pOut, CONST D3DXVECTOR3 *pV, CONST D3DVIEWPORT9 *pViewport,
      CONST D3DXMATRIX *pProjection, CONST D3DXMATRIX *pView, CONST D3DXMATRIX *pWorld);

// Project vector from screen space into object space
D3DXVECTOR3* WINAPI D3DXVec3Unproject
    ( D3DXVECTOR3 *pOut, CONST D3DXVECTOR3 *pV, CONST D3DVIEWPORT9 *pViewport,
      CONST D3DXMATRIX *pProjection, CONST D3DXMATRIX *pView, CONST D3DXMATRIX *pWorld);
      
// Project vector Array from object space into screen space
D3DXVECTOR3* WINAPI D3DXVec3ProjectArray
    ( D3DXVECTOR3 *pOut, UINT OutStride,CONST D3DXVECTOR3 *pV, UINT VStride,CONST D3DVIEWPORT9 *pViewport,
      CONST D3DXMATRIX *pProjection, CONST D3DXMATRIX *pView, CONST D3DXMATRIX *pWorld, UINT n);

// Project vector Array from screen space into object space
D3DXVECTOR3* WINAPI D3DXVec3UnprojectArray
    ( D3DXVECTOR3 *pOut, UINT OutStride, CONST D3DXVECTOR3 *pV, UINT VStride, CONST D3DVIEWPORT9 *pViewport,
      CONST D3DXMATRIX *pProjection, CONST D3DXMATRIX *pView, CONST D3DXMATRIX *pWorld, UINT n);


#ifdef __cplusplus
}
#endif



//--------------------------
// 4D Vector
//--------------------------

// inline

FLOAT D3DXVec4Length
    ( CONST D3DXVECTOR4 *pV );

FLOAT D3DXVec4LengthSq
    ( CONST D3DXVECTOR4 *pV );

FLOAT D3DXVec4Dot
    ( CONST D3DXVECTOR4 *pV1, CONST D3DXVECTOR4 *pV2 );

D3DXVECTOR4* D3DXVec4Add
    ( D3DXVECTOR4 *pOut, CONST D3DXVECTOR4 *pV1, CONST D3DXVECTOR4 *pV2);

D3DXVECTOR4* D3DXVec4Subtract
    ( D3DXVECTOR4 *pOut, CONST D3DXVECTOR4 *pV1, CONST D3DXVECTOR4 *pV2);

// Minimize each component.  x = min(x1, x2), y = min(y1, y2), ...
D3DXVECTOR4* D3DXVec4Minimize
    ( D3DXVECTOR4 *pOut, CONST D3DXVECTOR4 *pV1, CONST D3DXVECTOR4 *pV2);

// Maximize each component.  x = max(x1, x2), y = max(y1, y2), ...
D3DXVECTOR4* D3DXVec4Maximize
    ( D3DXVECTOR4 *pOut, CONST D3DXVECTOR4 *pV1, CONST D3DXVECTOR4 *pV2);

D3DXVECTOR4* D3DXVec4Scale
    ( D3DXVECTOR4 *pOut, CONST D3DXVECTOR4 *pV, FLOAT s);

// Linear interpolation. V1 + s(V2-V1)
D3DXVECTOR4* D3DXVec4Lerp
    ( D3DXVECTOR4 *pOut, CONST D3DXVECTOR4 *pV1, CONST D3DXVECTOR4 *pV2,
      FLOAT s );

// non-inline
#ifdef __cplusplus
extern "C" {
#endif

// Cross-product in 4 dimensions.
D3DXVECTOR4* WINAPI D3DXVec4Cross
    ( D3DXVECTOR4 *pOut, CONST D3DXVECTOR4 *pV1, CONST D3DXVECTOR4 *pV2,
      CONST D3DXVECTOR4 *pV3);

D3DXVECTOR4* WINAPI D3DXVec4Normalize
    ( D3DXVECTOR4 *pOut, CONST D3DXVECTOR4 *pV );

// Hermite interpolation between position V1, tangent T1 (when s == 0)
// and position V2, tangent T2 (when s == 1).
D3DXVECTOR4* WINAPI D3DXVec4Hermite
    ( D3DXVECTOR4 *pOut, CONST D3DXVECTOR4 *pV1, CONST D3DXVECTOR4 *pT1,
      CONST D3DXVECTOR4 *pV2, CONST D3DXVECTOR4 *pT2, FLOAT s );

// CatmullRom interpolation between V1 (when s == 0) and V2 (when s == 1)
D3DXVECTOR4* WINAPI D3DXVec4CatmullRom
    ( D3DXVECTOR4 *pOut, CONST D3DXVECTOR4 *pV0, CONST D3DXVECTOR4 *pV1,
      CONST D3DXVECTOR4 *pV2, CONST D3DXVECTOR4 *pV3, FLOAT s );

// Barycentric coordinates.  V1 + f(V2-V1) + g(V3-V1)
D3DXVECTOR4* WINAPI D3DXVec4BaryCentric
    ( D3DXVECTOR4 *pOut, CONST D3DXVECTOR4 *pV1, CONST D3DXVECTOR4 *pV2,
      CONST D3DXVECTOR4 *pV3, FLOAT f, FLOAT g);

// Transform vector by matrix.
D3DXVECTOR4* WINAPI D3DXVec4Transform
    ( D3DXVECTOR4 *pOut, CONST D3DXVECTOR4 *pV, CONST D3DXMATRIX *pM );
    
// Transform vector array by matrix.
D3DXVECTOR4* WINAPI D3DXVec4TransformArray
    ( D3DXVECTOR4 *pOut, UINT OutStride, CONST D3DXVECTOR4 *pV, UINT VStride, CONST D3DXMATRIX *pM, UINT n );

#ifdef __cplusplus
}
#endif


//--------------------------
// 4D Matrix
//--------------------------

// inline

D3DXMATRIX* D3DXMatrixIdentity
    ( D3DXMATRIX *pOut );

BOOL D3DXMatrixIsIdentity
    ( CONST D3DXMATRIX *pM );


// non-inline
#ifdef __cplusplus
extern "C" {
#endif

FLOAT WINAPI D3DXMatrixDeterminant
    ( CONST D3DXMATRIX *pM );

HRESULT WINAPI D3DXMatrixDecompose
    ( D3DXVECTOR3 *pOutScale, D3DXQUATERNION *pOutRotation, 
	  D3DXVECTOR3 *pOutTranslation, CONST D3DXMATRIX *pM );

D3DXMATRIX* WINAPI D3DXMatrixTranspose
    ( D3DXMATRIX *pOut, CONST D3DXMATRIX *pM );

// Matrix multiplication.  The result represents the transformation M2
// followed by the transformation M1.  (Out = M1 * M2)
D3DXMATRIX* WINAPI D3DXMatrixMultiply
    ( D3DXMATRIX *pOut, CONST D3DXMATRIX *pM1, CONST D3DXMATRIX *pM2 );

// Matrix multiplication, followed by a transpose. (Out = T(M1 * M2))
D3DXMATRIX* WINAPI D3DXMatrixMultiplyTranspose
    ( D3DXMATRIX *pOut, CONST D3DXMATRIX *pM1, CONST D3DXMATRIX *pM2 );

// Calculate inverse of matrix.  Inversion my fail, in which case NULL will
// be returned.  The determinant of pM is also returned it pfDeterminant
// is non-NULL.
D3DXMATRIX* WINAPI D3DXMatrixInverse
    ( D3DXMATRIX *pOut, FLOAT *pDeterminant, CONST D3DXMATRIX *pM );

// Build a matrix which scales by (sx, sy, sz)
D3DXMATRIX* WINAPI D3DXMatrixScaling
    ( D3DXMATRIX *pOut, FLOAT sx, FLOAT sy, FLOAT sz );

// Build a matrix which translates by (x, y, z)
D3DXMATRIX* WINAPI D3DXMatrixTranslation
    ( D3DXMATRIX *pOut, FLOAT x, FLOAT y, FLOAT z );

// Build a matrix which rotates around the X axis
D3DXMATRIX* WINAPI D3DXMatrixRotationX
    ( D3DXMATRIX *pOut, FLOAT Angle );

// Build a matrix which rotates around the Y axis
D3DXMATRIX* WINAPI D3DXMatrixRotationY
    ( D3DXMATRIX *pOut, FLOAT Angle );

// Build a matrix which rotates around the Z axis
D3DXMATRIX* WINAPI D3DXMatrixRotationZ
    ( D3DXMATRIX *pOut, FLOAT Angle );

// Build a matrix which rotates around an arbitrary axis
D3DXMATRIX* WINAPI D3DXMatrixRotationAxis
    ( D3DXMATRIX *pOut, CONST D3DXVECTOR3 *pV, FLOAT Angle );

// Build a matrix from a quaternion
D3DXMATRIX* WINAPI D3DXMatrixRotationQuaternion
    ( D3DXMATRIX *pOut, CONST D3DXQUATERNION *pQ);

// Yaw around the Y axis, a pitch around the X axis,
// and a roll around the Z axis.
D3DXMATRIX* WINAPI D3DXMatrixRotationYawPitchRoll
    ( D3DXMATRIX *pOut, FLOAT Yaw, FLOAT Pitch, FLOAT Roll );

// Build transformation matrix.  NULL arguments are treated as identity.
// Mout = Msc-1 * Msr-1 * Ms * Msr * Msc * Mrc-1 * Mr * Mrc * Mt
D3DXMATRIX* WINAPI D3DXMatrixTransformation
    ( D3DXMATRIX *pOut, CONST D3DXVECTOR3 *pScalingCenter,
      CONST D3DXQUATERNION *pScalingRotation, CONST D3DXVECTOR3 *pScaling,
      CONST D3DXVECTOR3 *pRotationCenter, CONST D3DXQUATERNION *pRotation,
      CONST D3DXVECTOR3 *pTranslation);

// Build 2D transformation matrix in XY plane.  NULL arguments are treated as identity.
// Mout = Msc-1 * Msr-1 * Ms * Msr * Msc * Mrc-1 * Mr * Mrc * Mt
D3DXMATRIX* WINAPI D3DXMatrixTransformation2D
    ( D3DXMATRIX *pOut, CONST D3DXVECTOR2* pScalingCenter, 
      FLOAT ScalingRotation, CONST D3DXVECTOR2* pScaling, 
      CONST D3DXVECTOR2* pRotationCenter, FLOAT Rotation, 
      CONST D3DXVECTOR2* pTranslation);

// Build affine transformation matrix.  NULL arguments are treated as identity.
// Mout = Ms * Mrc-1 * Mr * Mrc * Mt
D3DXMATRIX* WINAPI D3DXMatrixAffineTransformation
    ( D3DXMATRIX *pOut, FLOAT Scaling, CONST D3DXVECTOR3 *pRotationCenter,
      CONST D3DXQUATERNION *pRotation, CONST D3DXVECTOR3 *pTranslation);

// Build 2D affine transformation matrix in XY plane.  NULL arguments are treated as identity.
// Mout = Ms * Mrc-1 * Mr * Mrc * Mt
D3DXMATRIX* WINAPI D3DXMatrixAffineTransformation2D
    ( D3DXMATRIX *pOut, FLOAT Scaling, CONST D3DXVECTOR2* pRotationCenter, 
      FLOAT Rotation, CONST D3DXVECTOR2* pTranslation);

// Build a lookat matrix. (right-handed)
D3DXMATRIX* WINAPI D3DXMatrixLookAtRH
    ( D3DXMATRIX *pOut, CONST D3DXVECTOR3 *pEye, CONST D3DXVECTOR3 *pAt,
      CONST D3DXVECTOR3 *pUp );

// Build a lookat matrix. (left-handed)
D3DXMATRIX* WINAPI D3DXMatrixLookAtLH
    ( D3DXMATRIX *pOut, CONST D3DXVECTOR3 *pEye, CONST D3DXVECTOR3 *pAt,
      CONST D3DXVECTOR3 *pUp );

// Build a perspective projection matrix. (right-handed)
D3DXMATRIX* WINAPI D3DXMatrixPerspectiveRH
    ( D3DXMATRIX *pOut, FLOAT w, FLOAT h, FLOAT zn, FLOAT zf );

// Build a perspective projection matrix. (left-handed)
D3DXMATRIX* WINAPI D3DXMatrixPerspectiveLH
    ( D3DXMATRIX *pOut, FLOAT w, FLOAT h, FLOAT zn, FLOAT zf );

// Build a perspective projection matrix. (right-handed)
D3DXMATRIX* WINAPI D3DXMatrixPerspectiveFovRH
    ( D3DXMATRIX *pOut, FLOAT fovy, FLOAT Aspect, FLOAT zn, FLOAT zf );

// Build a perspective projection matrix. (left-handed)
D3DXMATRIX* WINAPI D3DXMatrixPerspectiveFovLH
    ( D3DXMATRIX *pOut, FLOAT fovy, FLOAT Aspect, FLOAT zn, FLOAT zf );

// Build a perspective projection matrix. (right-handed)
D3DXMATRIX* WINAPI D3DXMatrixPerspectiveOffCenterRH
    ( D3DXMATRIX *pOut, FLOAT l, FLOAT r, FLOAT b, FLOAT t, FLOAT zn,
      FLOAT zf );

// Build a perspective projection matrix. (left-handed)
D3DXMATRIX* WINAPI D3DXMatrixPerspectiveOffCenterLH
    ( D3DXMATRIX *pOut, FLOAT l, FLOAT r, FLOAT b, FLOAT t, FLOAT zn,
      FLOAT zf );

// Build an ortho projection matrix. (right-handed)
D3DXMATRIX* WINAPI D3DXMatrixOrthoRH
    ( D3DXMATRIX *pOut, FLOAT w, FLOAT h, FLOAT zn, FLOAT zf );

// Build an ortho projection matrix. (left-handed)
D3DXMATRIX* WINAPI D3DXMatrixOrthoLH
    ( D3DXMATRIX *pOut, FLOAT w, FLOAT h, FLOAT zn, FLOAT zf );

// Build an ortho projection matrix. (right-handed)
D3DXMATRIX* WINAPI D3DXMatrixOrthoOffCenterRH
    ( D3DXMATRIX *pOut, FLOAT l, FLOAT r, FLOAT b, FLOAT t, FLOAT zn,
      FLOAT zf );

// Build an ortho projection matrix. (left-handed)
D3DXMATRIX* WINAPI D3DXMatrixOrthoOffCenterLH
    ( D3DXMATRIX *pOut, FLOAT l, FLOAT r, FLOAT b, FLOAT t, FLOAT zn,
      FLOAT zf );

// Build a matrix which flattens geometry into a plane, as if casting
// a shadow from a light.
D3DXMATRIX* WINAPI D3DXMatrixShadow
    ( D3DXMATRIX *pOut, CONST D3DXVECTOR4 *pLight,
      CONST D3DXPLANE *pPlane );

// Build a matrix which reflects the coordinate system about a plane
D3DXMATRIX* WINAPI D3DXMatrixReflect
    ( D3DXMATRIX *pOut, CONST D3DXPLANE *pPlane );

#ifdef __cplusplus
}
#endif


//--------------------------
// Quaternion
//--------------------------

// inline

FLOAT D3DXQuaternionLength
    ( CONST D3DXQUATERNION *pQ );

// Length squared, or "norm"
FLOAT D3DXQuaternionLengthSq
    ( CONST D3DXQUATERNION *pQ );

FLOAT D3DXQuaternionDot
    ( CONST D3DXQUATERNION *pQ1, CONST D3DXQUATERNION *pQ2 );

// (0, 0, 0, 1)
D3DXQUATERNION* D3DXQuaternionIdentity
    ( D3DXQUATERNION *pOut );

BOOL D3DXQuaternionIsIdentity
    ( CONST D3DXQUATERNION *pQ );

// (-x, -y, -z, w)
D3DXQUATERNION* D3DXQuaternionConjugate
    ( D3DXQUATERNION *pOut, CONST D3DXQUATERNION *pQ );


// non-inline
#ifdef __cplusplus
extern "C" {
#endif

// Compute a quaternin's axis and angle of rotation. Expects unit quaternions.
void WINAPI D3DXQuaternionToAxisAngle
    ( CONST D3DXQUATERNION *pQ, D3DXVECTOR3 *pAxis, FLOAT *pAngle );

// Build a quaternion from a rotation matrix.
D3DXQUATERNION* WINAPI D3DXQuaternionRotationMatrix
    ( D3DXQUATERNION *pOut, CONST D3DXMATRIX *pM);

// Rotation about arbitrary axis.
D3DXQUATERNION* WINAPI D3DXQuaternionRotationAxis
    ( D3DXQUATERNION *pOut, CONST D3DXVECTOR3 *pV, FLOAT Angle );

// Yaw around the Y axis, a pitch around the X axis,
// and a roll around the Z axis.
D3DXQUATERNION* WINAPI D3DXQuaternionRotationYawPitchRoll
    ( D3DXQUATERNION *pOut, FLOAT Yaw, FLOAT Pitch, FLOAT Roll );

// Quaternion multiplication.  The result represents the rotation Q2
// followed by the rotation Q1.  (Out = Q2 * Q1)
D3DXQUATERNION* WINAPI D3DXQuaternionMultiply
    ( D3DXQUATERNION *pOut, CONST D3DXQUATERNION *pQ1,
      CONST D3DXQUATERNION *pQ2 );

D3DXQUATERNION* WINAPI D3DXQuaternionNormalize
    ( D3DXQUATERNION *pOut, CONST D3DXQUATERNION *pQ );

// Conjugate and re-norm
D3DXQUATERNION* WINAPI D3DXQuaternionInverse
    ( D3DXQUATERNION *pOut, CONST D3DXQUATERNION *pQ );

// Expects unit quaternions.
// if q = (cos(theta), sin(theta) * v); ln(q) = (0, theta * v)
D3DXQUATERNION* WINAPI D3DXQuaternionLn
    ( D3DXQUATERNION *pOut, CONST D3DXQUATERNION *pQ );

// Expects pure quaternions. (w == 0)  w is ignored in calculation.
// if q = (0, theta * v); exp(q) = (cos(theta), sin(theta) * v)
D3DXQUATERNION* WINAPI D3DXQuaternionExp
    ( D3DXQUATERNION *pOut, CONST D3DXQUATERNION *pQ );
      
// Spherical linear interpolation between Q1 (t == 0) and Q2 (t == 1).
// Expects unit quaternions.
D3DXQUATERNION* WINAPI D3DXQuaternionSlerp
    ( D3DXQUATERNION *pOut, CONST D3DXQUATERNION *pQ1,
      CONST D3DXQUATERNION *pQ2, FLOAT t );

// Spherical quadrangle interpolation.
// Slerp(Slerp(Q1, C, t), Slerp(A, B, t), 2t(1-t))
D3DXQUATERNION* WINAPI D3DXQuaternionSquad
    ( D3DXQUATERNION *pOut, CONST D3DXQUATERNION *pQ1,
      CONST D3DXQUATERNION *pA, CONST D3DXQUATERNION *pB,
      CONST D3DXQUATERNION *pC, FLOAT t );

// Setup control points for spherical quadrangle interpolation
// from Q1 to Q2.  The control points are chosen in such a way 
// to ensure the continuity of tangents with adjacent segments.
void WINAPI D3DXQuaternionSquadSetup
    ( D3DXQUATERNION *pAOut, D3DXQUATERNION *pBOut, D3DXQUATERNION *pCOut,
      CONST D3DXQUATERNION *pQ0, CONST D3DXQUATERNION *pQ1, 
      CONST D3DXQUATERNION *pQ2, CONST D3DXQUATERNION *pQ3 );

// Barycentric interpolation.
// Slerp(Slerp(Q1, Q2, f+g), Slerp(Q1, Q3, f+g), g/(f+g))
D3DXQUATERNION* WINAPI D3DXQuaternionBaryCentric
    ( D3DXQUATERNION *pOut, CONST D3DXQUATERNION *pQ1,
      CONST D3DXQUATERNION *pQ2, CONST D3DXQUATERNION *pQ3,
      FLOAT f, FLOAT g );

#ifdef __cplusplus
}
#endif


//--------------------------
// Plane
//--------------------------

// inline

// ax + by + cz + dw
FLOAT D3DXPlaneDot
    ( CONST D3DXPLANE *pP, CONST D3DXVECTOR4 *pV);

// ax + by + cz + d
FLOAT D3DXPlaneDotCoord
    ( CONST D3DXPLANE *pP, CONST D3DXVECTOR3 *pV);

// ax + by + cz
FLOAT D3DXPlaneDotNormal
    ( CONST D3DXPLANE *pP, CONST D3DXVECTOR3 *pV);

D3DXPLANE* D3DXPlaneScale
    (D3DXPLANE *pOut, CONST D3DXPLANE *pP, FLOAT s);

// non-inline
#ifdef __cplusplus
extern "C" {
#endif

// Normalize plane (so that |a,b,c| == 1)
D3DXPLANE* WINAPI D3DXPlaneNormalize
    ( D3DXPLANE *pOut, CONST D3DXPLANE *pP);

// Find the intersection between a plane and a line.  If the line is
// parallel to the plane, NULL is returned.
D3DXVECTOR3* WINAPI D3DXPlaneIntersectLine
    ( D3DXVECTOR3 *pOut, CONST D3DXPLANE *pP, CONST D3DXVECTOR3 *pV1,
      CONST D3DXVECTOR3 *pV2);

// Construct a plane from a point and a normal
D3DXPLANE* WINAPI D3DXPlaneFromPointNormal
    ( D3DXPLANE *pOut, CONST D3DXVECTOR3 *pPoint, CONST D3DXVECTOR3 *pNormal);

// Construct a plane from 3 points
D3DXPLANE* WINAPI D3DXPlaneFromPoints
    ( D3DXPLANE *pOut, CONST D3DXVECTOR3 *pV1, CONST D3DXVECTOR3 *pV2,
      CONST D3DXVECTOR3 *pV3);

// Transform a plane by a matrix.  The vector (a,b,c) must be normal.
// M should be the inverse transpose of the transformation desired.
D3DXPLANE* WINAPI D3DXPlaneTransform
    ( D3DXPLANE *pOut, CONST D3DXPLANE *pP, CONST D3DXMATRIX *pM );
    
// Transform an array of planes by a matrix.  The vectors (a,b,c) must be normal.
// M should be the inverse transpose of the transformation desired.
D3DXPLANE* WINAPI D3DXPlaneTransformArray
    ( D3DXPLANE *pOut, UINT OutStride, CONST D3DXPLANE *pP, UINT PStride, CONST D3DXMATRIX *pM, UINT n );

#ifdef __cplusplus
}
#endif


//--------------------------
// Color
//--------------------------

// inline

// (1-r, 1-g, 1-b, a)
D3DXCOLOR* D3DXColorNegative
    (D3DXCOLOR *pOut, CONST D3DXCOLOR *pC);

D3DXCOLOR* D3DXColorAdd
    (D3DXCOLOR *pOut, CONST D3DXCOLOR *pC1, CONST D3DXCOLOR *pC2);

D3DXCOLOR* D3DXColorSubtract
    (D3DXCOLOR *pOut, CONST D3DXCOLOR *pC1, CONST D3DXCOLOR *pC2);

D3DXCOLOR* D3DXColorScale
    (D3DXCOLOR *pOut, CONST D3DXCOLOR *pC, FLOAT s);

// (r1*r2, g1*g2, b1*b2, a1*a2)
D3DXCOLOR* D3DXColorModulate
    (D3DXCOLOR *pOut, CONST D3DXCOLOR *pC1, CONST D3DXCOLOR *pC2);

// Linear interpolation of r,g,b, and a. C1 + s(C2-C1)
D3DXCOLOR* D3DXColorLerp
    (D3DXCOLOR *pOut, CONST D3DXCOLOR *pC1, CONST D3DXCOLOR *pC2, FLOAT s);

// non-inline
#ifdef __cplusplus
extern "C" {
#endif

// Interpolate r,g,b between desaturated color and color.
// DesaturatedColor + s(Color - DesaturatedColor)
D3DXCOLOR* WINAPI D3DXColorAdjustSaturation
    (D3DXCOLOR *pOut, CONST D3DXCOLOR *pC, FLOAT s);

// Interpolate r,g,b between 50% grey and color.  Grey + s(Color - Grey)
D3DXCOLOR* WINAPI D3DXColorAdjustContrast
    (D3DXCOLOR *pOut, CONST D3DXCOLOR *pC, FLOAT c);

#ifdef __cplusplus
}
#endif




//--------------------------
// Misc
//--------------------------

#ifdef __cplusplus
extern "C" {
#endif

// Calculate Fresnel term given the cosine of theta (likely obtained by
// taking the dot of two normals), and the refraction index of the material.
FLOAT WINAPI D3DXFresnelTerm
    (FLOAT CosTheta, FLOAT RefractionIndex);     

#ifdef __cplusplus
}
#endif



//===========================================================================
//
//    Matrix Stack
//
//===========================================================================

typedef interface ID3DXMatrixStack ID3DXMatrixStack;
typedef interface ID3DXMatrixStack *LPD3DXMATRIXSTACK;

// {C7885BA7-F990-4fe7-922D-8515E477DD85}
DEFINE_GUID(IID_ID3DXMatrixStack, 
0xc7885ba7, 0xf990, 0x4fe7, 0x92, 0x2d, 0x85, 0x15, 0xe4, 0x77, 0xdd, 0x85);


#undef INTERFACE
#define INTERFACE ID3DXMatrixStack

DECLARE_INTERFACE_(ID3DXMatrixStack, IUnknown)
{
    //
    // IUnknown methods
    //
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

    //
    // ID3DXMatrixStack methods
    //

    // Pops the top of the stack, returns the current top
    // *after* popping the top.
    STDMETHOD(Pop)(THIS) PURE;

    // Pushes the stack by one, duplicating the current matrix.
    STDMETHOD(Push)(THIS) PURE;

    // Loads identity in the current matrix.
    STDMETHOD(LoadIdentity)(THIS) PURE;

    // Loads the given matrix into the current matrix
    STDMETHOD(LoadMatrix)(THIS_ CONST D3DXMATRIX* pM ) PURE;

    // Right-Multiplies the given matrix to the current matrix.
    // (transformation is about the current world origin)
    STDMETHOD(MultMatrix)(THIS_ CONST D3DXMATRIX* pM ) PURE;

    // Left-Multiplies the given matrix to the current matrix
    // (transformation is about the local origin of the object)
    STDMETHOD(MultMatrixLocal)(THIS_ CONST D3DXMATRIX* pM ) PURE;

    // Right multiply the current matrix with the computed rotation
    // matrix, counterclockwise about the given axis with the given angle.
    // (rotation is about the current world origin)
    STDMETHOD(RotateAxis)
        (THIS_ CONST D3DXVECTOR3* pV, FLOAT Angle) PURE;

    // Left multiply the current matrix with the computed rotation
    // matrix, counterclockwise about the given axis with the given angle.
    // (rotation is about the local origin of the object)
    STDMETHOD(RotateAxisLocal)
        (THIS_ CONST D3DXVECTOR3* pV, FLOAT Angle) PURE;

    // Right multiply the current matrix with the computed rotation
    // matrix. All angles are counterclockwise. (rotation is about the
    // current world origin)

    // The rotation is composed of a yaw around the Y axis, a pitch around
    // the X axis, and a roll around the Z axis.
    STDMETHOD(RotateYawPitchRoll)
        (THIS_ FLOAT Yaw, FLOAT Pitch, FLOAT Roll) PURE;

    // Left multiply the current matrix with the computed rotation
    // matrix. All angles are counterclockwise. (rotation is about the
    // local origin of the object)

    // The rotation is composed of a yaw around the Y axis, a pitch around
    // the X axis, and a roll around the Z axis.
    STDMETHOD(RotateYawPitchRollLocal)
        (THIS_ FLOAT Yaw, FLOAT Pitch, FLOAT Roll) PURE;

    // Right multiply the current matrix with the computed scale
    // matrix. (transformation is about the current world origin)
    STDMETHOD(Scale)(THIS_ FLOAT x, FLOAT y, FLOAT z) PURE;

    // Left multiply the current matrix with the computed scale
    // matrix. (transformation is about the local origin of the object)
    STDMETHOD(ScaleLocal)(THIS_ FLOAT x, FLOAT y, FLOAT z) PURE;

    // Right multiply the current matrix with the computed translation
    // matrix. (transformation is about the current world origin)
    STDMETHOD(Translate)(THIS_ FLOAT x, FLOAT y, FLOAT z ) PURE;

    // Left multiply the current matrix with the computed translation
    // matrix. (transformation is about the local origin of the object)
    STDMETHOD(TranslateLocal)(THIS_ FLOAT x, FLOAT y, FLOAT z) PURE;

    // Obtain the current matrix at the top of the stack
    STDMETHOD_(D3DXMATRIX*, GetTop)(THIS) PURE;
};

#ifdef __cplusplus
extern "C" {
#endif

HRESULT WINAPI 
    D3DXCreateMatrixStack( 
        DWORD               Flags, 
        LPD3DXMATRIXSTACK*  ppStack);

#ifdef __cplusplus
}
#endif

//===========================================================================
//
//  Spherical Harmonic Runtime Routines
//
// NOTE:
//  * Most of these functions can take the same object as in and out parameters.
//    The exceptions are the rotation functions.  
//
//  * Out parameters are typically also returned as return values, so that
//    the output of one function may be used as a parameter to another.
//
//============================================================================


// non-inline
#ifdef __cplusplus
extern "C" {
#endif

//============================================================================
//
//  Basic Spherical Harmonic math routines
//
//============================================================================

#define D3DXSH_MINORDER 2
#define D3DXSH_MAXORDER 6

//============================================================================
//
//  D3DXSHEvalDirection:
//  --------------------
//  Evaluates the Spherical Harmonic basis functions
//
//  Parameters:
//   pOut
//      Output SH coefficients - basis function Ylm is stored at l*l + m+l
//      This is the pointer that is returned.
//   Order
//      Order of the SH evaluation, generates Order^2 coefs, degree is Order-1
//   pDir
//      Direction to evaluate in - assumed to be normalized
//
//============================================================================

FLOAT* WINAPI D3DXSHEvalDirection
    (  FLOAT *pOut, UINT Order, CONST D3DXVECTOR3 *pDir );
    
//============================================================================
//
//  D3DXSHRotate:
//  --------------------
//  Rotates SH vector by a rotation matrix
//
//  Parameters:
//   pOut
//      Output SH coefficients - basis function Ylm is stored at l*l + m+l
//      This is the pointer that is returned (should not alias with pIn.)
//   Order
//      Order of the SH evaluation, generates Order^2 coefs, degree is Order-1
//   pMatrix
//      Matrix used for rotation - rotation sub matrix should be orthogonal
//      and have a unit determinant.
//   pIn
//      Input SH coeffs (rotated), incorect results if this is also output.
//
//============================================================================

FLOAT* WINAPI D3DXSHRotate
    ( FLOAT *pOut, UINT Order, CONST D3DXMATRIX *pMatrix, CONST FLOAT *pIn );
    
//============================================================================
//
//  D3DXSHRotateZ:
//  --------------------
//  Rotates the SH vector in the Z axis by an angle
//
//  Parameters:
//   pOut
//      Output SH coefficients - basis function Ylm is stored at l*l + m+l
//      This is the pointer that is returned (should not alias with pIn.)
//   Order
//      Order of the SH evaluation, generates Order^2 coefs, degree is Order-1
//   Angle
//      Angle in radians to rotate around the Z axis.
//   pIn
//      Input SH coeffs (rotated), incorect results if this is also output.
//
//============================================================================


FLOAT* WINAPI D3DXSHRotateZ
    ( FLOAT *pOut, UINT Order, FLOAT Angle, CONST FLOAT *pIn );
    
//============================================================================
//
//  D3DXSHAdd:
//  --------------------
//  Adds two SH vectors, pOut[i] = pA[i] + pB[i];
//
//  Parameters:
//   pOut
//      Output SH coefficients - basis function Ylm is stored at l*l + m+l
//      This is the pointer that is returned.
//   Order
//      Order of the SH evaluation, generates Order^2 coefs, degree is Order-1
//   pA
//      Input SH coeffs.
//   pB
//      Input SH coeffs (second vector.)
//
//============================================================================

FLOAT* WINAPI D3DXSHAdd
    ( FLOAT *pOut, UINT Order, CONST FLOAT *pA, CONST FLOAT *pB );

//============================================================================
//
//  D3DXSHScale:
//  --------------------
//  Adds two SH vectors, pOut[i] = pA[i]*Scale;
//
//  Parameters:
//   pOut
//      Output SH coefficients - basis function Ylm is stored at l*l + m+l
//      This is the pointer that is returned.
//   Order
//      Order of the SH evaluation, generates Order^2 coefs, degree is Order-1
//   pIn
//      Input SH coeffs.
//   Scale
//      Scale factor.
//
//============================================================================

FLOAT* WINAPI D3DXSHScale
    ( FLOAT *pOut, UINT Order, CONST FLOAT *pIn, CONST FLOAT Scale );
    
//============================================================================
//
//  D3DXSHDot:
//  --------------------
//  Computes the dot product of two SH vectors
//
//  Parameters:
//   Order
//      Order of the SH evaluation, generates Order^2 coefs, degree is Order-1
//   pA
//      Input SH coeffs.
//   pB
//      Second set of input SH coeffs.
//
//============================================================================

FLOAT WINAPI D3DXSHDot
    ( UINT Order, CONST FLOAT *pA, CONST FLOAT *pB );

//============================================================================
//
//  D3DXSHMultiply[O]:
//  --------------------
//  Computes the product of two functions represented using SH (f and g), where:
//  pOut[i] = int(y_i(s) * f(s) * g(s)), where y_i(s) is the ith SH basis
//  function, f(s) and g(s) are SH functions (sum_i(y_i(s)*c_i)).  The order O
//  determines the lengths of the arrays, where there should always be O^2 
//  coefficients.  In general the product of two SH functions of order O generates
//  and SH function of order 2*O - 1, but we truncate the result.  This means
//  that the product commutes (f*g == g*f) but doesn't associate 
//  (f*(g*h) != (f*g)*h.
//
//  Parameters:
//   pOut
//      Output SH coefficients - basis function Ylm is stored at l*l + m+l
//      This is the pointer that is returned.
//   pF
//      Input SH coeffs for first function.
//   pG
//      Second set of input SH coeffs.
//
//============================================================================

FLOAT* WINAPI D3DXSHMultiply2( FLOAT *pOut, CONST FLOAT *pF, CONST FLOAT *pG);
FLOAT* WINAPI D3DXSHMultiply3( FLOAT *pOut, CONST FLOAT *pF, CONST FLOAT *pG);
FLOAT* WINAPI D3DXSHMultiply4( FLOAT *pOut, CONST FLOAT *pF, CONST FLOAT *pG);
FLOAT* WINAPI D3DXSHMultiply5( FLOAT *pOut, CONST FLOAT *pF, CONST FLOAT *pG);
FLOAT* WINAPI D3DXSHMultiply6( FLOAT *pOut, CONST FLOAT *pF, CONST FLOAT *pG);


//============================================================================
//
//  Basic Spherical Harmonic lighting routines
//
//============================================================================

//============================================================================
//
//  D3DXSHEvalDirectionalLight:
//  --------------------
//  Evaluates a directional light and returns spectral SH data.  The output 
//  vector is computed so that if the intensity of R/G/B is unit the resulting
//  exit radiance of a point directly under the light on a diffuse object with
//  an albedo of 1 would be 1.0.  This will compute 3 spectral samples, pROut
//  has to be specified, while pGout and pBout are optional.
//
//  Parameters:
//   Order
//      Order of the SH evaluation, generates Order^2 coefs, degree is Order-1
//   pDir
//      Direction light is coming from (assumed to be normalized.)
//   RIntensity
//      Red intensity of light.
//   GIntensity
//      Green intensity of light.
//   BIntensity
//      Blue intensity of light.
//   pROut
//      Output SH vector for Red.
//   pGOut
//      Output SH vector for Green (optional.)
//   pBOut
//      Output SH vector for Blue (optional.)        
//
//============================================================================

HRESULT WINAPI D3DXSHEvalDirectionalLight
    ( UINT Order, CONST D3DXVECTOR3 *pDir, 
      FLOAT RIntensity, FLOAT GIntensity, FLOAT BIntensity,
      FLOAT *pROut, FLOAT *pGOut, FLOAT *pBOut );

//============================================================================
//
//  D3DXSHEvalSphericalLight:
//  --------------------
//  Evaluates a spherical light and returns spectral SH data.  There is no 
//  normalization of the intensity of the light like there is for directional
//  lights, care has to be taken when specifiying the intensities.  This will 
//  compute 3 spectral samples, pROut has to be specified, while pGout and 
//  pBout are optional.
//
//  Parameters:
//   Order
//      Order of the SH evaluation, generates Order^2 coefs, degree is Order-1
//   pPos
//      Position of light - reciever is assumed to be at the origin.
//   Radius
//      Radius of the spherical light source.
//   RIntensity
//      Red intensity of light.
//   GIntensity
//      Green intensity of light.
//   BIntensity
//      Blue intensity of light.
//   pROut
//      Output SH vector for Red.
//   pGOut
//      Output SH vector for Green (optional.)
//   pBOut
//      Output SH vector for Blue (optional.)        
//
//============================================================================

HRESULT WINAPI D3DXSHEvalSphericalLight
    ( UINT Order, CONST D3DXVECTOR3 *pPos, FLOAT Radius,
      FLOAT RIntensity, FLOAT GIntensity, FLOAT BIntensity,
      FLOAT *pROut, FLOAT *pGOut, FLOAT *pBOut );

//============================================================================
//
//  D3DXSHEvalConeLight:
//  --------------------
//  Evaluates a light that is a cone of constant intensity and returns spectral
//  SH data.  The output vector is computed so that if the intensity of R/G/B is
//  unit the resulting exit radiance of a point directly under the light oriented
//  in the cone direction on a diffuse object with an albedo of 1 would be 1.0.
//  This will compute 3 spectral samples, pROut has to be specified, while pGout
//  and pBout are optional.
//
//  Parameters:
//   Order
//      Order of the SH evaluation, generates Order^2 coefs, degree is Order-1
//   pDir
//      Direction light is coming from (assumed to be normalized.)
//   Radius
//      Radius of cone in radians.
//   RIntensity
//      Red intensity of light.
//   GIntensity
//      Green intensity of light.
//   BIntensity
//      Blue intensity of light.
//   pROut
//      Output SH vector for Red.
//   pGOut
//      Output SH vector for Green (optional.)
//   pBOut
//      Output SH vector for Blue (optional.)        
//
//============================================================================

HRESULT WINAPI D3DXSHEvalConeLight
    ( UINT Order, CONST D3DXVECTOR3 *pDir, FLOAT Radius,
      FLOAT RIntensity, FLOAT GIntensity, FLOAT BIntensity,
      FLOAT *pROut, FLOAT *pGOut, FLOAT *pBOut );
      
//============================================================================
//
//  D3DXSHEvalHemisphereLight:
//  --------------------
//  Evaluates a light that is a linear interpolant between two colors over the
//  sphere.  The interpolant is linear along the axis of the two points, not
//  over the surface of the sphere (ie: if the axis was (0,0,1) it is linear in
//  Z, not in the azimuthal angle.)  The resulting spherical lighting function
//  is normalized so that a point on a perfectly diffuse surface with no
//  shadowing and a normal pointed in the direction pDir would result in exit
//  radiance with a value of 1 if the top color was white and the bottom color
//  was black.  This is a very simple model where Top represents the intensity 
//  of the "sky" and Bottom represents the intensity of the "ground".
//
//  Parameters:
//   Order
//      Order of the SH evaluation, generates Order^2 coefs, degree is Order-1
//   pDir
//      Axis of the hemisphere.
//   Top
//      Color of the upper hemisphere.
//   Bottom
//      Color of the lower hemisphere.
//   pROut
//      Output SH vector for Red.
//   pGOut
//      Output SH vector for Green
//   pBOut
//      Output SH vector for Blue        
//
//============================================================================

HRESULT WINAPI D3DXSHEvalHemisphereLight
    ( UINT Order, CONST D3DXVECTOR3 *pDir, D3DXCOLOR Top, D3DXCOLOR Bottom,
      FLOAT *pROut, FLOAT *pGOut, FLOAT *pBOut );

//============================================================================
//
//  Basic Spherical Harmonic projection routines
//
//============================================================================

//============================================================================
//
//  D3DXSHProjectCubeMap:
//  --------------------
//  Projects a function represented on a cube map into spherical harmonics.
//
//  Parameters:
//   Order
//      Order of the SH evaluation, generates Order^2 coefs, degree is Order-1
//   pCubeMap
//      CubeMap that is going to be projected into spherical harmonics
//   pROut
//      Output SH vector for Red.
//   pGOut
//      Output SH vector for Green
//   pBOut
//      Output SH vector for Blue        
//
//============================================================================

HRESULT WINAPI D3DXSHProjectCubeMap
    ( UINT uOrder, LPDIRECT3DCUBETEXTURE9 pCubeMap,
      FLOAT *pROut, FLOAT *pGOut, FLOAT *pBOut );


#ifdef __cplusplus
}
#endif


#include "d3dx9math.inl"

//#if _MSC_VER >= 1200
//#pragma warning(pop)
//#else
//#pragma warning(default:4201)
//#endif

#endif // __D3DX9MATH_H__

