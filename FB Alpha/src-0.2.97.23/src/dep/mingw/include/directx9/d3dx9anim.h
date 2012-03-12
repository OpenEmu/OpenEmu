//////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
//  File:       d3dx9anim.h
//  Content:    D3DX mesh types and functions
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __D3DX9ANIM_H__
#define __D3DX9ANIM_H__

// {698CFB3F-9289-4d95-9A57-33A94B5A65F9}
DEFINE_GUID(IID_ID3DXAnimationSet, 
0x698cfb3f, 0x9289, 0x4d95, 0x9a, 0x57, 0x33, 0xa9, 0x4b, 0x5a, 0x65, 0xf9);

// {FA4E8E3A-9786-407d-8B4C-5995893764AF}
DEFINE_GUID(IID_ID3DXKeyframedAnimationSet, 
0xfa4e8e3a, 0x9786, 0x407d, 0x8b, 0x4c, 0x59, 0x95, 0x89, 0x37, 0x64, 0xaf);

// {6CC2480D-3808-4739-9F88-DE49FACD8D4C}
DEFINE_GUID(IID_ID3DXCompressedAnimationSet, 
0x6cc2480d, 0x3808, 0x4739, 0x9f, 0x88, 0xde, 0x49, 0xfa, 0xcd, 0x8d, 0x4c);

// {AC8948EC-F86D-43e2-96DE-31FC35F96D9E}
DEFINE_GUID(IID_ID3DXAnimationController, 
0xac8948ec, 0xf86d, 0x43e2, 0x96, 0xde, 0x31, 0xfc, 0x35, 0xf9, 0x6d, 0x9e);


//----------------------------------------------------------------------------
// D3DXMESHDATATYPE:
// -----------------
// This enum defines the type of mesh data present in a MeshData structure.
//----------------------------------------------------------------------------
typedef enum _D3DXMESHDATATYPE {
    D3DXMESHTYPE_MESH      = 0x001,             // Normal ID3DXMesh data
    D3DXMESHTYPE_PMESH     = 0x002,             // Progressive Mesh - ID3DXPMesh
    D3DXMESHTYPE_PATCHMESH = 0x003,             // Patch Mesh - ID3DXPatchMesh

    D3DXMESHTYPE_FORCE_DWORD    = 0x7fffffff   /* force 32-bit size enum */
} D3DXMESHDATATYPE;

//----------------------------------------------------------------------------
// D3DXMESHDATA:
// -------------
// This struct encapsulates a the mesh data that can be present in a mesh 
// container.  The supported mesh types are pMesh, pPMesh, pPatchMesh.
// The valid way to access this is determined by the Type enum.
//----------------------------------------------------------------------------
typedef struct _D3DXMESHDATA
{
    D3DXMESHDATATYPE Type;

    // current mesh data interface
    union
    {
        LPD3DXMESH              pMesh;
        LPD3DXPMESH             pPMesh;
        LPD3DXPATCHMESH         pPatchMesh;
    };
} D3DXMESHDATA, *LPD3DXMESHDATA;

//----------------------------------------------------------------------------
// D3DXMESHCONTAINER:
// ------------------
// This struct encapsulates a mesh object in a transformation frame
// hierarchy. The app can derive from this structure to add other app specific
// data to this.
//----------------------------------------------------------------------------
typedef struct _D3DXMESHCONTAINER
{
    LPSTR                   Name;

    D3DXMESHDATA            MeshData;

    LPD3DXMATERIAL          pMaterials;
    LPD3DXEFFECTINSTANCE    pEffects;
    DWORD                   NumMaterials;
    DWORD                  *pAdjacency;

    LPD3DXSKININFO          pSkinInfo;

    struct _D3DXMESHCONTAINER *pNextMeshContainer;
} D3DXMESHCONTAINER, *LPD3DXMESHCONTAINER;

//----------------------------------------------------------------------------
// D3DXFRAME:
// ----------
// This struct is the encapsulates a transform frame in a transformation frame
// hierarchy. The app can derive from this structure to add other app specific
// data to this
//----------------------------------------------------------------------------
typedef struct _D3DXFRAME
{
    LPSTR                   Name;
    D3DXMATRIX              TransformationMatrix;

    LPD3DXMESHCONTAINER     pMeshContainer;

    struct _D3DXFRAME       *pFrameSibling;
    struct _D3DXFRAME       *pFrameFirstChild;
} D3DXFRAME, *LPD3DXFRAME;


//----------------------------------------------------------------------------
// ID3DXAllocateHierarchy:
// -----------------------
// This interface is implemented by the application to allocate/free frame and
// mesh container objects. Methods on this are called during loading and
// destroying frame hierarchies
//----------------------------------------------------------------------------
typedef interface ID3DXAllocateHierarchy ID3DXAllocateHierarchy;
typedef interface ID3DXAllocateHierarchy *LPD3DXALLOCATEHIERARCHY;

#undef INTERFACE
#define INTERFACE ID3DXAllocateHierarchy

