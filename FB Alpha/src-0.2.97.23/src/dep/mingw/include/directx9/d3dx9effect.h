
//////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//
//  File:       d3dx9effect.h
//  Content:    D3DX effect types and Shaders
//
//////////////////////////////////////////////////////////////////////////////

#include "d3dx9.h"

#ifndef __D3DX9EFFECT_H__
#define __D3DX9EFFECT_H__


//----------------------------------------------------------------------------
// D3DXFX_DONOTSAVESTATE
//   This flag is used as a parameter to ID3DXEffect::Begin().  When this flag
//   is specified, device state is not saved or restored in Begin/End.
// D3DXFX_DONOTSAVESHADERSTATE
//   This flag is used as a parameter to ID3DXEffect::Begin().  When this flag
//   is specified, shader device state is not saved or restored in Begin/End.
//   This includes pixel/vertex shaders and shader constants
// D3DXFX_DONOTSAVESAMPLERSTATE
//   This flag is used as a parameter to ID3DXEffect::Begin(). When this flag
//   is specified, sampler device state is not saved or restored in Begin/End.
// D3DXFX_NOT_CLONEABLE
//   This flag is used as a parameter to the D3DXCreateEffect family of APIs.
//   When this flag is specified, the effect will be non-cloneable and will not
//   contain any shader binary data.
//   Furthermore, GetPassDesc will not return shader function pointers. 
//   Setting this flag reduces effect memory usage by about 50%.
//----------------------------------------------------------------------------

#define D3DXFX_DONOTSAVESTATE         (1 << 0)
#define D3DXFX_DONOTSAVESHADERSTATE   (1 << 1)
#define D3DXFX_DONOTSAVESAMPLERSTATE  (1 << 2)

#define D3DXFX_NOT_CLONEABLE          (1 << 11)
#define D3DXFX_LARGEADDRESSAWARE      (1 << 17)

//----------------------------------------------------------------------------
// D3DX_PARAMETER_SHARED
//   Indicates that the value of a parameter will be shared with all effects
//   which share the same namespace.  Changing the value in one effect will
//   change it in all.
//
// D3DX_PARAMETER_LITERAL
//   Indicates that the value of this parameter can be treated as literal.
//   Literal parameters can be marked when the effect is compiled, and their
//   cannot be changed after the effect is compiled.  Shared parameters cannot
//   be literal.
//----------------------------------------------------------------------------

#define D3DX_PARAMETER_SHARED       (1 << 0)
#define D3DX_PARAMETER_LITERAL      (1 << 1)
#define D3DX_PARAMETER_ANNOTATION   (1 << 2)

//----------------------------------------------------------------------------
// D3DXEFFECT_DESC:
//----------------------------------------------------------------------------

typedef struct _D3DXEFFECT_DESC
{
    LPCSTR Creator;                     // Creator string
    UINT Parameters;                    // Number of parameters
    UINT Techniques;                    // Number of techniques
    UINT Functions;                     // Number of function entrypoints

} D3DXEFFECT_DESC;


//----------------------------------------------------------------------------
// D3DXPARAMETER_DESC:
//----------------------------------------------------------------------------

typedef struct _D3DXPARAMETER_DESC
{
    LPCSTR Name;                        // Parameter name
    LPCSTR Semantic;                    // Parameter semantic
    D3DXPARAMETER_CLASS Class;          // Class
    D3DXPARAMETER_TYPE Type;            // Component type
    UINT Rows;                          // Number of rows
    UINT Columns;                       // Number of columns
    UINT Elements;                      // Number of array elements
    UINT Annotations;                   // Number of annotations
    UINT StructMembers;                 // Number of structure member sub-parameters
    DWORD Flags;                        // D3DX_PARAMETER_* flags
    UINT Bytes;                         // Parameter size, in bytes

} D3DXPARAMETER_DESC;


//----------------------------------------------------------------------------
// D3DXTECHNIQUE_DESC:
//----------------------------------------------------------------------------

typedef struct _D3DXTECHNIQUE_DESC
{
    LPCSTR Name;                        // Technique name
    UINT Passes;                        // Number of passes
    UINT Annotations;                   // Number of annotations

} D3DXTECHNIQUE_DESC;


//----------------------------------------------------------------------------
// D3DXPASS_DESC:
//----------------------------------------------------------------------------

typedef struct _D3DXPASS_DESC
{
    LPCSTR Name;                        // Pass name
    UINT Annotations;                   // Number of annotations

    CONST DWORD *pVertexShaderFunction; // Vertex shader function
    CONST DWORD *pPixelShaderFunction;  // Pixel shader function

} D3DXPASS_DESC;


//----------------------------------------------------------------------------
// D3DXFUNCTION_DESC:
//----------------------------------------------------------------------------

typedef struct _D3DXFUNCTION_DESC
{
    LPCSTR Name;                        // Function name
    UINT Annotations;                   // Number of annotations

} D3DXFUNCTION_DESC;



//////////////////////////////////////////////////////////////////////////////
// ID3DXEffectPool ///////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

typedef interface ID3DXEffectPool ID3DXEffectPool;
typedef interface ID3DXEffectPool *LPD3DXEFFECTPOOL;

// {9537AB04-3250-412e-8213-FCD2F8677933}
DEFINE_GUID(IID_ID3DXEffectPool, 
0x9537ab04, 0x3250, 0x412e, 0x82, 0x13, 0xfc, 0xd2, 0xf8, 0x67, 0x79, 0x33);


#undef INTERFACE
#define INTERFACE ID3DXEffectPool

DECLARE_INTERFACE_(ID3DXEffectPool, IUnknown)
{
    // IUnknown
    STDMETHOD(QueryInterface)(THIS_ REFIID iid, LPVOID *ppv) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

    // No public methods
};


//////////////////////////////////////////////////////////////////////////////
// ID3DXBaseEffect ///////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

typedef interface ID3DXBaseEffect ID3DXBaseEffect;
typedef interface ID3DXBaseEffect *LPD3DXBASEEFFECT;

// {017C18AC-103F-4417-8C51-6BF6EF1E56BE}
DEFINE_GUID(IID_ID3DXBaseEffect, 
0x17c18ac, 0x103f, 0x4417, 0x8c, 0x51, 0x6b, 0xf6, 0xef, 0x1e, 0x56, 0xbe);


#undef INTERFACE
#define INTERFACE ID3DXBaseEffect

