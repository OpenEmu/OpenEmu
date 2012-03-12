//////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
//  File:       d3dx9mesh.h
//  Content:    D3DX mesh types and functions
//
//////////////////////////////////////////////////////////////////////////////

#include "d3dx9.h"

#ifndef __D3DX9MESH_H__
#define __D3DX9MESH_H__

// {7ED943DD-52E8-40b5-A8D8-76685C406330}
DEFINE_GUID(IID_ID3DXBaseMesh, 
0x7ed943dd, 0x52e8, 0x40b5, 0xa8, 0xd8, 0x76, 0x68, 0x5c, 0x40, 0x63, 0x30);

// {4020E5C2-1403-4929-883F-E2E849FAC195}
DEFINE_GUID(IID_ID3DXMesh, 
0x4020e5c2, 0x1403, 0x4929, 0x88, 0x3f, 0xe2, 0xe8, 0x49, 0xfa, 0xc1, 0x95);

// {8875769A-D579-4088-AAEB-534D1AD84E96}
DEFINE_GUID(IID_ID3DXPMesh, 
0x8875769a, 0xd579, 0x4088, 0xaa, 0xeb, 0x53, 0x4d, 0x1a, 0xd8, 0x4e, 0x96);

// {667EA4C7-F1CD-4386-B523-7C0290B83CC5}
DEFINE_GUID(IID_ID3DXSPMesh, 
0x667ea4c7, 0xf1cd, 0x4386, 0xb5, 0x23, 0x7c, 0x2, 0x90, 0xb8, 0x3c, 0xc5);

// {11EAA540-F9A6-4d49-AE6A-E19221F70CC4}
DEFINE_GUID(IID_ID3DXSkinInfo, 
0x11eaa540, 0xf9a6, 0x4d49, 0xae, 0x6a, 0xe1, 0x92, 0x21, 0xf7, 0xc, 0xc4);

// {3CE6CC22-DBF2-44f4-894D-F9C34A337139}
DEFINE_GUID(IID_ID3DXPatchMesh, 
0x3ce6cc22, 0xdbf2, 0x44f4, 0x89, 0x4d, 0xf9, 0xc3, 0x4a, 0x33, 0x71, 0x39);

//patch mesh can be quads or tris
typedef enum _D3DXPATCHMESHTYPE {
    D3DXPATCHMESH_RECT   = 0x001,
    D3DXPATCHMESH_TRI    = 0x002,
    D3DXPATCHMESH_NPATCH = 0x003,

    D3DXPATCHMESH_FORCE_DWORD    = 0x7fffffff /* force 32-bit size enum */
} D3DXPATCHMESHTYPE;

// Mesh options - lower 3 bytes only, upper byte used by _D3DXMESHOPT option flags
enum _D3DXMESH {
    D3DXMESH_32BIT                  = 0x001, // If set, then use 32 bit indices, if not set use 16 bit indices.
    D3DXMESH_DONOTCLIP              = 0x002, // Use D3DUSAGE_DONOTCLIP for VB & IB.
    D3DXMESH_POINTS                 = 0x004, // Use D3DUSAGE_POINTS for VB & IB. 
    D3DXMESH_RTPATCHES              = 0x008, // Use D3DUSAGE_RTPATCHES for VB & IB. 
    D3DXMESH_NPATCHES               = 0x4000,// Use D3DUSAGE_NPATCHES for VB & IB. 
    D3DXMESH_VB_SYSTEMMEM           = 0x010, // Use D3DPOOL_SYSTEMMEM for VB. Overrides D3DXMESH_MANAGEDVERTEXBUFFER
    D3DXMESH_VB_MANAGED             = 0x020, // Use D3DPOOL_MANAGED for VB. 
    D3DXMESH_VB_WRITEONLY           = 0x040, // Use D3DUSAGE_WRITEONLY for VB.
    D3DXMESH_VB_DYNAMIC             = 0x080, // Use D3DUSAGE_DYNAMIC for VB.
    D3DXMESH_VB_SOFTWAREPROCESSING = 0x8000, // Use D3DUSAGE_SOFTWAREPROCESSING for VB.
    D3DXMESH_IB_SYSTEMMEM           = 0x100, // Use D3DPOOL_SYSTEMMEM for IB. Overrides D3DXMESH_MANAGEDINDEXBUFFER
    D3DXMESH_IB_MANAGED             = 0x200, // Use D3DPOOL_MANAGED for IB.
    D3DXMESH_IB_WRITEONLY           = 0x400, // Use D3DUSAGE_WRITEONLY for IB.
    D3DXMESH_IB_DYNAMIC             = 0x800, // Use D3DUSAGE_DYNAMIC for IB.
    D3DXMESH_IB_SOFTWAREPROCESSING= 0x10000, // Use D3DUSAGE_SOFTWAREPROCESSING for IB.

    D3DXMESH_VB_SHARE               = 0x1000, // Valid for Clone* calls only, forces cloned mesh/pmesh to share vertex buffer

    D3DXMESH_USEHWONLY              = 0x2000, // Valid for ID3DXSkinInfo::ConvertToBlendedMesh

    // Helper options
    D3DXMESH_SYSTEMMEM              = 0x110, // D3DXMESH_VB_SYSTEMMEM | D3DXMESH_IB_SYSTEMMEM
    D3DXMESH_MANAGED                = 0x220, // D3DXMESH_VB_MANAGED | D3DXMESH_IB_MANAGED
    D3DXMESH_WRITEONLY              = 0x440, // D3DXMESH_VB_WRITEONLY | D3DXMESH_IB_WRITEONLY
    D3DXMESH_DYNAMIC                = 0x880, // D3DXMESH_VB_DYNAMIC | D3DXMESH_IB_DYNAMIC
    D3DXMESH_SOFTWAREPROCESSING   = 0x18000 // D3DXMESH_VB_SOFTWAREPROCESSING | D3DXMESH_IB_SOFTWAREPROCESSING

};

//patch mesh options
enum _D3DXPATCHMESH {
    D3DXPATCHMESH_DEFAULT = 000
};
// option field values for specifying min value in D3DXGeneratePMesh and D3DXSimplifyMesh
enum _D3DXMESHSIMP
{
    D3DXMESHSIMP_VERTEX   = 0x1,
    D3DXMESHSIMP_FACE     = 0x2

};

typedef enum _D3DXCLEANTYPE {
	D3DXCLEAN_BACKFACING	= 0x00000001,
	D3DXCLEAN_BOWTIES		= 0x00000002,
	
	// Helper options
	D3DXCLEAN_SKINNING		= D3DXCLEAN_BACKFACING,	// Bowtie cleaning modifies geometry and breaks skinning
	D3DXCLEAN_OPTIMIZATION	= D3DXCLEAN_BACKFACING,
	D3DXCLEAN_SIMPLIFICATION= D3DXCLEAN_BACKFACING | D3DXCLEAN_BOWTIES	
} D3DXCLEANTYPE;

enum _MAX_FVF_DECL_SIZE
{
    MAX_FVF_DECL_SIZE = MAXD3DDECLLENGTH + 1 // +1 for END
};

typedef enum _D3DXTANGENT
{
    D3DXTANGENT_WRAP_U =                    0x01,
    D3DXTANGENT_WRAP_V =                    0x02,
    D3DXTANGENT_WRAP_UV =                   0x03,
    D3DXTANGENT_DONT_NORMALIZE_PARTIALS =   0x04,
    D3DXTANGENT_DONT_ORTHOGONALIZE =        0x08,
    D3DXTANGENT_ORTHOGONALIZE_FROM_V =      0x010,
    D3DXTANGENT_ORTHOGONALIZE_FROM_U =      0x020,
    D3DXTANGENT_WEIGHT_BY_AREA =            0x040,
    D3DXTANGENT_WEIGHT_EQUAL =              0x080,
    D3DXTANGENT_WIND_CW =                   0x0100,
    D3DXTANGENT_CALCULATE_NORMALS =         0x0200,
    D3DXTANGENT_GENERATE_IN_PLACE =         0x0400
} D3DXTANGENT;

// D3DXIMT_WRAP_U means the texture wraps in the U direction
// D3DXIMT_WRAP_V means the texture wraps in the V direction
// D3DXIMT_WRAP_UV means the texture wraps in both directions
typedef enum _D3DXIMT
{
    D3DXIMT_WRAP_U =                    0x01,
    D3DXIMT_WRAP_V =                    0x02,
    D3DXIMT_WRAP_UV =                   0x03
} D3DXIMT;

// These options are only valid for UVAtlasCreate and UVAtlasPartition, we may add more for UVAtlasPack if necessary
// D3DXUVATLAS_DEFAULT - Meshes with more than 25k faces go through fast, meshes with fewer than 25k faces go through quality
// D3DXUVATLAS_GEODESIC_FAST - Uses approximations to improve charting speed at the cost of added stretch or more charts.
// D3DXUVATLAS_GEODESIC_QUALITY - Provides better quality charts, but requires more time and memory than fast.
typedef enum _D3DXUVATLAS
{
    D3DXUVATLAS_DEFAULT               = 0x00,
    D3DXUVATLAS_GEODESIC_FAST         = 0x01,
    D3DXUVATLAS_GEODESIC_QUALITY      = 0x02
} D3DXUVATLAS;

typedef struct ID3DXBaseMesh *LPD3DXBASEMESH;
typedef struct ID3DXMesh *LPD3DXMESH;
typedef struct ID3DXPMesh *LPD3DXPMESH;
typedef struct ID3DXSPMesh *LPD3DXSPMESH;
typedef struct ID3DXSkinInfo *LPD3DXSKININFO;
typedef struct ID3DXPatchMesh *LPD3DXPATCHMESH;
typedef interface ID3DXTextureGutterHelper *LPD3DXTEXTUREGUTTERHELPER;
typedef interface ID3DXPRTBuffer *LPD3DXPRTBUFFER;


typedef struct _D3DXATTRIBUTERANGE
{
    DWORD AttribId;
    DWORD FaceStart;
    DWORD FaceCount;
    DWORD VertexStart;
    DWORD VertexCount;
} D3DXATTRIBUTERANGE;

typedef D3DXATTRIBUTERANGE* LPD3DXATTRIBUTERANGE;

typedef struct _D3DXMATERIAL
{
    D3DMATERIAL9  MatD3D;
    LPSTR         pTextureFilename;
} D3DXMATERIAL;
typedef D3DXMATERIAL *LPD3DXMATERIAL;

typedef enum _D3DXEFFECTDEFAULTTYPE
{
    D3DXEDT_STRING = 0x1,       // pValue points to a null terminated ASCII string 
    D3DXEDT_FLOATS = 0x2,       // pValue points to an array of floats - number of floats is NumBytes / sizeof(float)
    D3DXEDT_DWORD  = 0x3,       // pValue points to a DWORD

    D3DXEDT_FORCEDWORD = 0x7fffffff
} D3DXEFFECTDEFAULTTYPE;

typedef struct _D3DXEFFECTDEFAULT
{
    LPSTR                 pParamName;
    D3DXEFFECTDEFAULTTYPE Type;           // type of the data pointed to by pValue
    DWORD                 NumBytes;       // size in bytes of the data pointed to by pValue
    LPVOID                pValue;         // data for the default of the effect
} D3DXEFFECTDEFAULT, *LPD3DXEFFECTDEFAULT;

typedef struct _D3DXEFFECTINSTANCE
{
    LPSTR               pEffectFilename;
    DWORD               NumDefaults;
    LPD3DXEFFECTDEFAULT pDefaults;
} D3DXEFFECTINSTANCE, *LPD3DXEFFECTINSTANCE;

typedef struct _D3DXATTRIBUTEWEIGHTS
{
    FLOAT Position;
    FLOAT Boundary;
    FLOAT Normal;
    FLOAT Diffuse;
    FLOAT Specular;
    FLOAT Texcoord[8];
    FLOAT Tangent;
    FLOAT Binormal;
} D3DXATTRIBUTEWEIGHTS, *LPD3DXATTRIBUTEWEIGHTS;

enum _D3DXWELDEPSILONSFLAGS
{
    D3DXWELDEPSILONS_WELDALL             = 0x1,  // weld all vertices marked by adjacency as being overlapping

    D3DXWELDEPSILONS_WELDPARTIALMATCHES  = 0x2,  // if a given vertex component is within epsilon, modify partial matched 
                                                    // vertices so that both components identical AND if all components "equal"
                                                    // remove one of the vertices
    D3DXWELDEPSILONS_DONOTREMOVEVERTICES = 0x4,  // instructs weld to only allow modifications to vertices and not removal
                                                    // ONLY valid if D3DXWELDEPSILONS_WELDPARTIALMATCHES is set
                                                    // useful to modify vertices to be equal, but not allow vertices to be removed

    D3DXWELDEPSILONS_DONOTSPLIT          = 0x8  // instructs weld to specify the D3DXMESHOPT_DONOTSPLIT flag when doing an Optimize(ATTR_SORT)
                                                    // if this flag is not set, all vertices that are in separate attribute groups
                                                    // will remain split and not welded.  Setting this flag can slow down software vertex processing

};

typedef struct _D3DXWELDEPSILONS
{
    FLOAT Position;                 // NOTE: This does NOT replace the epsilon in GenerateAdjacency
                                            // in general, it should be the same value or greater than the one passed to GeneratedAdjacency
    FLOAT BlendWeights;
    FLOAT Normal;
    FLOAT PSize;
    FLOAT Specular;
    FLOAT Diffuse;
    FLOAT Texcoord[8];
    FLOAT Tangent;
    FLOAT Binormal;
    FLOAT TessFactor;
} D3DXWELDEPSILONS;

typedef D3DXWELDEPSILONS* LPD3DXWELDEPSILONS;


#undef INTERFACE
#define INTERFACE ID3DXBaseMesh

DECLARE_INTERFACE_(ID3DXBaseMesh, IUnknown)
{
    // IUnknown
    STDMETHOD(QueryInterface)(THIS_ REFIID iid, LPVOID *ppv) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

    // ID3DXBaseMesh
    STDMETHOD(DrawSubset)(THIS_ DWORD AttribId) PURE;
    STDMETHOD_(DWORD, GetNumFaces)(THIS) PURE;
    STDMETHOD_(DWORD, GetNumVertices)(THIS) PURE;
    STDMETHOD_(DWORD, GetFVF)(THIS) PURE;
    STDMETHOD(GetDeclaration)(THIS_ D3DVERTEXELEMENT9 Declaration[MAX_FVF_DECL_SIZE]) PURE;
    STDMETHOD_(DWORD, GetNumBytesPerVertex)(THIS) PURE;
    STDMETHOD_(DWORD, GetOptions)(THIS) PURE;
    STDMETHOD(GetDevice)(THIS_ LPDIRECT3DDEVICE9* ppDevice) PURE;
    STDMETHOD(CloneMeshFVF)(THIS_ DWORD Options, 
                DWORD FVF, LPDIRECT3DDEVICE9 pD3DDevice, LPD3DXMESH* ppCloneMesh) PURE;
    STDMETHOD(CloneMesh)(THIS_ DWORD Options, 
                CONST D3DVERTEXELEMENT9 *pDeclaration, LPDIRECT3DDEVICE9 pD3DDevice, LPD3DXMESH* ppCloneMesh) PURE;
    STDMETHOD(GetVertexBuffer)(THIS_ LPDIRECT3DVERTEXBUFFER9* ppVB) PURE;
    STDMETHOD(GetIndexBuffer)(THIS_ LPDIRECT3DINDEXBUFFER9* ppIB) PURE;
    STDMETHOD(LockVertexBuffer)(THIS_ DWORD Flags, LPVOID *ppData) PURE;
    STDMETHOD(UnlockVertexBuffer)(THIS) PURE;
    STDMETHOD(LockIndexBuffer)(THIS_ DWORD Flags, LPVOID *ppData) PURE;
    STDMETHOD(UnlockIndexBuffer)(THIS) PURE;
    STDMETHOD(GetAttributeTable)(
                THIS_ D3DXATTRIBUTERANGE *pAttribTable, DWORD* pAttribTableSize) PURE;

    STDMETHOD(ConvertPointRepsToAdjacency)(THIS_ CONST DWORD* pPRep, DWORD* pAdjacency) PURE;
    STDMETHOD(ConvertAdjacencyToPointReps)(THIS_ CONST DWORD* pAdjacency, DWORD* pPRep) PURE;
    STDMETHOD(GenerateAdjacency)(THIS_ FLOAT Epsilon, DWORD* pAdjacency) PURE;

    STDMETHOD(UpdateSemantics)(THIS_ D3DVERTEXELEMENT9 Declaration[MAX_FVF_DECL_SIZE]) PURE;
};


#undef INTERFACE
#define INTERFACE ID3DXMesh