DECLARE_INTERFACE(ID3DXAllocateHierarchy)
{
    // ID3DXAllocateHierarchy

	//------------------------------------------------------------------------
	// CreateFrame:
	// ------------
	// Requests allocation of a frame object.
	//
	// Parameters:
	//  Name
	//		Name of the frame to be created
	//	ppNewFrame
	//		Returns the created frame object
	//
	//------------------------------------------------------------------------
    STDMETHOD(CreateFrame)(THIS_ LPCSTR Name, 
                            LPD3DXFRAME *ppNewFrame) PURE;

	//------------------------------------------------------------------------
	// CreateMeshContainer:
	// --------------------
	// Requests allocation of a mesh container object.
	//
	// Parameters:
	//  Name
	//		Name of the mesh
	//	pMesh
	//		Pointer to the mesh object if basic polygon data found
	//	pPMesh
	//		Pointer to the progressive mesh object if progressive mesh data found
	//	pPatchMesh
	//		Pointer to the patch mesh object if patch data found
	//	pMaterials
	//		Array of materials used in the mesh
	//	pEffectInstances
	//		Array of effect instances used in the mesh
	//	NumMaterials
	//		Num elements in the pMaterials array
	//	pAdjacency
	//		Adjacency array for the mesh
	//	pSkinInfo
	//		Pointer to the skininfo object if the mesh is skinned
	//	pBoneNames
	//		Array of names, one for each bone in the skinned mesh. 
	//		The numberof bones can be found from the pSkinMesh object
	//	pBoneOffsetMatrices
	//		Array of matrices, one for each bone in the skinned mesh.
	//
	//------------------------------------------------------------------------
    STDMETHOD(CreateMeshContainer)(THIS_ 
        LPCSTR Name, 
        CONST D3DXMESHDATA *pMeshData, 
        CONST D3DXMATERIAL *pMaterials, 
        CONST D3DXEFFECTINSTANCE *pEffectInstances, 
        DWORD NumMaterials, 
        CONST DWORD *pAdjacency, 
        LPD3DXSKININFO pSkinInfo, 
        LPD3DXMESHCONTAINER *ppNewMeshContainer) PURE;

	//------------------------------------------------------------------------
	// DestroyFrame:
	// -------------
	// Requests de-allocation of a frame object.
	//
	// Parameters:
	//  pFrameToFree
	//		Pointer to the frame to be de-allocated
	//
	//------------------------------------------------------------------------
    STDMETHOD(DestroyFrame)(THIS_ LPD3DXFRAME pFrameToFree) PURE; 

	//------------------------------------------------------------------------
	// DestroyMeshContainer:
	// ---------------------
	// Requests de-allocation of a mesh container object.
	//
	// Parameters:
	//  pMeshContainerToFree
	//		Pointer to the mesh container object to be de-allocated
	//
	//------------------------------------------------------------------------
    STDMETHOD(DestroyMeshContainer)(THIS_ LPD3DXMESHCONTAINER pMeshContainerToFree) PURE; 
};

//----------------------------------------------------------------------------
// ID3DXLoadUserData:
// ------------------
// This interface is implemented by the application to load user data in a .X file
// When user data is found, these callbacks will be used to allow the application
// to load the data.
//----------------------------------------------------------------------------
typedef interface ID3DXLoadUserData ID3DXLoadUserData;
typedef interface ID3DXLoadUserData *LPD3DXLOADUSERDATA;

#undef INTERFACE
#define INTERFACE ID3DXLoadUserData

DECLARE_INTERFACE(ID3DXLoadUserData)
{
    STDMETHOD(LoadTopLevelData)(LPD3DXFILEDATA pXofChildData) PURE;
                            
    STDMETHOD(LoadFrameChildData)(LPD3DXFRAME pFrame, 
                            LPD3DXFILEDATA pXofChildData) PURE;
                            
    STDMETHOD(LoadMeshChildData)(LPD3DXMESHCONTAINER pMeshContainer, 
                            LPD3DXFILEDATA pXofChildData) PURE;                            
};

//----------------------------------------------------------------------------
// ID3DXSaveUserData:
// ------------------
// This interface is implemented by the application to save user data in a .X file
// The callbacks are called for all data saved.  The user can then add any
// child data objects to the object provided to the callback.
//----------------------------------------------------------------------------
typedef interface ID3DXSaveUserData ID3DXSaveUserData;
typedef interface ID3DXSaveUserData *LPD3DXSAVEUSERDATA;

#undef INTERFACE
#define INTERFACE ID3DXSaveUserData

DECLARE_INTERFACE(ID3DXSaveUserData)
{
    STDMETHOD(AddFrameChildData)(CONST D3DXFRAME *pFrame, 
                            LPD3DXFILESAVEOBJECT pXofSave, 
                            LPD3DXFILESAVEDATA pXofFrameData) PURE;
                            
    STDMETHOD(AddMeshChildData)(CONST D3DXMESHCONTAINER *pMeshContainer, 
                            LPD3DXFILESAVEOBJECT pXofSave, 
                            LPD3DXFILESAVEDATA pXofMeshData) PURE;
                            
    // NOTE: this is called once per Save.  All top level objects should be added using the 
    //    provided interface.  One call adds objects before the frame hierarchy, the other after
    STDMETHOD(AddTopLevelDataObjectsPre)(LPD3DXFILESAVEOBJECT pXofSave) PURE; 
    STDMETHOD(AddTopLevelDataObjectsPost)(LPD3DXFILESAVEOBJECT pXofSave) PURE;                             

    // callbacks for the user to register and then save templates to the XFile
    STDMETHOD(RegisterTemplates)(LPD3DXFILE pXFileApi) PURE;                             
    STDMETHOD(SaveTemplates)(LPD3DXFILESAVEOBJECT pXofSave) PURE;                             
};


//----------------------------------------------------------------------------
// D3DXCALLBACK_SEARCH_FLAGS:
// --------------------------
// Flags that can be passed into ID3DXAnimationSet::GetCallback. 
//----------------------------------------------------------------------------
typedef enum _D3DXCALLBACK_SEARCH_FLAGS
{
    D3DXCALLBACK_SEARCH_EXCLUDING_INITIAL_POSITION = 0x01,  // exclude callbacks at the initial position from the search
    D3DXCALLBACK_SEARCH_BEHIND_INITIAL_POSITION    = 0x02,  // reverse the callback search direction

    D3DXCALLBACK_SEARCH_FORCE_DWORD                = 0x7fffffff
} D3DXCALLBACK_SEARCH_FLAGS;

//----------------------------------------------------------------------------
// ID3DXAnimationSet:
// ------------------
// This interface implements an animation set.
//----------------------------------------------------------------------------
typedef interface ID3DXAnimationSet ID3DXAnimationSet;
typedef interface ID3DXAnimationSet *LPD3DXANIMATIONSET;

#undef INTERFACE
#define INTERFACE ID3DXAnimationSet