DECLARE_INTERFACE_(ID3DXBaseEffect, IUnknown)
{
    // IUnknown
    STDMETHOD(QueryInterface)(THIS_ REFIID iid, LPVOID *ppv) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

    // Descs
    STDMETHOD(GetDesc)(THIS_ D3DXEFFECT_DESC* pDesc) PURE;
    STDMETHOD(GetParameterDesc)(THIS_ D3DXHANDLE hParameter, D3DXPARAMETER_DESC* pDesc) PURE;
    STDMETHOD(GetTechniqueDesc)(THIS_ D3DXHANDLE hTechnique, D3DXTECHNIQUE_DESC* pDesc) PURE;
    STDMETHOD(GetPassDesc)(THIS_ D3DXHANDLE hPass, D3DXPASS_DESC* pDesc) PURE;
    STDMETHOD(GetFunctionDesc)(THIS_ D3DXHANDLE hShader, D3DXFUNCTION_DESC* pDesc) PURE;

    // Handle operations
    STDMETHOD_(D3DXHANDLE, GetParameter)(THIS_ D3DXHANDLE hParameter, UINT Index) PURE;
    STDMETHOD_(D3DXHANDLE, GetParameterByName)(THIS_ D3DXHANDLE hParameter, LPCSTR pName) PURE;
    STDMETHOD_(D3DXHANDLE, GetParameterBySemantic)(THIS_ D3DXHANDLE hParameter, LPCSTR pSemantic) PURE;
    STDMETHOD_(D3DXHANDLE, GetParameterElement)(THIS_ D3DXHANDLE hParameter, UINT Index) PURE;
    STDMETHOD_(D3DXHANDLE, GetTechnique)(THIS_ UINT Index) PURE;
    STDMETHOD_(D3DXHANDLE, GetTechniqueByName)(THIS_ LPCSTR pName) PURE;
    STDMETHOD_(D3DXHANDLE, GetPass)(THIS_ D3DXHANDLE hTechnique, UINT Index) PURE;
    STDMETHOD_(D3DXHANDLE, GetPassByName)(THIS_ D3DXHANDLE hTechnique, LPCSTR pName) PURE;
    STDMETHOD_(D3DXHANDLE, GetFunction)(THIS_ UINT Index) PURE;
    STDMETHOD_(D3DXHANDLE, GetFunctionByName)(THIS_ LPCSTR pName) PURE;
    STDMETHOD_(D3DXHANDLE, GetAnnotation)(THIS_ D3DXHANDLE hObject, UINT Index) PURE;
    STDMETHOD_(D3DXHANDLE, GetAnnotationByName)(THIS_ D3DXHANDLE hObject, LPCSTR pName) PURE;

    // Get/Set Parameters
    STDMETHOD(SetValue)(THIS_ D3DXHANDLE hParameter, LPCVOID pData, UINT Bytes) PURE;
    STDMETHOD(GetValue)(THIS_ D3DXHANDLE hParameter, LPVOID pData, UINT Bytes) PURE;
    STDMETHOD(SetBool)(THIS_ D3DXHANDLE hParameter, BOOL b) PURE;
    STDMETHOD(GetBool)(THIS_ D3DXHANDLE hParameter, BOOL* pb) PURE;
    STDMETHOD(SetBoolArray)(THIS_ D3DXHANDLE hParameter, CONST BOOL* pb, UINT Count) PURE;
    STDMETHOD(GetBoolArray)(THIS_ D3DXHANDLE hParameter, BOOL* pb, UINT Count) PURE;
    STDMETHOD(SetInt)(THIS_ D3DXHANDLE hParameter, INT n) PURE;
    STDMETHOD(GetInt)(THIS_ D3DXHANDLE hParameter, INT* pn) PURE;
    STDMETHOD(SetIntArray)(THIS_ D3DXHANDLE hParameter, CONST INT* pn, UINT Count) PURE;
    STDMETHOD(GetIntArray)(THIS_ D3DXHANDLE hParameter, INT* pn, UINT Count) PURE;
    STDMETHOD(SetFloat)(THIS_ D3DXHANDLE hParameter, FLOAT f) PURE;
    STDMETHOD(GetFloat)(THIS_ D3DXHANDLE hParameter, FLOAT* pf) PURE;
    STDMETHOD(SetFloatArray)(THIS_ D3DXHANDLE hParameter, CONST FLOAT* pf, UINT Count) PURE;
    STDMETHOD(GetFloatArray)(THIS_ D3DXHANDLE hParameter, FLOAT* pf, UINT Count) PURE;
    STDMETHOD(SetVector)(THIS_ D3DXHANDLE hParameter, CONST D3DXVECTOR4* pVector) PURE;
    STDMETHOD(GetVector)(THIS_ D3DXHANDLE hParameter, D3DXVECTOR4* pVector) PURE;
    STDMETHOD(SetVectorArray)(THIS_ D3DXHANDLE hParameter, CONST D3DXVECTOR4* pVector, UINT Count) PURE;
    STDMETHOD(GetVectorArray)(THIS_ D3DXHANDLE hParameter, D3DXVECTOR4* pVector, UINT Count) PURE;
    STDMETHOD(SetMatrix)(THIS_ D3DXHANDLE hParameter, CONST D3DXMATRIX* pMatrix) PURE;
    STDMETHOD(GetMatrix)(THIS_ D3DXHANDLE hParameter, D3DXMATRIX* pMatrix) PURE;
    STDMETHOD(SetMatrixArray)(THIS_ D3DXHANDLE hParameter, CONST D3DXMATRIX* pMatrix, UINT Count) PURE;
    STDMETHOD(GetMatrixArray)(THIS_ D3DXHANDLE hParameter, D3DXMATRIX* pMatrix, UINT Count) PURE;
    STDMETHOD(SetMatrixPointerArray)(THIS_ D3DXHANDLE hParameter, CONST D3DXMATRIX** ppMatrix, UINT Count) PURE;
    STDMETHOD(GetMatrixPointerArray)(THIS_ D3DXHANDLE hParameter, D3DXMATRIX** ppMatrix, UINT Count) PURE;
    STDMETHOD(SetMatrixTranspose)(THIS_ D3DXHANDLE hParameter, CONST D3DXMATRIX* pMatrix) PURE;
    STDMETHOD(GetMatrixTranspose)(THIS_ D3DXHANDLE hParameter, D3DXMATRIX* pMatrix) PURE;
    STDMETHOD(SetMatrixTransposeArray)(THIS_ D3DXHANDLE hParameter, CONST D3DXMATRIX* pMatrix, UINT Count) PURE;
    STDMETHOD(GetMatrixTransposeArray)(THIS_ D3DXHANDLE hParameter, D3DXMATRIX* pMatrix, UINT Count) PURE;
    STDMETHOD(SetMatrixTransposePointerArray)(THIS_ D3DXHANDLE hParameter, CONST D3DXMATRIX** ppMatrix, UINT Count) PURE;
    STDMETHOD(GetMatrixTransposePointerArray)(THIS_ D3DXHANDLE hParameter, D3DXMATRIX** ppMatrix, UINT Count) PURE;
    STDMETHOD(SetString)(THIS_ D3DXHANDLE hParameter, LPCSTR pString) PURE;
    STDMETHOD(GetString)(THIS_ D3DXHANDLE hParameter, LPCSTR* ppString) PURE;
    STDMETHOD(SetTexture)(THIS_ D3DXHANDLE hParameter, LPDIRECT3DBASETEXTURE9 pTexture) PURE;
    STDMETHOD(GetTexture)(THIS_ D3DXHANDLE hParameter, LPDIRECT3DBASETEXTURE9 *ppTexture) PURE;
    STDMETHOD(GetPixelShader)(THIS_ D3DXHANDLE hParameter, LPDIRECT3DPIXELSHADER9 *ppPShader) PURE;
    STDMETHOD(GetVertexShader)(THIS_ D3DXHANDLE hParameter, LPDIRECT3DVERTEXSHADER9 *ppVShader) PURE;

    //Set Range of an Array to pass to device
    //Useful for sending only a subrange of an array down to the device
    STDMETHOD(SetArrayRange)(THIS_ D3DXHANDLE hParameter, UINT uStart, UINT uEnd) PURE; 

};


