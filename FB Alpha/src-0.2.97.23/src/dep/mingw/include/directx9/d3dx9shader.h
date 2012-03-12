//////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//
//  File:       d3dx9shader.h
//  Content:    D3DX Shader APIs
//
//////////////////////////////////////////////////////////////////////////////

#include "d3dx9.h"

#ifndef __D3DX9SHADER_H__
#define __D3DX9SHADER_H__


//---------------------------------------------------------------------------
// D3DXTX_VERSION:
// --------------
// Version token used to create a procedural texture filler in effects
// Used by D3DXFill[]TX functions
//---------------------------------------------------------------------------
#define D3DXTX_VERSION(_Major,_Minor) (('T' << 24) | ('X' << 16) | ((_Major) << 8) | (_Minor))



//----------------------------------------------------------------------------
// D3DXSHADER flags:
// -----------------
// D3DXSHADER_DEBUG
//   Insert debug file/line/type/symbol information.
//
// D3DXSHADER_SKIPVALIDATION
//   Do not validate the generated code against known capabilities and
//   constraints.  This option is only recommended when compiling shaders
//   you KNOW will work.  (ie. have compiled before without this option.)
//   Shaders are always validated by D3D before they are set to the device.
//
// D3DXSHADER_SKIPOPTIMIZATION 
//   Instructs the compiler to skip optimization steps during code generation.
//   Unless you are trying to isolate a problem in your code using this option 
//   is not recommended.
//
// D3DXSHADER_PACKMATRIX_ROWMAJOR
//   Unless explicitly specified, matrices will be packed in row-major order
//   on input and output from the shader.
//
// D3DXSHADER_PACKMATRIX_COLUMNMAJOR
//   Unless explicitly specified, matrices will be packed in column-major 
//   order on input and output from the shader.  This is generally more 
//   efficient, since it allows vector-matrix multiplication to be performed
//   using a series of dot-products.
//
// D3DXSHADER_PARTIALPRECISION
//   Force all computations in resulting shader to occur at partial precision.
//   This may result in faster evaluation of shaders on some hardware.
//
// D3DXSHADER_FORCE_VS_SOFTWARE_NOOPT
//   Force compiler to compile against the next highest available software
//   target for vertex shaders.  This flag also turns optimizations off, 
//   and debugging on.  
//
// D3DXSHADER_FORCE_PS_SOFTWARE_NOOPT
//   Force compiler to compile against the next highest available software
//   target for pixel shaders.  This flag also turns optimizations off, 
//   and debugging on.
//
// D3DXSHADER_NO_PRESHADER
//   Disables Preshaders. Using this flag will cause the compiler to not 
//   pull out static expression for evaluation on the host cpu
//
// D3DXSHADER_AVOID_FLOW_CONTROL
//   Hint compiler to avoid flow-control constructs where possible.
//
// D3DXSHADER_PREFER_FLOW_CONTROL
//   Hint compiler to prefer flow-control constructs where possible.
//
//----------------------------------------------------------------------------

#define D3DXSHADER_DEBUG                          (1 << 0)
#define D3DXSHADER_SKIPVALIDATION                 (1 << 1)
#define D3DXSHADER_SKIPOPTIMIZATION               (1 << 2)
#define D3DXSHADER_PACKMATRIX_ROWMAJOR            (1 << 3)
#define D3DXSHADER_PACKMATRIX_COLUMNMAJOR         (1 << 4)
#define D3DXSHADER_PARTIALPRECISION               (1 << 5)
#define D3DXSHADER_FORCE_VS_SOFTWARE_NOOPT        (1 << 6)
#define D3DXSHADER_FORCE_PS_SOFTWARE_NOOPT        (1 << 7)
#define D3DXSHADER_NO_PRESHADER                   (1 << 8)
#define D3DXSHADER_AVOID_FLOW_CONTROL             (1 << 9)
#define D3DXSHADER_PREFER_FLOW_CONTROL            (1 << 10)
#define D3DXSHADER_ENABLE_BACKWARDS_COMPATIBILITY (1 << 12)
#define D3DXSHADER_IEEE_STRICTNESS                (1 << 13)
#define D3DXSHADER_USE_LEGACY_D3DX9_31_DLL        (1 << 16)


// optimization level flags
#define D3DXSHADER_OPTIMIZATION_LEVEL0            (1 << 14)
#define D3DXSHADER_OPTIMIZATION_LEVEL1            0
#define D3DXSHADER_OPTIMIZATION_LEVEL2            ((1 << 14) | (1 << 15))
#define D3DXSHADER_OPTIMIZATION_LEVEL3            (1 << 15)



//----------------------------------------------------------------------------
// D3DXCONSTTABLE flags:
// -------------------

#define D3DXCONSTTABLE_LARGEADDRESSAWARE          (1 << 17)



//----------------------------------------------------------------------------
// D3DXHANDLE:
// -----------
// Handle values used to efficiently reference shader and effect parameters.
// Strings can be used as handles.  However, handles are not always strings.
//----------------------------------------------------------------------------

#ifndef D3DXFX_LARGEADDRESS_HANDLE
typedef LPCSTR D3DXHANDLE;
#else
typedef UINT_PTR D3DXHANDLE;
#endif
typedef D3DXHANDLE *LPD3DXHANDLE;