DECLARE_INTERFACE_(ID3DXAnimationSet, IUnknown)
{
    // IUnknown
    STDMETHOD(QueryInterface)(THIS_ REFIID iid, LPVOID *ppv) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

    // Name
    STDMETHOD_(LPCSTR, GetName)(THIS) PURE;

    // Period
    STDMETHOD_(DOUBLE, GetPeriod)(THIS) PURE;
    STDMETHOD_(DOUBLE, GetPeriodicPosition)(THIS_ DOUBLE Position) PURE;    // Maps position into animation period

    // Animation names
    STDMETHOD_(UINT, GetNumAnimations)(THIS) PURE;
    STDMETHOD(GetAnimationNameByIndex)(THIS_ UINT Index, LPCSTR *ppName) PURE;
    STDMETHOD(GetAnimationIndexByName)(THIS_ LPCSTR pName, UINT *pIndex) PURE;

    // SRT
    STDMETHOD(GetSRT)(THIS_ 
        DOUBLE PeriodicPosition,            // Position mapped to period (use GetPeriodicPosition)
        UINT Animation,                     // Animation index
        D3DXVECTOR3 *pScale,                // Returns the scale
        D3DXQUATERNION *pRotation,          // Returns the rotation as a quaternion
        D3DXVECTOR3 *pTranslation) PURE;    // Returns the translation

    // Callbacks
    STDMETHOD(GetCallback)(THIS_ 
        DOUBLE Position,                    // Position from which to find callbacks
        DWORD Flags,                        // Callback search flags
        DOUBLE *pCallbackPosition,          // Returns the position of the callback
        LPVOID *ppCallbackData) PURE;       // Returns the callback data pointer
};


//----------------------------------------------------------------------------
// D3DXPLAYBACK_TYPE:
// ------------------
// This enum defines the type of animation set loop modes.
//----------------------------------------------------------------------------
typedef enum _D3DXPLAYBACK_TYPE
{
    D3DXPLAY_LOOP          = 0,
    D3DXPLAY_ONCE          = 1,
    D3DXPLAY_PINGPONG      = 2,

    D3DXPLAY_FORCE_DWORD   = 0x7fffffff /* force 32-bit size enum */
} D3DXPLAYBACK_TYPE;


//----------------------------------------------------------------------------
// D3DXKEY_VECTOR3:
// ----------------
// This structure describes a vector key for use in keyframe animation.
// It specifies a vector Value at a given Time. This is used for scale and
// translation keys.
//----------------------------------------------------------------------------
typedef struct _D3DXKEY_VECTOR3
{
    FLOAT Time;
    D3DXVECTOR3 Value;
} D3DXKEY_VECTOR3, *LPD3DXKEY_VECTOR3;


//----------------------------------------------------------------------------
// D3DXKEY_QUATERNION:
// -------------------
// This structure describes a quaternion key for use in keyframe animation.
// It specifies a quaternion Value at a given Time. This is used for rotation
// keys.
//----------------------------------------------------------------------------
typedef struct _D3DXKEY_QUATERNION
{
    FLOAT Time;
    D3DXQUATERNION Value;
} D3DXKEY_QUATERNION, *LPD3DXKEY_QUATERNION;


//----------------------------------------------------------------------------
// D3DXKEY_CALLBACK:
// -----------------
// This structure describes an callback key for use in keyframe animation.
// It specifies a pointer to user data at a given Time. 
//----------------------------------------------------------------------------
typedef struct _D3DXKEY_CALLBACK
{
    FLOAT Time;
    LPVOID pCallbackData;
} D3DXKEY_CALLBACK, *LPD3DXKEY_CALLBACK;


//----------------------------------------------------------------------------
// D3DXCOMPRESSION_FLAGS:
// ----------------------
// Flags that can be passed into ID3DXKeyframedAnimationSet::Compress. 
//----------------------------------------------------------------------------
typedef enum _D3DXCOMPRESSION_FLAGS
{
    D3DXCOMPRESS_DEFAULT     = 0x00,

    D3DXCOMPRESS_FORCE_DWORD = 0x7fffffff
} D3DXCOMPRESSION_FLAGS;


//----------------------------------------------------------------------------
// ID3DXKeyframedAnimationSet:
// ---------------------------
// This interface implements a compressable keyframed animation set.
//----------------------------------------------------------------------------
typedef interface ID3DXKeyframedAnimationSet ID3DXKeyframedAnimationSet;
typedef interface ID3DXKeyframedAnimationSet *LPD3DXKEYFRAMEDANIMATIONSET;

#undef INTERFACE
#define INTERFACE ID3DXKeyframedAnimationSet