//----------------------------------------------------------------------------
// ID3DXEffectStateManager:
// ------------------------
// This is a user implemented interface that can be used to manage device 
// state changes made by an Effect.
//----------------------------------------------------------------------------

typedef interface ID3DXEffectStateManager ID3DXEffectStateManager;
typedef interface ID3DXEffectStateManager *LPD3DXEFFECTSTATEMANAGER;

// {79AAB587-6DBC-4fa7-82DE-37FA1781C5CE}
DEFINE_GUID(IID_ID3DXEffectStateManager, 
0x79aab587, 0x6dbc, 0x4fa7, 0x82, 0xde, 0x37, 0xfa, 0x17, 0x81, 0xc5, 0xce);

#undef INTERFACE
#define INTERFACE ID3DXEffectStateManager

DECLARE_INTERFACE_(ID3DXEffectStateManager, IUnknown)
{
    // The user must correctly implement QueryInterface, AddRef, and Release.

    // IUnknown
    STDMETHOD(QueryInterface)(THIS_ REFIID iid, LPVOID *ppv) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

    // The following methods are called by the Effect when it wants to make 
    // the corresponding device call.  Note that:
    // 1. Users manage the state and are therefore responsible for making the 
    //    the corresponding device calls themselves inside their callbacks.  
    // 2. Effects pay attention to the return values of the callbacks, and so 
    //    users must pay attention to what they return in their callbacks.

    STDMETHOD(SetTransform)(THIS_ D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX *pMatrix) PURE;
    STDMETHOD(SetMaterial)(THIS_ CONST D3DMATERIAL9 *pMaterial) PURE;
    STDMETHOD(SetLight)(THIS_ DWORD Index, CONST D3DLIGHT9 *pLight) PURE;
    STDMETHOD(LightEnable)(THIS_ DWORD Index, BOOL Enable) PURE;
    STDMETHOD(SetRenderState)(THIS_ D3DRENDERSTATETYPE State, DWORD Value) PURE;
    STDMETHOD(SetTexture)(THIS_ DWORD Stage, LPDIRECT3DBASETEXTURE9 pTexture) PURE;
    STDMETHOD(SetTextureStageState)(THIS_ DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value) PURE;
    STDMETHOD(SetSamplerState)(THIS_ DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value) PURE;
    STDMETHOD(SetNPatchMode)(THIS_ FLOAT NumSegments) PURE;
    STDMETHOD(SetFVF)(THIS_ DWORD FVF) PURE;
    STDMETHOD(SetVertexShader)(THIS_ LPDIRECT3DVERTEXSHADER9 pShader) PURE;
    STDMETHOD(SetVertexShaderConstantF)(THIS_ UINT RegisterIndex, CONST FLOAT *pConstantData, UINT RegisterCount) PURE;
    STDMETHOD(SetVertexShaderConstantI)(THIS_ UINT RegisterIndex, CONST INT *pConstantData, UINT RegisterCount) PURE;
    STDMETHOD(SetVertexShaderConstantB)(THIS_ UINT RegisterIndex, CONST BOOL *pConstantData, UINT RegisterCount) PURE;
    STDMETHOD(SetPixelShader)(THIS_ LPDIRECT3DPIXELSHADER9 pShader) PURE;
    STDMETHOD(SetPixelShaderConstantF)(THIS_ UINT RegisterIndex, CONST FLOAT *pConstantData, UINT RegisterCount) PURE;
    STDMETHOD(SetPixelShaderConstantI)(THIS_ UINT RegisterIndex, CONST INT *pConstantData, UINT RegisterCount) PURE;
    STDMETHOD(SetPixelShaderConstantB)(THIS_ UINT RegisterIndex, CONST BOOL *pConstantData, UINT RegisterCount) PURE;
};


//////////////////////////////////////////////////////////////////////////////
// ID3DXEffect ///////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

typedef interface ID3DXEffect ID3DXEffect;
typedef interface ID3DXEffect *LPD3DXEFFECT;

// {F6CEB4B3-4E4C-40dd-B883-8D8DE5EA0CD5}
DEFINE_GUID(IID_ID3DXEffect, 
0xf6ceb4b3, 0x4e4c, 0x40dd, 0xb8, 0x83, 0x8d, 0x8d, 0xe5, 0xea, 0xc, 0xd5);

#undef INTERFACE
#define INTERFACE ID3DXEffect