DECLARE_INTERFACE_(ID3DXMesh, ID3DXBaseMesh)
{
    // IUnknown
    STDMETHOD(QueryInterface)(THIS_ REFIID iid, LPVOID *ppv) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

    // ID3DXBaseMesh
    STDMETHOD(DrawSubset)(THIS_ DWORD AttribId) PURE;
    STDMETHOD_(DWORD, GetNumFaces)(THIS) PURE;
    STDMETHOD_(DWORD, GetNumVertices)(THIS) PURE;
    STDMETHOD_(DWORD, GetFVF)(THIS) PURE;
    STDMETHOD(GetDeclaration)(THIS_ D3DVERTEXELEMENT9 Declaration[MAX_FVF_DECL_SIZE]) PURE;
    STDMETHOD_(DWORD, GetNumBytesPerVertex)(THIS) PURE;
    STDMETHOD_(DWORD, GetOptions)(THIS) PURE;
    STDMETHOD(GetDevice)(THIS_ LPDIRECT3DDEVICE9* ppDevice) PURE;
    STDMETHOD(CloneMeshFVF)(THIS_ DWORD Options, 
                DWORD FVF, LPDIRECT3DDEVICE9 pD3DDevice, LPD3DXMESH* ppCloneMesh) PURE;
    STDMETHOD(CloneMesh)(THIS_ DWORD Options, 
                CONST D3DVERTEXELEMENT9 *pDeclaration, LPDIRECT3DDEVICE9 pD3DDevice, LPD3DXMESH* ppCloneMesh) PURE;
    STDMETHOD(GetVertexBuffer)(THIS_ LPDIRECT3DVERTEXBUFFER9* ppVB) PURE;
    STDMETHOD(GetIndexBuffer)(THIS_ LPDIRECT3DINDEXBUFFER9* ppIB) PURE;
    STDMETHOD(LockVertexBuffer)(THIS_ DWORD Flags, LPVOID *ppData) PURE;
    STDMETHOD(UnlockVertexBuffer)(THIS) PURE;
    STDMETHOD(LockIndexBuffer)(THIS_ DWORD Flags, LPVOID *ppData) PURE;
    STDMETHOD(UnlockIndexBuffer)(THIS) PURE;
    STDMETHOD(GetAttributeTable)(
                THIS_ D3DXATTRIBUTERANGE *pAttribTable, DWORD* pAttribTableSize) PURE;

    STDMETHOD(ConvertPointRepsToAdjacency)(THIS_ CONST DWORD* pPRep, DWORD* pAdjacency) PURE;
    STDMETHOD(ConvertAdjacencyToPointReps)(THIS_ CONST DWORD* pAdjacency, DWORD* pPRep) PURE;
    STDMETHOD(GenerateAdjacency)(THIS_ FLOAT Epsilon, DWORD* pAdjacency) PURE;

    STDMETHOD(UpdateSemantics)(THIS_ D3DVERTEXELEMENT9 Declaration[MAX_FVF_DECL_SIZE]) PURE;

    // ID3DXMesh
    STDMETHOD(LockAttributeBuffer)(THIS_ DWORD Flags, DWORD** ppData) PURE;
    STDMETHOD(UnlockAttributeBuffer)(THIS) PURE;
    STDMETHOD(Optimize)(THIS_ DWORD Flags, CONST DWORD* pAdjacencyIn, DWORD* pAdjacencyOut, 
                     DWORD* pFaceRemap, LPD3DXBUFFER *ppVertexRemap,  
                     LPD3DXMESH* ppOptMesh) PURE;
    STDMETHOD(OptimizeInplace)(THIS_ DWORD Flags, CONST DWORD* pAdjacencyIn, DWORD* pAdjacencyOut, 
                     DWORD* pFaceRemap, LPD3DXBUFFER *ppVertexRemap) PURE;
    STDMETHOD(SetAttributeTable)(THIS_ CONST D3DXATTRIBUTERANGE *pAttribTable, DWORD cAttribTableSize) PURE;
};


#undef INTERFACE
#define INTERFACE ID3DXPMesh

DECLARE_INTERFACE_(ID3DXPMesh, ID3DXBaseMesh)
{
    // IUnknown
    STDMETHOD(QueryInterface)(THIS_ REFIID iid, LPVOID *ppv) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

    // ID3DXBaseMesh
    STDMETHOD(DrawSubset)(THIS_ DWORD AttribId) PURE;
    STDMETHOD_(DWORD, GetNumFaces)(THIS) PURE;
    STDMETHOD_(DWORD, GetNumVertices)(THIS) PURE;
    STDMETHOD_(DWORD, GetFVF)(THIS) PURE;
    STDMETHOD(GetDeclaration)(THIS_ D3DVERTEXELEMENT9 Declaration[MAX_FVF_DECL_SIZE]) PURE;
    STDMETHOD_(DWORD, GetNumBytesPerVertex)(THIS) PURE;
    STDMETHOD_(DWORD, GetOptions)(THIS) PURE;
    STDMETHOD(GetDevice)(THIS_ LPDIRECT3DDEVICE9* ppDevice) PURE;
    STDMETHOD(CloneMeshFVF)(THIS_ DWORD Options, 
                DWORD FVF, LPDIRECT3DDEVICE9 pD3DDevice, LPD3DXMESH* ppCloneMesh) PURE;
    STDMETHOD(CloneMesh)(THIS_ DWORD Options, 
                CONST D3DVERTEXELEMENT9 *pDeclaration, LPDIRECT3DDEVICE9 pD3DDevice, LPD3DXMESH* ppCloneMesh) PURE;
    STDMETHOD(GetVertexBuffer)(THIS_ LPDIRECT3DVERTEXBUFFER9* ppVB) PURE;
    STDMETHOD(GetIndexBuffer)(THIS_ LPDIRECT3DINDEXBUFFER9* ppIB) PURE;
    STDMETHOD(LockVertexBuffer)(THIS_ DWORD Flags, LPVOID *ppData) PURE;
    STDMETHOD(UnlockVertexBuffer)(THIS) PURE;
    STDMETHOD(LockIndexBuffer)(THIS_ DWORD Flags, LPVOID *ppData) PURE;
    STDMETHOD(UnlockIndexBuffer)(THIS) PURE;
    STDMETHOD(GetAttributeTable)(
                THIS_ D3DXATTRIBUTERANGE *pAttribTable, DWORD* pAttribTableSize) PURE;

    STDMETHOD(ConvertPointRepsToAdjacency)(THIS_ CONST DWORD* pPRep, DWORD* pAdjacency) PURE;
    STDMETHOD(ConvertAdjacencyToPointReps)(THIS_ CONST DWORD* pAdjacency, DWORD* pPRep) PURE;
    STDMETHOD(GenerateAdjacency)(THIS_ FLOAT Epsilon, DWORD* pAdjacency) PURE;

    STDMETHOD(UpdateSemantics)(THIS_ D3DVERTEXELEMENT9 Declaration[MAX_FVF_DECL_SIZE]) PURE;

    // ID3DXPMesh
    STDMETHOD(ClonePMeshFVF)(THIS_ DWORD Options, 
                DWORD FVF, LPDIRECT3DDEVICE9 pD3DDevice, LPD3DXPMESH* ppCloneMesh) PURE;
    STDMETHOD(ClonePMesh)(THIS_ DWORD Options, 
                CONST D3DVERTEXELEMENT9 *pDeclaration, LPDIRECT3DDEVICE9 pD3DDevice, LPD3DXPMESH* ppCloneMesh) PURE;
    STDMETHOD(SetNumFaces)(THIS_ DWORD Faces) PURE;
    STDMETHOD(SetNumVertices)(THIS_ DWORD Vertices) PURE;
    STDMETHOD_(DWORD, GetMaxFaces)(THIS) PURE;
    STDMETHOD_(DWORD, GetMinFaces)(THIS) PURE;
    STDMETHOD_(DWORD, GetMaxVertices)(THIS) PURE;
    STDMETHOD_(DWORD, GetMinVertices)(THIS) PURE;
    STDMETHOD(Save)(THIS_ IStream *pStream, CONST D3DXMATERIAL* pMaterials, CONST D3DXEFFECTINSTANCE* pEffectInstances, DWORD NumMaterials) PURE;

    STDMETHOD(Optimize)(THIS_ DWORD Flags, DWORD* pAdjacencyOut, 
                     DWORD* pFaceRemap, LPD3DXBUFFER *ppVertexRemap,  
                     LPD3DXMESH* ppOptMesh) PURE;

    STDMETHOD(OptimizeBaseLOD)(THIS_ DWORD Flags, DWORD* pFaceRemap) PURE;
    STDMETHOD(TrimByFaces)(THIS_ DWORD NewFacesMin, DWORD NewFacesMax, DWORD *rgiFaceRemap, DWORD *rgiVertRemap) PURE;
    STDMETHOD(TrimByVertices)(THIS_ DWORD NewVerticesMin, DWORD NewVerticesMax, DWORD *rgiFaceRemap, DWORD *rgiVertRemap) PURE;

    STDMETHOD(GetAdjacency)(THIS_ DWORD* pAdjacency) PURE;

    //  Used to generate the immediate "ancestor" for each vertex when it is removed by a vsplit.  Allows generation of geomorphs
    //     Vertex buffer must be equal to or greater than the maximum number of vertices in the pmesh
    STDMETHOD(GenerateVertexHistory)(THIS_ DWORD* pVertexHistory) PURE;
};


#undef INTERFACE
#define INTERFACE ID3DXSPMesh

DECLARE_INTERFACE_(ID3DXSPMesh, IUnknown)
{
    // IUnknown
    STDMETHOD(QueryInterface)(THIS_ REFIID iid, LPVOID *ppv) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

    // ID3DXSPMesh
    STDMETHOD_(DWORD, GetNumFaces)(THIS) PURE;
    STDMETHOD_(DWORD, GetNumVertices)(THIS) PURE;
    STDMETHOD_(DWORD, GetFVF)(THIS) PURE;
    STDMETHOD(GetDeclaration)(THIS_ D3DVERTEXELEMENT9 Declaration[MAX_FVF_DECL_SIZE]) PURE;
    STDMETHOD_(DWORD, GetOptions)(THIS) PURE;
    STDMETHOD(GetDevice)(THIS_ LPDIRECT3DDEVICE9* ppDevice) PURE;
    STDMETHOD(CloneMeshFVF)(THIS_ DWORD Options, 
                DWORD FVF, LPDIRECT3DDEVICE9 pD3DDevice, DWORD *pAdjacencyOut, DWORD *pVertexRemapOut, LPD3DXMESH* ppCloneMesh) PURE;
    STDMETHOD(CloneMesh)(THIS_ DWORD Options, 
                CONST D3DVERTEXELEMENT9 *pDeclaration, LPDIRECT3DDEVICE9 pD3DDevice, DWORD *pAdjacencyOut, DWORD *pVertexRemapOut, LPD3DXMESH* ppCloneMesh) PURE;
    STDMETHOD(ClonePMeshFVF)(THIS_ DWORD Options, 
                DWORD FVF, LPDIRECT3DDEVICE9 pD3DDevice, DWORD *pVertexRemapOut, FLOAT *pErrorsByFace, LPD3DXPMESH* ppCloneMesh) PURE;
    STDMETHOD(ClonePMesh)(THIS_ DWORD Options, 
                CONST D3DVERTEXELEMENT9 *pDeclaration, LPDIRECT3DDEVICE9 pD3DDevice, DWORD *pVertexRemapOut, FLOAT *pErrorsbyFace, LPD3DXPMESH* ppCloneMesh) PURE;
    STDMETHOD(ReduceFaces)(THIS_ DWORD Faces) PURE;
    STDMETHOD(ReduceVertices)(THIS_ DWORD Vertices) PURE;
    STDMETHOD_(DWORD, GetMaxFaces)(THIS) PURE;
    STDMETHOD_(DWORD, GetMaxVertices)(THIS) PURE;
    STDMETHOD(GetVertexAttributeWeights)(THIS_ LPD3DXATTRIBUTEWEIGHTS pVertexAttributeWeights) PURE;
    STDMETHOD(GetVertexWeights)(THIS_ FLOAT *pVertexWeights) PURE;
};

#define UNUSED16 (0xffff)
#define UNUSED32 (0xffffffff)

// ID3DXMesh::Optimize options - upper byte only, lower 3 bytes used from _D3DXMESH option flags
enum _D3DXMESHOPT {
    D3DXMESHOPT_COMPACT       = 0x01000000,
    D3DXMESHOPT_ATTRSORT      = 0x02000000,
    D3DXMESHOPT_VERTEXCACHE   = 0x04000000,
    D3DXMESHOPT_STRIPREORDER  = 0x08000000,
    D3DXMESHOPT_IGNOREVERTS   = 0x10000000,  // optimize faces only, don't touch vertices
    D3DXMESHOPT_DONOTSPLIT    = 0x20000000,  // do not split vertices shared between attribute groups when attribute sorting
    D3DXMESHOPT_DEVICEINDEPENDENT = 0x00400000  // Only affects VCache.  uses a static known good cache size for all cards
                            
    // D3DXMESHOPT_SHAREVB has been removed, please use D3DXMESH_VB_SHARE instead

};

// Subset of the mesh that has the same attribute and bone combination.
// This subset can be rendered in a single draw call
typedef struct _D3DXBONECOMBINATION
{
    DWORD AttribId;
    DWORD FaceStart;
    DWORD FaceCount;
    DWORD VertexStart;
    DWORD VertexCount;
    DWORD* BoneId;
} D3DXBONECOMBINATION, *LPD3DXBONECOMBINATION;

// The following types of patch combinations are supported:
// Patch type   Basis       Degree
// Rect         Bezier      2,3,5
// Rect         B-Spline    2,3,5
// Rect         Catmull-Rom 3
// Tri          Bezier      2,3,5
// N-Patch      N/A         3

typedef struct _D3DXPATCHINFO
{
    D3DXPATCHMESHTYPE PatchType;
    D3DDEGREETYPE Degree;
    D3DBASISTYPE Basis;
} D3DXPATCHINFO, *LPD3DXPATCHINFO;

#undef INTERFACE
#define INTERFACE ID3DXPatchMesh

DECLARE_INTERFACE_(ID3DXPatchMesh, IUnknown)
{
    // IUnknown
    STDMETHOD(QueryInterface)(THIS_ REFIID iid, LPVOID *ppv) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

    // ID3DXPatchMesh

    // Return creation parameters
    STDMETHOD_(DWORD, GetNumPatches)(THIS) PURE;
    STDMETHOD_(DWORD, GetNumVertices)(THIS) PURE;
    STDMETHOD(GetDeclaration)(THIS_ D3DVERTEXELEMENT9 Declaration[MAX_FVF_DECL_SIZE]) PURE;
    STDMETHOD_(DWORD, GetControlVerticesPerPatch)(THIS) PURE;
    STDMETHOD_(DWORD, GetOptions)(THIS) PURE;
    STDMETHOD(GetDevice)(THIS_ LPDIRECT3DDEVICE9 *ppDevice) PURE;
    STDMETHOD(GetPatchInfo)(THIS_ LPD3DXPATCHINFO PatchInfo) PURE;

    // Control mesh access    
    STDMETHOD(GetVertexBuffer)(THIS_ LPDIRECT3DVERTEXBUFFER9* ppVB) PURE;
    STDMETHOD(GetIndexBuffer)(THIS_ LPDIRECT3DINDEXBUFFER9* ppIB) PURE;
    STDMETHOD(LockVertexBuffer)(THIS_ DWORD flags, LPVOID *ppData) PURE;
    STDMETHOD(UnlockVertexBuffer)(THIS) PURE;
    STDMETHOD(LockIndexBuffer)(THIS_ DWORD flags, LPVOID *ppData) PURE;
    STDMETHOD(UnlockIndexBuffer)(THIS) PURE;
    STDMETHOD(LockAttributeBuffer)(THIS_ DWORD flags, DWORD** ppData) PURE;
    STDMETHOD(UnlockAttributeBuffer)(THIS) PURE;

    // This function returns the size of the tessellated mesh given a tessellation level.
    // This assumes uniform tessellation. For adaptive tessellation the Adaptive parameter must
    // be set to TRUE and TessellationLevel should be the max tessellation.
    // This will result in the max mesh size necessary for adaptive tessellation.    
    STDMETHOD(GetTessSize)(THIS_ FLOAT fTessLevel,DWORD Adaptive, DWORD *NumTriangles,DWORD *NumVertices) PURE;
    
    //GenerateAdjacency determines which patches are adjacent with provided tolerance
    //this information is used internally to optimize tessellation
    STDMETHOD(GenerateAdjacency)(THIS_ FLOAT Tolerance) PURE;
    
    //CloneMesh Creates a new patchmesh with the specified decl, and converts the vertex buffer
    //to the new decl. Entries in the new decl which are new are set to 0. If the current mesh
    //has adjacency, the new mesh will also have adjacency
    STDMETHOD(CloneMesh)(THIS_ DWORD Options, CONST D3DVERTEXELEMENT9 *pDecl, LPD3DXPATCHMESH *pMesh) PURE;
    
    // Optimizes the patchmesh for efficient tessellation. This function is designed
    // to perform one time optimization for patch meshes that need to be tessellated
    // repeatedly by calling the Tessellate() method. The optimization performed is
    // independent of the actual tessellation level used.
    // Currently Flags is unused.
    // If vertices are changed, Optimize must be called again
    STDMETHOD(Optimize)(THIS_ DWORD flags) PURE;

    //gets and sets displacement parameters
    //displacement maps can only be 2D textures MIP-MAPPING is ignored for non adapative tessellation
    STDMETHOD(SetDisplaceParam)(THIS_ LPDIRECT3DBASETEXTURE9 Texture,
                              D3DTEXTUREFILTERTYPE MinFilter,
                              D3DTEXTUREFILTERTYPE MagFilter,
                              D3DTEXTUREFILTERTYPE MipFilter,
                              D3DTEXTUREADDRESS Wrap,
                              DWORD dwLODBias) PURE;

    STDMETHOD(GetDisplaceParam)(THIS_ LPDIRECT3DBASETEXTURE9 *Texture,
                                D3DTEXTUREFILTERTYPE *MinFilter,
                                D3DTEXTUREFILTERTYPE *MagFilter,
                                D3DTEXTUREFILTERTYPE *MipFilter,
                                D3DTEXTUREADDRESS *Wrap,
                                DWORD *dwLODBias) PURE;
        
    // Performs the uniform tessellation based on the tessellation level. 
    // This function will perform more efficiently if the patch mesh has been optimized using the Optimize() call.
    STDMETHOD(Tessellate)(THIS_ FLOAT fTessLevel,LPD3DXMESH pMesh) PURE;

    // Performs adaptive tessellation based on the Z based adaptive tessellation criterion.
    // pTrans specifies a 4D vector that is dotted with the vertices to get the per vertex
    // adaptive tessellation amount. Each edge is tessellated to the average of the criterion
    // at the 2 vertices it connects.
    // MaxTessLevel specifies the upper limit for adaptive tesselation.
    // This function will perform more efficiently if the patch mesh has been optimized using the Optimize() call.
    STDMETHOD(TessellateAdaptive)(THIS_ 
        CONST D3DXVECTOR4 *pTrans,
        DWORD dwMaxTessLevel, 
        DWORD dwMinTessLevel,
        LPD3DXMESH pMesh) PURE;

};