DECLARE_INTERFACE_(ID3DXKeyframedAnimationSet, ID3DXAnimationSet)
{
    // ID3DXAnimationSet
    STDMETHOD(QueryInterface)(THIS_ REFIID iid, LPVOID *ppv) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

    // Name
    STDMETHOD_(LPCSTR, GetName)(THIS) PURE;

    // Period
    STDMETHOD_(DOUBLE, GetPeriod)(THIS) PURE;
    STDMETHOD_(DOUBLE, GetPeriodicPosition)(THIS_ DOUBLE Position) PURE;    // Maps position into animation period

    // Animation names
    STDMETHOD_(UINT, GetNumAnimations)(THIS) PURE;
    STDMETHOD(GetAnimationNameByIndex)(THIS_ UINT Index, LPCSTR *ppName) PURE;
    STDMETHOD(GetAnimationIndexByName)(THIS_ LPCSTR pName, UINT *pIndex) PURE;

    // SRT
    STDMETHOD(GetSRT)(THIS_ 
        DOUBLE PeriodicPosition,            // Position mapped to period (use GetPeriodicPosition)
        UINT Animation,                     // Animation index
        D3DXVECTOR3 *pScale,                // Returns the scale
        D3DXQUATERNION *pRotation,          // Returns the rotation as a quaternion
        D3DXVECTOR3 *pTranslation) PURE;    // Returns the translation

    // Callbacks
    STDMETHOD(GetCallback)(THIS_ 
        DOUBLE Position,                    // Position from which to find callbacks
        DWORD Flags,                        // Callback search flags
        DOUBLE *pCallbackPosition,          // Returns the position of the callback
        LPVOID *ppCallbackData) PURE;       // Returns the callback data pointer

    // Playback
    STDMETHOD_(D3DXPLAYBACK_TYPE, GetPlaybackType)(THIS) PURE;
    STDMETHOD_(DOUBLE, GetSourceTicksPerSecond)(THIS) PURE;

    // Scale keys
    STDMETHOD_(UINT, GetNumScaleKeys)(THIS_ UINT Animation) PURE;
	STDMETHOD(GetScaleKeys)(THIS_ UINT Animation, LPD3DXKEY_VECTOR3 pScaleKeys) PURE;
	STDMETHOD(GetScaleKey)(THIS_ UINT Animation, UINT Key, LPD3DXKEY_VECTOR3 pScaleKey) PURE;
	STDMETHOD(SetScaleKey)(THIS_ UINT Animation, UINT Key, LPD3DXKEY_VECTOR3 pScaleKey) PURE;

    // Rotation keys
    STDMETHOD_(UINT, GetNumRotationKeys)(THIS_ UINT Animation) PURE;
	STDMETHOD(GetRotationKeys)(THIS_ UINT Animation, LPD3DXKEY_QUATERNION pRotationKeys) PURE;
	STDMETHOD(GetRotationKey)(THIS_ UINT Animation, UINT Key, LPD3DXKEY_QUATERNION pRotationKey) PURE;
	STDMETHOD(SetRotationKey)(THIS_ UINT Animation, UINT Key, LPD3DXKEY_QUATERNION pRotationKey) PURE;

    // Translation keys
    STDMETHOD_(UINT, GetNumTranslationKeys)(THIS_ UINT Animation) PURE;
	STDMETHOD(GetTranslationKeys)(THIS_ UINT Animation, LPD3DXKEY_VECTOR3 pTranslationKeys) PURE;
	STDMETHOD(GetTranslationKey)(THIS_ UINT Animation, UINT Key, LPD3DXKEY_VECTOR3 pTranslationKey) PURE;
	STDMETHOD(SetTranslationKey)(THIS_ UINT Animation, UINT Key, LPD3DXKEY_VECTOR3 pTranslationKey) PURE;

    // Callback keys
    STDMETHOD_(UINT, GetNumCallbackKeys)(THIS) PURE;
	STDMETHOD(GetCallbackKeys)(THIS_ LPD3DXKEY_CALLBACK pCallbackKeys) PURE;
	STDMETHOD(GetCallbackKey)(THIS_ UINT Key, LPD3DXKEY_CALLBACK pCallbackKey) PURE;
	STDMETHOD(SetCallbackKey)(THIS_ UINT Key, LPD3DXKEY_CALLBACK pCallbackKey) PURE;

	// Key removal methods. These are slow, and should not be used once the animation starts playing
	STDMETHOD(UnregisterScaleKey)(THIS_ UINT Animation, UINT Key) PURE;
	STDMETHOD(UnregisterRotationKey)(THIS_ UINT Animation, UINT Key) PURE;
	STDMETHOD(UnregisterTranslationKey)(THIS_ UINT Animation, UINT Key) PURE;

    // One-time animaton SRT keyframe registration
    STDMETHOD(RegisterAnimationSRTKeys)(THIS_ 
        LPCSTR pName,                                   // Animation name
        UINT NumScaleKeys,                              // Number of scale keys
        UINT NumRotationKeys,                           // Number of rotation keys
        UINT NumTranslationKeys,                        // Number of translation keys
        CONST D3DXKEY_VECTOR3 *pScaleKeys,              // Array of scale keys
        CONST D3DXKEY_QUATERNION *pRotationKeys,        // Array of rotation keys
        CONST D3DXKEY_VECTOR3 *pTranslationKeys,		// Array of translation keys
		DWORD *pAnimationIndex) PURE;					// Returns the animation index 

    // Compression
    STDMETHOD(Compress)(THIS_ 
        DWORD Flags,                            // Compression flags (use D3DXCOMPRESS_STRONG for better results)
        FLOAT Lossiness,                        // Compression loss ratio in the [0, 1] range
        LPD3DXFRAME pHierarchy,                 // Frame hierarchy (optional)
        LPD3DXBUFFER *ppCompressedData) PURE;   // Returns the compressed animation set

    STDMETHOD(UnregisterAnimation)(THIS_ UINT Index) PURE;
};


//----------------------------------------------------------------------------
// ID3DXCompressedAnimationSet:
// ----------------------------
// This interface implements a compressed keyframed animation set.
//----------------------------------------------------------------------------
typedef interface ID3DXCompressedAnimationSet ID3DXCompressedAnimationSet;
typedef interface ID3DXCompressedAnimationSet *LPD3DXCOMPRESSEDANIMATIONSET;

#undef INTERFACE
#define INTERFACE ID3DXCompressedAnimationSet