DECLARE_INTERFACE_(ID3DXEffect, ID3DXBaseEffect)
{
    // ID3DXBaseEffect
    STDMETHOD(QueryInterface)(THIS_ REFIID iid, LPVOID *ppv) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

    // Descs
    STDMETHOD(GetDesc)(THIS_ D3DXEFFECT_DESC* pDesc) PURE;
    STDMETHOD(GetParameterDesc)(THIS_ D3DXHANDLE hParameter, D3DXPARAMETER_DESC* pDesc) PURE;
    STDMETHOD(GetTechniqueDesc)(THIS_ D3DXHANDLE hTechnique, D3DXTECHNIQUE_DESC* pDesc) PURE;
    STDMETHOD(GetPassDesc)(THIS_ D3DXHANDLE hPass, D3DXPASS_DESC* pDesc) PURE;
    STDMETHOD(GetFunctionDesc)(THIS_ D3DXHANDLE hShader, D3DXFUNCTION_DESC* pDesc) PURE;

    // Handle operations
    STDMETHOD_(D3DXHANDLE, GetParameter)(THIS_ D3DXHANDLE hParameter, UINT Index) PURE;
    STDMETHOD_(D3DXHANDLE, GetParameterByName)(THIS_ D3DXHANDLE hParameter, LPCSTR pName) PURE;
    STDMETHOD_(D3DXHANDLE, GetParameterBySemantic)(THIS_ D3DXHANDLE hParameter, LPCSTR pSemantic) PURE;
    STDMETHOD_(D3DXHANDLE, GetParameterElement)(THIS_ D3DXHANDLE hParameter, UINT Index) PURE;
    STDMETHOD_(D3DXHANDLE, GetTechnique)(THIS_ UINT Index) PURE;
    STDMETHOD_(D3DXHANDLE, GetTechniqueByName)(THIS_ LPCSTR pName) PURE;
    STDMETHOD_(D3DXHANDLE, GetPass)(THIS_ D3DXHANDLE hTechnique, UINT Index) PURE;
    STDMETHOD_(D3DXHANDLE, GetPassByName)(THIS_ D3DXHANDLE hTechnique, LPCSTR pName) PURE;
    STDMETHOD_(D3DXHANDLE, GetFunction)(THIS_ UINT Index) PURE;
    STDMETHOD_(D3DXHANDLE, GetFunctionByName)(THIS_ LPCSTR pName) PURE;
    STDMETHOD_(D3DXHANDLE, GetAnnotation)(THIS_ D3DXHANDLE hObject, UINT Index) PURE;
    STDMETHOD_(D3DXHANDLE, GetAnnotationByName)(THIS_ D3DXHANDLE hObject, LPCSTR pName) PURE;

    // Get/Set Parameters
    STDMETHOD(SetValue)(THIS_ D3DXHANDLE hParameter, LPCVOID pData, UINT Bytes) PURE;
    STDMETHOD(GetValue)(THIS_ D3DXHANDLE hParameter, LPVOID pData, UINT Bytes) PURE;
    STDMETHOD(SetBool)(THIS_ D3DXHANDLE hParameter, BOOL b) PURE;
    STDMETHOD(GetBool)(THIS_ D3DXHANDLE hParameter, BOOL* pb) PURE;
    STDMETHOD(SetBoolArray)(THIS_ D3DXHANDLE hParameter, CONST BOOL* pb, UINT Count) PURE;
    STDMETHOD(GetBoolArray)(THIS_ D3DXHANDLE hParameter, BOOL* pb, UINT Count) PURE;
    STDMETHOD(SetInt)(THIS_ D3DXHANDLE hParameter, INT n) PURE;
    STDMETHOD(GetInt)(THIS_ D3DXHANDLE hParameter, INT* pn) PURE;
    STDMETHOD(SetIntArray)(THIS_ D3DXHANDLE hParameter, CONST INT* pn, UINT Count) PURE;
    STDMETHOD(GetIntArray)(THIS_ D3DXHANDLE hParameter, INT* pn, UINT Count) PURE;
    STDMETHOD(SetFloat)(THIS_ D3DXHANDLE hParameter, FLOAT f) PURE;
    STDMETHOD(GetFloat)(THIS_ D3DXHANDLE hParameter, FLOAT* pf) PURE;
    STDMETHOD(SetFloatArray)(THIS_ D3DXHANDLE hParameter, CONST FLOAT* pf, UINT Count) PURE;
    STDMETHOD(GetFloatArray)(THIS_ D3DXHANDLE hParameter, FLOAT* pf, UINT Count) PURE;
    STDMETHOD(SetVector)(THIS_ D3DXHANDLE hParameter, CONST D3DXVECTOR4* pVector) PURE;
    STDMETHOD(GetVector)(THIS_ D3DXHANDLE hParameter, D3DXVECTOR4* pVector) PURE;
    STDMETHOD(SetVectorArray)(THIS_ D3DXHANDLE hParameter, CONST D3DXVECTOR4* pVector, UINT Count) PURE;
    STDMETHOD(GetVectorArray)(THIS_ D3DXHANDLE hParameter, D3DXVECTOR4* pVector, UINT Count) PURE;
    STDMETHOD(SetMatrix)(THIS_ D3DXHANDLE hParameter, CONST D3DXMATRIX* pMatrix) PURE;
    STDMETHOD(GetMatrix)(THIS_ D3DXHANDLE hParameter, D3DXMATRIX* pMatrix) PURE;
    STDMETHOD(SetMatrixArray)(THIS_ D3DXHANDLE hParameter, CONST D3DXMATRIX* pMatrix, UINT Count) PURE;
    STDMETHOD(GetMatrixArray)(THIS_ D3DXHANDLE hParameter, D3DXMATRIX* pMatrix, UINT Count) PURE;
    STDMETHOD(SetMatrixPointerArray)(THIS_ D3DXHANDLE hParameter, CONST D3DXMATRIX** ppMatrix, UINT Count) PURE;
    STDMETHOD(GetMatrixPointerArray)(THIS_ D3DXHANDLE hParameter, D3DXMATRIX** ppMatrix, UINT Count) PURE;
    STDMETHOD(SetMatrixTranspose)(THIS_ D3DXHANDLE hParameter, CONST D3DXMATRIX* pMatrix) PURE;
    STDMETHOD(GetMatrixTranspose)(THIS_ D3DXHANDLE hParameter, D3DXMATRIX* pMatrix) PURE;
    STDMETHOD(SetMatrixTransposeArray)(THIS_ D3DXHANDLE hParameter, CONST D3DXMATRIX* pMatrix, UINT Count) PURE;
    STDMETHOD(GetMatrixTransposeArray)(THIS_ D3DXHANDLE hParameter, D3DXMATRIX* pMatrix, UINT Count) PURE;
    STDMETHOD(SetMatrixTransposePointerArray)(THIS_ D3DXHANDLE hParameter, CONST D3DXMATRIX** ppMatrix, UINT Count) PURE;
    STDMETHOD(GetMatrixTransposePointerArray)(THIS_ D3DXHANDLE hParameter, D3DXMATRIX** ppMatrix, UINT Count) PURE;
    STDMETHOD(SetString)(THIS_ D3DXHANDLE hParameter, LPCSTR pString) PURE;
    STDMETHOD(GetString)(THIS_ D3DXHANDLE hParameter, LPCSTR* ppString) PURE;
    STDMETHOD(SetTexture)(THIS_ D3DXHANDLE hParameter, LPDIRECT3DBASETEXTURE9 pTexture) PURE;
    STDMETHOD(GetTexture)(THIS_ D3DXHANDLE hParameter, LPDIRECT3DBASETEXTURE9 *ppTexture) PURE;
    STDMETHOD(GetPixelShader)(THIS_ D3DXHANDLE hParameter, LPDIRECT3DPIXELSHADER9 *ppPShader) PURE;
    STDMETHOD(GetVertexShader)(THIS_ D3DXHANDLE hParameter, LPDIRECT3DVERTEXSHADER9 *ppVShader) PURE;

	//Set Range of an Array to pass to device
	//Usefull for sending only a subrange of an array down to the device
	STDMETHOD(SetArrayRange)(THIS_ D3DXHANDLE hParameter, UINT uStart, UINT uEnd) PURE; 
	// ID3DXBaseEffect
    
    
    // Pool
    STDMETHOD(GetPool)(THIS_ LPD3DXEFFECTPOOL* ppPool) PURE;

    // Selecting and setting a technique
    STDMETHOD(SetTechnique)(THIS_ D3DXHANDLE hTechnique) PURE;
    STDMETHOD_(D3DXHANDLE, GetCurrentTechnique)(THIS) PURE;
    STDMETHOD(ValidateTechnique)(THIS_ D3DXHANDLE hTechnique) PURE;
    STDMETHOD(FindNextValidTechnique)(THIS_ D3DXHANDLE hTechnique, D3DXHANDLE *pTechnique) PURE;
    STDMETHOD_(BOOL, IsParameterUsed)(THIS_ D3DXHANDLE hParameter, D3DXHANDLE hTechnique) PURE;

    // Using current technique
    // Begin           starts active technique
    // BeginPass       begins a pass
    // CommitChanges   updates changes to any set calls in the pass. This should be called before
    //                 any DrawPrimitive call to d3d
    // EndPass         ends a pass
    // End             ends active technique
    STDMETHOD(Begin)(THIS_ UINT *pPasses, DWORD Flags) PURE;
    STDMETHOD(BeginPass)(THIS_ UINT Pass) PURE;
    STDMETHOD(CommitChanges)(THIS) PURE;
    STDMETHOD(EndPass)(THIS) PURE;
    STDMETHOD(End)(THIS) PURE;

    // Managing D3D Device
    STDMETHOD(GetDevice)(THIS_ LPDIRECT3DDEVICE9* ppDevice) PURE;
    STDMETHOD(OnLostDevice)(THIS) PURE;
    STDMETHOD(OnResetDevice)(THIS) PURE;

    // Logging device calls
    STDMETHOD(SetStateManager)(THIS_ LPD3DXEFFECTSTATEMANAGER pManager) PURE;
    STDMETHOD(GetStateManager)(THIS_ LPD3DXEFFECTSTATEMANAGER *ppManager) PURE;

    // Parameter blocks
    STDMETHOD(BeginParameterBlock)(THIS) PURE;
    STDMETHOD_(D3DXHANDLE, EndParameterBlock)(THIS) PURE;
    STDMETHOD(ApplyParameterBlock)(THIS_ D3DXHANDLE hParameterBlock) PURE;
    STDMETHOD(DeleteParameterBlock)(THIS_ D3DXHANDLE hParameterBlock) PURE;

    // Cloning
    STDMETHOD(CloneEffect)(THIS_ LPDIRECT3DDEVICE9 pDevice, LPD3DXEFFECT* ppEffect) PURE;
    
    // Fast path for setting variables directly in ID3DXEffect
    STDMETHOD(SetRawValue)(THIS_ D3DXHANDLE hParameter, LPCVOID pData, UINT ByteOffset, UINT Bytes) PURE;
};