#undef INTERFACE
#define INTERFACE ID3DXSkinInfo

DECLARE_INTERFACE_(ID3DXSkinInfo, IUnknown)
{
    // IUnknown
    STDMETHOD(QueryInterface)(THIS_ REFIID iid, LPVOID *ppv) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

    // Specify the which vertices do each bones influence and by how much
    STDMETHOD(SetBoneInfluence)(THIS_ DWORD bone, DWORD numInfluences, CONST DWORD* vertices, CONST FLOAT* weights) PURE;
	STDMETHOD(SetBoneVertexInfluence)(THIS_ DWORD boneNum, DWORD influenceNum, float weight) PURE;
    STDMETHOD_(DWORD, GetNumBoneInfluences)(THIS_ DWORD bone) PURE;
	STDMETHOD(GetBoneInfluence)(THIS_ DWORD bone, DWORD* vertices, FLOAT* weights) PURE;
	STDMETHOD(GetBoneVertexInfluence)(THIS_ DWORD boneNum, DWORD influenceNum, float *pWeight, DWORD *pVertexNum) PURE;
    STDMETHOD(GetMaxVertexInfluences)(THIS_ DWORD* maxVertexInfluences) PURE;
    STDMETHOD_(DWORD, GetNumBones)(THIS) PURE;
	STDMETHOD(FindBoneVertexInfluenceIndex)(THIS_ DWORD boneNum, DWORD vertexNum, DWORD *pInfluenceIndex) PURE;

    // This gets the max face influences based on a triangle mesh with the specified index buffer
    STDMETHOD(GetMaxFaceInfluences)(THIS_ LPDIRECT3DINDEXBUFFER9 pIB, DWORD NumFaces, DWORD* maxFaceInfluences) PURE;
    
    // Set min bone influence. Bone influences that are smaller than this are ignored
    STDMETHOD(SetMinBoneInfluence)(THIS_ FLOAT MinInfl) PURE;
    // Get min bone influence. 
    STDMETHOD_(FLOAT, GetMinBoneInfluence)(THIS) PURE;
    
    // Bone names are returned by D3DXLoadSkinMeshFromXof. They are not used by any other method of this object
    STDMETHOD(SetBoneName)(THIS_ DWORD Bone, LPCSTR pName) PURE; // pName is copied to an internal string buffer
    STDMETHOD_(LPCSTR, GetBoneName)(THIS_ DWORD Bone) PURE; // A pointer to an internal string buffer is returned. Do not free this.
    
    // Bone offset matrices are returned by D3DXLoadSkinMeshFromXof. They are not used by any other method of this object
    STDMETHOD(SetBoneOffsetMatrix)(THIS_ DWORD Bone, CONST D3DXMATRIX *pBoneTransform) PURE; // pBoneTransform is copied to an internal buffer
    STDMETHOD_(LPD3DXMATRIX, GetBoneOffsetMatrix)(THIS_ DWORD Bone) PURE; // A pointer to an internal matrix is returned. Do not free this.
    
    // Clone a skin info object
    STDMETHOD(Clone)(THIS_ LPD3DXSKININFO* ppSkinInfo) PURE;
    
    // Update bone influence information to match vertices after they are reordered. This should be called 
    // if the target vertex buffer has been reordered externally.
    STDMETHOD(Remap)(THIS_ DWORD NumVertices, DWORD* pVertexRemap) PURE;

    // These methods enable the modification of the vertex layout of the vertices that will be skinned
    STDMETHOD(SetFVF)(THIS_ DWORD FVF) PURE;
    STDMETHOD(SetDeclaration)(THIS_ CONST D3DVERTEXELEMENT9 *pDeclaration) PURE;
    STDMETHOD_(DWORD, GetFVF)(THIS) PURE;
    STDMETHOD(GetDeclaration)(THIS_ D3DVERTEXELEMENT9 Declaration[MAX_FVF_DECL_SIZE]) PURE;

    // Apply SW skinning based on current pose matrices to the target vertices.
    STDMETHOD(UpdateSkinnedMesh)(THIS_ 
        CONST D3DXMATRIX* pBoneTransforms, 
        CONST D3DXMATRIX* pBoneInvTransposeTransforms, 
        LPCVOID pVerticesSrc, 
        PVOID pVerticesDst) PURE;

    // Takes a mesh and returns a new mesh with per vertex blend weights and a bone combination
    // table that describes which bones affect which subsets of the mesh
    STDMETHOD(ConvertToBlendedMesh)(THIS_ 
        LPD3DXMESH pMesh,
        DWORD Options, 
        CONST DWORD *pAdjacencyIn, 
        LPDWORD pAdjacencyOut,
        DWORD* pFaceRemap, 
        LPD3DXBUFFER *ppVertexRemap, 
        DWORD* pMaxFaceInfl,
        DWORD* pNumBoneCombinations, 
        LPD3DXBUFFER* ppBoneCombinationTable, 
        LPD3DXMESH* ppMesh) PURE;

    // Takes a mesh and returns a new mesh with per vertex blend weights and indices 
    // and a bone combination table that describes which bones palettes affect which subsets of the mesh
    STDMETHOD(ConvertToIndexedBlendedMesh)(THIS_ 
        LPD3DXMESH pMesh,
        DWORD Options, 
        DWORD paletteSize, 
        CONST DWORD *pAdjacencyIn, 
        LPDWORD pAdjacencyOut, 
        DWORD* pFaceRemap, 
        LPD3DXBUFFER *ppVertexRemap, 
        DWORD* pMaxVertexInfl,
        DWORD* pNumBoneCombinations, 
        LPD3DXBUFFER* ppBoneCombinationTable, 
        LPD3DXMESH* ppMesh) PURE;
};

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus


HRESULT WINAPI 
    D3DXCreateMesh(
        DWORD NumFaces, 
        DWORD NumVertices, 
        DWORD Options, 
        CONST D3DVERTEXELEMENT9 *pDeclaration, 
        LPDIRECT3DDEVICE9 pD3DDevice, 
        LPD3DXMESH* ppMesh);

HRESULT WINAPI 
    D3DXCreateMeshFVF(
        DWORD NumFaces, 
        DWORD NumVertices, 
        DWORD Options, 
        DWORD FVF, 
        LPDIRECT3DDEVICE9 pD3DDevice, 
        LPD3DXMESH* ppMesh);

HRESULT WINAPI 
    D3DXCreateSPMesh(
        LPD3DXMESH pMesh, 
        CONST DWORD* pAdjacency, 
        CONST D3DXATTRIBUTEWEIGHTS *pVertexAttributeWeights,
        CONST FLOAT *pVertexWeights,
        LPD3DXSPMESH* ppSMesh);

// clean a mesh up for simplification, try to make manifold
HRESULT WINAPI
    D3DXCleanMesh(
    D3DXCLEANTYPE CleanType,
    LPD3DXMESH pMeshIn,
    CONST DWORD* pAdjacencyIn,
    LPD3DXMESH* ppMeshOut,
    DWORD* pAdjacencyOut,
    LPD3DXBUFFER* ppErrorsAndWarnings);

HRESULT WINAPI
    D3DXValidMesh(
    LPD3DXMESH pMeshIn,
    CONST DWORD* pAdjacency,
    LPD3DXBUFFER* ppErrorsAndWarnings);

HRESULT WINAPI 
    D3DXGeneratePMesh(
        LPD3DXMESH pMesh, 
        CONST DWORD* pAdjacency, 
        CONST D3DXATTRIBUTEWEIGHTS *pVertexAttributeWeights,
        CONST FLOAT *pVertexWeights,
        DWORD MinValue, 
        DWORD Options, 
        LPD3DXPMESH* ppPMesh);

HRESULT WINAPI 
    D3DXSimplifyMesh(
        LPD3DXMESH pMesh, 
        CONST DWORD* pAdjacency, 
        CONST D3DXATTRIBUTEWEIGHTS *pVertexAttributeWeights,
        CONST FLOAT *pVertexWeights,
        DWORD MinValue, 
        DWORD Options, 
        LPD3DXMESH* ppMesh);

HRESULT WINAPI 
    D3DXComputeBoundingSphere(
        CONST D3DXVECTOR3 *pFirstPosition,  // pointer to first position
        DWORD NumVertices, 
        DWORD dwStride,                     // count in bytes to subsequent position vectors
        D3DXVECTOR3 *pCenter, 
        FLOAT *pRadius);

HRESULT WINAPI 
    D3DXComputeBoundingBox(
        CONST D3DXVECTOR3 *pFirstPosition,  // pointer to first position
        DWORD NumVertices, 
        DWORD dwStride,                     // count in bytes to subsequent position vectors
        D3DXVECTOR3 *pMin, 
        D3DXVECTOR3 *pMax);

HRESULT WINAPI 
    D3DXComputeNormals(
        LPD3DXBASEMESH pMesh,
        CONST DWORD *pAdjacency);

HRESULT WINAPI 
    D3DXCreateBuffer(
        DWORD NumBytes, 
        LPD3DXBUFFER *ppBuffer);


HRESULT WINAPI
    D3DXLoadMeshFromXA(
        LPCSTR pFilename, 
        DWORD Options, 
        LPDIRECT3DDEVICE9 pD3DDevice, 
        LPD3DXBUFFER *ppAdjacency,
        LPD3DXBUFFER *ppMaterials, 
        LPD3DXBUFFER *ppEffectInstances, 
        DWORD *pNumMaterials,
        LPD3DXMESH *ppMesh);

HRESULT WINAPI
    D3DXLoadMeshFromXW(
        LPCWSTR pFilename, 
        DWORD Options, 
        LPDIRECT3DDEVICE9 pD3DDevice, 
        LPD3DXBUFFER *ppAdjacency,
        LPD3DXBUFFER *ppMaterials, 
        LPD3DXBUFFER *ppEffectInstances, 
        DWORD *pNumMaterials,
        LPD3DXMESH *ppMesh);

#ifdef UNICODE
#define D3DXLoadMeshFromX D3DXLoadMeshFromXW
#else
#define D3DXLoadMeshFromX D3DXLoadMeshFromXA
#endif

HRESULT WINAPI 
    D3DXLoadMeshFromXInMemory(
        LPCVOID Memory,
        DWORD SizeOfMemory,
        DWORD Options, 
        LPDIRECT3DDEVICE9 pD3DDevice, 
        LPD3DXBUFFER *ppAdjacency,
        LPD3DXBUFFER *ppMaterials, 
        LPD3DXBUFFER *ppEffectInstances, 
        DWORD *pNumMaterials,
        LPD3DXMESH *ppMesh);

HRESULT WINAPI 
    D3DXLoadMeshFromXResource(
        HMODULE Module,
        LPCSTR Name,
        LPCSTR Type,
        DWORD Options, 
        LPDIRECT3DDEVICE9 pD3DDevice, 
        LPD3DXBUFFER *ppAdjacency,
        LPD3DXBUFFER *ppMaterials, 
        LPD3DXBUFFER *ppEffectInstances, 
        DWORD *pNumMaterials,
        LPD3DXMESH *ppMesh);

HRESULT WINAPI 
    D3DXSaveMeshToXA(
        LPCSTR pFilename,
        LPD3DXMESH pMesh,
        CONST DWORD* pAdjacency,
        CONST D3DXMATERIAL* pMaterials,
        CONST D3DXEFFECTINSTANCE* pEffectInstances, 
        DWORD NumMaterials,
        DWORD Format
        );

HRESULT WINAPI 
    D3DXSaveMeshToXW(
        LPCWSTR pFilename,
        LPD3DXMESH pMesh,
        CONST DWORD* pAdjacency,
        CONST D3DXMATERIAL* pMaterials,
        CONST D3DXEFFECTINSTANCE* pEffectInstances, 
        DWORD NumMaterials,
        DWORD Format
        );
        
#ifdef UNICODE
#define D3DXSaveMeshToX D3DXSaveMeshToXW
#else
#define D3DXSaveMeshToX D3DXSaveMeshToXA
#endif
        

HRESULT WINAPI 
    D3DXCreatePMeshFromStream(
        IStream *pStream, 
        DWORD Options,
        LPDIRECT3DDEVICE9 pD3DDevice, 
        LPD3DXBUFFER *ppMaterials,
        LPD3DXBUFFER *ppEffectInstances, 
        DWORD* pNumMaterials,
        LPD3DXPMESH *ppPMesh);

// Creates a skin info object based on the number of vertices, number of bones, and a declaration describing the vertex layout of the target vertices
// The bone names and initial bone transforms are not filled in the skin info object by this method.
HRESULT WINAPI
    D3DXCreateSkinInfo(
        DWORD NumVertices,
        CONST D3DVERTEXELEMENT9 *pDeclaration, 
        DWORD NumBones,
        LPD3DXSKININFO* ppSkinInfo);
        
// Creates a skin info object based on the number of vertices, number of bones, and a FVF describing the vertex layout of the target vertices
// The bone names and initial bone transforms are not filled in the skin info object by this method.
HRESULT WINAPI
    D3DXCreateSkinInfoFVF(
        DWORD NumVertices,
        DWORD FVF,
        DWORD NumBones,
        LPD3DXSKININFO* ppSkinInfo);
        
#ifdef __cplusplus
}