//----------------------------------------------------------------------------
// D3DXMACRO:
// ----------
// Preprocessor macro definition.  The application pass in a NULL-terminated
// array of this structure to various D3DX APIs.  This enables the application
// to #define tokens at runtime, before the file is parsed.
//----------------------------------------------------------------------------

typedef struct _D3DXMACRO
{
    LPCSTR Name;
    LPCSTR Definition;

} D3DXMACRO, *LPD3DXMACRO;


//----------------------------------------------------------------------------
// D3DXSEMANTIC:
//----------------------------------------------------------------------------

typedef struct _D3DXSEMANTIC
{
    UINT Usage;
    UINT UsageIndex;

} D3DXSEMANTIC, *LPD3DXSEMANTIC;



//----------------------------------------------------------------------------
// D3DXREGISTER_SET:
//----------------------------------------------------------------------------

typedef enum _D3DXREGISTER_SET
{
    D3DXRS_BOOL,
    D3DXRS_INT4,
    D3DXRS_FLOAT4,
    D3DXRS_SAMPLER,

    // force 32-bit size enum
    D3DXRS_FORCE_DWORD = 0x7fffffff

} D3DXREGISTER_SET, *LPD3DXREGISTER_SET;


//----------------------------------------------------------------------------
// D3DXPARAMETER_CLASS:
//----------------------------------------------------------------------------

typedef enum _D3DXPARAMETER_CLASS
{
    D3DXPC_SCALAR,
    D3DXPC_VECTOR,
    D3DXPC_MATRIX_ROWS,
    D3DXPC_MATRIX_COLUMNS,
    D3DXPC_OBJECT,
    D3DXPC_STRUCT,

    // force 32-bit size enum
    D3DXPC_FORCE_DWORD = 0x7fffffff

} D3DXPARAMETER_CLASS, *LPD3DXPARAMETER_CLASS;


//----------------------------------------------------------------------------
// D3DXPARAMETER_TYPE:
//----------------------------------------------------------------------------

typedef enum _D3DXPARAMETER_TYPE
{
    D3DXPT_VOID,
    D3DXPT_BOOL,
    D3DXPT_INT,
    D3DXPT_FLOAT,
    D3DXPT_STRING,
    D3DXPT_TEXTURE,
    D3DXPT_TEXTURE1D,
    D3DXPT_TEXTURE2D,
    D3DXPT_TEXTURE3D,
    D3DXPT_TEXTURECUBE,
    D3DXPT_SAMPLER,
    D3DXPT_SAMPLER1D,
    D3DXPT_SAMPLER2D,
    D3DXPT_SAMPLER3D,
    D3DXPT_SAMPLERCUBE,
    D3DXPT_PIXELSHADER,
    D3DXPT_VERTEXSHADER,
    D3DXPT_PIXELFRAGMENT,
    D3DXPT_VERTEXFRAGMENT,
    D3DXPT_UNSUPPORTED,

    // force 32-bit size enum
    D3DXPT_FORCE_DWORD = 0x7fffffff

} D3DXPARAMETER_TYPE, *LPD3DXPARAMETER_TYPE;


//----------------------------------------------------------------------------
// D3DXCONSTANTTABLE_DESC:
//----------------------------------------------------------------------------

typedef struct _D3DXCONSTANTTABLE_DESC
{
    LPCSTR Creator;                     // Creator string
    DWORD Version;                      // Shader version
    UINT Constants;                     // Number of constants

} D3DXCONSTANTTABLE_DESC, *LPD3DXCONSTANTTABLE_DESC;


//----------------------------------------------------------------------------
// D3DXCONSTANT_DESC:
//----------------------------------------------------------------------------

typedef struct _D3DXCONSTANT_DESC
{
    LPCSTR Name;                        // Constant name

    D3DXREGISTER_SET RegisterSet;       // Register set
    UINT RegisterIndex;                 // Register index
    UINT RegisterCount;                 // Number of registers occupied

    D3DXPARAMETER_CLASS Class;          // Class
    D3DXPARAMETER_TYPE Type;            // Component type

    UINT Rows;                          // Number of rows
    UINT Columns;                       // Number of columns
    UINT Elements;                      // Number of array elements
    UINT StructMembers;                 // Number of structure member sub-parameters

    UINT Bytes;                         // Data size, in bytes
    LPCVOID DefaultValue;               // Pointer to default value

} D3DXCONSTANT_DESC, *LPD3DXCONSTANT_DESC;



//----------------------------------------------------------------------------
// ID3DXConstantTable:
//----------------------------------------------------------------------------

typedef interface ID3DXConstantTable ID3DXConstantTable;
typedef interface ID3DXConstantTable *LPD3DXCONSTANTTABLE;

// {AB3C758F-093E-4356-B762-4DB18F1B3A01}
DEFINE_GUID(IID_ID3DXConstantTable, 
0xab3c758f, 0x93e, 0x4356, 0xb7, 0x62, 0x4d, 0xb1, 0x8f, 0x1b, 0x3a, 0x1);


#undef INTERFACE
#define INTERFACE ID3DXConstantTable