//////////////////////////////////////////////////////////////////////////////
// ID3DXEffectCompiler ///////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

typedef interface ID3DXEffectCompiler ID3DXEffectCompiler;
typedef interface ID3DXEffectCompiler *LPD3DXEFFECTCOMPILER;

// {51B8A949-1A31-47e6-BEA0-4B30DB53F1E0}
DEFINE_GUID(IID_ID3DXEffectCompiler, 
0x51b8a949, 0x1a31, 0x47e6, 0xbe, 0xa0, 0x4b, 0x30, 0xdb, 0x53, 0xf1, 0xe0);


#undef INTERFACE
#define INTERFACE ID3DXEffectCompiler

DECLARE_INTERFACE_(ID3DXEffectCompiler, ID3DXBaseEffect)
{
    // ID3DXBaseEffect
    STDMETHOD(QueryInterface)(THIS_ REFIID iid, LPVOID *ppv) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

    // Descs
    STDMETHOD(GetDesc)(THIS_ D3DXEFFECT_DESC* pDesc) PURE;
    STDMETHOD(GetParameterDesc)(THIS_ D3DXHANDLE hParameter, D3DXPARAMETER_DESC* pDesc) PURE;
    STDMETHOD(GetTechniqueDesc)(THIS_ D3DXHANDLE hTechnique, D3DXTECHNIQUE_DESC* pDesc) PURE;
    STDMETHOD(GetPassDesc)(THIS_ D3DXHANDLE hPass, D3DXPASS_DESC* pDesc) PURE;
    STDMETHOD(GetFunctionDesc)(THIS_ D3DXHANDLE hShader, D3DXFUNCTION_DESC* pDesc) PURE;

    // Handle operations
    STDMETHOD_(D3DXHANDLE, GetParameter)(THIS_ D3DXHANDLE hParameter, UINT Index) PURE;
    STDMETHOD_(D3DXHANDLE, GetParameterByName)(THIS_ D3DXHANDLE hParameter, LPCSTR pName) PURE;
    STDMETHOD_(D3DXHANDLE, GetParameterBySemantic)(THIS_ D3DXHANDLE hParameter, LPCSTR pSemantic) PURE;
    STDMETHOD_(D3DXHANDLE, GetParameterElement)(THIS_ D3DXHANDLE hParameter, UINT Index) PURE;
    STDMETHOD_(D3DXHANDLE, GetTechnique)(THIS_ UINT Index) PURE;
    STDMETHOD_(D3DXHANDLE, GetTechniqueByName)(THIS_ LPCSTR pName) PURE;
    STDMETHOD_(D3DXHANDLE, GetPass)(THIS_ D3DXHANDLE hTechnique, UINT Index) PURE;
    STDMETHOD_(D3DXHANDLE, GetPassByName)(THIS_ D3DXHANDLE hTechnique, LPCSTR pName) PURE;
    STDMETHOD_(D3DXHANDLE, GetFunction)(THIS_ UINT Index) PURE;
    STDMETHOD_(D3DXHANDLE, GetFunctionByName)(THIS_ LPCSTR pName) PURE;
    STDMETHOD_(D3DXHANDLE, GetAnnotation)(THIS_ D3DXHANDLE hObject, UINT Index) PURE;
    STDMETHOD_(D3DXHANDLE, GetAnnotationByName)(THIS_ D3DXHANDLE hObject, LPCSTR pName) PURE;

    // Get/Set Parameters
    STDMETHOD(SetValue)(THIS_ D3DXHANDLE hParameter, LPCVOID pData, UINT Bytes) PURE;
    STDMETHOD(GetValue)(THIS_ D3DXHANDLE hParameter, LPVOID pData, UINT Bytes) PURE;
    STDMETHOD(SetBool)(THIS_ D3DXHANDLE hParameter, BOOL b) PURE;
    STDMETHOD(GetBool)(THIS_ D3DXHANDLE hParameter, BOOL* pb) PURE;
    STDMETHOD(SetBoolArray)(THIS_ D3DXHANDLE hParameter, CONST BOOL* pb, UINT Count) PURE;
    STDMETHOD(GetBoolArray)(THIS_ D3DXHANDLE hParameter, BOOL* pb, UINT Count) PURE;
    STDMETHOD(SetInt)(THIS_ D3DXHANDLE hParameter, INT n) PURE;
    STDMETHOD(GetInt)(THIS_ D3DXHANDLE hParameter, INT* pn) PURE;
    STDMETHOD(SetIntArray)(THIS_ D3DXHANDLE hParameter, CONST INT* pn, UINT Count) PURE;
    STDMETHOD(GetIntArray)(THIS_ D3DXHANDLE hParameter, INT* pn, UINT Count) PURE;
    STDMETHOD(SetFloat)(THIS_ D3DXHANDLE hParameter, FLOAT f) PURE;
    STDMETHOD(GetFloat)(THIS_ D3DXHANDLE hParameter, FLOAT* pf) PURE;
    STDMETHOD(SetFloatArray)(THIS_ D3DXHANDLE hParameter, CONST FLOAT* pf, UINT Count) PURE;
    STDMETHOD(GetFloatArray)(THIS_ D3DXHANDLE hParameter, FLOAT* pf, UINT Count) PURE;
    STDMETHOD(SetVector)(THIS_ D3DXHANDLE hParameter, CONST D3DXVECTOR4* pVector) PURE;
    STDMETHOD(GetVector)(THIS_ D3DXHANDLE hParameter, D3DXVECTOR4* pVector) PURE;
    STDMETHOD(SetVectorArray)(THIS_ D3DXHANDLE hParameter, CONST D3DXVECTOR4* pVector, UINT Count) PURE;
    STDMETHOD(GetVectorArray)(THIS_ D3DXHANDLE hParameter, D3DXVECTOR4* pVector, UINT Count) PURE;
    STDMETHOD(SetMatrix)(THIS_ D3DXHANDLE hParameter, CONST D3DXMATRIX* pMatrix) PURE;
    STDMETHOD(GetMatrix)(THIS_ D3DXHANDLE hParameter, D3DXMATRIX* pMatrix) PURE;
    STDMETHOD(SetMatrixArray)(THIS_ D3DXHANDLE hParameter, CONST D3DXMATRIX* pMatrix, UINT Count) PURE;
    STDMETHOD(GetMatrixArray)(THIS_ D3DXHANDLE hParameter, D3DXMATRIX* pMatrix, UINT Count) PURE;
    STDMETHOD(SetMatrixPointerArray)(THIS_ D3DXHANDLE hParameter, CONST D3DXMATRIX** ppMatrix, UINT Count) PURE;
    STDMETHOD(GetMatrixPointerArray)(THIS_ D3DXHANDLE hParameter, D3DXMATRIX** ppMatrix, UINT Count) PURE;
    STDMETHOD(SetMatrixTranspose)(THIS_ D3DXHANDLE hParameter, CONST D3DXMATRIX* pMatrix) PURE;
    STDMETHOD(GetMatrixTranspose)(THIS_ D3DXHANDLE hParameter, D3DXMATRIX* pMatrix) PURE;
    STDMETHOD(SetMatrixTransposeArray)(THIS_ D3DXHANDLE hParameter, CONST D3DXMATRIX* pMatrix, UINT Count) PURE;
    STDMETHOD(GetMatrixTransposeArray)(THIS_ D3DXHANDLE hParameter, D3DXMATRIX* pMatrix, UINT Count) PURE;
    STDMETHOD(SetMatrixTransposePointerArray)(THIS_ D3DXHANDLE hParameter, CONST D3DXMATRIX** ppMatrix, UINT Count) PURE;
    STDMETHOD(GetMatrixTransposePointerArray)(THIS_ D3DXHANDLE hParameter, D3DXMATRIX** ppMatrix, UINT Count) PURE;
    STDMETHOD(SetString)(THIS_ D3DXHANDLE hParameter, LPCSTR pString) PURE;
    STDMETHOD(GetString)(THIS_ D3DXHANDLE hParameter, LPCSTR* ppString) PURE;
    STDMETHOD(SetTexture)(THIS_ D3DXHANDLE hParameter, LPDIRECT3DBASETEXTURE9 pTexture) PURE;
    STDMETHOD(GetTexture)(THIS_ D3DXHANDLE hParameter, LPDIRECT3DBASETEXTURE9 *ppTexture) PURE;
    STDMETHOD(GetPixelShader)(THIS_ D3DXHANDLE hParameter, LPDIRECT3DPIXELSHADER9 *ppPShader) PURE;
    STDMETHOD(GetVertexShader)(THIS_ D3DXHANDLE hParameter, LPDIRECT3DVERTEXSHADER9 *ppVShader) PURE;
    
	//Set Range of an Array to pass to device
	//Usefull for sending only a subrange of an array down to the device
	STDMETHOD(SetArrayRange)(THIS_ D3DXHANDLE hParameter, UINT uStart, UINT uEnd) PURE; 
	// ID3DXBaseEffect

    // Parameter sharing, specialization, and information
    STDMETHOD(SetLiteral)(THIS_ D3DXHANDLE hParameter, BOOL Literal) PURE;
    STDMETHOD(GetLiteral)(THIS_ D3DXHANDLE hParameter, BOOL *pLiteral) PURE;

    // Compilation
    STDMETHOD(CompileEffect)(THIS_ DWORD Flags,
        LPD3DXBUFFER* ppEffect, LPD3DXBUFFER* ppErrorMsgs) PURE;

    STDMETHOD(CompileShader)(THIS_ D3DXHANDLE hFunction, LPCSTR pTarget, DWORD Flags,
        LPD3DXBUFFER* ppShader, LPD3DXBUFFER* ppErrorMsgs, LPD3DXCONSTANTTABLE* ppConstantTable) PURE;
};