extern "C" {
#endif //__cplusplus

HRESULT WINAPI 
    D3DXLoadMeshFromXof(
        LPD3DXFILEDATA pxofMesh, 
        DWORD Options, 
        LPDIRECT3DDEVICE9 pD3DDevice, 
        LPD3DXBUFFER *ppAdjacency,
        LPD3DXBUFFER *ppMaterials, 
        LPD3DXBUFFER *ppEffectInstances, 
        DWORD *pNumMaterials,
        LPD3DXMESH *ppMesh);

// This similar to D3DXLoadMeshFromXof, except also returns skinning info if present in the file
// If skinning info is not present, ppSkinInfo will be NULL     
HRESULT WINAPI
    D3DXLoadSkinMeshFromXof(
        LPD3DXFILEDATA pxofMesh, 
        DWORD Options,
        LPDIRECT3DDEVICE9 pD3DDevice,
        LPD3DXBUFFER* ppAdjacency,
        LPD3DXBUFFER* ppMaterials,
        LPD3DXBUFFER *ppEffectInstances, 
        DWORD *pMatOut,
        LPD3DXSKININFO* ppSkinInfo,
        LPD3DXMESH* ppMesh);


// The inverse of D3DXConvertTo{Indexed}BlendedMesh() functions. It figures out the skinning info from
// the mesh and the bone combination table and populates a skin info object with that data. The bone
// names and initial bone transforms are not filled in the skin info object by this method. This works
// with either a non-indexed or indexed blended mesh. It examines the FVF or declarator of the mesh to
// determine what type it is.
HRESULT WINAPI
    D3DXCreateSkinInfoFromBlendedMesh(
        LPD3DXBASEMESH pMesh,
        DWORD NumBones,
        CONST D3DXBONECOMBINATION *pBoneCombinationTable,
        LPD3DXSKININFO* ppSkinInfo);
        
HRESULT WINAPI
    D3DXTessellateNPatches(
        LPD3DXMESH pMeshIn,             
        CONST DWORD* pAdjacencyIn,             
        FLOAT NumSegs,                    
        BOOL  QuadraticInterpNormals,     // if false use linear intrep for normals, if true use quadratic
        LPD3DXMESH *ppMeshOut,
        LPD3DXBUFFER *ppAdjacencyOut);


//generates implied outputdecl from input decl
//the decl generated from this should be used to generate the output decl for
//the tessellator subroutines. 

HRESULT WINAPI
    D3DXGenerateOutputDecl(
        D3DVERTEXELEMENT9 *pOutput,
        CONST D3DVERTEXELEMENT9 *pInput);

//loads patches from an XFileData
//since an X file can have up to 6 different patch meshes in it,
//returns them in an array - pNumPatches will contain the number of
//meshes in the actual file. 
HRESULT WINAPI
    D3DXLoadPatchMeshFromXof(
        LPD3DXFILEDATA pXofObjMesh,
        DWORD Options,
        LPDIRECT3DDEVICE9 pD3DDevice,
        LPD3DXBUFFER *ppMaterials,
        LPD3DXBUFFER *ppEffectInstances, 
        PDWORD pNumMaterials,
        LPD3DXPATCHMESH *ppMesh);

//computes the size a single rect patch.
HRESULT WINAPI
    D3DXRectPatchSize(
        CONST FLOAT *pfNumSegs, //segments for each edge (4)
        DWORD *pdwTriangles,    //output number of triangles
        DWORD *pdwVertices);    //output number of vertices

//computes the size of a single triangle patch      
HRESULT WINAPI
    D3DXTriPatchSize(
        CONST FLOAT *pfNumSegs, //segments for each edge (3)    
        DWORD *pdwTriangles,    //output number of triangles
        DWORD *pdwVertices);    //output number of vertices


//tessellates a patch into a created mesh
//similar to D3D RT patch
HRESULT WINAPI
    D3DXTessellateRectPatch(
        LPDIRECT3DVERTEXBUFFER9 pVB,
        CONST FLOAT *pNumSegs,
        CONST D3DVERTEXELEMENT9 *pdwInDecl,
        CONST D3DRECTPATCH_INFO *pRectPatchInfo,
        LPD3DXMESH pMesh);


HRESULT WINAPI
    D3DXTessellateTriPatch(
      LPDIRECT3DVERTEXBUFFER9 pVB,
      CONST FLOAT *pNumSegs,
      CONST D3DVERTEXELEMENT9 *pInDecl,
      CONST D3DTRIPATCH_INFO *pTriPatchInfo,
      LPD3DXMESH pMesh);



//creates an NPatch PatchMesh from a D3DXMESH 
HRESULT WINAPI
    D3DXCreateNPatchMesh(
        LPD3DXMESH pMeshSysMem,
        LPD3DXPATCHMESH *pPatchMesh);

      
//creates a patch mesh
HRESULT WINAPI
    D3DXCreatePatchMesh(
        CONST D3DXPATCHINFO *pInfo,     //patch type
        DWORD dwNumPatches,             //number of patches
        DWORD dwNumVertices,            //number of control vertices
        DWORD dwOptions,                //options 
        CONST D3DVERTEXELEMENT9 *pDecl, //format of control vertices
        LPDIRECT3DDEVICE9 pD3DDevice, 
        LPD3DXPATCHMESH *pPatchMesh);

        
//returns the number of degenerates in a patch mesh -
//text output put in string.
HRESULT WINAPI
    D3DXValidPatchMesh(LPD3DXPATCHMESH pMesh,
                        DWORD *dwcDegenerateVertices,
                        DWORD *dwcDegeneratePatches,
                        LPD3DXBUFFER *ppErrorsAndWarnings);

UINT WINAPI
    D3DXGetFVFVertexSize(DWORD FVF);

UINT WINAPI 
    D3DXGetDeclVertexSize(CONST D3DVERTEXELEMENT9 *pDecl,DWORD Stream);

UINT WINAPI 
    D3DXGetDeclLength(CONST D3DVERTEXELEMENT9 *pDecl);

HRESULT WINAPI
    D3DXDeclaratorFromFVF(
        DWORD FVF,
        D3DVERTEXELEMENT9 pDeclarator[MAX_FVF_DECL_SIZE]);

HRESULT WINAPI
    D3DXFVFFromDeclarator(
        CONST D3DVERTEXELEMENT9 *pDeclarator,
        DWORD *pFVF);

HRESULT WINAPI 
    D3DXWeldVertices(
        LPD3DXMESH pMesh,         
        DWORD Flags,
        CONST D3DXWELDEPSILONS *pEpsilons,                 
        CONST DWORD *pAdjacencyIn, 
        DWORD *pAdjacencyOut,
        DWORD *pFaceRemap, 
        LPD3DXBUFFER *ppVertexRemap);

typedef struct _D3DXINTERSECTINFO
{
    DWORD FaceIndex;                // index of face intersected
    FLOAT U;                        // Barycentric Hit Coordinates    
    FLOAT V;                        // Barycentric Hit Coordinates
    FLOAT Dist;                     // Ray-Intersection Parameter Distance
} D3DXINTERSECTINFO, *LPD3DXINTERSECTINFO;


HRESULT WINAPI
    D3DXIntersect(
        LPD3DXBASEMESH pMesh,
        CONST D3DXVECTOR3 *pRayPos,
        CONST D3DXVECTOR3 *pRayDir, 
        BOOL    *pHit,              // True if any faces were intersected
        DWORD   *pFaceIndex,        // index of closest face intersected
        FLOAT   *pU,                // Barycentric Hit Coordinates    
        FLOAT   *pV,                // Barycentric Hit Coordinates
        FLOAT   *pDist,             // Ray-Intersection Parameter Distance
        LPD3DXBUFFER *ppAllHits,    // Array of D3DXINTERSECTINFOs for all hits (not just closest) 
        DWORD   *pCountOfHits);     // Number of entries in AllHits array

HRESULT WINAPI
    D3DXIntersectSubset(
        LPD3DXBASEMESH pMesh,
        DWORD AttribId,
        CONST D3DXVECTOR3 *pRayPos,
        CONST D3DXVECTOR3 *pRayDir, 
        BOOL    *pHit,              // True if any faces were intersected
        DWORD   *pFaceIndex,        // index of closest face intersected
        FLOAT   *pU,                // Barycentric Hit Coordinates    
        FLOAT   *pV,                // Barycentric Hit Coordinates
        FLOAT   *pDist,             // Ray-Intersection Parameter Distance
        LPD3DXBUFFER *ppAllHits,    // Array of D3DXINTERSECTINFOs for all hits (not just closest) 
        DWORD   *pCountOfHits);     // Number of entries in AllHits array


HRESULT WINAPI D3DXSplitMesh
    (
    LPD3DXMESH pMeshIn,         
    CONST DWORD *pAdjacencyIn, 
    CONST DWORD MaxSize,
    CONST DWORD Options,
    DWORD *pMeshesOut,
    LPD3DXBUFFER *ppMeshArrayOut,
    LPD3DXBUFFER *ppAdjacencyArrayOut,
    LPD3DXBUFFER *ppFaceRemapArrayOut,
    LPD3DXBUFFER *ppVertRemapArrayOut
    );

BOOL WINAPI D3DXIntersectTri 
(
    CONST D3DXVECTOR3 *p0,           // Triangle vertex 0 position
    CONST D3DXVECTOR3 *p1,           // Triangle vertex 1 position
    CONST D3DXVECTOR3 *p2,           // Triangle vertex 2 position
    CONST D3DXVECTOR3 *pRayPos,      // Ray origin
    CONST D3DXVECTOR3 *pRayDir,      // Ray direction
    FLOAT *pU,                       // Barycentric Hit Coordinates
    FLOAT *pV,                       // Barycentric Hit Coordinates
    FLOAT *pDist);                   // Ray-Intersection Parameter Distance

BOOL WINAPI
    D3DXSphereBoundProbe(
        CONST D3DXVECTOR3 *pCenter,
        FLOAT Radius,
        CONST D3DXVECTOR3 *pRayPosition,
        CONST D3DXVECTOR3 *pRayDirection);

BOOL WINAPI 
    D3DXBoxBoundProbe(
        CONST D3DXVECTOR3 *pMin, 
        CONST D3DXVECTOR3 *pMax,
        CONST D3DXVECTOR3 *pRayPosition,
        CONST D3DXVECTOR3 *pRayDirection);


HRESULT WINAPI D3DXComputeTangentFrame(ID3DXMesh *pMesh,
                                       DWORD dwOptions);

HRESULT WINAPI D3DXComputeTangentFrameEx(ID3DXMesh *pMesh,
                                         DWORD dwTextureInSemantic,
                                         DWORD dwTextureInIndex,
                                         DWORD dwUPartialOutSemantic,
                                         DWORD dwUPartialOutIndex,
                                         DWORD dwVPartialOutSemantic,
                                         DWORD dwVPartialOutIndex,
                                         DWORD dwNormalOutSemantic,
                                         DWORD dwNormalOutIndex,
                                         DWORD dwOptions,
                                         CONST DWORD *pdwAdjacency,
                                         FLOAT fPartialEdgeThreshold,
                                         FLOAT fSingularPointThreshold,
                                         FLOAT fNormalEdgeThreshold,
                                         ID3DXMesh **ppMeshOut,
                                         ID3DXBuffer **ppVertexMapping);


//D3DXComputeTangent
//
//Computes the Tangent vectors for the TexStage texture coordinates
//and places the results in the TANGENT[TangentIndex] specified in the meshes' DECL
//puts the binorm in BINORM[BinormIndex] also specified in the decl.
//
//If neither the binorm or the tangnet are in the meshes declaration,
//the function will fail. 
//
//If a tangent or Binorm field is in the Decl, but the user does not
//wish D3DXComputeTangent to replace them, then D3DX_DEFAULT specified
//in the TangentIndex or BinormIndex will cause it to ignore the specified 
//semantic.
//
//Wrap should be specified if the texture coordinates wrap.

HRESULT WINAPI D3DXComputeTangent(LPD3DXMESH Mesh,
                                 DWORD TexStage,
                                 DWORD TangentIndex,
                                 DWORD BinormIndex,
                                 DWORD Wrap,
                                 CONST DWORD *pAdjacency);

//============================================================================
//
// UVAtlas apis
//
//============================================================================
typedef HRESULT (WINAPI *LPD3DXUVATLASCB)(FLOAT fPercentDone,  LPVOID lpUserContext);

// This function creates atlases for meshes. There are two modes of operation,
// either based on the number of charts, or the maximum allowed stretch. If the
// maximum allowed stretch is 0, then each triangle will likely be in its own
// chart.

//
// The parameters are as follows:
//  pMesh - Input mesh to calculate an atlas for. This must have a position
//          channel and at least a 2-d texture channel.
//  uMaxChartNumber - The maximum number of charts required for the atlas.
//                    If this is 0, it will be parameterized based solely on
//                    stretch.
//  fMaxStretch - The maximum amount of stretch, if 0, no stretching is allowed,
//                if 1, then any amount of stretching is allowed.
//  uWidth - The width of the texture the atlas will be used on.
//  uHeight - The height of the texture the atlas will be used on.
//  fGutter - The minimum distance, in texels between two charts on the atlas.
//            this gets scaled by the width, so if fGutter is 2.5, and it is
//            used on a 512x512 texture, then the minimum distance will be
//            2.5 / 512 in u-v space.
//  dwTextureIndex - Specifies which texture coordinate to write to in the
//                   output mesh (which is cloned from the input mesh). Useful
//                   if your vertex has multiple texture coordinates.
//  pdwAdjacency - a pointer to an array with 3 DWORDs per face, indicating
//                 which triangles are adjacent to each other.
//  pdwFalseEdgeAdjacency - a pointer to an array with 3 DWORDS per face, indicating
//                          at each face, whether an edge is a false edge or not (using
//                          the same ordering as the adjacency data structure). If this
//                          is NULL, then it is assumed that there are no false edges. If
//                          not NULL, then a non-false edge is indicated by -1 and a false
//                          edge is indicated by any other value (it is not required, but
//                          it may be useful for the caller to use the original adjacency
//                          value). This allows you to parameterize a mesh of quads, and
//                          the edges down the middle of each quad will not be cut when
//                          parameterizing the mesh.
//  pfIMTArray - a pointer to an array with 3 FLOATs per face, describing the
//               integrated metric tensor for that face. This lets you control
//               the way this triangle may be stretched in the atlas. The IMT
//               passed in will be 3 floats (a,b,c) and specify a symmetric
//               matrix (a b) that, given a vector (s,t), specifies the 
//                      (b c)
//               distance between a vector v1 and a vector v2 = v1 + (s,t) as
//               sqrt((s, t) * M * (s, t)^T).
//               In other words, this lets one specify the magnitude of the
//               stretch in an arbitrary direction in u-v space. For example
//               if a = b = c = 1, then this scales the vector (1,1) by 2, and
//               the vector (1,-1) by 0. Note that this is multiplying the edge
//               length by the square of the matrix, so if you want the face to
//               stretch to twice its
//               size with no shearing, the IMT value should be (2, 0, 2), which
//               is just the identity matrix times 2.
//               Note that this assumes you have an orientation for the triangle
//               in some 2-D space. For D3DXUVAtlas, this space is created by
//               letting S be the direction from the first to the second
//               vertex, and T be the cross product between the normal and S.
//               
//  pStatusCallback - Since the atlas creation process can be very CPU intensive,
//                    this allows the programmer to specify a function to be called
//                    periodically, similarly to how it is done in the PRT simulation
//                    engine.
//  fCallbackFrequency - This lets you specify how often the callback will be
//                       called. A decent default should be 0.0001f.
//  pUserContext - a void pointer to be passed back to the callback function
//  dwOptions - A combination of flags in the D3DXUVATLAS enum
//  ppMeshOut - A pointer to a location to store a pointer for the newly created
//              mesh.
//  ppFacePartitioning - A pointer to a location to store a pointer for an array,
//                       one DWORD per face, giving the final partitioning
//                       created by the atlasing algorithm.
//  ppVertexRemapArray - A pointer to a location to store a pointer for an array,
//                       one DWORD per vertex, giving the vertex it was copied
//                       from, if any vertices needed to be split.
//  pfMaxStretchOut - A location to store the maximum stretch resulting from the
//                    atlasing algorithm.
//  puNumChartsOut - A location to store the number of charts created, or if the
//                   maximum number of charts was too low, this gives the minimum
//                    number of charts needed to create an atlas.

HRESULT WINAPI D3DXUVAtlasCreate(LPD3DXMESH pMesh,
                                 UINT uMaxChartNumber,
                                 FLOAT fMaxStretch,
                                 UINT uWidth,
                                 UINT uHeight,
                                 FLOAT fGutter,
                                 DWORD dwTextureIndex,
                                 CONST DWORD *pdwAdjacency,
                                 CONST DWORD *pdwFalseEdgeAdjacency,
                                 CONST FLOAT *pfIMTArray,
                                 LPD3DXUVATLASCB pStatusCallback,
                                 FLOAT fCallbackFrequency,
                                 LPVOID pUserContext,
                                 DWORD dwOptions,
                                 LPD3DXMESH *ppMeshOut,
                                 LPD3DXBUFFER *ppFacePartitioning,
                                 LPD3DXBUFFER *ppVertexRemapArray,
                                 FLOAT *pfMaxStretchOut,
                                 UINT *puNumChartsOut);

// This has the same exact arguments as Create, except that it does not perform the
// final packing step. This method allows one to get a partitioning out, and possibly
// modify it before sending it to be repacked. Note that if you change the
// partitioning, you'll also need to calculate new texture coordinates for any faces
// that have switched charts.
//
// The partition result adjacency output parameter is meant to be passed to the
// UVAtlasPack function, this adjacency cuts edges that are between adjacent
// charts, and also can include cuts inside of a chart in order to make it
// equivalent to a disc. For example:
//
// _______
// | ___ |
// | |_| |
// |_____|
//
// In order to make this equivalent to a disc, we would need to add a cut, and it
// Would end up looking like:
// _______
// | ___ |
// | |_|_|
// |_____|
//
// The resulting partition adjacency parameter cannot be NULL, because it is
// required for the packing step.



HRESULT WINAPI D3DXUVAtlasPartition(LPD3DXMESH pMesh,
                                    UINT uMaxChartNumber,
                                    FLOAT fMaxStretch,
                                    DWORD dwTextureIndex,
                                    CONST DWORD *pdwAdjacency,
                                    CONST DWORD *pdwFalseEdgeAdjacency,
                                    CONST FLOAT *pfIMTArray,
                                    LPD3DXUVATLASCB pStatusCallback,
                                    FLOAT fCallbackFrequency,
                                    LPVOID pUserContext,
                                    DWORD dwOptions,
                                    LPD3DXMESH *ppMeshOut,
                                    LPD3DXBUFFER *ppFacePartitioning,
                                    LPD3DXBUFFER *ppVertexRemapArray,
                                    LPD3DXBUFFER *ppPartitionResultAdjacency,
                                    FLOAT *pfMaxStretchOut,
                                    UINT *puNumChartsOut);

// This takes the face partitioning result from Partition and packs it into an
// atlas of the given size. pdwPartitionResultAdjacency should be derived from
// the adjacency returned from the partition step. This value cannot be NULL
// because Pack needs to know where charts were cut in the partition step in
// order to find the edges of each chart.
// The options parameter is currently reserved.
HRESULT WINAPI D3DXUVAtlasPack(ID3DXMesh *pMesh,
                               UINT uWidth,
                               UINT uHeight,
                               FLOAT fGutter,
                               DWORD dwTextureIndex,
                               CONST DWORD *pdwPartitionResultAdjacency,
                               LPD3DXUVATLASCB pStatusCallback,
                               FLOAT fCallbackFrequency,
                               LPVOID pUserContext,
                               DWORD dwOptions,
                               LPD3DXBUFFER pFacePartitioning);


//============================================================================
//
// IMT Calculation apis
//
// These functions all compute the Integrated Metric Tensor for use in the
// UVAtlas API. They all calculate the IMT with respect to the canonical
// triangle, where the coordinate system is set up so that the u axis goes
// from vertex 0 to 1 and the v axis is N x u. So, for example, the second
// vertex's canonical uv coordinates are (d,0) where d is the distance between
// vertices 0 and 1. This way the IMT does not depend on the parameterization
// of the mesh, and if the signal over the surface doesn't change, then
// the IMT doesn't need to be recalculated.
//============================================================================

// This callback is used by D3DXComputeIMTFromSignal.
//
// uv               - The texture coordinate for the vertex.
// uPrimitiveID     - Face ID of the triangle on which to compute the signal.
// uSignalDimension - The number of floats to store in pfSignalOut.
// pUserData        - The pUserData pointer passed in to ComputeIMTFromSignal.
// pfSignalOut      - A pointer to where to store the signal data.
typedef HRESULT (WINAPI* LPD3DXIMTSIGNALCALLBACK)
    (CONST D3DXVECTOR2 *uv,
     UINT uPrimitiveID,
     UINT uSignalDimension,
     VOID *pUserData,
     FLOAT *pfSignalOut);

// This function is used to calculate the IMT from per vertex data. It sets
// up a linear system over the triangle, solves for the jacobian J, then
// constructs the IMT from that (J^TJ).
// This function allows you to calculate the IMT based off of any value in a
// mesh (color, normal, etc) by specifying the correct stride of the array.
// The IMT computed will cause areas of the mesh that have similar values to
// take up less space in the texture.
//
// pMesh            - The mesh to calculate the IMT for.
// pVertexSignal    - A float array of size uSignalStride * v, where v is the
//                    number of vertices in the mesh.
// uSignalDimension - How many floats per vertex to use in calculating the IMT.
// uSignalStride    - The number of bytes per vertex in the array. This must be
//                    a multiple of sizeof(float)
// ppIMTData        - Where to store the buffer holding the IMT data

HRESULT WINAPI D3DXComputeIMTFromPerVertexSignal (
    LPD3DXMESH pMesh,
    CONST FLOAT *pfVertexSignal, // uSignalDimension floats per vertex
    UINT uSignalDimension,
    UINT uSignalStride,         // stride of signal in bytes
    DWORD dwOptions,            // reserved for future use
    LPD3DXUVATLASCB pStatusCallback,
    LPVOID pUserContext,
    LPD3DXBUFFER *ppIMTData);

// This function is used to calculate the IMT from data that varies over the
// surface of the mesh (generally at a higher frequency than vertex data).
// This function requires the mesh to already be parameterized (so it already
// has texture coordinates). It allows the user to define a signal arbitrarily
// over the surface of the mesh.
//
// pMesh            - The mesh to calculate the IMT for.
// dwTextureIndex   - This describes which set of texture coordinates in the
//                    mesh to use.
// uSignalDimension - How many components there are in the signal.
// fMaxUVDistance   - The subdivision will continue until the distance between
//                    all vertices is at most fMaxUVDistance.
// dwOptions        - reserved for future use
// pSignalCallback  - The callback to use to get the signal.
// pUserData        - A pointer that will be passed in to the callback.
// ppIMTData        - Where to store the buffer holding the IMT data
HRESULT WINAPI D3DXComputeIMTFromSignal(
    LPD3DXMESH pMesh,
    DWORD dwTextureIndex,
    UINT uSignalDimension,
    FLOAT fMaxUVDistance,
    DWORD dwOptions, // reserved for future use
    LPD3DXIMTSIGNALCALLBACK pSignalCallback,
    VOID *pUserData,
    LPD3DXUVATLASCB pStatusCallback,
    LPVOID pUserContext,
    LPD3DXBUFFER *ppIMTData);

// This function is used to calculate the IMT from texture data. Given a texture
// that maps over the surface of the mesh, the algorithm computes the IMT for
// each face. This will cause large areas that are very similar to take up less
// room when parameterized with UVAtlas. The texture is assumed to be
// interpolated over the mesh bilinearly.
//
// pMesh            - The mesh to calculate the IMT for.
// pTexture         - The texture to load data from.
// dwTextureIndex   - This describes which set of texture coordinates in the
//                    mesh to use.
// dwOptions        - Combination of one or more D3DXIMT flags.
// ppIMTData        - Where to store the buffer holding the IMT data
HRESULT WINAPI D3DXComputeIMTFromTexture (
    LPD3DXMESH pMesh,
    LPDIRECT3DTEXTURE9 pTexture,
    DWORD dwTextureIndex,
    DWORD dwOptions,
    LPD3DXUVATLASCB pStatusCallback,
    LPVOID pUserContext,
    LPD3DXBUFFER *ppIMTData);

// This function is very similar to ComputeIMTFromTexture, but it uses a
// float array to pass in the data, and it can calculate higher dimensional
// values than 4.
//
// pMesh            - The mesh to calculate the IMT for.
// dwTextureIndex   - This describes which set of texture coordinates in the
//                    mesh to use.
// pfFloatArray     - a pointer to a float array of size
//                    uWidth*uHeight*uComponents
// uWidth           - The width of the texture
// uHeight          - The height of the texture
// uSignalDimension - The number of floats per texel in the signal
// uComponents      - The number of floats in each texel
// dwOptions        - Combination of one or more D3DXIMT flags
// ppIMTData        - Where to store the buffer holding the IMT data
HRESULT WINAPI D3DXComputeIMTFromPerTexelSignal(
    LPD3DXMESH pMesh,
    DWORD dwTextureIndex,
    FLOAT *pfTexelSignal,
    UINT uWidth,
    UINT uHeight,
    UINT uSignalDimension,
    UINT uComponents,
    DWORD dwOptions,
    LPD3DXUVATLASCB pStatusCallback,
    LPVOID pUserContext,
    LPD3DXBUFFER *ppIMTData);

HRESULT WINAPI
    D3DXConvertMeshSubsetToSingleStrip(
        LPD3DXBASEMESH MeshIn,
        DWORD AttribId,
        DWORD IBOptions,
        LPDIRECT3DINDEXBUFFER9 *ppIndexBuffer,
        DWORD *pNumIndices);

HRESULT WINAPI
    D3DXConvertMeshSubsetToStrips(
        LPD3DXBASEMESH MeshIn,
        DWORD AttribId,
        DWORD IBOptions,
        LPDIRECT3DINDEXBUFFER9 *ppIndexBuffer,
        DWORD *pNumIndices,
        LPD3DXBUFFER *ppStripLengths,
        DWORD *pNumStrips);

        
//============================================================================
//
//  D3DXOptimizeFaces:
//  --------------------
//  Generate a face remapping for a triangle list that more effectively utilizes
//    vertex caches.  This optimization is identical to the one provided
//    by ID3DXMesh::Optimize with the hardware independent option enabled.
//
//  Parameters:
//   pbIndices
//      Triangle list indices to use for generating a vertex ordering
//   NumFaces
//      Number of faces in the triangle list
//   NumVertices
//      Number of vertices referenced by the triangle list
//   b32BitIndices
//      TRUE if indices are 32 bit, FALSE if indices are 16 bit
//   pFaceRemap
//      Destination buffer to store face ordering
//      The number stored for a given element is where in the new ordering
//        the face will have come from.  See ID3DXMesh::Optimize for more info.
//
//============================================================================
HRESULT WINAPI
    D3DXOptimizeFaces(
        LPCVOID pbIndices, 
        UINT cFaces, 
        UINT cVertices, 
        BOOL b32BitIndices, 
        DWORD* pFaceRemap);
        
//============================================================================
//
//  D3DXOptimizeVertices:
//  --------------------
//  Generate a vertex remapping to optimize for in order use of vertices for 
//    a given set of indices.  This is commonly used after applying the face
//    remap generated by D3DXOptimizeFaces
//
//  Parameters:
//   pbIndices
//      Triangle list indices to use for generating a vertex ordering
//   NumFaces
//      Number of faces in the triangle list
//   NumVertices
//      Number of vertices referenced by the triangle list
//   b32BitIndices
//      TRUE if indices are 32 bit, FALSE if indices are 16 bit
//   pVertexRemap
//      Destination buffer to store vertex ordering
//      The number stored for a given element is where in the new ordering
//        the vertex will have come from.  See ID3DXMesh::Optimize for more info.
//
//============================================================================
HRESULT WINAPI
    D3DXOptimizeVertices(
        LPCVOID pbIndices, 
        UINT cFaces, 
        UINT cVertices, 
        BOOL b32BitIndices, 
        DWORD* pVertexRemap);

#ifdef __cplusplus
}
#endif //__cplusplus