DECLARE_INTERFACE_(ID3DXConstantTable, IUnknown)
{
    // IUnknown
    STDMETHOD(QueryInterface)(THIS_ REFIID iid, LPVOID *ppv) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

    // Buffer
    STDMETHOD_(LPVOID, GetBufferPointer)(THIS) PURE;
    STDMETHOD_(DWORD, GetBufferSize)(THIS) PURE;

    // Descs
    STDMETHOD(GetDesc)(THIS_ D3DXCONSTANTTABLE_DESC *pDesc) PURE;
    STDMETHOD(GetConstantDesc)(THIS_ D3DXHANDLE hConstant, D3DXCONSTANT_DESC *pConstantDesc, UINT *pCount) PURE;
    STDMETHOD_(UINT, GetSamplerIndex)(THIS_ D3DXHANDLE hConstant) PURE;

    // Handle operations
    STDMETHOD_(D3DXHANDLE, GetConstant)(THIS_ D3DXHANDLE hConstant, UINT Index) PURE;
    STDMETHOD_(D3DXHANDLE, GetConstantByName)(THIS_ D3DXHANDLE hConstant, LPCSTR pName) PURE;
    STDMETHOD_(D3DXHANDLE, GetConstantElement)(THIS_ D3DXHANDLE hConstant, UINT Index) PURE;

    // Set Constants
    STDMETHOD(SetDefaults)(THIS_ LPDIRECT3DDEVICE9 pDevice) PURE;
    STDMETHOD(SetValue)(THIS_ LPDIRECT3DDEVICE9 pDevice, D3DXHANDLE hConstant, LPCVOID pData, UINT Bytes) PURE;
    STDMETHOD(SetBool)(THIS_ LPDIRECT3DDEVICE9 pDevice, D3DXHANDLE hConstant, BOOL b) PURE;
    STDMETHOD(SetBoolArray)(THIS_ LPDIRECT3DDEVICE9 pDevice, D3DXHANDLE hConstant, CONST BOOL* pb, UINT Count) PURE;
    STDMETHOD(SetInt)(THIS_ LPDIRECT3DDEVICE9 pDevice, D3DXHANDLE hConstant, INT n) PURE;
    STDMETHOD(SetIntArray)(THIS_ LPDIRECT3DDEVICE9 pDevice, D3DXHANDLE hConstant, CONST INT* pn, UINT Count) PURE;
    STDMETHOD(SetFloat)(THIS_ LPDIRECT3DDEVICE9 pDevice, D3DXHANDLE hConstant, FLOAT f) PURE;
    STDMETHOD(SetFloatArray)(THIS_ LPDIRECT3DDEVICE9 pDevice, D3DXHANDLE hConstant, CONST FLOAT* pf, UINT Count) PURE;
    STDMETHOD(SetVector)(THIS_ LPDIRECT3DDEVICE9 pDevice, D3DXHANDLE hConstant, CONST D3DXVECTOR4* pVector) PURE;
    STDMETHOD(SetVectorArray)(THIS_ LPDIRECT3DDEVICE9 pDevice, D3DXHANDLE hConstant, CONST D3DXVECTOR4* pVector, UINT Count) PURE;
    STDMETHOD(SetMatrix)(THIS_ LPDIRECT3DDEVICE9 pDevice, D3DXHANDLE hConstant, CONST D3DXMATRIX* pMatrix) PURE;
    STDMETHOD(SetMatrixArray)(THIS_ LPDIRECT3DDEVICE9 pDevice, D3DXHANDLE hConstant, CONST D3DXMATRIX* pMatrix, UINT Count) PURE;
    STDMETHOD(SetMatrixPointerArray)(THIS_ LPDIRECT3DDEVICE9 pDevice, D3DXHANDLE hConstant, CONST D3DXMATRIX** ppMatrix, UINT Count) PURE;
    STDMETHOD(SetMatrixTranspose)(THIS_ LPDIRECT3DDEVICE9 pDevice, D3DXHANDLE hConstant, CONST D3DXMATRIX* pMatrix) PURE;
    STDMETHOD(SetMatrixTransposeArray)(THIS_ LPDIRECT3DDEVICE9 pDevice, D3DXHANDLE hConstant, CONST D3DXMATRIX* pMatrix, UINT Count) PURE;
    STDMETHOD(SetMatrixTransposePointerArray)(THIS_ LPDIRECT3DDEVICE9 pDevice, D3DXHANDLE hConstant, CONST D3DXMATRIX** ppMatrix, UINT Count) PURE;
};


//----------------------------------------------------------------------------
// ID3DXTextureShader:
//----------------------------------------------------------------------------

typedef interface ID3DXTextureShader ID3DXTextureShader;
typedef interface ID3DXTextureShader *LPD3DXTEXTURESHADER;

// {3E3D67F8-AA7A-405d-A857-BA01D4758426}
DEFINE_GUID(IID_ID3DXTextureShader, 
0x3e3d67f8, 0xaa7a, 0x405d, 0xa8, 0x57, 0xba, 0x1, 0xd4, 0x75, 0x84, 0x26);

#undef INTERFACE
#define INTERFACE ID3DXTextureShader

