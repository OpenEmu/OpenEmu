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
 * Render3D.h
 * 
 * Header file defining the CRender3D class: OpenGL Real3D graphics engine.
 */

#ifndef INCLUDED_RENDER3D_H
#define INCLUDED_RENDER3D_H

#include "Pkgs/glew.h"

/******************************************************************************
 Internal Definitions and Data Structures
 
 NOTE: These should probably be moved inside the Render3D namespace at some
 point.
******************************************************************************/

// Model caches sort models by alpha (translucency) state
enum POLY_STATE
{
	POLY_STATE_NORMAL = 0,
	POLY_STATE_ALPHA
};

struct Vertex
{
	GLfloat	x,y,z;	// vertex
	GLfloat	n[3];	// normal X, Y, Z
	GLfloat	u,v;	// texture U, V coordinates (in texels, relative to selected texture)
};

struct Poly
{
	Vertex			Vert[4];
	GLfloat			n[3];		// polygon normal (used for backface culling)
	POLY_STATE		state;		// alpha or normal?
	unsigned		numVerts;	// triangle (3) or quad (4)
	const UINT32	*header;	// pointer to Real3D 7-word polygon header
};

/*
 * VBORef:
 *
 * Reference to model polygons stored in a VBO. Each reference has two sets of
 * vertices: normal and alpha. Copies of the model with different texture
 * offsets applied are searchable via the linked list of texture offset states.
 */
struct VBORef
{
	unsigned		index[2];	// index of model polygons in VBO
	unsigned		numVerts[2];// number of vertices
	unsigned		lutIdx;		// LUT index associated with this model (for fast LUT clearing)
	struct VBORef	*nextTexOffset;	// linked list of models with different texture offset states
	UINT16			texOffset;	// texture offset data for this model
};

// Display list items: model instances and viewport settings
struct DisplayList
{
	bool		isViewport;				// if true, this is a viewport node
	
	union
	{
		// Viewport data
		struct
		{
			GLfloat		projectionMatrix[4*4];	// projection matrix
			GLfloat		lightingParams[6];		// lighting parameters (see RenderViewport() and vertex shader)
			GLfloat		spotEllipse[4];			// spotlight ellipse (see RenderViewport())
			GLfloat		spotRange[2];			// Z range
			GLfloat		spotColor[3];			// color
			GLfloat		fogParams[5];			// fog parameters (...)
			GLint		x, y;					// viewport coordinates (scaled and in OpenGL format)
			GLint		width, height;			// viewport dimensions (scaled for display surface size)
		} Viewport;
		
		// Model data
		struct
		{
			GLfloat		modelViewMatrix[4*4];	// model-view matrix
			unsigned	index;					// index in VBO
			unsigned	numVerts;				// number of vertices
		} Model;
	} Data;
			
	DisplayList	*next;					// next display list item with the same state (alpha or non-alpha)
};

/*
 * ModelCache:
 *
 * A model cache tracks all models in a particular region (ie., VROM or polygon
 * RAM). It contains a look-up table to quickly obtain VBO indices. Be careful
 * when accessing the LUT, there are some special cases.
 *
 * If the model cache is marked dynamic, cached models may not necessarily be
 * retained. Clearing the model cache is also much faster. The LUT entry for 
 * the last model cached will be valid, but because the LUT may not be
 * cleared, one cannot assume a model exists because there is a LUT entry
 * pointing to it. Always use NeedToCache() to determine whether caching is
 * necessary before reading the LUT!
 */
struct ModelCache
{
	// Cache type
	bool		dynamic;
	
	// Vertex buffer object
	unsigned	vboMaxOffset;	// size of VBO (in bytes)
	unsigned	vboCurOffset;	// current offset in VBO (in bytes)
	GLuint		vboID;			// OpenGL VBO handle
	
	// Local vertex buffers (enough for a single model)
	unsigned	maxVertIdx;		// size of each local vertex buffer (in vertices)
	unsigned	curVertIdx[2];	// current vertex index (in vertices)
	GLfloat		*verts[2];
	
	// Array of cached models
	unsigned	maxModels;	// maximum number of models
	unsigned	numModels;	// current number stored
	VBORef		*Models;
	
	/*
	 * Look-Up Table:
	 *
	 * Can be accessed directly with a LUT index to determine the model index.
	 * However, it should not be used to determine whether a model needs to be
	 * cached. Use NeedToCache() instead. A valid index, for example, may still
	 * have to be re-cached if the model cache is dynamic (polygon RAM).
	 */
	unsigned	lutSize;	// number of elements in LUT
	INT16		*lut;		// stores indices into Models[] or -1 if not yet cached

	// Display list
	unsigned	maxListSize;	// maximum number of display list items
	unsigned	listSize;		// number of items in display list
	DisplayList	*List;			// holds all display list items
	DisplayList	*ListHead[2];	// heads of linked lists for each state
	DisplayList	*ListTail[2];	// current tail node for each state
};


/******************************************************************************
 CRender3D Classes
******************************************************************************/

/*
 * CRender3DConfig:
 *
 * Settings used by CRender3D.
 */