//===========================================================================
//
//  Data structures for Spherical Harmonic Precomputation
//
//
//============================================================================

typedef enum _D3DXSHCOMPRESSQUALITYTYPE {
    D3DXSHCQUAL_FASTLOWQUALITY  = 1,
    D3DXSHCQUAL_SLOWHIGHQUALITY = 2,
    D3DXSHCQUAL_FORCE_DWORD     = 0x7fffffff
} D3DXSHCOMPRESSQUALITYTYPE;

typedef enum _D3DXSHGPUSIMOPT {
    D3DXSHGPUSIMOPT_SHADOWRES256  = 1,
    D3DXSHGPUSIMOPT_SHADOWRES512  = 0,
    D3DXSHGPUSIMOPT_SHADOWRES1024 = 2,
    D3DXSHGPUSIMOPT_SHADOWRES2048 = 3,

    D3DXSHGPUSIMOPT_HIGHQUALITY = 4,    
    
    D3DXSHGPUSIMOPT_FORCE_DWORD = 0x7fffffff
} D3DXSHGPUSIMOPT;

// for all properties that are colors the luminance is computed
// if the simulator is run with a single channel using the following
// formula:  R * 0.2125 + G * 0.7154 + B * 0.0721

typedef struct _D3DXSHMATERIAL {
    D3DCOLORVALUE Diffuse;  // Diffuse albedo of the surface.  (Ignored if object is a Mirror)
    BOOL          bMirror;  // Must be set to FALSE.  bMirror == TRUE not currently supported
    BOOL          bSubSurf; // true if the object does subsurface scattering - can't do this and be a mirror

    // subsurface scattering parameters 
    FLOAT         RelativeIndexOfRefraction;
    D3DCOLORVALUE Absorption;
    D3DCOLORVALUE ReducedScattering;

} D3DXSHMATERIAL;

// allocated in D3DXSHPRTCompSplitMeshSC
// vertices are duplicated into multiple super clusters but
// only have a valid status in one super cluster (fill in the rest)

typedef struct _D3DXSHPRTSPLITMESHVERTDATA {
    UINT  uVertRemap;   // vertex in original mesh this corresponds to
    UINT  uSubCluster;  // cluster index relative to super cluster
    UCHAR ucVertStatus; // 1 if vertex has valid data, 0 if it is "fill"
} D3DXSHPRTSPLITMESHVERTDATA;

// used in D3DXSHPRTCompSplitMeshSC
// information for each super cluster that maps into face/vert arrays

typedef struct _D3DXSHPRTSPLITMESHCLUSTERDATA {
    UINT uVertStart;     // initial index into remapped vertex array
    UINT uVertLength;    // number of vertices in this super cluster
    
    UINT uFaceStart;     // initial index into face array
    UINT uFaceLength;    // number of faces in this super cluster
    
    UINT uClusterStart;  // initial index into cluster array
    UINT uClusterLength; // number of clusters in this super cluster
} D3DXSHPRTSPLITMESHCLUSTERDATA;

// call back function for simulator
// return S_OK to keep running the simulator - anything else represents
// failure and the simulator will abort.

typedef HRESULT (WINAPI *LPD3DXSHPRTSIMCB)(float fPercentDone,  LPVOID lpUserContext);

// interfaces for PRT buffers/simulator

// GUIDs
// {F1827E47-00A8-49cd-908C-9D11955F8728}
DEFINE_GUID(IID_ID3DXPRTBuffer, 
0xf1827e47, 0xa8, 0x49cd, 0x90, 0x8c, 0x9d, 0x11, 0x95, 0x5f, 0x87, 0x28);

// {A758D465-FE8D-45ad-9CF0-D01E56266A07}
DEFINE_GUID(IID_ID3DXPRTCompBuffer, 
0xa758d465, 0xfe8d, 0x45ad, 0x9c, 0xf0, 0xd0, 0x1e, 0x56, 0x26, 0x6a, 0x7);

// {838F01EC-9729-4527-AADB-DF70ADE7FEA9}
DEFINE_GUID(IID_ID3DXTextureGutterHelper, 
0x838f01ec, 0x9729, 0x4527, 0xaa, 0xdb, 0xdf, 0x70, 0xad, 0xe7, 0xfe, 0xa9);

// {683A4278-CD5F-4d24-90AD-C4E1B6855D53}
DEFINE_GUID(IID_ID3DXPRTEngine, 
0x683a4278, 0xcd5f, 0x4d24, 0x90, 0xad, 0xc4, 0xe1, 0xb6, 0x85, 0x5d, 0x53);

// interface defenitions

typedef interface ID3DXTextureGutterHelper ID3DXTextureGutterHelper;
typedef interface ID3DXPRTBuffer ID3DXPRTBuffer;

#undef INTERFACE
#define INTERFACE ID3DXPRTBuffer

// Buffer interface - contains "NumSamples" samples
// each sample in memory is stored as NumCoeffs scalars per channel (1 or 3)
// Same interface is used for both Vertex and Pixel PRT buffers

DECLARE_INTERFACE_(ID3DXPRTBuffer, IUnknown)
{
    // IUnknown
    STDMETHOD(QueryInterface)(THIS_ REFIID iid, LPVOID *ppv) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

    // ID3DXPRTBuffer
    STDMETHOD_(UINT, GetNumSamples)(THIS) PURE;
    STDMETHOD_(UINT, GetNumCoeffs)(THIS) PURE;
    STDMETHOD_(UINT, GetNumChannels)(THIS) PURE;

    STDMETHOD_(BOOL, IsTexture)(THIS) PURE;
    STDMETHOD_(UINT, GetWidth)(THIS) PURE;
    STDMETHOD_(UINT, GetHeight)(THIS) PURE;

    // changes the number of samples allocated in the buffer
    STDMETHOD(Resize)(THIS_ UINT NewSize) PURE;

    // ppData will point to the memory location where sample Start begins
    // pointer is valid for at least NumSamples samples
    STDMETHOD(LockBuffer)(THIS_ UINT Start, UINT NumSamples, FLOAT **ppData) PURE;
    STDMETHOD(UnlockBuffer)(THIS) PURE;

    // every scalar in buffer is multiplied by Scale
    STDMETHOD(ScaleBuffer)(THIS_ FLOAT Scale) PURE;
    
    // every scalar contains the sum of this and pBuffers values
    // pBuffer must have the same storage class/dimensions 
    STDMETHOD(AddBuffer)(THIS_ LPD3DXPRTBUFFER pBuffer) PURE;

    // GutterHelper (described below) will fill in the gutter
    // regions of a texture by interpolating "internal" values
    STDMETHOD(AttachGH)(THIS_ LPD3DXTEXTUREGUTTERHELPER) PURE;
    STDMETHOD(ReleaseGH)(THIS) PURE;
    
    // Evaluates attached gutter helper on the contents of this buffer
    STDMETHOD(EvalGH)(THIS) PURE;

    // extracts a given channel into texture pTexture
    // NumCoefficients starting from StartCoefficient are copied
    STDMETHOD(ExtractTexture)(THIS_ UINT Channel, UINT StartCoefficient, 
                              UINT NumCoefficients, LPDIRECT3DTEXTURE9 pTexture) PURE;

    // extracts NumCoefficients coefficients into mesh - only applicable on single channel
    // buffers, otherwise just lockbuffer and copy data.  With SHPRT data NumCoefficients 
    // should be Order^2
    STDMETHOD(ExtractToMesh)(THIS_ UINT NumCoefficients, D3DDECLUSAGE Usage, UINT UsageIndexStart,
                             LPD3DXMESH pScene) PURE;

};

typedef interface ID3DXPRTCompBuffer ID3DXPRTCompBuffer;
typedef interface ID3DXPRTCompBuffer *LPD3DXPRTCOMPBUFFER;

#undef INTERFACE
#define INTERFACE ID3DXPRTCompBuffer

// compressed buffers stored a compressed version of a PRTBuffer

DECLARE_INTERFACE_(ID3DXPRTCompBuffer, IUnknown)
{
    // IUnknown
    STDMETHOD(QueryInterface)(THIS_ REFIID iid, LPVOID *ppv) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

    // ID3DPRTCompBuffer

    // NumCoeffs and NumChannels are properties of input buffer
    STDMETHOD_(UINT, GetNumSamples)(THIS) PURE;
    STDMETHOD_(UINT, GetNumCoeffs)(THIS) PURE;
    STDMETHOD_(UINT, GetNumChannels)(THIS) PURE;

    STDMETHOD_(BOOL, IsTexture)(THIS) PURE;
    STDMETHOD_(UINT, GetWidth)(THIS) PURE;
    STDMETHOD_(UINT, GetHeight)(THIS) PURE;

    // number of clusters, and PCA vectors per-cluster
    STDMETHOD_(UINT, GetNumClusters)(THIS) PURE;
    STDMETHOD_(UINT, GetNumPCA)(THIS) PURE;

    // normalizes PCA weights so that they are between [-1,1]
    // basis vectors are modified to reflect this
    STDMETHOD(NormalizeData)(THIS) PURE;

    // copies basis vectors for cluster "Cluster" into pClusterBasis
    // (NumPCA+1)*NumCoeffs*NumChannels floats
    STDMETHOD(ExtractBasis)(THIS_ UINT Cluster, FLOAT *pClusterBasis) PURE;
    
    // UINT per sample - which cluster it belongs to
    STDMETHOD(ExtractClusterIDs)(THIS_ UINT *pClusterIDs) PURE;
    
    // copies NumExtract PCA projection coefficients starting at StartPCA
    // into pPCACoefficients - NumSamples*NumExtract floats copied
    STDMETHOD(ExtractPCA)(THIS_ UINT StartPCA, UINT NumExtract, FLOAT *pPCACoefficients) PURE;

    // copies NumPCA projection coefficients starting at StartPCA
    // into pTexture - should be able to cope with signed formats
    STDMETHOD(ExtractTexture)(THIS_ UINT StartPCA, UINT NumpPCA, 
                              LPDIRECT3DTEXTURE9 pTexture) PURE;
                              
    // copies NumPCA projection coefficients into mesh pScene
    // Usage is D3DDECLUSAGE where coefficients are to be stored
    // UsageIndexStart is starting index
    STDMETHOD(ExtractToMesh)(THIS_ UINT NumPCA, D3DDECLUSAGE Usage, UINT UsageIndexStart,
                             LPD3DXMESH pScene) PURE;
};


#undef INTERFACE
#define INTERFACE ID3DXTextureGutterHelper

// ID3DXTextureGutterHelper will build and manage
// "gutter" regions in a texture - this will allow for
// bi-linear interpolation to not have artifacts when rendering
// It generates a map (in texture space) where each texel
// is in one of 3 states:
//   0  Invalid - not used at all
//   1  Inside triangle
//   2  Gutter texel
//   4  represents a gutter texel that will be computed during PRT
// For each Inside/Gutter texel it stores the face it
// belongs to and barycentric coordinates for the 1st two
// vertices of that face.  Gutter vertices are assigned to
// the closest edge in texture space.
//
// When used with PRT this requires a unique parameterization
// of the model - every texel must correspond to a single point
// on the surface of the model and vice versa