DECLARE_INTERFACE_(ID3DXCompressedAnimationSet, ID3DXAnimationSet)
{
    // ID3DXAnimationSet
    STDMETHOD(QueryInterface)(THIS_ REFIID iid, LPVOID *ppv) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

    // Name
    STDMETHOD_(LPCSTR, GetName)(THIS) PURE;

    // Period
    STDMETHOD_(DOUBLE, GetPeriod)(THIS) PURE;
    STDMETHOD_(DOUBLE, GetPeriodicPosition)(THIS_ DOUBLE Position) PURE;    // Maps position into animation period

    // Animation names
    STDMETHOD_(UINT, GetNumAnimations)(THIS) PURE;
    STDMETHOD(GetAnimationNameByIndex)(THIS_ UINT Index, LPCSTR *ppName) PURE;
    STDMETHOD(GetAnimationIndexByName)(THIS_ LPCSTR pName, UINT *pIndex) PURE;

    // SRT
    STDMETHOD(GetSRT)(THIS_ 
        DOUBLE PeriodicPosition,            // Position mapped to period (use GetPeriodicPosition)
        UINT Animation,                     // Animation index
        D3DXVECTOR3 *pScale,                // Returns the scale
        D3DXQUATERNION *pRotation,          // Returns the rotation as a quaternion
        D3DXVECTOR3 *pTranslation) PURE;    // Returns the translation

    // Callbacks
    STDMETHOD(GetCallback)(THIS_ 
        DOUBLE Position,                    // Position from which to find callbacks
        DWORD Flags,                        // Callback search flags
        DOUBLE *pCallbackPosition,          // Returns the position of the callback
        LPVOID *ppCallbackData) PURE;       // Returns the callback data pointer

    // Playback
    STDMETHOD_(D3DXPLAYBACK_TYPE, GetPlaybackType)(THIS) PURE;
    STDMETHOD_(DOUBLE, GetSourceTicksPerSecond)(THIS) PURE;

    // Scale keys
    STDMETHOD(GetCompressedData)(THIS_ LPD3DXBUFFER *ppCompressedData) PURE;

    // Callback keys
    STDMETHOD_(UINT, GetNumCallbackKeys)(THIS) PURE;
    STDMETHOD(GetCallbackKeys)(THIS_ LPD3DXKEY_CALLBACK pCallbackKeys) PURE;
};


//----------------------------------------------------------------------------
// D3DXPRIORITY_TYPE:
// ------------------
// This enum defines the type of priority group that a track can be assigned to.
//----------------------------------------------------------------------------
typedef enum _D3DXPRIORITY_TYPE {
    D3DXPRIORITY_LOW         = 0,           // This track should be blended with all low priority tracks before mixed with the high priority result
    D3DXPRIORITY_HIGH        = 1,           // This track should be blended with all high priority tracks before mixed with the low priority result

    D3DXPRIORITY_FORCE_DWORD = 0x7fffffff  /* force 32-bit size enum */
} D3DXPRIORITY_TYPE;

//----------------------------------------------------------------------------
// D3DXTRACK_DESC:
// ---------------
// This structure describes the mixing information of an animation track. 
// The mixing information consists of the current position, speed, and blending 
// weight for the track.  The Flags field also specifies whether the track is 
// low or high priority.  Tracks with the same priority are blended together
// and then the two resulting values are blended using the priority blend factor.
// A track also has an animation set (stored separately) associated with it.  
//----------------------------------------------------------------------------
typedef struct _D3DXTRACK_DESC
{
    D3DXPRIORITY_TYPE   Priority;
    FLOAT               Weight;
    FLOAT               Speed;
    DOUBLE              Position;
    BOOL                Enable;
} D3DXTRACK_DESC, *LPD3DXTRACK_DESC;

//----------------------------------------------------------------------------
// D3DXEVENT_TYPE:
// ---------------
// This enum defines the type of events keyable via the animation controller.
//----------------------------------------------------------------------------
typedef enum _D3DXEVENT_TYPE
{
    D3DXEVENT_TRACKSPEED    = 0,
    D3DXEVENT_TRACKWEIGHT   = 1,
    D3DXEVENT_TRACKPOSITION = 2,
    D3DXEVENT_TRACKENABLE   = 3,
    D3DXEVENT_PRIORITYBLEND = 4,

    D3DXEVENT_FORCE_DWORD   = 0x7fffffff /* force 32-bit size enum */
} D3DXEVENT_TYPE;

//----------------------------------------------------------------------------
// D3DXTRANSITION_TYPE:
// --------------------
// This enum defines the type of transtion performed on a event that 
// transitions from one value to another.
//----------------------------------------------------------------------------
typedef enum _D3DXTRANSITION_TYPE {
    D3DXTRANSITION_LINEAR        = 0x000,     // Linear transition from one value to the next
    D3DXTRANSITION_EASEINEASEOUT = 0x001,     // Ease-In Ease-Out spline transtion from one value to the next

    D3DXTRANSITION_FORCE_DWORD   = 0x7fffffff /* force 32-bit size enum */
} D3DXTRANSITION_TYPE;

//----------------------------------------------------------------------------
// D3DXEVENT_DESC:
// ---------------
// This structure describes a animation controller event.
// It gives the event's type, track (if the event is a track event), global 
// start time, duration, transition method, and target value.
//----------------------------------------------------------------------------
typedef struct _D3DXEVENT_DESC
{
    D3DXEVENT_TYPE      Type;
    UINT                Track;
    DOUBLE              StartTime;
    DOUBLE              Duration;
    D3DXTRANSITION_TYPE Transition;
    union
    {
        FLOAT           Weight;
        FLOAT           Speed;
        DOUBLE          Position;
        BOOL            Enable;
    };
} D3DXEVENT_DESC, *LPD3DXEVENT_DESC;

//----------------------------------------------------------------------------
// D3DXEVENTHANDLE:
// ----------------
// Handle values used to efficiently reference animation controller events.
//----------------------------------------------------------------------------
typedef DWORD D3DXEVENTHANDLE;
typedef D3DXEVENTHANDLE *LPD3DXEVENTHANDLE;


//----------------------------------------------------------------------------
// ID3DXAnimationCallbackHandler:
// ------------------------------
// This interface is intended to be implemented by the application, and can
// be used to handle callbacks in animation sets generated when 
// ID3DXAnimationController::AdvanceTime() is called.  
//----------------------------------------------------------------------------
typedef interface ID3DXAnimationCallbackHandler ID3DXAnimationCallbackHandler;
typedef interface ID3DXAnimationCallbackHandler *LPD3DXANIMATIONCALLBACKHANDLER;

#undef INTERFACE
#define INTERFACE ID3DXAnimationCallbackHandler