//////////////////////////////////////////////////////////////////////////////
// APIs //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

//----------------------------------------------------------------------------
// D3DXCreateEffectPool:
// ---------------------
// Creates an effect pool.  Pools are used for sharing parameters between
// multiple effects.  For all effects within a pool, shared parameters of the
// same name all share the same value.
//
// Parameters:
//  ppPool
//      Returns the created pool.
//----------------------------------------------------------------------------

HRESULT WINAPI
    D3DXCreateEffectPool(
        LPD3DXEFFECTPOOL*               ppPool);


//----------------------------------------------------------------------------
// D3DXCreateEffect:
// -----------------
// Creates an effect from an ascii or binary effect description.
//
// Parameters:
//  pDevice
//      Pointer of the device on which to create the effect
//  pSrcFile
//      Name of the file containing the effect description
//  hSrcModule
//      Module handle. if NULL, current module will be used.
//  pSrcResource
//      Resource name in module
//  pSrcData
//      Pointer to effect description
//  SrcDataSize
//      Size of the effect description in bytes
//  pDefines
//      Optional NULL-terminated array of preprocessor macro definitions.
//  Flags
//      See D3DXSHADER_xxx flags.
//  pSkipConstants
//      A list of semi-colon delimited variable names.  The effect will
//      not set these variables to the device when they are referenced
//      by a shader.  NOTE: the variables specified here must be
//      register bound in the file and must not be used in expressions
//      in passes or samplers or the file will not load.
//  pInclude
//      Optional interface pointer to use for handling #include directives.
//      If this parameter is NULL, #includes will be honored when compiling
//      from file, and will error when compiling from resource or memory.
//  pPool
//      Pointer to ID3DXEffectPool object to use for shared parameters.
//      If NULL, no parameters will be shared.
//  ppEffect
//      Returns a buffer containing created effect.
//  ppCompilationErrors
//      Returns a buffer containing any error messages which occurred during
//      compile.  Or NULL if you do not care about the error messages.
//
//----------------------------------------------------------------------------