DECLARE_INTERFACE_(ID3DXTextureShader, IUnknown)
{
    // IUnknown
    STDMETHOD(QueryInterface)(THIS_ REFIID iid, LPVOID *ppv) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

    // Gets
    STDMETHOD(GetFunction)(THIS_ LPD3DXBUFFER *ppFunction) PURE;
    STDMETHOD(GetConstantBuffer)(THIS_ LPD3DXBUFFER *ppConstantBuffer) PURE;

    // Descs
    STDMETHOD(GetDesc)(THIS_ D3DXCONSTANTTABLE_DESC *pDesc) PURE;
    STDMETHOD(GetConstantDesc)(THIS_ D3DXHANDLE hConstant, D3DXCONSTANT_DESC *pConstantDesc, UINT *pCount) PURE;

    // Handle operations
    STDMETHOD_(D3DXHANDLE, GetConstant)(THIS_ D3DXHANDLE hConstant, UINT Index) PURE;
    STDMETHOD_(D3DXHANDLE, GetConstantByName)(THIS_ D3DXHANDLE hConstant, LPCSTR pName) PURE;
    STDMETHOD_(D3DXHANDLE, GetConstantElement)(THIS_ D3DXHANDLE hConstant, UINT Index) PURE;

    // Set Constants
    STDMETHOD(SetDefaults)(THIS) PURE;
    STDMETHOD(SetValue)(THIS_ D3DXHANDLE hConstant, LPCVOID pData, UINT Bytes) PURE;
    STDMETHOD(SetBool)(THIS_ D3DXHANDLE hConstant, BOOL b) PURE;
    STDMETHOD(SetBoolArray)(THIS_ D3DXHANDLE hConstant, CONST BOOL* pb, UINT Count) PURE;
    STDMETHOD(SetInt)(THIS_ D3DXHANDLE hConstant, INT n) PURE;
    STDMETHOD(SetIntArray)(THIS_ D3DXHANDLE hConstant, CONST INT* pn, UINT Count) PURE;
    STDMETHOD(SetFloat)(THIS_ D3DXHANDLE hConstant, FLOAT f) PURE;
    STDMETHOD(SetFloatArray)(THIS_ D3DXHANDLE hConstant, CONST FLOAT* pf, UINT Count) PURE;
    STDMETHOD(SetVector)(THIS_ D3DXHANDLE hConstant, CONST D3DXVECTOR4* pVector) PURE;
    STDMETHOD(SetVectorArray)(THIS_ D3DXHANDLE hConstant, CONST D3DXVECTOR4* pVector, UINT Count) PURE;
    STDMETHOD(SetMatrix)(THIS_ D3DXHANDLE hConstant, CONST D3DXMATRIX* pMatrix) PURE;
    STDMETHOD(SetMatrixArray)(THIS_ D3DXHANDLE hConstant, CONST D3DXMATRIX* pMatrix, UINT Count) PURE;
    STDMETHOD(SetMatrixPointerArray)(THIS_ D3DXHANDLE hConstant, CONST D3DXMATRIX** ppMatrix, UINT Count) PURE;
    STDMETHOD(SetMatrixTranspose)(THIS_ D3DXHANDLE hConstant, CONST D3DXMATRIX* pMatrix) PURE;
    STDMETHOD(SetMatrixTransposeArray)(THIS_ D3DXHANDLE hConstant, CONST D3DXMATRIX* pMatrix, UINT Count) PURE;
    STDMETHOD(SetMatrixTransposePointerArray)(THIS_ D3DXHANDLE hConstant, CONST D3DXMATRIX** ppMatrix, UINT Count) PURE;
};


//----------------------------------------------------------------------------
// D3DXINCLUDE_TYPE:
//----------------------------------------------------------------------------

typedef enum _D3DXINCLUDE_TYPE
{
    D3DXINC_LOCAL,
    D3DXINC_SYSTEM,

    // force 32-bit size enum
    D3DXINC_FORCE_DWORD = 0x7fffffff

} D3DXINCLUDE_TYPE, *LPD3DXINCLUDE_TYPE;


//----------------------------------------------------------------------------
// ID3DXInclude:
// -------------
// This interface is intended to be implemented by the application, and can
// be used by various D3DX APIs.  This enables application-specific handling
// of #include directives in source files.
//
// Open()
//    Opens an include file.  If successful, it should fill in ppData and
//    pBytes.  The data pointer returned must remain valid until Close is
//    subsequently called.  The name of the file is encoded in UTF-8 format.
// Close()
//    Closes an include file.  If Open was successful, Close is guaranteed
//    to be called before the API using this interface returns.
//----------------------------------------------------------------------------

typedef interface ID3DXInclude ID3DXInclude;
typedef interface ID3DXInclude *LPD3DXINCLUDE;

#undef INTERFACE
#define INTERFACE ID3DXInclude

DECLARE_INTERFACE(ID3DXInclude)
{
    STDMETHOD(Open)(THIS_ D3DXINCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes) PURE;
    STDMETHOD(Close)(THIS_ LPCVOID pData) PURE;
};


//////////////////////////////////////////////////////////////////////////////
// APIs //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus


//----------------------------------------------------------------------------
// D3DXAssembleShader:
// -------------------
// Assembles a shader.
//
// Parameters:
//  pSrcFile
//      Source file name
//  hSrcModule
//      Module handle. if NULL, current module will be used
//  pSrcResource
//      Resource name in module
//  pSrcData
//      Pointer to source code
//  SrcDataLen
//      Size of source code, in bytes
//  pDefines
//      Optional NULL-terminated array of preprocessor macro definitions.
//  pInclude
//      Optional interface pointer to use for handling #include directives.
//      If this parameter is NULL, #includes will be honored when assembling
//      from file, and will error when assembling from resource or memory.
//  Flags
//      See D3DXSHADER_xxx flags
//  ppShader
//      Returns a buffer containing the created shader.  This buffer contains
//      the assembled shader code, as well as any embedded debug info.
//  ppErrorMsgs
//      Returns a buffer containing a listing of errors and warnings that were
//      encountered during assembly.  If you are running in a debugger,
//      these are the same messages you will see in your debug output.
//----------------------------------------------------------------------------


HRESULT WINAPI
    D3DXAssembleShaderFromFileA(
        LPCSTR                          pSrcFile,
        CONST D3DXMACRO*                pDefines,
        LPD3DXINCLUDE                   pInclude,
        DWORD                           Flags,
        LPD3DXBUFFER*                   ppShader,
        LPD3DXBUFFER*                   ppErrorMsgs);

HRESULT WINAPI
    D3DXAssembleShaderFromFileW(
        LPCWSTR                         pSrcFile,
        CONST D3DXMACRO*                pDefines,
        LPD3DXINCLUDE                   pInclude,
        DWORD                           Flags,
        LPD3DXBUFFER*                   ppShader,
        LPD3DXBUFFER*                   ppErrorMsgs);

#ifdef UNICODE
#define D3DXAssembleShaderFromFile D3DXAssembleShaderFromFileW
#else
#define D3DXAssembleShaderFromFile D3DXAssembleShaderFromFileA
#endif


HRESULT WINAPI
    D3DXAssembleShaderFromResourceA(
        HMODULE                         hSrcModule,
        LPCSTR                          pSrcResource,
        CONST D3DXMACRO*                pDefines,
        LPD3DXINCLUDE                   pInclude,
        DWORD                           Flags,
        LPD3DXBUFFER*                   ppShader,
        LPD3DXBUFFER*                   ppErrorMsgs);

HRESULT WINAPI
    D3DXAssembleShaderFromResourceW(
        HMODULE                         hSrcModule,
        LPCWSTR                         pSrcResource,
        CONST D3DXMACRO*                pDefines,
        LPD3DXINCLUDE                   pInclude,
        DWORD                           Flags,
        LPD3DXBUFFER*                   ppShader,
        LPD3DXBUFFER*                   ppErrorMsgs);

#ifdef UNICODE
#define D3DXAssembleShaderFromResource D3DXAssembleShaderFromResourceW
#else
#define D3DXAssembleShaderFromResource D3DXAssembleShaderFromResourceA
#endif


HRESULT WINAPI
    D3DXAssembleShader(
        LPCSTR                          pSrcData,
        UINT                            SrcDataLen,
        CONST D3DXMACRO*                pDefines,
        LPD3DXINCLUDE                   pInclude,
        DWORD                           Flags,
        LPD3DXBUFFER*                   ppShader,
        LPD3DXBUFFER*                   ppErrorMsgs);



//----------------------------------------------------------------------------
// D3DXCompileShader:
// ------------------
// Compiles a shader.
//
// Parameters:
//  pSrcFile
//      Source file name.
//  hSrcModule
//      Module handle. if NULL, current module will be used.
//  pSrcResource
//      Resource name in module.
//  pSrcData
//      Pointer to source code.
//  SrcDataLen
//      Size of source code, in bytes.
//  pDefines
//      Optional NULL-terminated array of preprocessor macro definitions.
//  pInclude
//      Optional interface pointer to use for handling #include directives.
//      If this parameter is NULL, #includes will be honored when compiling
//      from file, and will error when compiling from resource or memory.
//  pFunctionName
//      Name of the entrypoint function where execution should begin.
//  pProfile
//      Instruction set to be used when generating code.  Currently supported
//      profiles are "vs_1_1", "vs_2_0", "vs_2_a", "vs_2_sw", "ps_1_1", 
//      "ps_1_2", "ps_1_3", "ps_1_4", "ps_2_0", "ps_2_a", "ps_2_sw", "tx_1_0"
//  Flags
//      See D3DXSHADER_xxx flags.
//  ppShader
//      Returns a buffer containing the created shader.  This buffer contains
//      the compiled shader code, as well as any embedded debug and symbol
//      table info.  (See D3DXGetShaderConstantTable)
//  ppErrorMsgs
//      Returns a buffer containing a listing of errors and warnings that were
//      encountered during the compile.  If you are running in a debugger,
//      these are the same messages you will see in your debug output.
//  ppConstantTable
//      Returns a ID3DXConstantTable object which can be used to set
//      shader constants to the device.  Alternatively, an application can
//      parse the D3DXSHADER_CONSTANTTABLE block embedded as a comment within
//      the shader.
//----------------------------------------------------------------------------