DECLARE_INTERFACE(ID3DXAnimationCallbackHandler)
{
    //----------------------------------------------------------------------------
    // ID3DXAnimationCallbackHandler::HandleCallback:
    // ----------------------------------------------
    // This method gets called when a callback occurs for an animation set in one
    // of the tracks during the ID3DXAnimationController::AdvanceTime() call.  
    //
    // Parameters:
    //  Track
    //      Index of the track on which the callback occured.
    //  pCallbackData
    //      Pointer to user owned callback data.
    //
    //----------------------------------------------------------------------------
    STDMETHOD(HandleCallback)(THIS_ UINT Track, LPVOID pCallbackData) PURE; 
};


//----------------------------------------------------------------------------
// ID3DXAnimationController:
// -------------------------
// This interface implements the main animation functionality. It connects
// animation sets with the transform frames that are being animated. Allows
// mixing multiple animations for blended animations or for transistions
// It adds also has methods to modify blending parameters over time to 
// enable smooth transistions and other effects.
//----------------------------------------------------------------------------
typedef interface ID3DXAnimationController ID3DXAnimationController;
typedef interface ID3DXAnimationController *LPD3DXANIMATIONCONTROLLER;

#undef INTERFACE
#define INTERFACE ID3DXAnimationController

DECLARE_INTERFACE_(ID3DXAnimationController, IUnknown)
{
    // IUnknown
    STDMETHOD(QueryInterface)(THIS_ REFIID iid, LPVOID *ppv) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

    // Max sizes
    STDMETHOD_(UINT, GetMaxNumAnimationOutputs)(THIS) PURE;
    STDMETHOD_(UINT, GetMaxNumAnimationSets)(THIS) PURE;
    STDMETHOD_(UINT, GetMaxNumTracks)(THIS) PURE;
    STDMETHOD_(UINT, GetMaxNumEvents)(THIS) PURE;

    // Animation output registration
    STDMETHOD(RegisterAnimationOutput)(THIS_ 
        LPCSTR pName, 
        D3DXMATRIX *pMatrix, 
        D3DXVECTOR3 *pScale, 
        D3DXQUATERNION *pRotation, 
        D3DXVECTOR3 *pTranslation) PURE;

    // Animation set registration
    STDMETHOD(RegisterAnimationSet)(THIS_ LPD3DXANIMATIONSET pAnimSet) PURE;
    STDMETHOD(UnregisterAnimationSet)(THIS_ LPD3DXANIMATIONSET pAnimSet) PURE;

    STDMETHOD_(UINT, GetNumAnimationSets)(THIS) PURE;
    STDMETHOD(GetAnimationSet)(THIS_ UINT Index, LPD3DXANIMATIONSET *ppAnimationSet) PURE;
    STDMETHOD(GetAnimationSetByName)(THIS_ LPCSTR szName, LPD3DXANIMATIONSET *ppAnimationSet) PURE;

    // Global time
    STDMETHOD(AdvanceTime)(THIS_ DOUBLE TimeDelta, LPD3DXANIMATIONCALLBACKHANDLER pCallbackHandler) PURE;
    STDMETHOD(ResetTime)(THIS) PURE;
    STDMETHOD_(DOUBLE, GetTime)(THIS) PURE;

    // Tracks
    STDMETHOD(SetTrackAnimationSet)(THIS_ UINT Track, LPD3DXANIMATIONSET pAnimSet) PURE;
    STDMETHOD(GetTrackAnimationSet)(THIS_ UINT Track, LPD3DXANIMATIONSET *ppAnimSet) PURE;

    STDMETHOD(SetTrackPriority)(THIS_ UINT Track, D3DXPRIORITY_TYPE Priority) PURE;

    STDMETHOD(SetTrackSpeed)(THIS_ UINT Track, FLOAT Speed) PURE;
    STDMETHOD(SetTrackWeight)(THIS_ UINT Track, FLOAT Weight) PURE;
    STDMETHOD(SetTrackPosition)(THIS_ UINT Track, DOUBLE Position) PURE;
    STDMETHOD(SetTrackEnable)(THIS_ UINT Track, BOOL Enable) PURE;

    STDMETHOD(SetTrackDesc)(THIS_ UINT Track, LPD3DXTRACK_DESC pDesc) PURE;
    STDMETHOD(GetTrackDesc)(THIS_ UINT Track, LPD3DXTRACK_DESC pDesc) PURE;

    // Priority blending
    STDMETHOD(SetPriorityBlend)(THIS_ FLOAT BlendWeight) PURE;
    STDMETHOD_(FLOAT, GetPriorityBlend)(THIS) PURE;

    // Event keying
    STDMETHOD_(D3DXEVENTHANDLE, KeyTrackSpeed)(THIS_ UINT Track, FLOAT NewSpeed, DOUBLE StartTime, DOUBLE Duration, D3DXTRANSITION_TYPE Transition) PURE;
    STDMETHOD_(D3DXEVENTHANDLE, KeyTrackWeight)(THIS_ UINT Track, FLOAT NewWeight, DOUBLE StartTime, DOUBLE Duration, D3DXTRANSITION_TYPE Transition) PURE;
    STDMETHOD_(D3DXEVENTHANDLE, KeyTrackPosition)(THIS_ UINT Track, DOUBLE NewPosition, DOUBLE StartTime) PURE;
    STDMETHOD_(D3DXEVENTHANDLE, KeyTrackEnable)(THIS_ UINT Track, BOOL NewEnable, DOUBLE StartTime) PURE;

    STDMETHOD_(D3DXEVENTHANDLE, KeyPriorityBlend)(THIS_ FLOAT NewBlendWeight, DOUBLE StartTime, DOUBLE Duration, D3DXTRANSITION_TYPE Transition) PURE;

    // Event unkeying
    STDMETHOD(UnkeyEvent)(THIS_ D3DXEVENTHANDLE hEvent) PURE;

    STDMETHOD(UnkeyAllTrackEvents)(THIS_ UINT Track) PURE;
    STDMETHOD(UnkeyAllPriorityBlends)(THIS) PURE;

    // Event enumeration
    STDMETHOD_(D3DXEVENTHANDLE, GetCurrentTrackEvent)(THIS_ UINT Track, D3DXEVENT_TYPE EventType) PURE;
    STDMETHOD_(D3DXEVENTHANDLE, GetCurrentPriorityBlend)(THIS) PURE;

    STDMETHOD_(D3DXEVENTHANDLE, GetUpcomingTrackEvent)(THIS_ UINT Track, D3DXEVENTHANDLE hEvent) PURE;
    STDMETHOD_(D3DXEVENTHANDLE, GetUpcomingPriorityBlend)(THIS_ D3DXEVENTHANDLE hEvent) PURE;

    STDMETHOD(ValidateEvent)(THIS_ D3DXEVENTHANDLE hEvent) PURE;

    STDMETHOD(GetEventDesc)(THIS_ D3DXEVENTHANDLE hEvent, LPD3DXEVENT_DESC pDesc) PURE;

    // Cloning
    STDMETHOD(CloneAnimationController)(THIS_ 
        UINT MaxNumAnimationOutputs, 
        UINT MaxNumAnimationSets, 
        UINT MaxNumTracks, 
        UINT MaxNumEvents, 
        LPD3DXANIMATIONCONTROLLER *ppAnimController) PURE;
};

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus


//----------------------------------------------------------------------------
// D3DXLoadMeshHierarchyFromX:
// ---------------------------
// Loads the first frame hierarchy in a .X file.
//
// Parameters:
//  Filename
//      Name of the .X file
//  MeshOptions
//      Mesh creation options for meshes in the file (see d3dx9mesh.h)
//  pD3DDevice
//      D3D9 device on which meshes in the file are created in
//  pAlloc
//      Allocation interface used to allocate nodes of the frame hierarchy
//  pUserDataLoader
//      Application provided interface to allow loading of user data
//  ppFrameHierarchy
//      Returns root node pointer of the loaded frame hierarchy
//  ppAnimController
//      Returns pointer to an animation controller corresponding to animation
//		in the .X file. This is created with default max tracks and events
//
//----------------------------------------------------------------------------
HRESULT WINAPI 
D3DXLoadMeshHierarchyFromXA
    (
    LPCSTR Filename,
    DWORD MeshOptions,
    LPDIRECT3DDEVICE9 pD3DDevice,
    LPD3DXALLOCATEHIERARCHY pAlloc,
    LPD3DXLOADUSERDATA pUserDataLoader, 
    LPD3DXFRAME *ppFrameHierarchy,
    LPD3DXANIMATIONCONTROLLER *ppAnimController
    );

HRESULT WINAPI 
D3DXLoadMeshHierarchyFromXW
    (
    LPCWSTR Filename,
    DWORD MeshOptions,
    LPDIRECT3DDEVICE9 pD3DDevice,
    LPD3DXALLOCATEHIERARCHY pAlloc,
    LPD3DXLOADUSERDATA pUserDataLoader, 
    LPD3DXFRAME *ppFrameHierarchy,
    LPD3DXANIMATIONCONTROLLER *ppAnimController
    );

#ifdef UNICODE
#define D3DXLoadMeshHierarchyFromX D3DXLoadMeshHierarchyFromXW
#else
#define D3DXLoadMeshHierarchyFromX D3DXLoadMeshHierarchyFromXA
#endif

HRESULT WINAPI 
D3DXLoadMeshHierarchyFromXInMemory
    (
    LPCVOID Memory,
    DWORD SizeOfMemory,
    DWORD MeshOptions,
    LPDIRECT3DDEVICE9 pD3DDevice,
    LPD3DXALLOCATEHIERARCHY pAlloc,
    LPD3DXLOADUSERDATA pUserDataLoader, 
    LPD3DXFRAME *ppFrameHierarchy,
    LPD3DXANIMATIONCONTROLLER *ppAnimController
    );

//----------------------------------------------------------------------------
// D3DXSaveMeshHierarchyToFile:
// ----------------------------
// Creates a .X file and saves the mesh hierarchy and corresponding animations
// in it
//
// Parameters:
//  Filename
//      Name of the .X file
//  XFormat
//      Format of the .X file (text or binary, compressed or not, etc)
//  pFrameRoot
//      Root node of the hierarchy to be saved
//  pAnimController
//      The animation controller whose animation sets are to be stored
//  pUserDataSaver
//      Application provided interface to allow adding of user data to
//        data objects saved to .X file
//
//----------------------------------------------------------------------------
HRESULT WINAPI 
D3DXSaveMeshHierarchyToFileA
    (
    LPCSTR Filename,
    DWORD XFormat,
    CONST D3DXFRAME *pFrameRoot, 
    LPD3DXANIMATIONCONTROLLER pAnimcontroller,
    LPD3DXSAVEUSERDATA pUserDataSaver
    );

HRESULT WINAPI 
D3DXSaveMeshHierarchyToFileW
    (
    LPCWSTR Filename,
    DWORD XFormat,
    CONST D3DXFRAME *pFrameRoot, 
    LPD3DXANIMATIONCONTROLLER pAnimController,
    LPD3DXSAVEUSERDATA pUserDataSaver
    );

#ifdef UNICODE
#define D3DXSaveMeshHierarchyToFile D3DXSaveMeshHierarchyToFileW
#else
#define D3DXSaveMeshHierarchyToFile D3DXSaveMeshHierarchyToFileA
#endif

//----------------------------------------------------------------------------
// D3DXFrameDestroy:
// -----------------
// Destroys the subtree of frames under the root, including the root
//
// Parameters:
//	pFrameRoot
//		Pointer to the root node
//  pAlloc
//      Allocation interface used to de-allocate nodes of the frame hierarchy
//
//----------------------------------------------------------------------------
HRESULT WINAPI
D3DXFrameDestroy
    (
    LPD3DXFRAME pFrameRoot,
    LPD3DXALLOCATEHIERARCHY pAlloc
    );