HRESULT WINAPI
    D3DXCreateEffectFromFileA(
        LPDIRECT3DDEVICE9               pDevice,
        LPCSTR                          pSrcFile,
        CONST D3DXMACRO*                pDefines,
        LPD3DXINCLUDE                   pInclude,
        DWORD                           Flags,
        LPD3DXEFFECTPOOL                pPool,
        LPD3DXEFFECT*                   ppEffect,
        LPD3DXBUFFER*                   ppCompilationErrors);

HRESULT WINAPI
    D3DXCreateEffectFromFileW(
        LPDIRECT3DDEVICE9               pDevice,
        LPCWSTR                         pSrcFile,
        CONST D3DXMACRO*                pDefines,
        LPD3DXINCLUDE                   pInclude,
        DWORD                           Flags,
        LPD3DXEFFECTPOOL                pPool,
        LPD3DXEFFECT*                   ppEffect,
        LPD3DXBUFFER*                   ppCompilationErrors);

#ifdef UNICODE
#define D3DXCreateEffectFromFile D3DXCreateEffectFromFileW
#else
#define D3DXCreateEffectFromFile D3DXCreateEffectFromFileA
#endif


HRESULT WINAPI
    D3DXCreateEffectFromResourceA(
        LPDIRECT3DDEVICE9               pDevice,
        HMODULE                         hSrcModule,
        LPCSTR                          pSrcResource,
        CONST D3DXMACRO*                pDefines,
        LPD3DXINCLUDE                   pInclude,
        DWORD                           Flags,
        LPD3DXEFFECTPOOL                pPool,
        LPD3DXEFFECT*                   ppEffect,
        LPD3DXBUFFER*                   ppCompilationErrors);

HRESULT WINAPI
    D3DXCreateEffectFromResourceW(
        LPDIRECT3DDEVICE9               pDevice,
        HMODULE                         hSrcModule,
        LPCWSTR                         pSrcResource,
        CONST D3DXMACRO*                pDefines,
        LPD3DXINCLUDE                   pInclude,
        DWORD                           Flags,
        LPD3DXEFFECTPOOL                pPool,
        LPD3DXEFFECT*                   ppEffect,
        LPD3DXBUFFER*                   ppCompilationErrors);

#ifdef UNICODE
#define D3DXCreateEffectFromResource D3DXCreateEffectFromResourceW
#else
#define D3DXCreateEffectFromResource D3DXCreateEffectFromResourceA
#endif


HRESULT WINAPI
    D3DXCreateEffect(
        LPDIRECT3DDEVICE9               pDevice,
        LPCVOID                         pSrcData,
        UINT                            SrcDataLen,
        CONST D3DXMACRO*                pDefines,
        LPD3DXINCLUDE                   pInclude,
        DWORD                           Flags,
        LPD3DXEFFECTPOOL                pPool,
        LPD3DXEFFECT*                   ppEffect,
        LPD3DXBUFFER*                   ppCompilationErrors);

//
// Ex functions that accept pSkipConstants in addition to other parameters
//

HRESULT WINAPI
    D3DXCreateEffectFromFileExA(
        LPDIRECT3DDEVICE9               pDevice,
        LPCSTR                          pSrcFile,
        CONST D3DXMACRO*                pDefines,
        LPD3DXINCLUDE                   pInclude,
        LPCSTR                          pSkipConstants, 
        DWORD                           Flags,
        LPD3DXEFFECTPOOL                pPool,
        LPD3DXEFFECT*                   ppEffect,
        LPD3DXBUFFER*                   ppCompilationErrors);