HRESULT WINAPI
    D3DXCompileShaderFromFileA(
        LPCSTR                          pSrcFile,
        CONST D3DXMACRO*                pDefines,
        LPD3DXINCLUDE                   pInclude,
        LPCSTR                          pFunctionName,
        LPCSTR                          pProfile,
        DWORD                           Flags,
        LPD3DXBUFFER*                   ppShader,
        LPD3DXBUFFER*                   ppErrorMsgs,
        LPD3DXCONSTANTTABLE*            ppConstantTable);

HRESULT WINAPI
    D3DXCompileShaderFromFileW(
        LPCWSTR                         pSrcFile,
        CONST D3DXMACRO*                pDefines,
        LPD3DXINCLUDE                   pInclude,
        LPCSTR                          pFunctionName,
        LPCSTR                          pProfile,
        DWORD                           Flags,
        LPD3DXBUFFER*                   ppShader,
        LPD3DXBUFFER*                   ppErrorMsgs,
        LPD3DXCONSTANTTABLE*            ppConstantTable);

#ifdef UNICODE
#define D3DXCompileShaderFromFile D3DXCompileShaderFromFileW
#else
#define D3DXCompileShaderFromFile D3DXCompileShaderFromFileA
#endif


HRESULT WINAPI
    D3DXCompileShaderFromResourceA(
        HMODULE                         hSrcModule,
        LPCSTR                          pSrcResource,
        CONST D3DXMACRO*                pDefines,
        LPD3DXINCLUDE                   pInclude,
        LPCSTR                          pFunctionName,
        LPCSTR                          pProfile,
        DWORD                           Flags,
        LPD3DXBUFFER*                   ppShader,
        LPD3DXBUFFER*                   ppErrorMsgs,
        LPD3DXCONSTANTTABLE*            ppConstantTable);

HRESULT WINAPI
    D3DXCompileShaderFromResourceW(
        HMODULE                         hSrcModule,
        LPCWSTR                         pSrcResource,
        CONST D3DXMACRO*                pDefines,
        LPD3DXINCLUDE                   pInclude,
        LPCSTR                          pFunctionName,
        LPCSTR                          pProfile,
        DWORD                           Flags,
        LPD3DXBUFFER*                   ppShader,
        LPD3DXBUFFER*                   ppErrorMsgs,
        LPD3DXCONSTANTTABLE*            ppConstantTable);

#ifdef UNICODE
#define D3DXCompileShaderFromResource D3DXCompileShaderFromResourceW
#else
#define D3DXCompileShaderFromResource D3DXCompileShaderFromResourceA
#endif


HRESULT WINAPI
    D3DXCompileShader(
        LPCSTR                          pSrcData,
        UINT                            SrcDataLen,
        CONST D3DXMACRO*                pDefines,
        LPD3DXINCLUDE                   pInclude,
        LPCSTR                          pFunctionName,
        LPCSTR                          pProfile,
        DWORD                           Flags,
        LPD3DXBUFFER*                   ppShader,
        LPD3DXBUFFER*                   ppErrorMsgs,
        LPD3DXCONSTANTTABLE*            ppConstantTable);


//----------------------------------------------------------------------------
// D3DXDisassembleShader:
// ----------------------
// Takes a binary shader, and returns a buffer containing text assembly.
//
// Parameters:
//  pShader
//      Pointer to the shader byte code.
//  ShaderSizeInBytes
//      Size of the shader byte code in bytes.
//  EnableColorCode
//      Emit HTML tags for color coding the output?
//  pComments
//      Pointer to a comment string to include at the top of the shader.
//  ppDisassembly
//      Returns a buffer containing the disassembled shader.
//----------------------------------------------------------------------------

HRESULT WINAPI
    D3DXDisassembleShader(
        CONST DWORD*                    pShader, 
        BOOL                            EnableColorCode, 
        LPCSTR                          pComments, 
        LPD3DXBUFFER*                   ppDisassembly);


//----------------------------------------------------------------------------
// D3DXGetPixelShaderProfile/D3DXGetVertexShaderProfile:
// -----------------------------------------------------
// Returns the name of the HLSL profile best suited to a given device.
//
// Parameters:
//  pDevice
//      Pointer to the device in question
//----------------------------------------------------------------------------

LPCSTR WINAPI
    D3DXGetPixelShaderProfile(
        LPDIRECT3DDEVICE9               pDevice);

LPCSTR WINAPI
    D3DXGetVertexShaderProfile(
        LPDIRECT3DDEVICE9               pDevice);


//----------------------------------------------------------------------------
// D3DXFindShaderComment:
// ----------------------
// Searches through a shader for a particular comment, denoted by a FourCC in
// the first DWORD of the comment.  If the comment is not found, and no other
// error has occurred, S_FALSE is returned.
//
// Parameters:
//  pFunction
//      Pointer to the function DWORD stream
//  FourCC
//      FourCC used to identify the desired comment block.
//  ppData
//      Returns a pointer to the comment data (not including comment token
//      and FourCC).  Can be NULL.
//  pSizeInBytes
//      Returns the size of the comment data in bytes.  Can be NULL.
//----------------------------------------------------------------------------