DECLARE_INTERFACE_(ID3DXTextureGutterHelper, IUnknown)
{
    // IUnknown
    STDMETHOD(QueryInterface)(THIS_ REFIID iid, LPVOID *ppv) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

    // ID3DXTextureGutterHelper
    
    // dimensions of texture this is bound too
    STDMETHOD_(UINT, GetWidth)(THIS) PURE;
    STDMETHOD_(UINT, GetHeight)(THIS) PURE;


    // Applying gutters recomputes all of the gutter texels of class "2"
    // based on texels of class "1" or "4"
    
    // Applies gutters to a raw float buffer - each texel is NumCoeffs floats
    // Width and Height must match GutterHelper
    STDMETHOD(ApplyGuttersFloat)(THIS_ FLOAT *pDataIn, UINT NumCoeffs, UINT Width, UINT Height);
    
    // Applies gutters to pTexture
    // Dimensions must match GutterHelper
    STDMETHOD(ApplyGuttersTex)(THIS_ LPDIRECT3DTEXTURE9 pTexture);
    
    // Applies gutters to a D3DXPRTBuffer
    // Dimensions must match GutterHelper
    STDMETHOD(ApplyGuttersPRT)(THIS_ LPD3DXPRTBUFFER pBuffer);
       
    // Resamples a texture from a mesh onto this gutterhelpers
    // parameterization.  It is assumed that the UV coordinates
    // for this gutter helper are in TEXTURE 0 (usage/usage index)
    // and the texture coordinates should all be within [0,1] for
    // both sets.
    //
    // pTextureIn - texture represented using parameterization in pMeshIn
    // pMeshIn    - Mesh with texture coordinates that represent pTextureIn
    //              pTextureOut texture coordinates are assumed to be in
    //              TEXTURE 0
    // Usage      - field in DECL for pMeshIn that stores texture coordinates
    //              for pTextureIn
    // UsageIndex - which index for Usage above for pTextureIn
    // pTextureOut- Resampled texture
    // 
    // Usage would generally be D3DDECLUSAGE_TEXCOORD  and UsageIndex other than zero
    STDMETHOD(ResampleTex)(THIS_ LPDIRECT3DTEXTURE9 pTextureIn,
                                 LPD3DXMESH pMeshIn,
                                 D3DDECLUSAGE Usage, UINT UsageIndex,
                                 LPDIRECT3DTEXTURE9 pTextureOut);    
    
    // the routines below provide access to the data structures
    // used by the Apply functions

    // face map is a UINT per texel that represents the
    // face of the mesh that texel belongs too - 
    // only valid if same texel is valid in pGutterData
    // pFaceData must be allocated by the user
    STDMETHOD(GetFaceMap)(THIS_ UINT *pFaceData) PURE;
    
    // BaryMap is a D3DXVECTOR2 per texel
    // the 1st two barycentric coordinates for the corresponding
    // face (3rd weight is always 1-sum of first two)
    // only valid if same texel is valid in pGutterData
    // pBaryData must be allocated by the user
    STDMETHOD(GetBaryMap)(THIS_ D3DXVECTOR2 *pBaryData) PURE;
    
    // TexelMap is a D3DXVECTOR2 per texel that
    // stores the location in pixel coordinates where the
    // corresponding texel is mapped
    // pTexelData must be allocated by the user
    STDMETHOD(GetTexelMap)(THIS_ D3DXVECTOR2 *pTexelData) PURE;
    
    // GutterMap is a BYTE per texel
    // 0/1/2 for Invalid/Internal/Gutter texels
    // 4 represents a gutter texel that will be computed
    // during PRT
    // pGutterData must be allocated by the user
    STDMETHOD(GetGutterMap)(THIS_ BYTE *pGutterData) PURE;
    
    // face map is a UINT per texel that represents the
    // face of the mesh that texel belongs too - 
    // only valid if same texel is valid in pGutterData
    STDMETHOD(SetFaceMap)(THIS_ UINT *pFaceData) PURE;
    
    // BaryMap is a D3DXVECTOR2 per texel
    // the 1st two barycentric coordinates for the corresponding
    // face (3rd weight is always 1-sum of first two)
    // only valid if same texel is valid in pGutterData
    STDMETHOD(SetBaryMap)(THIS_ D3DXVECTOR2 *pBaryData) PURE;
    
    // TexelMap is a D3DXVECTOR2 per texel that
    // stores the location in pixel coordinates where the
    // corresponding texel is mapped
    STDMETHOD(SetTexelMap)(THIS_ D3DXVECTOR2 *pTexelData) PURE;
    
    // GutterMap is a BYTE per texel
    // 0/1/2 for Invalid/Internal/Gutter texels
    // 4 represents a gutter texel that will be computed
    // during PRT
    STDMETHOD(SetGutterMap)(THIS_ BYTE *pGutterData) PURE;    
};


typedef interface ID3DXPRTEngine ID3DXPRTEngine;
typedef interface ID3DXPRTEngine *LPD3DXPRTENGINE;

#undef INTERFACE
#define INTERFACE ID3DXPRTEngine

// ID3DXPRTEngine is used to compute a PRT simulation
// Use the following steps to compute PRT for SH
// (1) create an interface (which includes a scene)
// (2) call SetSamplingInfo
// (3) [optional] Set MeshMaterials/albedo's (required if doing bounces)
// (4) call ComputeDirectLightingSH
// (5) [optional] call ComputeBounce
// repeat step 5 for as many bounces as wanted.
// if you want to model subsurface scattering you
// need to call ComputeSS after direct lighting and
// each bounce.
// If you want to bake the albedo into the PRT signal, you
// must call MutliplyAlbedo, otherwise the user has to multiply
// the albedo themselves.  Not multiplying the albedo allows you
// to model albedo variation at a finer scale then illumination, and
// can result in better compression results.
// Luminance values are computed from RGB values using the following
// formula:  R * 0.2125 + G * 0.7154 + B * 0.0721

DECLARE_INTERFACE_(ID3DXPRTEngine, IUnknown)
{
    // IUnknown
    STDMETHOD(QueryInterface)(THIS_ REFIID iid, LPVOID *ppv) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

    // ID3DXPRTEngine
    
    // This sets a material per attribute in the scene mesh and it is
    // the only way to specify subsurface scattering parameters.  if
    // bSetAlbedo is FALSE, NumChannels must match the current
    // configuration of the PRTEngine.  If you intend to change
    // NumChannels (through some other SetAlbedo function) it must
    // happen before SetMeshMaterials is called.
    //
    // NumChannels 1 implies "grayscale" materials, set this to 3 to enable
    //  color bleeding effects
    // bSetAlbedo sets albedo from material if TRUE - which clobbers per texel/vertex
    //  albedo that might have been set before.  FALSE won't clobber.
    // fLengthScale is used for subsurface scattering - scene is mapped into a 1mm unit cube
    //  and scaled by this amount
    STDMETHOD(SetMeshMaterials)(THIS_ CONST D3DXSHMATERIAL **ppMaterials, UINT NumMeshes, 
                                UINT NumChannels, BOOL bSetAlbedo, FLOAT fLengthScale) PURE;
    
    // setting albedo per-vertex or per-texel over rides the albedos stored per mesh
    // but it does not over ride any other settings
    
    // sets an albedo to be used per vertex - the albedo is represented as a float
    // pDataIn input pointer (pointint to albedo of 1st sample)
    // NumChannels 1 implies "grayscale" materials, set this to 3 to enable
    //  color bleeding effects
    // Stride - stride in bytes to get to next samples albedo
    STDMETHOD(SetPerVertexAlbedo)(THIS_ CONST VOID *pDataIn, UINT NumChannels, UINT Stride) PURE;
    
    // represents the albedo per-texel instead of per-vertex (even if per-vertex PRT is used)
    // pAlbedoTexture - texture that stores the albedo (dimension arbitrary)
    // NumChannels 1 implies "grayscale" materials, set this to 3 to enable
    //  color bleeding effects
    // pGH - optional gutter helper, otherwise one is constructed in computation routines and
    //  destroyed (if not attached to buffers)
    STDMETHOD(SetPerTexelAlbedo)(THIS_ LPDIRECT3DTEXTURE9 pAlbedoTexture, 
                                 UINT NumChannels, 
                                 LPD3DXTEXTUREGUTTERHELPER pGH) PURE;
                                 
    // gets the per-vertex albedo
    STDMETHOD(GetVertexAlbedo)(THIS_ D3DXCOLOR *pVertColors, UINT NumVerts) PURE;                                 
                                 
    // If pixel PRT is being computed normals default to ones that are interpolated
    // from the vertex normals.  This specifies a texture that stores an object
    // space normal map instead (must use a texture format that can represent signed values)
    // pNormalTexture - normal map, must be same dimensions as PRTBuffers, signed                                
    STDMETHOD(SetPerTexelNormal)(THIS_ LPDIRECT3DTEXTURE9 pNormalTexture) PURE;
                                 
    // Copies per-vertex albedo from mesh
    // pMesh - mesh that represents the scene.  It must have the same
    //  properties as the mesh used to create the PRTEngine
    // Usage - D3DDECLUSAGE to extract albedos from
    // NumChannels 1 implies "grayscale" materials, set this to 3 to enable
    //  color bleeding effects
    STDMETHOD(ExtractPerVertexAlbedo)(THIS_ LPD3DXMESH pMesh, 
                                      D3DDECLUSAGE Usage, 
                                      UINT NumChannels) PURE;

    // Resamples the input buffer into the output buffer
    // can be used to move between per-vertex and per-texel buffers.  This can also be used
    // to convert single channel buffers to 3-channel buffers and vice-versa.
    STDMETHOD(ResampleBuffer)(THIS_ LPD3DXPRTBUFFER pBufferIn, LPD3DXPRTBUFFER pBufferOut) PURE;
    
    // Returns the scene mesh - including modifications from adaptive spatial sampling
    // The returned mesh only has positions, normals and texture coordinates (if defined)
    // pD3DDevice - d3d device that will be used to allocate the mesh
    // pFaceRemap - each face has a pointer back to the face on the original mesh that it comes from
    //  if the face hasn't been subdivided this will be an identity mapping
    // pVertRemap - each vertex contains 3 vertices that this is a linear combination of
    // pVertWeights - weights for each of above indices (sum to 1.0f)
    // ppMesh - mesh that will be allocated and filled
    STDMETHOD(GetAdaptedMesh)(THIS_ LPDIRECT3DDEVICE9 pD3DDevice,UINT *pFaceRemap, UINT *pVertRemap, FLOAT *pfVertWeights, LPD3DXMESH *ppMesh) PURE;

    // Number of vertices currently allocated (includes new vertices from adaptive sampling)
    STDMETHOD_(UINT, GetNumVerts)(THIS) PURE;
    // Number of faces currently allocated (includes new faces)
    STDMETHOD_(UINT, GetNumFaces)(THIS) PURE;

    // Sets the Minimum/Maximum intersection distances, this can be used to control
    // maximum distance that objects can shadow/reflect light, and help with "bad"
    // art that might have near features that you don't want to shadow.  This does not
    // apply for GPU simulations.
    //  fMin - minimum intersection distance, must be positive and less than fMax
    //  fMax - maximum intersection distance, if 0.0f use the previous value, otherwise
    //      must be strictly greater than fMin
    STDMETHOD(SetMinMaxIntersection)(THIS_ FLOAT fMin, FLOAT fMax) PURE;

    // This will subdivide faces on a mesh so that adaptively simulations can
    // use a more conservative threshold (it won't miss features.)
    // MinEdgeLength - minimum edge length that will be generated, if 0.0f a
    //  reasonable default will be used
    // MaxSubdiv - maximum level of subdivision, if 0 is specified a default
    //  value will be used (5)
    STDMETHOD(RobustMeshRefine)(THIS_ FLOAT MinEdgeLength, UINT MaxSubdiv) PURE;

    // This sets to sampling information used by the simulator.  Adaptive sampling
    // parameters are currently ignored.
    // NumRays - number of rays to shoot per sample
    // UseSphere - if TRUE uses spherical samples, otherwise samples over
    //  the hemisphere.  Should only be used with GPU and Vol computations
    // UseCosine - if TRUE uses a cosine weighting - not used for Vol computations
    //  or if only the visiblity function is desired
    // Adaptive - if TRUE adaptive sampling (angular) is used
    // AdaptiveThresh - threshold used to terminate adaptive angular sampling
    //  ignored if adaptive sampling is not set
    STDMETHOD(SetSamplingInfo)(THIS_ UINT NumRays, 
                               BOOL UseSphere, 
                               BOOL UseCosine, 
                               BOOL Adaptive, 
                               FLOAT AdaptiveThresh) PURE;

    // Methods that compute the direct lighting contribution for objects
    // always represente light using spherical harmonics (SH)
    // the albedo is not multiplied by the signal - it just integrates
    // incoming light.  If NumChannels is not 1 the vector is replicated
    //
    // SHOrder - order of SH to use
    // pDataOut - PRT buffer that is generated.  Can be single channel
    STDMETHOD(ComputeDirectLightingSH)(THIS_ UINT SHOrder, 
                                       LPD3DXPRTBUFFER pDataOut) PURE;
                                       
    // Adaptive variant of above function.  This will refine the mesh
    // generating new vertices/faces to approximate the PRT signal
    // more faithfully.
    // SHOrder - order of SH to use
    // AdaptiveThresh - threshold for adaptive subdivision (in PRT vector error)
    //  if value is less then 1e-6f, 1e-6f is specified
    // MinEdgeLength - minimum edge length that will be generated
    //  if value is too small a fairly conservative model dependent value
    //  is used
    // MaxSubdiv - maximum subdivision level, if 0 is specified it 
    //  will default to 4
    // pDataOut - PRT buffer that is generated.  Can be single channel.
    STDMETHOD(ComputeDirectLightingSHAdaptive)(THIS_ UINT SHOrder, 
                                               FLOAT AdaptiveThresh,
                                               FLOAT MinEdgeLength,
                                               UINT MaxSubdiv,
                                               LPD3DXPRTBUFFER pDataOut) PURE;
                                       
    // Function that computes the direct lighting contribution for objects
    // light is always represented using spherical harmonics (SH)
    // This is done on the GPU and is much faster then using the CPU.
    // The albedo is not multiplied by the signal - it just integrates
    // incoming light.  If NumChannels is not 1 the vector is replicated.
    // ZBias/ZAngleBias are akin to parameters used with shadow zbuffers.
    // A reasonable default for both values is 0.005, but the user should
    // experiment (ZAngleBias can be zero, ZBias should not be.)
    // Callbacks should not use the Direct3D9Device the simulator is using.
    // SetSamplingInfo must be called with TRUE for UseSphere and
    // FALSE for UseCosine before this method is called.
    //
    // pD3DDevice - device used to run GPU simulator - must support PS2.0
    //  and FP render targets
    // Flags - parameters for the GPU simulator, combination of one or more
    //  D3DXSHGPUSIMOPT flags.  Only one SHADOWRES setting should be set and
    //  the defaults is 512
    // SHOrder - order of SH to use
    // ZBias - bias in normal direction (for depth test)
    // ZAngleBias - scaled by one minus cosine of angle with light (offset in depth)
    // pDataOut - PRT buffer that is filled in.  Can be single channel
    STDMETHOD(ComputeDirectLightingSHGPU)(THIS_ LPDIRECT3DDEVICE9 pD3DDevice,
                                          UINT Flags,
                                          UINT SHOrder,
                                          FLOAT ZBias,
                                          FLOAT ZAngleBias,
                                          LPD3DXPRTBUFFER pDataOut) PURE;


    // Functions that computes subsurface scattering (using material properties)
    // Albedo is not multiplied by result.  This only works for per-vertex data
    // use ResampleBuffer to move per-vertex data into a texture and back.
    //
    // pDataIn - input data (previous bounce)
    // pDataOut - result of subsurface scattering simulation
    // pDataTotal - [optional] results can be summed into this buffer
    STDMETHOD(ComputeSS)(THIS_ LPD3DXPRTBUFFER pDataIn, 
                         LPD3DXPRTBUFFER pDataOut, LPD3DXPRTBUFFER pDataTotal) PURE;

    // Adaptive version of ComputeSS.
    //
    // pDataIn - input data (previous bounce)
    // AdaptiveThresh - threshold for adaptive subdivision (in PRT vector error)
    //  if value is less then 1e-6f, 1e-6f is specified
    // MinEdgeLength - minimum edge length that will be generated
    //  if value is too small a fairly conservative model dependent value
    //  is used
    // MaxSubdiv - maximum subdivision level, if 0 is specified it 
    //  will default to 4    
    // pDataOut - result of subsurface scattering simulation
    // pDataTotal - [optional] results can be summed into this buffer
    STDMETHOD(ComputeSSAdaptive)(THIS_ LPD3DXPRTBUFFER pDataIn, 
                                 FLOAT AdaptiveThresh,
                                 FLOAT MinEdgeLength,
                                 UINT MaxSubdiv,
                                 LPD3DXPRTBUFFER pDataOut, LPD3DXPRTBUFFER pDataTotal) PURE;

    // computes a single bounce of inter-reflected light
    // works for SH based PRT or generic lighting
    // Albedo is not multiplied by result
    //
    // pDataIn - previous bounces data 
    // pDataOut - PRT buffer that is generated
    // pDataTotal - [optional] can be used to keep a running sum
    STDMETHOD(ComputeBounce)(THIS_ LPD3DXPRTBUFFER pDataIn,
                             LPD3DXPRTBUFFER pDataOut,
                             LPD3DXPRTBUFFER pDataTotal) PURE;

    // Adaptive version of above function.
    //
    // pDataIn - previous bounces data, can be single channel 
    // AdaptiveThresh - threshold for adaptive subdivision (in PRT vector error)
    //  if value is less then 1e-6f, 1e-6f is specified
    // MinEdgeLength - minimum edge length that will be generated
    //  if value is too small a fairly conservative model dependent value
    //  is used
    // MaxSubdiv - maximum subdivision level, if 0 is specified it 
    //  will default to 4
    // pDataOut - PRT buffer that is generated
    // pDataTotal - [optional] can be used to keep a running sum    
    STDMETHOD(ComputeBounceAdaptive)(THIS_ LPD3DXPRTBUFFER pDataIn,
                                     FLOAT AdaptiveThresh,
                                     FLOAT MinEdgeLength,
                                     UINT MaxSubdiv,
                                     LPD3DXPRTBUFFER pDataOut,
                                     LPD3DXPRTBUFFER pDataTotal) PURE;

    // Computes projection of distant SH radiance into a local SH radiance
    // function.  This models how direct lighting is attenuated by the 
    // scene and is a form of "neighborhood transfer."  The result is
    // a linear operator (matrix) at every sample point, if you multiply
    // this matrix by the distant SH lighting coefficients you get an
    // approximation of the local incident radiance function from
    // direct lighting.  These resulting lighting coefficients can
    // than be projected into another basis or used with any rendering
    // technique that uses spherical harmonics as input.
    // SetSamplingInfo must be called with TRUE for UseSphere and
    // FALSE for UseCosine before this method is called.  
    // Generates SHOrderIn*SHOrderIn*SHOrderOut*SHOrderOut scalars 
    // per channel at each sample location.
    //
    // SHOrderIn  - Order of the SH representation of distant lighting
    // SHOrderOut - Order of the SH representation of local lighting
    // NumVolSamples  - Number of sample locations
    // pSampleLocs    - position of sample locations
    // pDataOut       - PRT Buffer that will store output results    
    STDMETHOD(ComputeVolumeSamplesDirectSH)(THIS_ UINT SHOrderIn, 
                                            UINT SHOrderOut, 
                                            UINT NumVolSamples,
                                            CONST D3DXVECTOR3 *pSampleLocs,
                                            LPD3DXPRTBUFFER pDataOut) PURE;
                                    
    // At each sample location computes a linear operator (matrix) that maps
    // the representation of source radiance (NumCoeffs in pSurfDataIn)
    // into a local incident radiance function approximated with spherical 
    // harmonics.  For example if a light map data is specified in pSurfDataIn
    // the result is an SH representation of the flow of light at each sample
    // point.  If PRT data for an outdoor scene is used, each sample point
    // contains a matrix that models how distant lighting bounces of the objects
    // in the scene and arrives at the given sample point.  Combined with
    // ComputeVolumeSamplesDirectSH this gives the complete representation for
    // how light arrives at each sample point parameterized by distant lighting.
    // SetSamplingInfo must be called with TRUE for UseSphere and
    // FALSE for UseCosine before this method is called.    
    // Generates pSurfDataIn->NumCoeffs()*SHOrder*SHOrder scalars
    // per channel at each sample location.
    //
    // pSurfDataIn    - previous bounce data
    // SHOrder        - order of SH to generate projection with
    // NumVolSamples  - Number of sample locations
    // pSampleLocs    - position of sample locations
    // pDataOut       - PRT Buffer that will store output results
    STDMETHOD(ComputeVolumeSamples)(THIS_ LPD3DXPRTBUFFER pSurfDataIn, 
                                    UINT SHOrder, 
                                    UINT NumVolSamples,
                                    CONST D3DXVECTOR3 *pSampleLocs,
                                    LPD3DXPRTBUFFER pDataOut) PURE;

    // Computes direct lighting (SH) for a point not on the mesh
    // with a given normal - cannot use texture buffers.
    //
    // SHOrder      - order of SH to use
    // NumSamples   - number of sample locations
    // pSampleLocs  - position for each sample
    // pSampleNorms - normal for each sample
    // pDataOut     - PRT Buffer that will store output results
    STDMETHOD(ComputeSurfSamplesDirectSH)(THIS_ UINT SHOrder,
                                          UINT NumSamples,
                                          CONST D3DXVECTOR3 *pSampleLocs,
                                          CONST D3DXVECTOR3 *pSampleNorms,
                                          LPD3DXPRTBUFFER pDataOut) PURE;


    // given the solution for PRT or light maps, computes transfer vector at arbitrary 
    // position/normal pairs in space
    //
    // pSurfDataIn  - input data
    // NumSamples   - number of sample locations
    // pSampleLocs  - position for each sample
    // pSampleNorms - normal for each sample
    // pDataOut     - PRT Buffer that will store output results
    // pDataTotal   - optional buffer to sum results into - can be NULL
    STDMETHOD(ComputeSurfSamplesBounce)(THIS_ LPD3DXPRTBUFFER pSurfDataIn,
                                        UINT NumSamples,
                                        CONST D3DXVECTOR3 *pSampleLocs,
                                        CONST D3DXVECTOR3 *pSampleNorms,
                                        LPD3DXPRTBUFFER pDataOut,
                                        LPD3DXPRTBUFFER pDataTotal) PURE;

    // Frees temporary data structures that can be created for subsurface scattering
    // this data is freed when the PRTComputeEngine is freed and is lazily created
    STDMETHOD(FreeSSData)(THIS) PURE;
    
    // Frees temporary data structures that can be created for bounce simulations
    // this data is freed when the PRTComputeEngine is freed and is lazily created
    STDMETHOD(FreeBounceData)(THIS) PURE;

    // This computes the Local Deformable PRT (LDPRT) coefficients relative to the 
    // per sample normals that minimize error in a least squares sense with respect 
    // to the input PRT data set.  These coefficients can be used with skinned/transformed 
    // normals to model global effects with dynamic objects.  Shading normals can 
    // optionally be solved for - these normals (along with the LDPRT coefficients) can
    // more accurately represent the PRT signal.  The coefficients are for zonal
    // harmonics oriented in the normal/shading normal direction.
    //
    // pDataIn  - SH PRT dataset that is input
    // SHOrder  - Order of SH to compute conv coefficients for 
    // pNormOut - Optional array of vectors (passed in) that will be filled with
    //             "shading normals", LDPRT coefficients are optimized for
    //             these normals.  This array must be the same size as the number of
    //             samples in pDataIn
    // pDataOut - Output buffer (SHOrder zonal harmonic coefficients per channel per sample)
    STDMETHOD(ComputeLDPRTCoeffs)(THIS_ LPD3DXPRTBUFFER pDataIn,
                                  UINT SHOrder,
                                  D3DXVECTOR3 *pNormOut,
                                  LPD3DXPRTBUFFER pDataOut) PURE;

    // scales all the samples associated with a given sub mesh
    // can be useful when using subsurface scattering
    // fScale - value to scale each vector in submesh by
    STDMETHOD(ScaleMeshChunk)(THIS_ UINT uMeshChunk, FLOAT fScale, LPD3DXPRTBUFFER pDataOut) PURE;
    
    // mutliplies each PRT vector by the albedo - can be used if you want to have the albedo
    // burned into the dataset, often better not to do this.  If this is not done the user
    // must mutliply the albedo themselves when rendering - just multiply the albedo times
    // the result of the PRT dot product.
    // If pDataOut is a texture simulation result and there is an albedo texture it
    // must be represented at the same resolution as the simulation buffer.  You can use
    // LoadSurfaceFromSurface and set a new albedo texture if this is an issue - but must
    // be careful about how the gutters are handled.
    //
    // pDataOut - dataset that will get albedo pushed into it
    STDMETHOD(MultiplyAlbedo)(THIS_ LPD3DXPRTBUFFER pDataOut) PURE;
    
    // Sets a pointer to an optional call back function that reports back to the
    // user percentage done and gives them the option of quitting
    // pCB - pointer to call back function, return S_OK for the simulation
    //  to continue
    // Frequency - 1/Frequency is roughly the number of times the call back
    //  will be invoked
    // lpUserContext - will be passed back to the users call back
    STDMETHOD(SetCallBack)(THIS_ LPD3DXSHPRTSIMCB pCB, FLOAT Frequency,  LPVOID lpUserContext) PURE;
    
    // Returns TRUE if the ray intersects the mesh, FALSE if it does not.  This function
    // takes into account settings from SetMinMaxIntersection.  If the closest intersection
    // is not needed this function is more efficient compared to the ClosestRayIntersection
    // method.
    // pRayPos - origin of ray
    // pRayDir - normalized ray direction (normalization required for SetMinMax to be meaningful)
    
    STDMETHOD_(BOOL, ShadowRayIntersects)(THIS_ CONST D3DXVECTOR3 *pRayPos, CONST D3DXVECTOR3 *pRayDir) PURE;
    
    // Returns TRUE if the ray intersects the mesh, FALSE if it does not.  If there is an
    // intersection the closest face that was intersected and its first two barycentric coordinates
    // are returned.  This function takes into account settings from SetMinMaxIntersection.
    // This is a slower function compared to ShadowRayIntersects and should only be used where
    // needed.  The third vertices barycentric coordinates will be 1 - pU - pV.
    // pRayPos     - origin of ray
    // pRayDir     - normalized ray direction (normalization required for SetMinMax to be meaningful)
    // pFaceIndex  - Closest face that intersects.  This index is based on stacking the pBlockerMesh
    //  faces before the faces from pMesh
    // pU          - Barycentric coordinate for vertex 0
    // pV          - Barycentric coordinate for vertex 1
    // pDist       - Distance along ray where the intersection occured
    
    STDMETHOD_(BOOL, ClosestRayIntersects)(THIS_ CONST D3DXVECTOR3 *pRayPos, CONST D3DXVECTOR3 *pRayDir,
                                           DWORD *pFaceIndex, FLOAT *pU, FLOAT *pV, FLOAT *pDist) PURE;
};