HRESULT WINAPI
    D3DXCreateEffectFromFileExW(
        LPDIRECT3DDEVICE9               pDevice,
        LPCWSTR                         pSrcFile,
        CONST D3DXMACRO*                pDefines,
        LPD3DXINCLUDE                   pInclude,
        LPCSTR                          pSkipConstants, 
        DWORD                           Flags,
        LPD3DXEFFECTPOOL                pPool,
        LPD3DXEFFECT*                   ppEffect,
        LPD3DXBUFFER*                   ppCompilationErrors);

#ifdef UNICODE
#define D3DXCreateEffectFromFileEx D3DXCreateEffectFromFileExW
#else
#define D3DXCreateEffectFromFileEx D3DXCreateEffectFromFileExA
#endif


HRESULT WINAPI
    D3DXCreateEffectFromResourceExA(
        LPDIRECT3DDEVICE9               pDevice,
        HMODULE                         hSrcModule,
        LPCSTR                          pSrcResource,
        CONST D3DXMACRO*                pDefines,
        LPD3DXINCLUDE                   pInclude,
        LPCSTR                          pSkipConstants, 
        DWORD                           Flags,
        LPD3DXEFFECTPOOL                pPool,
        LPD3DXEFFECT*                   ppEffect,
        LPD3DXBUFFER*                   ppCompilationErrors);

HRESULT WINAPI
    D3DXCreateEffectFromResourceExW(
        LPDIRECT3DDEVICE9               pDevice,
        HMODULE                         hSrcModule,
        LPCWSTR                         pSrcResource,
        CONST D3DXMACRO*                pDefines,
        LPD3DXINCLUDE                   pInclude,
        LPCSTR                          pSkipConstants, 
        DWORD                           Flags,
        LPD3DXEFFECTPOOL                pPool,
        LPD3DXEFFECT*                   ppEffect,
        LPD3DXBUFFER*                   ppCompilationErrors);

#ifdef UNICODE
#define D3DXCreateEffectFromResourceEx D3DXCreateEffectFromResourceExW
#else
#define D3DXCreateEffectFromResourceEx D3DXCreateEffectFromResourceExA
#endif


HRESULT WINAPI
    D3DXCreateEffectEx(
        LPDIRECT3DDEVICE9               pDevice,
        LPCVOID                         pSrcData,
        UINT                            SrcDataLen,
        CONST D3DXMACRO*                pDefines,
        LPD3DXINCLUDE                   pInclude,
        LPCSTR                          pSkipConstants, 
        DWORD                           Flags,
        LPD3DXEFFECTPOOL                pPool,
        LPD3DXEFFECT*                   ppEffect,
        LPD3DXBUFFER*                   ppCompilationErrors);

//----------------------------------------------------------------------------
// D3DXCreateEffectCompiler:
// -------------------------
// Creates an effect from an ascii or binary effect description.
//
// Parameters:
//  pSrcFile
//      Name of the file containing the effect description
//  hSrcModule
//      Module handle. if NULL, current module will be used.
//  pSrcResource
//      Resource name in module
//  pSrcData
//      Pointer to effect description
//  SrcDataSize
//      Size of the effect description in bytes
//  pDefines
//      Optional NULL-terminated array of preprocessor macro definitions.
//  pInclude
//      Optional interface pointer to use for handling #include directives.
//      If this parameter is NULL, #includes will be honored when compiling
//      from file, and will error when compiling from resource or memory.
//  pPool
//      Pointer to ID3DXEffectPool object to use for shared parameters.
//      If NULL, no parameters will be shared.
//  ppCompiler
//      Returns a buffer containing created effect compiler.
//  ppParseErrors
//      Returns a buffer containing any error messages which occurred during
//      parse.  Or NULL if you do not care about the error messages.
//
//----------------------------------------------------------------------------

HRESULT WINAPI
    D3DXCreateEffectCompilerFromFileA(
        LPCSTR                          pSrcFile,
        CONST D3DXMACRO*                pDefines,
        LPD3DXINCLUDE                   pInclude,
        DWORD                           Flags,
        LPD3DXEFFECTCOMPILER*           ppCompiler,
        LPD3DXBUFFER*                   ppParseErrors);

HRESULT WINAPI
    D3DXCreateEffectCompilerFromFileW(
        LPCWSTR                         pSrcFile,
        CONST D3DXMACRO*                pDefines,
        LPD3DXINCLUDE                   pInclude,
        DWORD                           Flags,
        LPD3DXEFFECTCOMPILER*           ppCompiler,
        LPD3DXBUFFER*                   ppParseErrors);

#ifdef UNICODE
#define D3DXCreateEffectCompilerFromFile D3DXCreateEffectCompilerFromFileW
#else
#define D3DXCreateEffectCompilerFromFile D3DXCreateEffectCompilerFromFileA
#endif


HRESULT WINAPI
    D3DXCreateEffectCompilerFromResourceA(
        HMODULE                         hSrcModule,
        LPCSTR                          pSrcResource,
        CONST D3DXMACRO*                pDefines,
        LPD3DXINCLUDE                   pInclude,
        DWORD                           Flags,
        LPD3DXEFFECTCOMPILER*           ppCompiler,
        LPD3DXBUFFER*                   ppParseErrors);

HRESULT WINAPI
    D3DXCreateEffectCompilerFromResourceW(
        HMODULE                         hSrcModule,
        LPCWSTR                         pSrcResource,
        CONST D3DXMACRO*                pDefines,
        LPD3DXINCLUDE                   pInclude,
        DWORD                           Flags,
        LPD3DXEFFECTCOMPILER*           ppCompiler,
        LPD3DXBUFFER*                   ppParseErrors);

#ifdef UNICODE
#define D3DXCreateEffectCompilerFromResource D3DXCreateEffectCompilerFromResourceW
#else
#define D3DXCreateEffectCompilerFromResource D3DXCreateEffectCompilerFromResourceA
#endif


HRESULT WINAPI
    D3DXCreateEffectCompiler(
        LPCSTR                          pSrcData,
        UINT                            SrcDataLen,
        CONST D3DXMACRO*                pDefines,
        LPD3DXINCLUDE                   pInclude,
        DWORD                           Flags,
        LPD3DXEFFECTCOMPILER*           ppCompiler,
        LPD3DXBUFFER*                   ppParseErrors);

//----------------------------------------------------------------------------
// D3DXDisassembleEffect:
// -----------------------
//
// Parameters:
//----------------------------------------------------------------------------

HRESULT WINAPI 
    D3DXDisassembleEffect(
        LPD3DXEFFECT pEffect, 
        BOOL EnableColorCode, 
        LPD3DXBUFFER *ppDisassembly);
        


#ifdef __cplusplus
}
#endif //__cplusplus

#endif //__D3DX9EFFECT_H__