//----------------------------------------------------------------------------
// D3DXFrameAppendChild:
// ---------------------
// Add a child frame to a frame
//
// Parameters:
//	pFrameParent
//		Pointer to the parent node
//  pFrameChild
//      Pointer to the child node
//
//----------------------------------------------------------------------------
HRESULT WINAPI 
D3DXFrameAppendChild
    (
    LPD3DXFRAME pFrameParent,
    CONST D3DXFRAME *pFrameChild
    );

//----------------------------------------------------------------------------
// D3DXFrameFind:
// --------------
// Finds a frame with the given name.  Returns NULL if no frame found.
//
// Parameters:
//	pFrameRoot
//		Pointer to the root node
//  Name
//      Name of frame to find
//
//----------------------------------------------------------------------------
LPD3DXFRAME WINAPI 
D3DXFrameFind
    (
    CONST D3DXFRAME *pFrameRoot,
    LPCSTR Name
    );

//----------------------------------------------------------------------------
// D3DXFrameRegisterNamedMatrices:
// -------------------------------
// Finds all frames that have non-null names and registers each of those frame
// matrices to the given animation controller
//
// Parameters:
//	pFrameRoot
//		Pointer to the root node
//	pAnimController
//		Pointer to the animation controller where the matrices are registered
//
//----------------------------------------------------------------------------
HRESULT WINAPI
D3DXFrameRegisterNamedMatrices
    (
    LPD3DXFRAME pFrameRoot,
    LPD3DXANIMATIONCONTROLLER pAnimController
    );

//----------------------------------------------------------------------------
// D3DXFrameNumNamedMatrices:
// --------------------------
// Counts number of frames in a subtree that have non-null names 
//
// Parameters:
//	pFrameRoot
//		Pointer to the root node of the subtree
// Return Value:
//		Count of frames
//
//----------------------------------------------------------------------------
UINT WINAPI
D3DXFrameNumNamedMatrices
    (
    CONST D3DXFRAME *pFrameRoot
    );

//----------------------------------------------------------------------------
// D3DXFrameCalculateBoundingSphere:
// ---------------------------------
// Computes the bounding sphere of all the meshes in the frame hierarchy.
//
// Parameters:
//	pFrameRoot
//		Pointer to the root node
//	pObjectCenter
//		Returns the center of the bounding sphere
//	pObjectRadius
//		Returns the radius of the bounding sphere
//
//----------------------------------------------------------------------------
HRESULT WINAPI
D3DXFrameCalculateBoundingSphere
    (
    CONST D3DXFRAME *pFrameRoot,       
    LPD3DXVECTOR3 pObjectCenter,  
    FLOAT *pObjectRadius          
    );


//----------------------------------------------------------------------------
// D3DXCreateKeyframedAnimationSet:
// --------------------------------
// This function creates a compressable keyframed animations set interface.
// 
// Parameters:
//  pName
//      Name of the animation set
//  TicksPerSecond
//      Number of keyframe ticks that elapse per second
//  Playback
//      Playback mode of keyframe looping
//  NumAnimations
//      Number of SRT animations
//  NumCallbackKeys
//      Number of callback keys
//  pCallbackKeys
//      Array of callback keys
//  ppAnimationSet
//      Returns the animation set interface
// 
//-----------------------------------------------------------------------------		
HRESULT WINAPI
D3DXCreateKeyframedAnimationSet
    (
    LPCSTR pName,                               
    DOUBLE TicksPerSecond,
    D3DXPLAYBACK_TYPE Playback,
    UINT NumAnimations,                         
    UINT NumCallbackKeys,                       
    CONST D3DXKEY_CALLBACK *pCallbackKeys,           
    LPD3DXKEYFRAMEDANIMATIONSET *ppAnimationSet 
    );


//----------------------------------------------------------------------------
// D3DXCreateCompressedAnimationSet:
// --------------------------------
// This function creates a compressed animations set interface from 
// compressed data.
// 
// Parameters:
//  pName
//      Name of the animation set
//  TicksPerSecond
//      Number of keyframe ticks that elapse per second
//  Playback
//      Playback mode of keyframe looping
//  pCompressedData
//      Compressed animation SRT data
//  NumCallbackKeys
//      Number of callback keys
//  pCallbackKeys
//      Array of callback keys
//  ppAnimationSet
//      Returns the animation set interface
// 
//-----------------------------------------------------------------------------		
HRESULT WINAPI
D3DXCreateCompressedAnimationSet
    (
    LPCSTR pName,                               
    DOUBLE TicksPerSecond,
    D3DXPLAYBACK_TYPE Playback,
    LPD3DXBUFFER pCompressedData,                         
    UINT NumCallbackKeys,                       
    CONST D3DXKEY_CALLBACK *pCallbackKeys,           
    LPD3DXCOMPRESSEDANIMATIONSET *ppAnimationSet 
    );


//----------------------------------------------------------------------------
// D3DXCreateAnimationController:
// ------------------------------
// This function creates an animation controller object.
//
// Parameters:
//  MaxNumMatrices
//      Maximum number of matrices that can be animated
//  MaxNumAnimationSets
//      Maximum number of animation sets that can be played
//  MaxNumTracks
//      Maximum number of animation sets that can be blended
//  MaxNumEvents
//      Maximum number of outstanding events that can be scheduled at any given time
//  ppAnimController
//      Returns the animation controller interface
//
//-----------------------------------------------------------------------------		
HRESULT WINAPI
D3DXCreateAnimationController
    (
    UINT MaxNumMatrices,                       
    UINT MaxNumAnimationSets,                  
    UINT MaxNumTracks,                         
    UINT MaxNumEvents,                         
    LPD3DXANIMATIONCONTROLLER *ppAnimController
    );


#ifdef __cplusplus
}
#endif //__cplusplus

#endif //__D3DX9ANIM_H__