// API functions for creating interfaces

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

//============================================================================
//
//  D3DXCreatePRTBuffer:
//  --------------------
//  Generates a PRT Buffer that can be compressed or filled by a simulator
//  This function should be used to create per-vertex or volume buffers.
//  When buffers are created all values are initialized to zero.
//
//  Parameters:
//    NumSamples
//      Number of sample locations represented
//    NumCoeffs
//      Number of coefficients per sample location (order^2 for SH)
//    NumChannels
//      Number of color channels to represent (1 or 3)
//    ppBuffer
//      Buffer that will be allocated
//
//============================================================================

HRESULT WINAPI 
    D3DXCreatePRTBuffer( 
        UINT NumSamples,
        UINT NumCoeffs,
        UINT NumChannels,
        LPD3DXPRTBUFFER* ppBuffer);

//============================================================================
//
//  D3DXCreatePRTBufferTex:
//  --------------------
//  Generates a PRT Buffer that can be compressed or filled by a simulator
//  This function should be used to create per-pixel buffers.
//  When buffers are created all values are initialized to zero.
//
//  Parameters:
//    Width
//      Width of texture
//    Height
//      Height of texture
//    NumCoeffs
//      Number of coefficients per sample location (order^2 for SH)
//    NumChannels
//      Number of color channels to represent (1 or 3)
//    ppBuffer
//      Buffer that will be allocated
//
//============================================================================

HRESULT WINAPI
    D3DXCreatePRTBufferTex( 
        UINT Width,
        UINT Height,
        UINT NumCoeffs,
        UINT NumChannels,
        LPD3DXPRTBUFFER* ppBuffer);

//============================================================================
//
//  D3DXLoadPRTBufferFromFile:
//  --------------------
//  Loads a PRT buffer that has been saved to disk.
//
//  Parameters:
//    pFilename
//      Name of the file to load
//    ppBuffer
//      Buffer that will be allocated
//
//============================================================================

HRESULT WINAPI
    D3DXLoadPRTBufferFromFileA(
        LPCSTR pFilename, 
        LPD3DXPRTBUFFER*       ppBuffer);
        
HRESULT WINAPI
    D3DXLoadPRTBufferFromFileW(
        LPCWSTR pFilename, 
        LPD3DXPRTBUFFER*       ppBuffer);

#ifdef UNICODE
#define D3DXLoadPRTBufferFromFile D3DXLoadPRTBufferFromFileW
#else
#define D3DXLoadPRTBufferFromFile D3DXLoadPRTBufferFromFileA
#endif


//============================================================================
//
//  D3DXSavePRTBufferToFile:
//  --------------------
//  Saves a PRTBuffer to disk.
//
//  Parameters:
//    pFilename
//      Name of the file to save
//    pBuffer
//      Buffer that will be saved
//
//============================================================================

HRESULT WINAPI
    D3DXSavePRTBufferToFileA(
        LPCSTR pFileName,
        LPD3DXPRTBUFFER pBuffer);
        
HRESULT WINAPI
    D3DXSavePRTBufferToFileW(
        LPCWSTR pFileName,
        LPD3DXPRTBUFFER pBuffer);

#ifdef UNICODE
#define D3DXSavePRTBufferToFile D3DXSavePRTBufferToFileW
#else
#define D3DXSavePRTBufferToFile D3DXSavePRTBufferToFileA
#endif                


//============================================================================
//
//  D3DXLoadPRTCompBufferFromFile:
//  --------------------
//  Loads a PRTComp buffer that has been saved to disk.
//
//  Parameters:
//    pFilename
//      Name of the file to load
//    ppBuffer
//      Buffer that will be allocated
//
//============================================================================

HRESULT WINAPI
    D3DXLoadPRTCompBufferFromFileA(
        LPCSTR pFilename, 
        LPD3DXPRTCOMPBUFFER*       ppBuffer);
        
HRESULT WINAPI
    D3DXLoadPRTCompBufferFromFileW(
        LPCWSTR pFilename, 
        LPD3DXPRTCOMPBUFFER*       ppBuffer);

#ifdef UNICODE
#define D3DXLoadPRTCompBufferFromFile D3DXLoadPRTCompBufferFromFileW
#else
#define D3DXLoadPRTCompBufferFromFile D3DXLoadPRTCompBufferFromFileA
#endif

//============================================================================
//
//  D3DXSavePRTCompBufferToFile:
//  --------------------
//  Saves a PRTCompBuffer to disk.
//
//  Parameters:
//    pFilename
//      Name of the file to save
//    pBuffer
//      Buffer that will be saved
//
//============================================================================

HRESULT WINAPI
    D3DXSavePRTCompBufferToFileA(
        LPCSTR pFileName,
        LPD3DXPRTCOMPBUFFER pBuffer);
        
HRESULT WINAPI
    D3DXSavePRTCompBufferToFileW(
        LPCWSTR pFileName,
        LPD3DXPRTCOMPBUFFER pBuffer);

#ifdef UNICODE
#define D3DXSavePRTCompBufferToFile D3DXSavePRTCompBufferToFileW
#else
#define D3DXSavePRTCompBufferToFile D3DXSavePRTCompBufferToFileA
#endif 

//============================================================================
//
//  D3DXCreatePRTCompBuffer:
//  --------------------
//  Compresses a PRT buffer (vertex or texel)
//
//  Parameters:
//    D3DXSHCOMPRESSQUALITYTYPE
//      Quality of compression - low is faster (computes PCA per voronoi cluster)
//      high is slower but better quality (clusters based on distance to affine subspace)
//    NumClusters
//      Number of clusters to compute
//    NumPCA
//      Number of basis vectors to compute
//    pCB
//      Optional Callback function
//    lpUserContext
//      Optional user context
//    pBufferIn
//      Buffer that will be compressed
//    ppBufferOut
//      Compressed buffer that will be created
//
//============================================================================


HRESULT WINAPI
    D3DXCreatePRTCompBuffer(
        D3DXSHCOMPRESSQUALITYTYPE Quality,
        UINT NumClusters, 
        UINT NumPCA,
        LPD3DXSHPRTSIMCB pCB,
        LPVOID lpUserContext,        
        LPD3DXPRTBUFFER  pBufferIn,
        LPD3DXPRTCOMPBUFFER *ppBufferOut
    );

//============================================================================
//
//  D3DXCreateTextureGutterHelper:
//  --------------------
//  Generates a "GutterHelper" for a given set of meshes and texture
//  resolution
//
//  Parameters:
//    Width
//      Width of texture
//    Height
//      Height of texture
//    pMesh
//      Mesh that represents the scene
//    GutterSize
//      Number of texels to over rasterize in texture space
//      this should be at least 1.0
//    ppBuffer
//      GutterHelper that will be created
//
//============================================================================


HRESULT WINAPI 
    D3DXCreateTextureGutterHelper( 
        UINT Width,
        UINT Height,
        LPD3DXMESH pMesh, 
        FLOAT GutterSize,
        LPD3DXTEXTUREGUTTERHELPER* ppBuffer);


//============================================================================
//
//  D3DXCreatePRTEngine:
//  --------------------
//  Computes a PRTEngine which can efficiently generate PRT simulations
//  of a scene
//
//  Parameters:
//    pMesh
//      Mesh that represents the scene - must have an AttributeTable
//      where vertices are in a unique attribute.
//    pAdjacency
//      Optional adjacency information
//    ExtractUVs
//      Set this to true if textures are going to be used for albedos
//      or to store PRT vectors
//    pBlockerMesh
//      Optional mesh that just blocks the scene
//    ppEngine
//      PRTEngine that will be created
//
//============================================================================


HRESULT WINAPI 
    D3DXCreatePRTEngine( 
        LPD3DXMESH pMesh,
        DWORD *pAdjacency,
        BOOL ExtractUVs,
        LPD3DXMESH pBlockerMesh, 
        LPD3DXPRTENGINE* ppEngine);

//============================================================================
//
//  D3DXConcatenateMeshes:
//  --------------------
//  Concatenates a group of meshes into one common mesh.  This can optionaly transform
//  each sub mesh or its texture coordinates.  If no DECL is given it will
//  generate a union of all of the DECL's of the sub meshes, promoting channels
//  and types if neccesary.  It will create an AttributeTable if possible, one can
//  call OptimizeMesh with attribute sort and compacting enabled to ensure this.
//
//  Parameters:
//    ppMeshes
//      Array of pointers to meshes that can store PRT vectors
//    NumMeshes
//      Number of meshes
//    Options
//      Passed through to D3DXCreateMesh
//    pGeomXForms
//      [optional] Each sub mesh is transformed by the corresponding
//      matrix if this array is supplied
//    pTextureXForms
//      [optional] UV coordinates for each sub mesh are transformed
//      by corresponding matrix if supplied
//    pDecl
//      [optional] Only information in this DECL is used when merging
//      data
//    pD3DDevice
//      D3D device that is used to create the new mesh
//    ppMeshOut
//      Mesh that will be created
//
//============================================================================