HRESULT WINAPI
    D3DXFindShaderComment(
        CONST DWORD*                    pFunction,
        DWORD                           FourCC,
        LPCVOID*                        ppData,
        UINT*                           pSizeInBytes);


//----------------------------------------------------------------------------
// D3DXGetShaderSize:
// ------------------
// Returns the size of the shader byte-code, in bytes.
//
// Parameters:
//  pFunction
//      Pointer to the function DWORD stream
//----------------------------------------------------------------------------

UINT WINAPI
    D3DXGetShaderSize(
        CONST DWORD*                    pFunction);


//----------------------------------------------------------------------------
// D3DXGetShaderVersion:
// -----------------------
// Returns the shader version of a given shader.  Returns zero if the shader 
// function is NULL.
//
// Parameters:
//  pFunction
//      Pointer to the function DWORD stream
//----------------------------------------------------------------------------

DWORD WINAPI
    D3DXGetShaderVersion(
        CONST DWORD*                    pFunction);

//----------------------------------------------------------------------------
// D3DXGetShaderSemantics:
// -----------------------
// Gets semantics for all input elements referenced inside a given shader.
//
// Parameters:
//  pFunction
//      Pointer to the function DWORD stream
//  pSemantics
//      Pointer to an array of D3DXSEMANTIC structures.  The function will
//      fill this array with the semantics for each input element referenced
//      inside the shader.  This array is assumed to contain at least
//      MAXD3DDECLLENGTH elements.
//  pCount
//      Returns the number of elements referenced by the shader
//----------------------------------------------------------------------------

HRESULT WINAPI
    D3DXGetShaderInputSemantics(
        CONST DWORD*                    pFunction,
        D3DXSEMANTIC*                   pSemantics,
        UINT*                           pCount);

HRESULT WINAPI
    D3DXGetShaderOutputSemantics(
        CONST DWORD*                    pFunction,
        D3DXSEMANTIC*                   pSemantics,
        UINT*                           pCount);


//----------------------------------------------------------------------------
// D3DXGetShaderSamplers:
// ----------------------
// Gets semantics for all input elements referenced inside a given shader.
//
// pFunction
//      Pointer to the function DWORD stream
// pSamplers
//      Pointer to an array of LPCSTRs.  The function will fill this array
//      with pointers to the sampler names contained within pFunction, for
//      each sampler referenced inside the shader.  This array is assumed to
//      contain at least 16 elements.
// pCount
//      Returns the number of samplers referenced by the shader
//----------------------------------------------------------------------------

HRESULT WINAPI
    D3DXGetShaderSamplers(
        CONST DWORD*                    pFunction,
        LPCSTR*                         pSamplers,
        UINT*                           pCount);


//----------------------------------------------------------------------------
// D3DXGetShaderConstantTable:
// ---------------------------
// Gets shader constant table embedded inside shader.  A constant table is
// generated by D3DXAssembleShader and D3DXCompileShader, and is embedded in
// the body of the shader.
//
// Parameters:
//  pFunction
//      Pointer to the function DWORD stream
//  Flags
//      See D3DXCONSTTABLE_xxx
//  ppConstantTable
//      Returns a ID3DXConstantTable object which can be used to set
//      shader constants to the device.  Alternatively, an application can
//      parse the D3DXSHADER_CONSTANTTABLE block embedded as a comment within
//      the shader.
//----------------------------------------------------------------------------

HRESULT WINAPI
    D3DXGetShaderConstantTable(
        CONST DWORD*                    pFunction,
        LPD3DXCONSTANTTABLE*            ppConstantTable);

HRESULT WINAPI
    D3DXGetShaderConstantTableEx(
        CONST DWORD*                    pFunction,
        DWORD                           Flags,
        LPD3DXCONSTANTTABLE*            ppConstantTable);



//----------------------------------------------------------------------------
// D3DXCreateTextureShader:
// ------------------------
// Creates a texture shader object, given the compiled shader.
//
// Parameters
//  pFunction
//      Pointer to the function DWORD stream
//  ppTextureShader
//      Returns a ID3DXTextureShader object which can be used to procedurally 
//      fill the contents of a texture using the D3DXFillTextureTX functions.
//----------------------------------------------------------------------------

HRESULT WINAPI
    D3DXCreateTextureShader(
        CONST DWORD*                    pFunction, 
        LPD3DXTEXTURESHADER*            ppTextureShader);


//----------------------------------------------------------------------------
// D3DXPreprocessShader:
// ---------------------
// Runs the preprocessor on the specified shader or effect, but does
// not actually compile it.  This is useful for evaluating the #includes
// and #defines in a shader and then emitting a reformatted token stream
// for debugging purposes or for generating a self-contained shader.
//
// Parameters:
//  pSrcFile
//      Source file name
//  hSrcModule
//      Module handle. if NULL, current module will be used
//  pSrcResource
//      Resource name in module
//  pSrcData
//      Pointer to source code
//  SrcDataLen
//      Size of source code, in bytes
//  pDefines
//      Optional NULL-terminated array of preprocessor macro definitions.
//  pInclude
//      Optional interface pointer to use for handling #include directives.
//      If this parameter is NULL, #includes will be honored when assembling
//      from file, and will error when assembling from resource or memory.
//  ppShaderText
//      Returns a buffer containing a single large string that represents
//      the resulting formatted token stream
//  ppErrorMsgs
//      Returns a buffer containing a listing of errors and warnings that were
//      encountered during assembly.  If you are running in a debugger,
//      these are the same messages you will see in your debug output.
//----------------------------------------------------------------------------

