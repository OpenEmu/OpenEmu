///////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
//  File:       d3dx9xof.h
//  Content:    D3DX .X File types and functions
//
///////////////////////////////////////////////////////////////////////////

#include "d3dx9.h"

#if !defined( __D3DX9XOF_H__ )
#define __D3DX9XOF_H__

#if defined( __cplusplus )
extern "C" {
#endif // defined( __cplusplus )

//----------------------------------------------------------------------------
// D3DXF_FILEFORMAT
//   This flag is used to specify what file type to use when saving to disk.
//   _BINARY, and _TEXT are mutually exclusive, while
//   _COMPRESSED is an optional setting that works with all file types.
//----------------------------------------------------------------------------
typedef DWORD D3DXF_FILEFORMAT;

#define D3DXF_FILEFORMAT_BINARY          0
#define D3DXF_FILEFORMAT_TEXT            1
#define D3DXF_FILEFORMAT_COMPRESSED      2

//----------------------------------------------------------------------------
// D3DXF_FILESAVEOPTIONS
//   This flag is used to specify where to save the file to. Each flag is
//   mutually exclusive, indicates the data location of the file, and also
//   chooses which additional data will specify the location.
//   _TOFILE is paired with a filename (LPCSTR)
//   _TOWFILE is paired with a filename (LPWSTR)
//----------------------------------------------------------------------------
typedef DWORD D3DXF_FILESAVEOPTIONS;

#define D3DXF_FILESAVE_TOFILE     0x00L
#define D3DXF_FILESAVE_TOWFILE    0x01L

//----------------------------------------------------------------------------
// D3DXF_FILELOADOPTIONS
//   This flag is used to specify where to load the file from. Each flag is
//   mutually exclusive, indicates the data location of the file, and also
//   chooses which additional data will specify the location.
//   _FROMFILE is paired with a filename (LPCSTR)
//   _FROMWFILE is paired with a filename (LPWSTR)
//   _FROMRESOURCE is paired with a (D3DXF_FILELOADRESOUCE*) description.
//   _FROMMEMORY is paired with a (D3DXF_FILELOADMEMORY*) description.
//----------------------------------------------------------------------------
typedef DWORD D3DXF_FILELOADOPTIONS;

#define D3DXF_FILELOAD_FROMFILE     0x00L
#define D3DXF_FILELOAD_FROMWFILE    0x01L
#define D3DXF_FILELOAD_FROMRESOURCE 0x02L
#define D3DXF_FILELOAD_FROMMEMORY   0x03L

//----------------------------------------------------------------------------
// D3DXF_FILELOADRESOURCE:
//----------------------------------------------------------------------------

typedef struct _D3DXF_FILELOADRESOURCE
{
    HMODULE hModule; // Desc
    LPCSTR lpName;  // Desc
    LPCSTR lpType;  // Desc
} D3DXF_FILELOADRESOURCE;

//----------------------------------------------------------------------------
// D3DXF_FILELOADMEMORY:
//----------------------------------------------------------------------------

typedef struct _D3DXF_FILELOADMEMORY
{
    LPCVOID lpMemory; // Desc
    SIZE_T  dSize;     // Desc
} D3DXF_FILELOADMEMORY;

#if defined( _WIN32 ) && !defined( _NO_COM )

// {cef08cf9-7b4f-4429-9624-2a690a933201}
DEFINE_GUID( IID_ID3DXFile, 
0xcef08cf9, 0x7b4f, 0x4429, 0x96, 0x24, 0x2a, 0x69, 0x0a, 0x93, 0x32, 0x01 );

// {cef08cfa-7b4f-4429-9624-2a690a933201}
DEFINE_GUID( IID_ID3DXFileSaveObject, 
0xcef08cfa, 0x7b4f, 0x4429, 0x96, 0x24, 0x2a, 0x69, 0x0a, 0x93, 0x32, 0x01 );

// {cef08cfb-7b4f-4429-9624-2a690a933201}
DEFINE_GUID( IID_ID3DXFileSaveData, 
0xcef08cfb, 0x7b4f, 0x4429, 0x96, 0x24, 0x2a, 0x69, 0x0a, 0x93, 0x32, 0x01 );

// {cef08cfc-7b4f-4429-9624-2a690a933201}
DEFINE_GUID( IID_ID3DXFileEnumObject, 
0xcef08cfc, 0x7b4f, 0x4429, 0x96, 0x24, 0x2a, 0x69, 0x0a, 0x93, 0x32, 0x01 );

// {cef08cfd-7b4f-4429-9624-2a690a933201}
DEFINE_GUID( IID_ID3DXFileData, 
0xcef08cfd, 0x7b4f, 0x4429, 0x96, 0x24, 0x2a, 0x69, 0x0a, 0x93, 0x32, 0x01 );

#endif // defined( _WIN32 ) && !defined( _NO_COM )

#if defined( __cplusplus )
#if !defined( DECLSPEC_UUID )
#if _MSC_VER >= 1100
#define DECLSPEC_UUID( x ) __declspec( uuid( x ) )
#else // !( _MSC_VER >= 1100 )
#define DECLSPEC_UUID( x )
#endif // !( _MSC_VER >= 1100 )
#endif // !defined( DECLSPEC_UUID )

interface DECLSPEC_UUID( "cef08cf9-7b4f-4429-9624-2a690a933201" )
    ID3DXFile;
interface DECLSPEC_UUID( "cef08cfa-7b4f-4429-9624-2a690a933201" )
    ID3DXFileSaveObject;
interface DECLSPEC_UUID( "cef08cfb-7b4f-4429-9624-2a690a933201" )
    ID3DXFileSaveData;
interface DECLSPEC_UUID( "cef08cfc-7b4f-4429-9624-2a690a933201" )
    ID3DXFileEnumObject;
interface DECLSPEC_UUID( "cef08cfd-7b4f-4429-9624-2a690a933201" )
    ID3DXFileData;

#if defined( _COM_SMARTPTR_TYPEDEF )
_COM_SMARTPTR_TYPEDEF( ID3DXFile,
    __uuidof( ID3DXFile ) );
_COM_SMARTPTR_TYPEDEF( ID3DXFileSaveObject,
    __uuidof( ID3DXFileSaveObject ) );
_COM_SMARTPTR_TYPEDEF( ID3DXFileSaveData,
    __uuidof( ID3DXFileSaveData ) );
_COM_SMARTPTR_TYPEDEF( ID3DXFileEnumObject,
    __uuidof( ID3DXFileEnumObject ) );
_COM_SMARTPTR_TYPEDEF( ID3DXFileData,
    __uuidof( ID3DXFileData ) );
#endif // defined( _COM_SMARTPTR_TYPEDEF )
#endif // defined( __cplusplus )

typedef interface ID3DXFile ID3DXFile;
typedef interface ID3DXFileSaveObject ID3DXFileSaveObject;
typedef interface ID3DXFileSaveData ID3DXFileSaveData;
typedef interface ID3DXFileEnumObject ID3DXFileEnumObject;
typedef interface ID3DXFileData ID3DXFileData;

//////////////////////////////////////////////////////////////////////////////
// ID3DXFile /////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#undef INTERFACE
#define INTERFACE ID3DXFile

DECLARE_INTERFACE_( ID3DXFile, IUnknown )
{
    STDMETHOD( QueryInterface )( THIS_ REFIID, LPVOID* ) PURE;
    STDMETHOD_( ULONG, AddRef )( THIS ) PURE;
    STDMETHOD_( ULONG, Release )( THIS ) PURE;
    
    STDMETHOD( CreateEnumObject )( THIS_ LPCVOID, D3DXF_FILELOADOPTIONS,
        ID3DXFileEnumObject** ) PURE;
    STDMETHOD( CreateSaveObject )( THIS_ LPCVOID, D3DXF_FILESAVEOPTIONS,
        D3DXF_FILEFORMAT, ID3DXFileSaveObject** ) PURE;
    STDMETHOD( RegisterTemplates )( THIS_ LPCVOID, SIZE_T ) PURE;
    STDMETHOD( RegisterEnumTemplates )( THIS_ ID3DXFileEnumObject* ) PURE;
};

//////////////////////////////////////////////////////////////////////////////
// ID3DXFileSaveObject ///////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#undef INTERFACE
#define INTERFACE ID3DXFileSaveObject

DECLARE_INTERFACE_( ID3DXFileSaveObject, IUnknown )
{
    STDMETHOD( QueryInterface )( THIS_ REFIID, LPVOID* ) PURE;
    STDMETHOD_( ULONG, AddRef )( THIS ) PURE;
    STDMETHOD_( ULONG, Release )( THIS ) PURE;
    
    STDMETHOD( GetFile )( THIS_ ID3DXFile** ) PURE;
    STDMETHOD( AddDataObject )( THIS_ REFGUID, LPCSTR, CONST GUID*,
        SIZE_T, LPCVOID, ID3DXFileSaveData** ) PURE;
    STDMETHOD( Save )( THIS ) PURE;
};

//////////////////////////////////////////////////////////////////////////////
// ID3DXFileSaveData /////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#undef INTERFACE
#define INTERFACE ID3DXFileSaveData

DECLARE_INTERFACE_( ID3DXFileSaveData, IUnknown )
{
    STDMETHOD( QueryInterface )( THIS_ REFIID, LPVOID* ) PURE;
    STDMETHOD_( ULONG, AddRef )( THIS ) PURE;
    STDMETHOD_( ULONG, Release )( THIS ) PURE;
    
    STDMETHOD( GetSave )( THIS_ ID3DXFileSaveObject** ) PURE;
    STDMETHOD( GetName )( THIS_ LPSTR, SIZE_T* ) PURE;
    STDMETHOD( GetId )( THIS_ LPGUID ) PURE;
    STDMETHOD( GetType )( THIS_ GUID* ) PURE;
    STDMETHOD( AddDataObject )( THIS_ REFGUID, LPCSTR, CONST GUID*,
        SIZE_T, LPCVOID, ID3DXFileSaveData** ) PURE;
    STDMETHOD( AddDataReference )( THIS_ LPCSTR, CONST GUID* ) PURE;
};

//////////////////////////////////////////////////////////////////////////////
// ID3DXFileEnumObject ///////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#undef INTERFACE
#define INTERFACE ID3DXFileEnumObject

DECLARE_INTERFACE_( ID3DXFileEnumObject, IUnknown )
{
    STDMETHOD( QueryInterface )( THIS_ REFIID, LPVOID* ) PURE;
    STDMETHOD_( ULONG, AddRef )( THIS ) PURE;
    STDMETHOD_( ULONG, Release )( THIS ) PURE;
    
    STDMETHOD( GetFile )( THIS_ ID3DXFile** ) PURE;
    STDMETHOD( GetChildren )( THIS_ SIZE_T* ) PURE;
    STDMETHOD( GetChild )( THIS_ SIZE_T, ID3DXFileData** ) PURE;
    STDMETHOD( GetDataObjectById )( THIS_ REFGUID, ID3DXFileData** ) PURE;
    STDMETHOD( GetDataObjectByName )( THIS_ LPCSTR, ID3DXFileData** ) PURE;
};

//////////////////////////////////////////////////////////////////////////////
// ID3DXFileData /////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#undef INTERFACE
#define INTERFACE ID3DXFileData

DECLARE_INTERFACE_( ID3DXFileData, IUnknown )
{
    STDMETHOD( QueryInterface )( THIS_ REFIID, LPVOID* ) PURE;
    STDMETHOD_( ULONG, AddRef )( THIS ) PURE;
    STDMETHOD_( ULONG, Release )( THIS ) PURE;
    
    STDMETHOD( GetEnum )( THIS_ ID3DXFileEnumObject** ) PURE;
    STDMETHOD( GetName )( THIS_ LPSTR, SIZE_T* ) PURE;
    STDMETHOD( GetId )( THIS_ LPGUID ) PURE;
    STDMETHOD( Lock )( THIS_ SIZE_T*, LPCVOID* ) PURE;
    STDMETHOD( Unlock )( THIS ) PURE;
    STDMETHOD( GetType )( THIS_ GUID* ) PURE;
    STDMETHOD_( BOOL, IsReference )( THIS ) PURE;
    STDMETHOD( GetChildren )( THIS_ SIZE_T* ) PURE;
    STDMETHOD( GetChild )( THIS_ SIZE_T, ID3DXFileData** ) PURE;
};

STDAPI D3DXFileCreate( ID3DXFile** lplpDirectXFile );

/*
 * DirectX File errors.
 */

#define _FACD3DXF 0x876

#define D3DXFERR_BADOBJECT              MAKE_HRESULT( 1, _FACD3DXF, 900 )
#define D3DXFERR_BADVALUE               MAKE_HRESULT( 1, _FACD3DXF, 901 )
#define D3DXFERR_BADTYPE                MAKE_HRESULT( 1, _FACD3DXF, 902 )
#define D3DXFERR_NOTFOUND               MAKE_HRESULT( 1, _FACD3DXF, 903 )
#define D3DXFERR_NOTDONEYET             MAKE_HRESULT( 1, _FACD3DXF, 904 )
#define D3DXFERR_FILENOTFOUND           MAKE_HRESULT( 1, _FACD3DXF, 905 )
#define D3DXFERR_RESOURCENOTFOUND       MAKE_HRESULT( 1, _FACD3DXF, 906 )
#define D3DXFERR_BADRESOURCE            MAKE_HRESULT( 1, _FACD3DXF, 907 )
#define D3DXFERR_BADFILETYPE            MAKE_HRESULT( 1, _FACD3DXF, 908 )
#define D3DXFERR_BADFILEVERSION         MAKE_HRESULT( 1, _FACD3DXF, 909 )
#define D3DXFERR_BADFILEFLOATSIZE       MAKE_HRESULT( 1, _FACD3DXF, 910 )
#define D3DXFERR_BADFILE                MAKE_HRESULT( 1, _FACD3DXF, 911 )
#define D3DXFERR_PARSEERROR             MAKE_HRESULT( 1, _FACD3DXF, 912 )
#define D3DXFERR_BADARRAYSIZE           MAKE_HRESULT( 1, _FACD3DXF, 913 )
#define D3DXFERR_BADDATAREFERENCE       MAKE_HRESULT( 1, _FACD3DXF, 914 )
#define D3DXFERR_NOMOREOBJECTS          MAKE_HRESULT( 1, _FACD3DXF, 915 )
#define D3DXFERR_NOMOREDATA             MAKE_HRESULT( 1, _FACD3DXF, 916 )
#define D3DXFERR_BADCACHEFILE           MAKE_HRESULT( 1, _FACD3DXF, 917 )

/*
 * DirectX File object types.
 */

#ifndef WIN_TYPES
#define WIN_TYPES(itype, ptype) typedef interface itype *LP##ptype, **LPLP##ptype
#endif

WIN_TYPES(ID3DXFile,                 D3DXFILE);
WIN_TYPES(ID3DXFileEnumObject,       D3DXFILEENUMOBJECT);
WIN_TYPES(ID3DXFileSaveObject,       D3DXFILESAVEOBJECT);
WIN_TYPES(ID3DXFileData,             D3DXFILEDATA);
WIN_TYPES(ID3DXFileSaveData,         D3DXFILESAVEDATA);

#if defined( __cplusplus )
} // extern "C"
#endif // defined( __cplusplus )

#endif // !defined( __D3DX9XOF_H__ )