HRESULT WINAPI 
    D3DXConcatenateMeshes(
        LPD3DXMESH *ppMeshes, 
        UINT NumMeshes, 
        DWORD Options, 
        CONST D3DXMATRIX *pGeomXForms, 
        CONST D3DXMATRIX *pTextureXForms, 
        CONST D3DVERTEXELEMENT9 *pDecl,
        LPDIRECT3DDEVICE9 pD3DDevice, 
        LPD3DXMESH *ppMeshOut);

//============================================================================
//
//  D3DXSHPRTCompSuperCluster:
//  --------------------------
//  Used with compressed results of D3DXSHPRTSimulation.
//  Generates "super clusters" - groups of clusters that can be drawn in
//  the same draw call.  A greedy algorithm that minimizes overdraw is used
//  to group the clusters.
//
//  Parameters:
//   pClusterIDs
//      NumVerts cluster ID's (extracted from a compressed buffer)
//   pScene
//      Mesh that represents composite scene passed to the simulator
//   MaxNumClusters
//      Maximum number of clusters allocated per super cluster
//   NumClusters
//      Number of clusters computed in the simulator
//   pSuperClusterIDs
//      Array of length NumClusters, contains index of super cluster
//      that corresponding cluster was assigned to
//   pNumSuperClusters
//      Returns the number of super clusters allocated
//      
//============================================================================

HRESULT WINAPI 
    D3DXSHPRTCompSuperCluster(
        UINT *pClusterIDs, 
        LPD3DXMESH pScene, 
        UINT MaxNumClusters, 
        UINT NumClusters,
        UINT *pSuperClusterIDs, 
        UINT *pNumSuperClusters);

//============================================================================
//
//  D3DXSHPRTCompSplitMeshSC:
//  -------------------------
//  Used with compressed results of the vertex version of the PRT simulator.
//  After D3DXSHRTCompSuperCluster has been called this function can be used
//  to split the mesh into a group of faces/vertices per super cluster.
//  Each super cluster contains all of the faces that contain any vertex
//  classified in one of its clusters.  All of the vertices connected to this
//  set of faces are also included with the returned array ppVertStatus 
//  indicating whether or not the vertex belongs to the supercluster.
//
//  Parameters:
//   pClusterIDs
//      NumVerts cluster ID's (extracted from a compressed buffer)
//   NumVertices
//      Number of vertices in original mesh
//   NumClusters
//      Number of clusters (input parameter to compression)
//   pSuperClusterIDs
//      Array of size NumClusters that will contain super cluster ID's (from
//      D3DXSHCompSuerCluster)
//   NumSuperClusters
//      Number of superclusters allocated in D3DXSHCompSuerCluster
//   pInputIB
//      Raw index buffer for mesh - format depends on bInputIBIs32Bit
//   InputIBIs32Bit
//      Indicates whether the input index buffer is 32-bit (otherwise 16-bit
//      is assumed)
//   NumFaces
//      Number of faces in the original mesh (pInputIB is 3 times this length)
//   ppIBData
//      LPD3DXBUFFER holds raw index buffer that will contain the resulting split faces.  
//      Format determined by bIBIs32Bit.  Allocated by function
//   pIBDataLength
//      Length of ppIBData, assigned in function
//   OutputIBIs32Bit
//      Indicates whether the output index buffer is to be 32-bit (otherwise 
//      16-bit is assumed)
//   ppFaceRemap
//      LPD3DXBUFFER mapping of each face in ppIBData to original faces.  Length is
//      *pIBDataLength/3.  Optional paramter, allocated in function
//   ppVertData
//      LPD3DXBUFFER contains new vertex data structure.  Size of pVertDataLength
//   pVertDataLength
//      Number of new vertices in split mesh.  Assigned in function
//   pSCClusterList
//      Array of length NumClusters which pSCData indexes into (Cluster* fields)
//      for each SC, contains clusters sorted by super cluster
//   pSCData
//      Structure per super cluster - contains indices into ppIBData,
//      pSCClusterList and ppVertData
//
//============================================================================

HRESULT WINAPI 
    D3DXSHPRTCompSplitMeshSC(
        UINT *pClusterIDs, 
        UINT NumVertices, 
        UINT NumClusters, 
        UINT *pSuperClusterIDs, 
        UINT NumSuperClusters,
        LPVOID pInputIB, 
        BOOL InputIBIs32Bit, 
        UINT NumFaces,
        LPD3DXBUFFER *ppIBData, 
        UINT *pIBDataLength, 
        BOOL OutputIBIs32Bit, 
        LPD3DXBUFFER *ppFaceRemap, 
        LPD3DXBUFFER *ppVertData, 
        UINT *pVertDataLength, 
        UINT *pSCClusterList,
        D3DXSHPRTSPLITMESHCLUSTERDATA *pSCData);
        
        
#ifdef __cplusplus
}
#endif //__cplusplus

//////////////////////////////////////////////////////////////////////////////
//
//  Definitions of .X file templates used by mesh load/save functions 
//    that are not RM standard
//
//////////////////////////////////////////////////////////////////////////////

// {3CF169CE-FF7C-44ab-93C0-F78F62D172E2}
DEFINE_GUID(DXFILEOBJ_XSkinMeshHeader,
0x3cf169ce, 0xff7c, 0x44ab, 0x93, 0xc0, 0xf7, 0x8f, 0x62, 0xd1, 0x72, 0xe2);

// {B8D65549-D7C9-4995-89CF-53A9A8B031E3}
DEFINE_GUID(DXFILEOBJ_VertexDuplicationIndices, 
0xb8d65549, 0xd7c9, 0x4995, 0x89, 0xcf, 0x53, 0xa9, 0xa8, 0xb0, 0x31, 0xe3);

// {A64C844A-E282-4756-8B80-250CDE04398C}
DEFINE_GUID(DXFILEOBJ_FaceAdjacency, 
0xa64c844a, 0xe282, 0x4756, 0x8b, 0x80, 0x25, 0xc, 0xde, 0x4, 0x39, 0x8c);

// {6F0D123B-BAD2-4167-A0D0-80224F25FABB}
DEFINE_GUID(DXFILEOBJ_SkinWeights, 
0x6f0d123b, 0xbad2, 0x4167, 0xa0, 0xd0, 0x80, 0x22, 0x4f, 0x25, 0xfa, 0xbb);

// {A3EB5D44-FC22-429d-9AFB-3221CB9719A6}
DEFINE_GUID(DXFILEOBJ_Patch, 
0xa3eb5d44, 0xfc22, 0x429d, 0x9a, 0xfb, 0x32, 0x21, 0xcb, 0x97, 0x19, 0xa6);

// {D02C95CC-EDBA-4305-9B5D-1820D7704BBF}
DEFINE_GUID(DXFILEOBJ_PatchMesh, 
0xd02c95cc, 0xedba, 0x4305, 0x9b, 0x5d, 0x18, 0x20, 0xd7, 0x70, 0x4b, 0xbf);

// {B9EC94E1-B9A6-4251-BA18-94893F02C0EA}
DEFINE_GUID(DXFILEOBJ_PatchMesh9, 
0xb9ec94e1, 0xb9a6, 0x4251, 0xba, 0x18, 0x94, 0x89, 0x3f, 0x2, 0xc0, 0xea);

// {B6C3E656-EC8B-4b92-9B62-681659522947}
DEFINE_GUID(DXFILEOBJ_PMInfo, 
0xb6c3e656, 0xec8b, 0x4b92, 0x9b, 0x62, 0x68, 0x16, 0x59, 0x52, 0x29, 0x47);

// {917E0427-C61E-4a14-9C64-AFE65F9E9844}
DEFINE_GUID(DXFILEOBJ_PMAttributeRange, 
0x917e0427, 0xc61e, 0x4a14, 0x9c, 0x64, 0xaf, 0xe6, 0x5f, 0x9e, 0x98, 0x44);

// {574CCC14-F0B3-4333-822D-93E8A8A08E4C}
DEFINE_GUID(DXFILEOBJ_PMVSplitRecord,
0x574ccc14, 0xf0b3, 0x4333, 0x82, 0x2d, 0x93, 0xe8, 0xa8, 0xa0, 0x8e, 0x4c);

// {B6E70A0E-8EF9-4e83-94AD-ECC8B0C04897}
DEFINE_GUID(DXFILEOBJ_FVFData, 
0xb6e70a0e, 0x8ef9, 0x4e83, 0x94, 0xad, 0xec, 0xc8, 0xb0, 0xc0, 0x48, 0x97);

// {F752461C-1E23-48f6-B9F8-8350850F336F}
DEFINE_GUID(DXFILEOBJ_VertexElement, 
0xf752461c, 0x1e23, 0x48f6, 0xb9, 0xf8, 0x83, 0x50, 0x85, 0xf, 0x33, 0x6f);

// {BF22E553-292C-4781-9FEA-62BD554BDD93}
DEFINE_GUID(DXFILEOBJ_DeclData, 
0xbf22e553, 0x292c, 0x4781, 0x9f, 0xea, 0x62, 0xbd, 0x55, 0x4b, 0xdd, 0x93);

// {F1CFE2B3-0DE3-4e28-AFA1-155A750A282D}
DEFINE_GUID(DXFILEOBJ_EffectFloats, 
0xf1cfe2b3, 0xde3, 0x4e28, 0xaf, 0xa1, 0x15, 0x5a, 0x75, 0xa, 0x28, 0x2d);

// {D55B097E-BDB6-4c52-B03D-6051C89D0E42}
DEFINE_GUID(DXFILEOBJ_EffectString, 
0xd55b097e, 0xbdb6, 0x4c52, 0xb0, 0x3d, 0x60, 0x51, 0xc8, 0x9d, 0xe, 0x42);

// {622C0ED0-956E-4da9-908A-2AF94F3CE716}
DEFINE_GUID(DXFILEOBJ_EffectDWord, 
0x622c0ed0, 0x956e, 0x4da9, 0x90, 0x8a, 0x2a, 0xf9, 0x4f, 0x3c, 0xe7, 0x16);

// {3014B9A0-62F5-478c-9B86-E4AC9F4E418B}
DEFINE_GUID(DXFILEOBJ_EffectParamFloats, 
0x3014b9a0, 0x62f5, 0x478c, 0x9b, 0x86, 0xe4, 0xac, 0x9f, 0x4e, 0x41, 0x8b);

// {1DBC4C88-94C1-46ee-9076-2C28818C9481}
DEFINE_GUID(DXFILEOBJ_EffectParamString, 
0x1dbc4c88, 0x94c1, 0x46ee, 0x90, 0x76, 0x2c, 0x28, 0x81, 0x8c, 0x94, 0x81);

// {E13963BC-AE51-4c5d-B00F-CFA3A9D97CE5}
DEFINE_GUID(DXFILEOBJ_EffectParamDWord,
0xe13963bc, 0xae51, 0x4c5d, 0xb0, 0xf, 0xcf, 0xa3, 0xa9, 0xd9, 0x7c, 0xe5);

// {E331F7E4-0559-4cc2-8E99-1CEC1657928F}
DEFINE_GUID(DXFILEOBJ_EffectInstance, 
0xe331f7e4, 0x559, 0x4cc2, 0x8e, 0x99, 0x1c, 0xec, 0x16, 0x57, 0x92, 0x8f);

// {9E415A43-7BA6-4a73-8743-B73D47E88476}
DEFINE_GUID(DXFILEOBJ_AnimTicksPerSecond, 
0x9e415a43, 0x7ba6, 0x4a73, 0x87, 0x43, 0xb7, 0x3d, 0x47, 0xe8, 0x84, 0x76);

// {7F9B00B3-F125-4890-876E-1CFFBF697C4D}
DEFINE_GUID(DXFILEOBJ_CompressedAnimationSet, 
0x7f9b00b3, 0xf125, 0x4890, 0x87, 0x6e, 0x1c, 0x42, 0xbf, 0x69, 0x7c, 0x4d);

#pragma pack(push, 1)
typedef struct _XFILECOMPRESSEDANIMATIONSET
{
    DWORD CompressedBlockSize;
    FLOAT TicksPerSec;
    DWORD PlaybackType;
    DWORD BufferLength;
} XFILECOMPRESSEDANIMATIONSET;
#pragma pack(pop)

#define XSKINEXP_TEMPLATES \
        "xof 0303txt 0032\
        template XSkinMeshHeader \
        { \
            <3CF169CE-FF7C-44ab-93C0-F78F62D172E2> \
            WORD nMaxSkinWeightsPerVertex; \
            WORD nMaxSkinWeightsPerFace; \
            WORD nBones; \
        } \
        template VertexDuplicationIndices \
        { \
            <B8D65549-D7C9-4995-89CF-53A9A8B031E3> \
            DWORD nIndices; \
            DWORD nOriginalVertices; \
            array DWORD indices[nIndices]; \
        } \
        template FaceAdjacency \
        { \
            <A64C844A-E282-4756-8B80-250CDE04398C> \
            DWORD nIndices; \
            array DWORD indices[nIndices]; \
        } \
        template SkinWeights \
        { \
            <6F0D123B-BAD2-4167-A0D0-80224F25FABB> \
            STRING transformNodeName; \
            DWORD nWeights; \
            array DWORD vertexIndices[nWeights]; \
            array float weights[nWeights]; \
            Matrix4x4 matrixOffset; \
        } \
        template Patch \
        { \
            <A3EB5D44-FC22-429D-9AFB-3221CB9719A6> \
            DWORD nControlIndices; \
            array DWORD controlIndices[nControlIndices]; \
        } \
        template PatchMesh \
        { \
            <D02C95CC-EDBA-4305-9B5D-1820D7704BBF> \
            DWORD nVertices; \
            array Vector vertices[nVertices]; \
            DWORD nPatches; \
            array Patch patches[nPatches]; \
            [ ... ] \
        } \
        template PatchMesh9 \
        { \
            <B9EC94E1-B9A6-4251-BA18-94893F02C0EA> \
            DWORD Type; \
            DWORD Degree; \
            DWORD Basis; \
            DWORD nVertices; \
            array Vector vertices[nVertices]; \
            DWORD nPatches; \
            array Patch patches[nPatches]; \
            [ ... ] \
        } " \
        "template EffectFloats \
        { \
            <F1CFE2B3-0DE3-4e28-AFA1-155A750A282D> \
            DWORD nFloats; \
            array float Floats[nFloats]; \
        } \
        template EffectString \
        { \
            <D55B097E-BDB6-4c52-B03D-6051C89D0E42> \
            STRING Value; \
        } \
        template EffectDWord \
        { \
            <622C0ED0-956E-4da9-908A-2AF94F3CE716> \
            DWORD Value; \
        } " \
        "template EffectParamFloats \
        { \
            <3014B9A0-62F5-478c-9B86-E4AC9F4E418B> \
            STRING ParamName; \
            DWORD nFloats; \
            array float Floats[nFloats]; \
        } " \
        "template EffectParamString \
        { \
            <1DBC4C88-94C1-46ee-9076-2C28818C9481> \
            STRING ParamName; \
            STRING Value; \
        } \
        template EffectParamDWord \
        { \
            <E13963BC-AE51-4c5d-B00F-CFA3A9D97CE5> \
            STRING ParamName; \
            DWORD Value; \
        } \
        template EffectInstance \
        { \
            <E331F7E4-0559-4cc2-8E99-1CEC1657928F> \
            STRING EffectFilename; \
            [ ... ] \
        } " \
        "template AnimTicksPerSecond \
        { \
            <9E415A43-7BA6-4a73-8743-B73D47E88476> \
            DWORD AnimTicksPerSecond; \
        } \
        template CompressedAnimationSet \
        { \
            <7F9B00B3-F125-4890-876E-1C42BF697C4D> \
            DWORD CompressedBlockSize; \
            FLOAT TicksPerSec; \
            DWORD PlaybackType; \
            DWORD BufferLength; \
            array DWORD CompressedData[BufferLength]; \
        } "

#define XEXTENSIONS_TEMPLATES \
        "xof 0303txt 0032\
        template FVFData \
        { \
            <B6E70A0E-8EF9-4e83-94AD-ECC8B0C04897> \
            DWORD dwFVF; \
            DWORD nDWords; \
            array DWORD data[nDWords]; \
        } \
        template VertexElement \
        { \
            <F752461C-1E23-48f6-B9F8-8350850F336F> \
            DWORD Type; \
            DWORD Method; \
            DWORD Usage; \
            DWORD UsageIndex; \
        } \
        template DeclData \
        { \
            <BF22E553-292C-4781-9FEA-62BD554BDD93> \
            DWORD nElements; \
            array VertexElement Elements[nElements]; \
            DWORD nDWords; \
            array DWORD data[nDWords]; \
        } \
        template PMAttributeRange \
        { \
            <917E0427-C61E-4a14-9C64-AFE65F9E9844> \
            DWORD iFaceOffset; \
            DWORD nFacesMin; \
            DWORD nFacesMax; \
            DWORD iVertexOffset; \
            DWORD nVerticesMin; \
            DWORD nVerticesMax; \
        } \
        template PMVSplitRecord \
        { \
            <574CCC14-F0B3-4333-822D-93E8A8A08E4C> \
            DWORD iFaceCLW; \
            DWORD iVlrOffset; \
            DWORD iCode; \
        } \
        template PMInfo \
        { \
            <B6C3E656-EC8B-4b92-9B62-681659522947> \
            DWORD nAttributes; \
            array PMAttributeRange attributeRanges[nAttributes]; \
            DWORD nMaxValence; \
            DWORD nMinLogicalVertices; \
            DWORD nMaxLogicalVertices; \
            DWORD nVSplits; \
            array PMVSplitRecord splitRecords[nVSplits]; \
            DWORD nAttributeMispredicts; \
            array DWORD attributeMispredicts[nAttributeMispredicts]; \
        } "
        
#endif //__D3DX9MESH_H__