HRESULT WINAPI 
    D3DXPreprocessShaderFromFileA(
        LPCSTR                       pSrcFile,
        CONST D3DXMACRO*             pDefines,
        LPD3DXINCLUDE                pInclude,
        LPD3DXBUFFER*                ppShaderText,
        LPD3DXBUFFER*                ppErrorMsgs);
                                             
HRESULT WINAPI 
    D3DXPreprocessShaderFromFileW(
        LPCWSTR                      pSrcFile,
        CONST D3DXMACRO*             pDefines,
        LPD3DXINCLUDE                pInclude,
        LPD3DXBUFFER*                ppShaderText,
        LPD3DXBUFFER*                ppErrorMsgs);

#ifdef UNICODE
#define D3DXPreprocessShaderFromFile D3DXPreprocessShaderFromFileW
#else
#define D3DXPreprocessShaderFromFile D3DXPreprocessShaderFromFileA
#endif
                                             
HRESULT WINAPI 
    D3DXPreprocessShaderFromResourceA(
        HMODULE                      hSrcModule,
        LPCSTR                       pSrcResource,
        CONST D3DXMACRO*             pDefines,
        LPD3DXINCLUDE                pInclude,
        LPD3DXBUFFER*                ppShaderText,
        LPD3DXBUFFER*                ppErrorMsgs);

HRESULT WINAPI 
    D3DXPreprocessShaderFromResourceW(
        HMODULE                      hSrcModule,
        LPCWSTR                      pSrcResource,
        CONST D3DXMACRO*             pDefines,
        LPD3DXINCLUDE                pInclude,
        LPD3DXBUFFER*                ppShaderText,
        LPD3DXBUFFER*                ppErrorMsgs);

#ifdef UNICODE
#define D3DXPreprocessShaderFromResource D3DXPreprocessShaderFromResourceW
#else
#define D3DXPreprocessShaderFromResource D3DXPreprocessShaderFromResourceA
#endif

HRESULT WINAPI 
    D3DXPreprocessShader(
        LPCSTR                       pSrcData,
        UINT                         SrcDataSize,
        CONST D3DXMACRO*             pDefines,
        LPD3DXINCLUDE                pInclude,
        LPD3DXBUFFER*                ppShaderText,
        LPD3DXBUFFER*                ppErrorMsgs);


#ifdef __cplusplus
}
#endif //__cplusplus


//////////////////////////////////////////////////////////////////////////////
// Shader comment block layouts //////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
// D3DXSHADER_CONSTANTTABLE:
// -------------------------
// Shader constant information; included as an CTAB comment block inside
// shaders.  All offsets are BYTE offsets from start of CONSTANTTABLE struct.
// Entries in the table are sorted by Name in ascending order.
//----------------------------------------------------------------------------

typedef struct _D3DXSHADER_CONSTANTTABLE
{
    DWORD Size;             // sizeof(D3DXSHADER_CONSTANTTABLE)
    DWORD Creator;          // LPCSTR offset
    DWORD Version;          // shader version
    DWORD Constants;        // number of constants
    DWORD ConstantInfo;     // D3DXSHADER_CONSTANTINFO[Constants] offset
    DWORD Flags;            // flags shader was compiled with
    DWORD Target;           // LPCSTR offset 

} D3DXSHADER_CONSTANTTABLE, *LPD3DXSHADER_CONSTANTTABLE;


typedef struct _D3DXSHADER_CONSTANTINFO
{
    DWORD Name;             // LPCSTR offset
    WORD  RegisterSet;      // D3DXREGISTER_SET
    WORD  RegisterIndex;    // register number
    WORD  RegisterCount;    // number of registers
    WORD  Reserved;         // reserved
    DWORD TypeInfo;         // D3DXSHADER_TYPEINFO offset
    DWORD DefaultValue;     // offset of default value

} D3DXSHADER_CONSTANTINFO, *LPD3DXSHADER_CONSTANTINFO;


typedef struct _D3DXSHADER_TYPEINFO
{
    WORD  Class;            // D3DXPARAMETER_CLASS
    WORD  Type;             // D3DXPARAMETER_TYPE
    WORD  Rows;             // number of rows (matrices)
    WORD  Columns;          // number of columns (vectors and matrices)
    WORD  Elements;         // array dimension
    WORD  StructMembers;    // number of struct members
    DWORD StructMemberInfo; // D3DXSHADER_STRUCTMEMBERINFO[Members] offset

} D3DXSHADER_TYPEINFO, *LPD3DXSHADER_TYPEINFO;


typedef struct _D3DXSHADER_STRUCTMEMBERINFO
{
    DWORD Name;             // LPCSTR offset
    DWORD TypeInfo;         // D3DXSHADER_TYPEINFO offset

} D3DXSHADER_STRUCTMEMBERINFO, *LPD3DXSHADER_STRUCTMEMBERINFO;



#endif //__D3DX9SHADER_H__