class CRender3DConfig
{
public:
	string vertexShaderFile;	// path to vertex shader or "" to use internal shader
	string fragmentShaderFile;	// fragment shader
	
	// Defaults
	CRender3DConfig(void)
	{
		// nothing to do, strings will be clear to begin with
	}
};

/*
 * CRender3D:
 *
 * 3D renderer. Lots of work to do here :)
 */
class CRender3D
{
public:
	/*
	 * RenderFrame(void):
	 *
	 * Renders the complete scene database. Must be called between BeginFrame() and
	 * EndFrame(). This function traverses the scene database and builds up display 
	 * lists.
	 */
	void RenderFrame(void);
	
	/*
	 * BeginFrame(void):
	 *
	 * Prepare to render a new frame. Must be called once per frame prior to
	 * drawing anything.
	 */
	void BeginFrame(void);
	
	/*
	 * EndFrame(void):
	 *
	 * Signals the end of rendering for this frame. Must be called last during
	 * the frame.
	 */
	void EndFrame(void);
	
	/*
	 * UploadTextures(x, y, width, height):
	 *
	 * Signals that a portion of texture RAM has been updated.
	 *
	 * Parameters:
	 *		x		X position within texture RAM.
	 *		y		Y position within texture RAM.
	 *		width	Width of texture data in texels.
	 *		height	Height.
	 */
	void UploadTextures(unsigned x, unsigned y, unsigned width, unsigned height);
	
	/*
	 * AttachMemory(cullingRAMLoPtr, cullingRAMHiPtr, polyRAMPtr, vromPtr,
	 * 				textureRAMPtr):
	 *
	 * Attaches RAM and ROM areas. This must be done prior to any rendering
	 * otherwise the program may crash with an access violation.
	 *
	 * Parameters:
	 *		cullingRAMLoPtr		Pointer to low culling RAM (4 MB).
	 *		cullingRAMHiPtr		Pointer to high culling RAM (1 MB).
	 *		polyRAMPtr			Pointer to polygon RAM (4 MB).
	 *		vromPtr				Pointer to video ROM (64 MB).
	 *		textureRAMPtr		Pointer to texture RAM (8 MB).
	 */
	void AttachMemory(const UINT32 *cullingRAMLoPtr,
					  const UINT32 *cullingRAMHiPtr, const UINT32 *polyRAMPtr,
					  const UINT32 *vromPtr, const UINT16 *textureRAMPtr);

	/*
	 * SetStep(stepID):
	 *
	 * Sets the Model 3 hardware stepping, which also determines the Real3D
	 * functionality. The default is Step 1.0. This should be called prior to 
	 * any other emulation functions and after Init().
	 *
	 * Parameters:
	 *		stepID	0x10 for Step 1.0, 0x15 for Step 1.5, 0x20 for Step 2.0,
	 *				or 0x21 for Step 2.1. Anything else defaults to 1.0.
	 */
	void SetStep(int stepID);
	
	/*
	 * Init(xOffset, yOffset, xRes, yRes):
	 *
	 * One-time initialization of the context. Must be called before any other
	 * members (meaning it should be called even before being attached to any
	 * other objects that want to use it).
	 *
	 * External shader files are loaded according to configuration settings.
	 *
	 * Parameters:
	 *		xOffset		X offset of display surface in pixels (in case resolution
	 *					is smaller than the true display surface).
	 *		yOffset		Y offset.
	 *		xRes		Horizontal resolution of display surface in pixels.
	 *		yRes		Vertical resolution.
	 *
	 * Returns:
	 *		OKAY is successful, otherwise FAILED if a non-recoverable error
	 *		occurred. Any allocated memory will not be freed until the
	 *		destructor is called. Prints own error messages.
	 */
	bool Init(unsigned xOffset, unsigned yOffset, unsigned xRes, unsigned yRes);
	 
	/*
	 * CRender3D(void):
	 * ~CRender3D(void):
	 *
	 * Constructor and destructor.
	 */
	CRender3D(void);
	~CRender3D(void);
	
private:
	/*
	 * Private Members
	 */
	
	// Real3D address translation
	const UINT32 *TranslateCullingAddress(UINT32 addr);
	const UINT32 *TranslateModelAddress(UINT32 addr);
	
	// Model caching and display list management
	void 			DrawDisplayList(ModelCache *Cache, POLY_STATE state);
	bool 			AppendDisplayList(ModelCache *Cache, bool isViewport, const struct VBORef *Model);
	void 			ClearDisplayList(ModelCache *Cache);
	bool 			InsertPolygon(ModelCache *cache, const Poly *p);
	void 			InsertVertex(ModelCache *cache, const Vertex *v, const Poly *p, float normFlip);
	bool 			BeginModel(ModelCache *cache);
	struct VBORef	*EndModel(ModelCache *cache, int lutIdx, UINT16 texOffset);
	struct VBORef	*CacheModel(ModelCache *cache, int lutIdx, UINT16 texOffset, const UINT32 *data);
	struct VBORef	*LookUpModel(ModelCache *cache, int lutIdx, UINT16 texOffset);
	void 			ClearModelCache(ModelCache *cache);
	bool 			CreateModelCache(ModelCache *cache, unsigned vboMaxVerts, unsigned localMaxVerts, unsigned maxNumModels, unsigned numLUTEntries, unsigned displayListSize, bool isDynamic);
	void 			DestroyModelCache(ModelCache *cache);
	
	// Texture management
	void DecodeTexture(int format, int x, int y, int width, int height);
	
	// Stack management
	void	MultMatrix(UINT32 matrixOffset);
	void 	InitMatrixStack(UINT32 matrixBaseAddr);
	void	Push(UINT32 ptr, bool pushMatrix);
	UINT32	Pop(void);
	void	ClearStack(void);
	
	// Scene database traversal
	bool DrawModel(UINT32 modelAddr);
	void DescendCullingNode(UINT32 addr);
	void DescendPointerList(UINT32 addr);
	void DescendNodePtr(UINT32 nodeAddr);
	void StackMachine(UINT32 nodeAddr);
	void RenderViewport(UINT32 addr, int pri);
	
	// In-frame error reporting
	bool ErrorLocalVertexOverflow(void);
	bool ErrorUnableToCacheModel(UINT32 modelAddr);
	void ClearErrors(void);
	
	/*
	 * Data
	 */
	
	// Stepping
	int		step;
	int		offset;			// offset to subtract for words 3 and higher of culling nodes
	GLfloat	vertexFactor;	// fixed-point conversion factor for vertices
	
	// Memory (passed from outside)
	const UINT32	*cullingRAMLo;	// 4 MB
	const UINT32	*cullingRAMHi;	// 1 MB
	const UINT32	*polyRAM;		// 4 MB
	const UINT32	*vrom;			// 64 MB
	const UINT16	*textureRAM;	// 8 MB
	
	// Error reporting
	unsigned	errorMsgFlags;	// tracks which errors have been printed this frame
	
	// Real3D Base Matrix Pointer
	const float	*matrixBasePtr;
	
	// Processing stack (experimental)
	UINT32	*stack;
	int		stackSize;		// number of elements stack can contain
	int		stackTop;		// current top of stack (free spot, last element is stackTop-1)
	bool	stackOverflow;	// records stack overflows (cleared by ClearStack())

	// Current viewport parameters (updated as viewports are traversed)
	GLfloat	lightingParams[6];
	GLfloat	fogParams[5];
	GLfloat	spotEllipse[4];
	GLfloat	spotRange[2];
	GLfloat	spotColor[3];
	GLint	viewportX, viewportY;
	GLint	viewportWidth, viewportHeight;
	
	// Scene graph stack
	int		listDepth;	// how many lists have we recursed into
	int		stackDepth;	// for debugging and error handling purposes
	
	// Texture offset (during scene graph processing)
	GLfloat	texOffsetXY[2];	// decoded X, Y offsets
	UINT16	texOffset;		// raw texture offset data as it appears in culling node
	
	// Resolution scaling factors (to support resolutions higher than 496x384) and offsets
	GLfloat		xRatio, yRatio;
	unsigned	xOffs, yOffs;
	
	// Texture ID for complete 2048x2048 texture map
	GLuint	texID;
	
	// Shader programs and input data locations
	GLuint	shaderProgram;			// shader program object
	GLuint	vertexShader;			// vertex shader handle
	GLuint	fragmentShader;			// fragment shader
	GLuint	textureMapLoc;			// location of "textureMap" uniform
	GLuint	modelViewMatrixLoc;		// uniform
	GLuint	projectionMatrixLoc;	// uniform
	GLuint	lightingLoc;			// uniform
	GLuint	spotEllipseLoc;			// uniform
	GLuint	spotRangeLoc;			// uniform
	GLuint	spotColorLoc;			// uniform
	GLuint	subTextureLoc;			// attribute
	GLuint	texParamsLoc;			// attribute
	GLuint	texFormatLoc;			// attribute
	GLuint	transLevelLoc;			// attribute
	GLuint	lightEnableLoc;			// attribute
	GLuint	fogIntensityLoc;		// attribute
	
	// Model caching
	ModelCache	VROMCache;	// VROM (static) models
	ModelCache	PolyCache;	// polygon RAM (dynamic) models

	/*
	 * Texture Format Buffer
	 *
	 * Records the format that a texture (at a given location within the
	 * texture sheet) is currently stored in. A negative value indicates the
	 * texture has not been accessed and converted yet and non-negative values
	 * correspond to the texture format bits in the polygon headers. They can
	 * be used to determine whether a texture needs to be updated.
	 */
	int		textureWidth[2048/32][2048/32];
	int		textureHeight[2048/32][2048/32];
	INT8	textureFormat[2048/32][2048/32];
	
	/*
 	 * Texture Decode Buffer
 	 *
 	 * Textures are decoded and copied from texture RAM into this temporary buffer
 	 * before being uploaded. Dimensions are 512x512.
 	 */
	GLfloat	*textureBuffer;	// RGBA8 format
};


#endif	// INCLUDED_RENDER3D_H
