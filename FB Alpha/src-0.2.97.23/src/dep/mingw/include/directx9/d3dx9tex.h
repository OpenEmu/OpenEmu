//////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
//  File:       d3dx9tex.h
//  Content:    D3DX texturing APIs
//
//////////////////////////////////////////////////////////////////////////////

#include "d3dx9.h"

#ifndef __D3DX9TEX_H__
#define __D3DX9TEX_H__


//----------------------------------------------------------------------------
// D3DX_FILTER flags:
// ------------------
//
// A valid filter must contain one of these values:
//
//  D3DX_FILTER_NONE
//      No scaling or filtering will take place.  Pixels outside the bounds
//      of the source image are assumed to be transparent black.
//  D3DX_FILTER_POINT
//      Each destination pixel is computed by sampling the nearest pixel
//      from the source image.
//  D3DX_FILTER_LINEAR
//      Each destination pixel is computed by linearly interpolating between
//      the nearest pixels in the source image.  This filter works best 
//      when the scale on each axis is less than 2.
//  D3DX_FILTER_TRIANGLE
//      Every pixel in the source image contributes equally to the
//      destination image.  This is the slowest of all the filters.
//  D3DX_FILTER_BOX
//      Each pixel is computed by averaging a 2x2(x2) box pixels from 
//      the source image. Only works when the dimensions of the 
//      destination are half those of the source. (as with mip maps)
//
// And can be OR'd with any of these optional flags:
//
//  D3DX_FILTER_MIRROR_U
//      Indicates that pixels off the edge of the texture on the U-axis
//      should be mirrored, not wraped.
//  D3DX_FILTER_MIRROR_V
//      Indicates that pixels off the edge of the texture on the V-axis
//      should be mirrored, not wraped.
//  D3DX_FILTER_MIRROR_W
//      Indicates that pixels off the edge of the texture on the W-axis
//      should be mirrored, not wraped.
//  D3DX_FILTER_MIRROR
//      Same as specifying D3DX_FILTER_MIRROR_U | D3DX_FILTER_MIRROR_V |
//      D3DX_FILTER_MIRROR_V
//  D3DX_FILTER_DITHER
//      Dithers the resulting image using a 4x4 order dither pattern.
//  D3DX_FILTER_SRGB_IN
//      Denotes that the input data is in sRGB (gamma 2.2) colorspace.
//  D3DX_FILTER_SRGB_OUT
//      Denotes that the output data is in sRGB (gamma 2.2) colorspace.
//  D3DX_FILTER_SRGB
//      Same as specifying D3DX_FILTER_SRGB_IN | D3DX_FILTER_SRGB_OUT
//
//----------------------------------------------------------------------------

#define D3DX_FILTER_NONE             (1 << 0)
#define D3DX_FILTER_POINT            (2 << 0)
#define D3DX_FILTER_LINEAR           (3 << 0)
#define D3DX_FILTER_TRIANGLE         (4 << 0)
#define D3DX_FILTER_BOX              (5 << 0)

#define D3DX_FILTER_MIRROR_U         (1 << 16)
#define D3DX_FILTER_MIRROR_V         (2 << 16)
#define D3DX_FILTER_MIRROR_W         (4 << 16)
#define D3DX_FILTER_MIRROR           (7 << 16)

#define D3DX_FILTER_DITHER           (1 << 19)
#define D3DX_FILTER_DITHER_DIFFUSION (2 << 19)

#define D3DX_FILTER_SRGB_IN          (1 << 21)
#define D3DX_FILTER_SRGB_OUT         (2 << 21)
#define D3DX_FILTER_SRGB             (3 << 21)


//-----------------------------------------------------------------------------
// D3DX_SKIP_DDS_MIP_LEVELS is used to skip mip levels when loading a DDS file:
//-----------------------------------------------------------------------------

#define D3DX_SKIP_DDS_MIP_LEVELS_MASK   0x1F
#define D3DX_SKIP_DDS_MIP_LEVELS_SHIFT  26
#define D3DX_SKIP_DDS_MIP_LEVELS(levels, filter) ((((levels) & D3DX_SKIP_DDS_MIP_LEVELS_MASK) << D3DX_SKIP_DDS_MIP_LEVELS_SHIFT) | ((filter) == D3DX_DEFAULT ? D3DX_FILTER_BOX : (filter)))




//----------------------------------------------------------------------------
// D3DX_NORMALMAP flags:
// ---------------------
// These flags are used to control how D3DXComputeNormalMap generates normal
// maps.  Any number of these flags may be OR'd together in any combination.
//
//  D3DX_NORMALMAP_MIRROR_U
//      Indicates that pixels off the edge of the texture on the U-axis
//      should be mirrored, not wraped.
//  D3DX_NORMALMAP_MIRROR_V
//      Indicates that pixels off the edge of the texture on the V-axis
//      should be mirrored, not wraped.
//  D3DX_NORMALMAP_MIRROR
//      Same as specifying D3DX_NORMALMAP_MIRROR_U | D3DX_NORMALMAP_MIRROR_V
//  D3DX_NORMALMAP_INVERTSIGN
//      Inverts the direction of each normal 
//  D3DX_NORMALMAP_COMPUTE_OCCLUSION
//      Compute the per pixel Occlusion term and encodes it into the alpha.
//      An Alpha of 1 means that the pixel is not obscured in anyway, and
//      an alpha of 0 would mean that the pixel is completly obscured.
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#define D3DX_NORMALMAP_MIRROR_U     (1 << 16)
#define D3DX_NORMALMAP_MIRROR_V     (2 << 16)
#define D3DX_NORMALMAP_MIRROR       (3 << 16)
#define D3DX_NORMALMAP_INVERTSIGN   (8 << 16)
#define D3DX_NORMALMAP_COMPUTE_OCCLUSION (16 << 16)




//----------------------------------------------------------------------------
// D3DX_CHANNEL flags:
// -------------------
// These flags are used by functions which operate on or more channels
// in a texture.
//
// D3DX_CHANNEL_RED
//     Indicates the red channel should be used
// D3DX_CHANNEL_BLUE
//     Indicates the blue channel should be used
// D3DX_CHANNEL_GREEN
//     Indicates the green channel should be used
// D3DX_CHANNEL_ALPHA
//     Indicates the alpha channel should be used
// D3DX_CHANNEL_LUMINANCE
//     Indicates the luminaces of the red green and blue channels should be 
//     used.
//
//----------------------------------------------------------------------------

#define D3DX_CHANNEL_RED            (1 << 0)
#define D3DX_CHANNEL_BLUE           (1 << 1)
#define D3DX_CHANNEL_GREEN          (1 << 2)
#define D3DX_CHANNEL_ALPHA          (1 << 3)
#define D3DX_CHANNEL_LUMINANCE      (1 << 4)




//----------------------------------------------------------------------------
// D3DXIMAGE_FILEFORMAT:
// ---------------------
// This enum is used to describe supported image file formats.
//
//----------------------------------------------------------------------------

typedef enum _D3DXIMAGE_FILEFORMAT
{
    D3DXIFF_BMP         = 0,
    D3DXIFF_JPG         = 1,
    D3DXIFF_TGA         = 2,
    D3DXIFF_PNG         = 3,
    D3DXIFF_DDS         = 4,
    D3DXIFF_PPM         = 5,
    D3DXIFF_DIB         = 6,
    D3DXIFF_HDR         = 7,       //high dynamic range formats
    D3DXIFF_PFM         = 8,       //
    D3DXIFF_FORCE_DWORD = 0x7fffffff

} D3DXIMAGE_FILEFORMAT;


//----------------------------------------------------------------------------
// LPD3DXFILL2D and LPD3DXFILL3D:
// ------------------------------
// Function types used by the texture fill functions.
//
// Parameters:
//  pOut
//      Pointer to a vector which the function uses to return its result.
//      X,Y,Z,W will be mapped to R,G,B,A respectivly. 
//  pTexCoord
//      Pointer to a vector containing the coordinates of the texel currently 
//      being evaluated.  Textures and VolumeTexture texcoord components 
//      range from 0 to 1. CubeTexture texcoord component range from -1 to 1.
//  pTexelSize
//      Pointer to a vector containing the dimensions of the current texel.
//  pData
//      Pointer to user data.
//
//----------------------------------------------------------------------------

typedef VOID (WINAPI *LPD3DXFILL2D)(D3DXVECTOR4 *pOut, 
    CONST D3DXVECTOR2 *pTexCoord, CONST D3DXVECTOR2 *pTexelSize, LPVOID pData);

typedef VOID (WINAPI *LPD3DXFILL3D)(D3DXVECTOR4 *pOut, 
    CONST D3DXVECTOR3 *pTexCoord, CONST D3DXVECTOR3 *pTexelSize, LPVOID pData);
 


//----------------------------------------------------------------------------
// D3DXIMAGE_INFO:
// ---------------
// This structure is used to return a rough description of what the
// the original contents of an image file looked like.
// 
//  Width
//      Width of original image in pixels
//  Height
//      Height of original image in pixels
//  Depth
//      Depth of original image in pixels
//  MipLevels
//      Number of mip levels in original image
//  Format
//      D3D format which most closely describes the data in original image
//  ResourceType
//      D3DRESOURCETYPE representing the type of texture stored in the file.
//      D3DRTYPE_TEXTURE, D3DRTYPE_VOLUMETEXTURE, or D3DRTYPE_CUBETEXTURE.
//  ImageFileFormat
//      D3DXIMAGE_FILEFORMAT representing the format of the image file.
//
//----------------------------------------------------------------------------

typedef struct _D3DXIMAGE_INFO
{
    UINT                    Width;
    UINT                    Height;
    UINT                    Depth;
    UINT                    MipLevels;
    D3DFORMAT               Format;
    D3DRESOURCETYPE         ResourceType;
    D3DXIMAGE_FILEFORMAT    ImageFileFormat;

} D3DXIMAGE_INFO;





#ifdef __cplusplus
extern "C" {
#endif //__cplusplus



//////////////////////////////////////////////////////////////////////////////
// Image File APIs ///////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//;
//----------------------------------------------------------------------------
// GetImageInfoFromFile/Resource:
// ------------------------------
// Fills in a D3DXIMAGE_INFO struct with information about an image file.
//
// Parameters:
//  pSrcFile
//      File name of the source image.
//  pSrcModule
//      Module where resource is located, or NULL for module associated
//      with image the os used to create the current process.
//  pSrcResource
//      Resource name
//  pSrcData
//      Pointer to file in memory.
//  SrcDataSize
//      Size in bytes of file in memory.
//  pSrcInfo
//      Pointer to a D3DXIMAGE_INFO structure to be filled in with the 
//      description of the data in the source image file.
//
//----------------------------------------------------------------------------

HRESULT WINAPI
    D3DXGetImageInfoFromFileA(
        LPCSTR                    pSrcFile,
        D3DXIMAGE_INFO*           pSrcInfo);

HRESULT WINAPI
    D3DXGetImageInfoFromFileW(
        LPCWSTR                   pSrcFile,
        D3DXIMAGE_INFO*           pSrcInfo);

#ifdef UNICODE
#define D3DXGetImageInfoFromFile D3DXGetImageInfoFromFileW
#else
#define D3DXGetImageInfoFromFile D3DXGetImageInfoFromFileA
#endif


HRESULT WINAPI
    D3DXGetImageInfoFromResourceA(
        HMODULE                   hSrcModule,
        LPCSTR                    pSrcResource,
        D3DXIMAGE_INFO*           pSrcInfo);

HRESULT WINAPI
    D3DXGetImageInfoFromResourceW(
        HMODULE                   hSrcModule,
        LPCWSTR                   pSrcResource,
        D3DXIMAGE_INFO*           pSrcInfo);

#ifdef UNICODE
#define D3DXGetImageInfoFromResource D3DXGetImageInfoFromResourceW
#else
#define D3DXGetImageInfoFromResource D3DXGetImageInfoFromResourceA
#endif


HRESULT WINAPI
    D3DXGetImageInfoFromFileInMemory(
        LPCVOID                   pSrcData,
        UINT                      SrcDataSize,
        D3DXIMAGE_INFO*           pSrcInfo);




//////////////////////////////////////////////////////////////////////////////
// Load/Save Surface APIs ////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
// D3DXLoadSurfaceFromFile/Resource:
// ---------------------------------
// Load surface from a file or resource
//
// Parameters:
//  pDestSurface
//      Destination surface, which will receive the image.
//  pDestPalette
//      Destination palette of 256 colors, or NULL
//  pDestRect
//      Destination rectangle, or NULL for entire surface
//  pSrcFile
//      File name of the source image.
//  pSrcModule
//      Module where resource is located, or NULL for module associated
//      with image the os used to create the current process.
//  pSrcResource
//      Resource name
//  pSrcData
//      Pointer to file in memory.
//  SrcDataSize
//      Size in bytes of file in memory.
//  pSrcRect
//      Source rectangle, or NULL for entire image
//  Filter
//      D3DX_FILTER flags controlling how the image is filtered.
//      Or D3DX_DEFAULT for D3DX_FILTER_TRIANGLE.
//  ColorKey
//      Color to replace with transparent black, or 0 to disable colorkey.
//      This is always a 32-bit ARGB color, independent of the source image
//      format.  Alpha is significant, and should usually be set to FF for 
//      opaque colorkeys.  (ex. Opaque black == 0xff000000)
//  pSrcInfo
//      Pointer to a D3DXIMAGE_INFO structure to be filled in with the 
//      description of the data in the source image file, or NULL.
//
//----------------------------------------------------------------------------

HRESULT WINAPI
    D3DXLoadSurfaceFromFileA(
        LPDIRECT3DSURFACE9        pDestSurface,
        CONST PALETTEENTRY*       pDestPalette,
        CONST RECT*               pDestRect,
        LPCSTR                    pSrcFile,
        CONST RECT*               pSrcRect,
        DWORD                     Filter,
        D3DCOLOR                  ColorKey,
        D3DXIMAGE_INFO*           pSrcInfo);

HRESULT WINAPI
    D3DXLoadSurfaceFromFileW(
        LPDIRECT3DSURFACE9        pDestSurface,
        CONST PALETTEENTRY*       pDestPalette,
        CONST RECT*               pDestRect,
        LPCWSTR                   pSrcFile,
        CONST RECT*               pSrcRect,
        DWORD                     Filter,
        D3DCOLOR                  ColorKey,
        D3DXIMAGE_INFO*           pSrcInfo);

#ifdef UNICODE
#define D3DXLoadSurfaceFromFile D3DXLoadSurfaceFromFileW
#else
#define D3DXLoadSurfaceFromFile D3DXLoadSurfaceFromFileA
#endif



HRESULT WINAPI
    D3DXLoadSurfaceFromResourceA(
        LPDIRECT3DSURFACE9        pDestSurface,
        CONST PALETTEENTRY*       pDestPalette,
        CONST RECT*               pDestRect,
        HMODULE                   hSrcModule,
        LPCSTR                    pSrcResource,
        CONST RECT*               pSrcRect,
        DWORD                     Filter,
        D3DCOLOR                  ColorKey,
        D3DXIMAGE_INFO*           pSrcInfo);

HRESULT WINAPI
    D3DXLoadSurfaceFromResourceW(
        LPDIRECT3DSURFACE9        pDestSurface,
        CONST PALETTEENTRY*       pDestPalette,
        CONST RECT*               pDestRect,
        HMODULE                   hSrcModule,
        LPCWSTR                   pSrcResource,
        CONST RECT*               pSrcRect,
        DWORD                     Filter,
        D3DCOLOR                  ColorKey,
        D3DXIMAGE_INFO*           pSrcInfo);


#ifdef UNICODE
#define D3DXLoadSurfaceFromResource D3DXLoadSurfaceFromResourceW
#else
#define D3DXLoadSurfaceFromResource D3DXLoadSurfaceFromResourceA
#endif



HRESULT WINAPI
    D3DXLoadSurfaceFromFileInMemory(
        LPDIRECT3DSURFACE9        pDestSurface,
        CONST PALETTEENTRY*       pDestPalette,
        CONST RECT*               pDestRect,
        LPCVOID                   pSrcData,
        UINT                      SrcDataSize,
        CONST RECT*               pSrcRect,
        DWORD                     Filter,
        D3DCOLOR                  ColorKey,
        D3DXIMAGE_INFO*           pSrcInfo);



//----------------------------------------------------------------------------
// D3DXLoadSurfaceFromSurface:
// ---------------------------
// Load surface from another surface (with color conversion)
//
// Parameters:
//  pDestSurface
//      Destination surface, which will receive the image.
//  pDestPalette
//      Destination palette of 256 colors, or NULL
//  pDestRect
//      Destination rectangle, or NULL for entire surface
//  pSrcSurface
//      Source surface
//  pSrcPalette
//      Source palette of 256 colors, or NULL
//  pSrcRect
//      Source rectangle, or NULL for entire surface
//  Filter
//      D3DX_FILTER flags controlling how the image is filtered.
//      Or D3DX_DEFAULT for D3DX_FILTER_TRIANGLE.
//  ColorKey
//      Color to replace with transparent black, or 0 to disable colorkey.
//      This is always a 32-bit ARGB color, independent of the source image
//      format.  Alpha is significant, and should usually be set to FF for 
//      opaque colorkeys.  (ex. Opaque black == 0xff000000)
//
//----------------------------------------------------------------------------

HRESULT WINAPI
    D3DXLoadSurfaceFromSurface(
        LPDIRECT3DSURFACE9        pDestSurface,
        CONST PALETTEENTRY*       pDestPalette,
        CONST RECT*               pDestRect,
        LPDIRECT3DSURFACE9        pSrcSurface,
        CONST PALETTEENTRY*       pSrcPalette,
        CONST RECT*               pSrcRect,
        DWORD                     Filter,
        D3DCOLOR                  ColorKey);


//----------------------------------------------------------------------------
// D3DXLoadSurfaceFromMemory:
// --------------------------
// Load surface from memory.
//
// Parameters:
//  pDestSurface
//      Destination surface, which will receive the image.
//  pDestPalette
//      Destination palette of 256 colors, or NULL
//  pDestRect
//      Destination rectangle, or NULL for entire surface
//  pSrcMemory
//      Pointer to the top-left corner of the source image in memory
//  SrcFormat
//      Pixel format of the source image.
//  SrcPitch
//      Pitch of source image, in bytes.  For DXT formats, this number
//      should represent the width of one row of cells, in bytes.
//  pSrcPalette
//      Source palette of 256 colors, or NULL
//  pSrcRect
//      Source rectangle.
//  Filter
//      D3DX_FILTER flags controlling how the image is filtered.
//      Or D3DX_DEFAULT for D3DX_FILTER_TRIANGLE.
//  ColorKey
//      Color to replace with transparent black, or 0 to disable colorkey.
//      This is always a 32-bit ARGB color, independent of the source image
//      format.  Alpha is significant, and should usually be set to FF for 
//      opaque colorkeys.  (ex. Opaque black == 0xff000000)
//
//----------------------------------------------------------------------------

HRESULT WINAPI
    D3DXLoadSurfaceFromMemory(
        LPDIRECT3DSURFACE9        pDestSurface,
        CONST PALETTEENTRY*       pDestPalette,
        CONST RECT*               pDestRect,
        LPCVOID                   pSrcMemory,
        D3DFORMAT                 SrcFormat,
        UINT                      SrcPitch,
        CONST PALETTEENTRY*       pSrcPalette,
        CONST RECT*               pSrcRect,
        DWORD                     Filter,
        D3DCOLOR                  ColorKey);


//----------------------------------------------------------------------------
// D3DXSaveSurfaceToFile:
// ----------------------
// Save a surface to a image file.
//
// Parameters:
//  pDestFile
//      File name of the destination file
//  DestFormat
//      D3DXIMAGE_FILEFORMAT specifying file format to use when saving.
//  pSrcSurface
//      Source surface, containing the image to be saved
//  pSrcPalette
//      Source palette of 256 colors, or NULL
//  pSrcRect
//      Source rectangle, or NULL for the entire image
//
//----------------------------------------------------------------------------

HRESULT WINAPI
    D3DXSaveSurfaceToFileA(
        LPCSTR                    pDestFile,
        D3DXIMAGE_FILEFORMAT      DestFormat,
        LPDIRECT3DSURFACE9        pSrcSurface,
        CONST PALETTEENTRY*       pSrcPalette,
        CONST RECT*               pSrcRect);

HRESULT WINAPI
    D3DXSaveSurfaceToFileW(
        LPCWSTR                   pDestFile,
        D3DXIMAGE_FILEFORMAT      DestFormat,
        LPDIRECT3DSURFACE9        pSrcSurface,
        CONST PALETTEENTRY*       pSrcPalette,
        CONST RECT*               pSrcRect);

#ifdef UNICODE
#define D3DXSaveSurfaceToFile D3DXSaveSurfaceToFileW
#else
#define D3DXSaveSurfaceToFile D3DXSaveSurfaceToFileA
#endif

//----------------------------------------------------------------------------
// D3DXSaveSurfaceToFileInMemory:
// ----------------------
// Save a surface to a image file.
//
// Parameters:
//  ppDestBuf
//      address of pointer to d3dxbuffer for returning data bits
//  DestFormat
//      D3DXIMAGE_FILEFORMAT specifying file format to use when saving.
//  pSrcSurface
//      Source surface, containing the image to be saved
//  pSrcPalette
//      Source palette of 256 colors, or NULL
//  pSrcRect
//      Source rectangle, or NULL for the entire image
//
//----------------------------------------------------------------------------

HRESULT WINAPI
    D3DXSaveSurfaceToFileInMemory(
        LPD3DXBUFFER*             ppDestBuf,
        D3DXIMAGE_FILEFORMAT      DestFormat,
        LPDIRECT3DSURFACE9        pSrcSurface,
        CONST PALETTEENTRY*       pSrcPalette,
        CONST RECT*               pSrcRect);


//////////////////////////////////////////////////////////////////////////////
// Load/Save Volume APIs /////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
// D3DXLoadVolumeFromFile/Resource:
// --------------------------------
// Load volume from a file or resource
//
// Parameters:
//  pDestVolume
//      Destination volume, which will receive the image.
//  pDestPalette
//      Destination palette of 256 colors, or NULL
//  pDestBox
//      Destination box, or NULL for entire volume
//  pSrcFile
//      File name of the source image.
//  pSrcModule
//      Module where resource is located, or NULL for module associated
//      with image the os used to create the current process.
//  pSrcResource
//      Resource name
//  pSrcData
//      Pointer to file in memory.
//  SrcDataSize
//      Size in bytes of file in memory.
//  pSrcBox
//      Source box, or NULL for entire image
//  Filter
//      D3DX_FILTER flags controlling how the image is filtered.
//      Or D3DX_DEFAULT for D3DX_FILTER_TRIANGLE.
//  ColorKey
//      Color to replace with transparent black, or 0 to disable colorkey.
//      This is always a 32-bit ARGB color, independent of the source image
//      format.  Alpha is significant, and should usually be set to FF for 
//      opaque colorkeys.  (ex. Opaque black == 0xff000000)
//  pSrcInfo
//      Pointer to a D3DXIMAGE_INFO structure to be filled in with the 
//      description of the data in the source image file, or NULL.
//
//----------------------------------------------------------------------------

HRESULT WINAPI
    D3DXLoadVolumeFromFileA(
        LPDIRECT3DVOLUME9         pDestVolume,
        CONST PALETTEENTRY*       pDestPalette,
        CONST D3DBOX*             pDestBox,
        LPCSTR                    pSrcFile,
        CONST D3DBOX*             pSrcBox,
        DWORD                     Filter,
        D3DCOLOR                  ColorKey,
        D3DXIMAGE_INFO*           pSrcInfo);

HRESULT WINAPI
    D3DXLoadVolumeFromFileW(
        LPDIRECT3DVOLUME9         pDestVolume,
        CONST PALETTEENTRY*       pDestPalette,
        CONST D3DBOX*             pDestBox,
        LPCWSTR                   pSrcFile,
        CONST D3DBOX*             pSrcBox,
        DWORD                     Filter,
        D3DCOLOR                  ColorKey,
        D3DXIMAGE_INFO*           pSrcInfo);

#ifdef UNICODE
#define D3DXLoadVolumeFromFile D3DXLoadVolumeFromFileW
#else
#define D3DXLoadVolumeFromFile D3DXLoadVolumeFromFileA
#endif


HRESULT WINAPI
    D3DXLoadVolumeFromResourceA(
        LPDIRECT3DVOLUME9         pDestVolume,
        CONST PALETTEENTRY*       pDestPalette,
        CONST D3DBOX*             pDestBox,
        HMODULE                   hSrcModule,
        LPCSTR                    pSrcResource,
        CONST D3DBOX*             pSrcBox,
        DWORD                     Filter,
        D3DCOLOR                  ColorKey,
        D3DXIMAGE_INFO*           pSrcInfo);

HRESULT WINAPI
    D3DXLoadVolumeFromResourceW(
        LPDIRECT3DVOLUME9         pDestVolume,
        CONST PALETTEENTRY*       pDestPalette,
        CONST D3DBOX*             pDestBox,
        HMODULE                   hSrcModule,
        LPCWSTR                   pSrcResource,
        CONST D3DBOX*             pSrcBox,
        DWORD                     Filter,
        D3DCOLOR                  ColorKey,
        D3DXIMAGE_INFO*           pSrcInfo);

#ifdef UNICODE
#define D3DXLoadVolumeFromResource D3DXLoadVolumeFromResourceW
#else
#define D3DXLoadVolumeFromResource D3DXLoadVolumeFromResourceA
#endif



HRESULT WINAPI
    D3DXLoadVolumeFromFileInMemory(
        LPDIRECT3DVOLUME9         pDestVolume,
        CONST PALETTEENTRY*       pDestPalette,
        CONST D3DBOX*             pDestBox,
        LPCVOID                   pSrcData,
        UINT                      SrcDataSize,
        CONST D3DBOX*             pSrcBox,
        DWORD                     Filter,
        D3DCOLOR                  ColorKey,
        D3DXIMAGE_INFO*           pSrcInfo);



//----------------------------------------------------------------------------
// D3DXLoadVolumeFromVolume:
// -------------------------
// Load volume from another volume (with color conversion)
//
// Parameters:
//  pDestVolume
//      Destination volume, which will receive the image.
//  pDestPalette
//      Destination palette of 256 colors, or NULL
//  pDestBox
//      Destination box, or NULL for entire volume
//  pSrcVolume
//      Source volume
//  pSrcPalette
//      Source palette of 256 colors, or NULL
//  pSrcBox
//      Source box, or NULL for entire volume
//  Filter
//      D3DX_FILTER flags controlling how the image is filtered.
//      Or D3DX_DEFAULT for D3DX_FILTER_TRIANGLE.
//  ColorKey
//      Color to replace with transparent black, or 0 to disable colorkey.
//      This is always a 32-bit ARGB color, independent of the source image
//      format.  Alpha is significant, and should usually be set to FF for 
//      opaque colorkeys.  (ex. Opaque black == 0xff000000)
//
//----------------------------------------------------------------------------

HRESULT WINAPI
    D3DXLoadVolumeFromVolume(
        LPDIRECT3DVOLUME9         pDestVolume,
        CONST PALETTEENTRY*       pDestPalette,
        CONST D3DBOX*             pDestBox,
        LPDIRECT3DVOLUME9         pSrcVolume,
        CONST PALETTEENTRY*       pSrcPalette,
        CONST D3DBOX*             pSrcBox,
        DWORD                     Filter,
        D3DCOLOR                  ColorKey);



//----------------------------------------------------------------------------
// D3DXLoadVolumeFromMemory:
// -------------------------
// Load volume from memory.
//
// Parameters:
//  pDestVolume
//      Destination volume, which will receive the image.
//  pDestPalette
//      Destination palette of 256 colors, or NULL
//  pDestBox
//      Destination box, or NULL for entire volume
//  pSrcMemory
//      Pointer to the top-left corner of the source volume in memory
//  SrcFormat
//      Pixel format of the source volume.
//  SrcRowPitch
//      Pitch of source image, in bytes.  For DXT formats, this number
//      should represent the size of one row of cells, in bytes.
//  SrcSlicePitch
//      Pitch of source image, in bytes.  For DXT formats, this number
//      should represent the size of one slice of cells, in bytes.
//  pSrcPalette
//      Source palette of 256 colors, or NULL
//  pSrcBox
//      Source box.
//  Filter
//      D3DX_FILTER flags controlling how the image is filtered.
//      Or D3DX_DEFAULT for D3DX_FILTER_TRIANGLE.
//  ColorKey
//      Color to replace with transparent black, or 0 to disable colorkey.
//      This is always a 32-bit ARGB color, independent of the source image
//      format.  Alpha is significant, and should usually be set to FF for 
//      opaque colorkeys.  (ex. Opaque black == 0xff000000)
//
//----------------------------------------------------------------------------

HRESULT WINAPI
    D3DXLoadVolumeFromMemory(
        LPDIRECT3DVOLUME9         pDestVolume,
        CONST PALETTEENTRY*       pDestPalette,
        CONST D3DBOX*             pDestBox,
        LPCVOID                   pSrcMemory,
        D3DFORMAT                 SrcFormat,
        UINT                      SrcRowPitch,
        UINT                      SrcSlicePitch,
        CONST PALETTEENTRY*       pSrcPalette,
        CONST D3DBOX*             pSrcBox,
        DWORD                     Filter,
        D3DCOLOR                  ColorKey);



//----------------------------------------------------------------------------
// D3DXSaveVolumeToFile:
// ---------------------
// Save a volume to a image file.
//
// Parameters:
//  pDestFile
//      File name of the destination file
//  DestFormat
//      D3DXIMAGE_FILEFORMAT specifying file format to use when saving.
//  pSrcVolume
//      Source volume, containing the image to be saved
//  pSrcPalette
//      Source palette of 256 colors, or NULL
//  pSrcBox
//      Source box, or NULL for the entire volume
//
//----------------------------------------------------------------------------

HRESULT WINAPI
    D3DXSaveVolumeToFileA(
        LPCSTR                    pDestFile,
        D3DXIMAGE_FILEFORMAT      DestFormat,
        LPDIRECT3DVOLUME9         pSrcVolume,
        CONST PALETTEENTRY*       pSrcPalette,
        CONST D3DBOX*             pSrcBox);

HRESULT WINAPI
    D3DXSaveVolumeToFileW(
        LPCWSTR                   pDestFile,
        D3DXIMAGE_FILEFORMAT      DestFormat,
        LPDIRECT3DVOLUME9         pSrcVolume,
        CONST PALETTEENTRY*       pSrcPalette,
        CONST D3DBOX*             pSrcBox);

#ifdef UNICODE
#define D3DXSaveVolumeToFile D3DXSaveVolumeToFileW
#else
#define D3DXSaveVolumeToFile D3DXSaveVolumeToFileA
#endif


//----------------------------------------------------------------------------
// D3DXSaveVolumeToFileInMemory:
// ---------------------
// Save a volume to a image file.
//
// Parameters:
//  pDestFile
//      File name of the destination file
//  DestFormat
//      D3DXIMAGE_FILEFORMAT specifying file format to use when saving.
//  pSrcVolume
//      Source volume, containing the image to be saved
//  pSrcPalette
//      Source palette of 256 colors, or NULL
//  pSrcBox
//      Source box, or NULL for the entire volume
//
//----------------------------------------------------------------------------

HRESULT WINAPI
    D3DXSaveVolumeToFileInMemory(
        LPD3DXBUFFER*             ppDestBuf,
        D3DXIMAGE_FILEFORMAT      DestFormat,
        LPDIRECT3DVOLUME9         pSrcVolume,
        CONST PALETTEENTRY*       pSrcPalette,
        CONST D3DBOX*             pSrcBox);

//////////////////////////////////////////////////////////////////////////////
// Create/Save Texture APIs //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
// D3DXCheckTextureRequirements:
// -----------------------------
// Checks texture creation parameters.  If parameters are invalid, this
// function returns corrected parameters.
//
// Parameters:
//
//  pDevice
//      The D3D device to be used
//  pWidth, pHeight, pDepth, pSize
//      Desired size in pixels, or NULL.  Returns corrected size.
//  pNumMipLevels
//      Number of desired mipmap levels, or NULL.  Returns corrected number.
//  Usage
//      Texture usage flags
//  pFormat
//      Desired pixel format, or NULL.  Returns corrected format.
//  Pool
//      Memory pool to be used to create texture
//
//----------------------------------------------------------------------------

HRESULT WINAPI
    D3DXCheckTextureRequirements(
        LPDIRECT3DDEVICE9         pDevice,
        UINT*                     pWidth,
        UINT*                     pHeight,
        UINT*                     pNumMipLevels,
        DWORD                     Usage,
        D3DFORMAT*                pFormat,
        D3DPOOL                   Pool);

HRESULT WINAPI
    D3DXCheckCubeTextureRequirements(
        LPDIRECT3DDEVICE9         pDevice,
        UINT*                     pSize,
        UINT*                     pNumMipLevels,
        DWORD                     Usage,
        D3DFORMAT*                pFormat,
        D3DPOOL                   Pool);

HRESULT WINAPI
    D3DXCheckVolumeTextureRequirements(
        LPDIRECT3DDEVICE9         pDevice,
        UINT*                     pWidth,
        UINT*                     pHeight,
        UINT*                     pDepth,
        UINT*                     pNumMipLevels,
        DWORD                     Usage,
        D3DFORMAT*                pFormat,
        D3DPOOL                   Pool);


//----------------------------------------------------------------------------
// D3DXCreateTexture:
// ------------------
// Create an empty texture
//
// Parameters:
//
//  pDevice
//      The D3D device with which the texture is going to be used.
//  Width, Height, Depth, Size
//      size in pixels. these must be non-zero
//  MipLevels
//      number of mip levels desired. if zero or D3DX_DEFAULT, a complete
//      mipmap chain will be created.
//  Usage
//      Texture usage flags
//  Format
//      Pixel format.
//  Pool
//      Memory pool to be used to create texture
//  ppTexture, ppCubeTexture, ppVolumeTexture
//      The texture object that will be created
//
//----------------------------------------------------------------------------

HRESULT WINAPI
    D3DXCreateTexture(
        LPDIRECT3DDEVICE9         pDevice,
        UINT                      Width,
        UINT                      Height,
        UINT                      MipLevels,
        DWORD                     Usage,
        D3DFORMAT                 Format,
        D3DPOOL                   Pool,
        LPDIRECT3DTEXTURE9*       ppTexture);

HRESULT WINAPI
    D3DXCreateCubeTexture(
        LPDIRECT3DDEVICE9         pDevice,
        UINT                      Size,
        UINT                      MipLevels,
        DWORD                     Usage,
        D3DFORMAT                 Format,
        D3DPOOL                   Pool,
        LPDIRECT3DCUBETEXTURE9*   ppCubeTexture);

HRESULT WINAPI
    D3DXCreateVolumeTexture(
        LPDIRECT3DDEVICE9         pDevice,
        UINT                      Width,
        UINT                      Height,
        UINT                      Depth,
        UINT                      MipLevels,
        DWORD                     Usage,
        D3DFORMAT                 Format,
        D3DPOOL                   Pool,
        LPDIRECT3DVOLUMETEXTURE9* ppVolumeTexture);



//----------------------------------------------------------------------------
// D3DXCreateTextureFromFile/Resource:
// -----------------------------------
// Create a texture object from a file or resource.
//
// Parameters:
//
//  pDevice
//      The D3D device with which the texture is going to be used.
//  pSrcFile
//      File name.
//  hSrcModule
//      Module handle. if NULL, current module will be used.
//  pSrcResource
//      Resource name in module
//  pvSrcData
//      Pointer to file in memory.
//  SrcDataSize
//      Size in bytes of file in memory.
//  Width, Height, Depth, Size
//      Size in pixels.  If zero or D3DX_DEFAULT, the size will be taken from 
//      the file and rounded up to a power of two.  If D3DX_DEFAULT_NONPOW2, 
//      and the device supports NONPOW2 textures, the size will not be rounded.
//      If D3DX_FROM_FILE, the size will be taken exactly as it is in the file, 
//      and the call will fail if this violates device capabilities.
//  MipLevels
//      Number of mip levels.  If zero or D3DX_DEFAULT, a complete mipmap
//      chain will be created.  If D3DX_FROM_FILE, the size will be taken 
//      exactly as it is in the file, and the call will fail if this violates 
//      device capabilities.
//  Usage
//      Texture usage flags
//  Format
//      Desired pixel format.  If D3DFMT_UNKNOWN, the format will be
//      taken from the file.  If D3DFMT_FROM_FILE, the format will be taken
//      exactly as it is in the file, and the call will fail if the device does
//      not support the given format.
//  Pool
//      Memory pool to be used to create texture
//  Filter
//      D3DX_FILTER flags controlling how the image is filtered.
//      Or D3DX_DEFAULT for D3DX_FILTER_TRIANGLE.
//  MipFilter
//      D3DX_FILTER flags controlling how each miplevel is filtered.
//      Or D3DX_DEFAULT for D3DX_FILTER_BOX.
//      Use the D3DX_SKIP_DDS_MIP_LEVELS macro to specify both a filter and the
//      number of mip levels to skip when loading DDS files.
//  ColorKey
//      Color to replace with transparent black, or 0 to disable colorkey.
//      This is always a 32-bit ARGB color, independent of the source image
//      format.  Alpha is significant, and should usually be set to FF for 
//      opaque colorkeys.  (ex. Opaque black == 0xff000000)
//  pSrcInfo
//      Pointer to a D3DXIMAGE_INFO structure to be filled in with the 
//      description of the data in the source image file, or NULL.
//  pPalette
//      256 color palette to be filled in, or NULL
//  ppTexture, ppCubeTexture, ppVolumeTexture
//      The texture object that will be created
//
//----------------------------------------------------------------------------

// FromFile

HRESULT WINAPI
    D3DXCreateTextureFromFileA(
        LPDIRECT3DDEVICE9         pDevice,
        LPCSTR                    pSrcFile,
        LPDIRECT3DTEXTURE9*       ppTexture);

HRESULT WINAPI
    D3DXCreateTextureFromFileW(
        LPDIRECT3DDEVICE9         pDevice,
        LPCWSTR                   pSrcFile,
        LPDIRECT3DTEXTURE9*       ppTexture);

#ifdef UNICODE
#define D3DXCreateTextureFromFile D3DXCreateTextureFromFileW
#else
#define D3DXCreateTextureFromFile D3DXCreateTextureFromFileA
#endif


HRESULT WINAPI
    D3DXCreateCubeTextureFromFileA(
        LPDIRECT3DDEVICE9         pDevice,
        LPCSTR                    pSrcFile,
        LPDIRECT3DCUBETEXTURE9*   ppCubeTexture);

HRESULT WINAPI
    D3DXCreateCubeTextureFromFileW(
        LPDIRECT3DDEVICE9         pDevice,
        LPCWSTR                   pSrcFile,
        LPDIRECT3DCUBETEXTURE9*   ppCubeTexture);

#ifdef UNICODE
#define D3DXCreateCubeTextureFromFile D3DXCreateCubeTextureFromFileW
#else
#define D3DXCreateCubeTextureFromFile D3DXCreateCubeTextureFromFileA
#endif


HRESULT WINAPI
    D3DXCreateVolumeTextureFromFileA(
        LPDIRECT3DDEVICE9         pDevice,
        LPCSTR                    pSrcFile,
        LPDIRECT3DVOLUMETEXTURE9* ppVolumeTexture);

HRESULT WINAPI
    D3DXCreateVolumeTextureFromFileW(
        LPDIRECT3DDEVICE9         pDevice,
        LPCWSTR                   pSrcFile,
        LPDIRECT3DVOLUMETEXTURE9* ppVolumeTexture);

#ifdef UNICODE
#define D3DXCreateVolumeTextureFromFile D3DXCreateVolumeTextureFromFileW
#else
#define D3DXCreateVolumeTextureFromFile D3DXCreateVolumeTextureFromFileA
#endif


// FromResource

HRESULT WINAPI
    D3DXCreateTextureFromResourceA(
        LPDIRECT3DDEVICE9         pDevice,
        HMODULE                   hSrcModule,
        LPCSTR                    pSrcResource,
        LPDIRECT3DTEXTURE9*       ppTexture);

HRESULT WINAPI
    D3DXCreateTextureFromResourceW(
        LPDIRECT3DDEVICE9         pDevice,
        HMODULE                   hSrcModule,
        LPCWSTR                   pSrcResource,
        LPDIRECT3DTEXTURE9*       ppTexture);

#ifdef UNICODE
#define D3DXCreateTextureFromResource D3DXCreateTextureFromResourceW
#else
#define D3DXCreateTextureFromResource D3DXCreateTextureFromResourceA
#endif


HRESULT WINAPI
    D3DXCreateCubeTextureFromResourceA(
        LPDIRECT3DDEVICE9         pDevice,
        HMODULE                   hSrcModule,
        LPCSTR                    pSrcResource,
        LPDIRECT3DCUBETEXTURE9*   ppCubeTexture);

HRESULT WINAPI
    D3DXCreateCubeTextureFromResourceW(
        LPDIRECT3DDEVICE9         pDevice,
        HMODULE                   hSrcModule,
        LPCWSTR                   pSrcResource,
        LPDIRECT3DCUBETEXTURE9*   ppCubeTexture);

#ifdef UNICODE
#define D3DXCreateCubeTextureFromResource D3DXCreateCubeTextureFromResourceW
#else
#define D3DXCreateCubeTextureFromResource D3DXCreateCubeTextureFromResourceA
#endif


HRESULT WINAPI
    D3DXCreateVolumeTextureFromResourceA(
        LPDIRECT3DDEVICE9         pDevice,
        HMODULE                   hSrcModule,
        LPCSTR                    pSrcResource,
        LPDIRECT3DVOLUMETEXTURE9* ppVolumeTexture);

HRESULT WINAPI
    D3DXCreateVolumeTextureFromResourceW(
        LPDIRECT3DDEVICE9         pDevice,
        HMODULE                   hSrcModule,
        LPCWSTR                   pSrcResource,
        LPDIRECT3DVOLUMETEXTURE9* ppVolumeTexture);

#ifdef UNICODE
#define D3DXCreateVolumeTextureFromResource D3DXCreateVolumeTextureFromResourceW
#else
#define D3DXCreateVolumeTextureFromResource D3DXCreateVolumeTextureFromResourceA
#endif


// FromFileEx

HRESULT WINAPI
    D3DXCreateTextureFromFileExA(
        LPDIRECT3DDEVICE9         pDevice,
        LPCSTR                    pSrcFile,
        UINT                      Width,
        UINT                      Height,
        UINT                      MipLevels,
        DWORD                     Usage,
        D3DFORMAT                 Format,
        D3DPOOL                   Pool,
        DWORD                     Filter,
        DWORD                     MipFilter,
        D3DCOLOR                  ColorKey,
        D3DXIMAGE_INFO*           pSrcInfo,
        PALETTEENTRY*             pPalette,
        LPDIRECT3DTEXTURE9*       ppTexture);

HRESULT WINAPI
    D3DXCreateTextureFromFileExW(
        LPDIRECT3DDEVICE9         pDevice,
        LPCWSTR                   pSrcFile,
        UINT                      Width,
        UINT                      Height,
        UINT                      MipLevels,
        DWORD                     Usage,
        D3DFORMAT                 Format,
        D3DPOOL                   Pool,
        DWORD                     Filter,
        DWORD                     MipFilter,
        D3DCOLOR                  ColorKey,
        D3DXIMAGE_INFO*           pSrcInfo,
        PALETTEENTRY*             pPalette,
        LPDIRECT3DTEXTURE9*       ppTexture);

#ifdef UNICODE
#define D3DXCreateTextureFromFileEx D3DXCreateTextureFromFileExW
#else
#define D3DXCreateTextureFromFileEx D3DXCreateTextureFromFileExA
#endif


HRESULT WINAPI
    D3DXCreateCubeTextureFromFileExA(
        LPDIRECT3DDEVICE9         pDevice,
        LPCSTR                    pSrcFile,
        UINT                      Size,
        UINT                      MipLevels,
        DWORD                     Usage,
        D3DFORMAT                 Format,
        D3DPOOL                   Pool,
        DWORD                     Filter,
        DWORD                     MipFilter,
        D3DCOLOR                  ColorKey,
        D3DXIMAGE_INFO*           pSrcInfo,
        PALETTEENTRY*             pPalette,
        LPDIRECT3DCUBETEXTURE9*   ppCubeTexture);

HRESULT WINAPI
    D3DXCreateCubeTextureFromFileExW(
        LPDIRECT3DDEVICE9         pDevice,
        LPCWSTR                   pSrcFile,
        UINT                      Size,
        UINT                      MipLevels,
        DWORD                     Usage,
        D3DFORMAT                 Format,
        D3DPOOL                   Pool,
        DWORD                     Filter,
        DWORD                     MipFilter,
        D3DCOLOR                  ColorKey,
        D3DXIMAGE_INFO*           pSrcInfo,
        PALETTEENTRY*             pPalette,
        LPDIRECT3DCUBETEXTURE9*   ppCubeTexture);

#ifdef UNICODE
#define D3DXCreateCubeTextureFromFileEx D3DXCreateCubeTextureFromFileExW
#else
#define D3DXCreateCubeTextureFromFileEx D3DXCreateCubeTextureFromFileExA
#endif


HRESULT WINAPI
    D3DXCreateVolumeTextureFromFileExA(
        LPDIRECT3DDEVICE9         pDevice,
        LPCSTR                    pSrcFile,
        UINT                      Width,
        UINT                      Height,
        UINT                      Depth,
        UINT                      MipLevels,
        DWORD                     Usage,
        D3DFORMAT                 Format,
        D3DPOOL                   Pool,
        DWORD                     Filter,
        DWORD                     MipFilter,
        D3DCOLOR                  ColorKey,
        D3DXIMAGE_INFO*           pSrcInfo,
        PALETTEENTRY*             pPalette,
        LPDIRECT3DVOLUMETEXTURE9* ppVolumeTexture);

HRESULT WINAPI
    D3DXCreateVolumeTextureFromFileExW(
        LPDIRECT3DDEVICE9         pDevice,
        LPCWSTR                   pSrcFile,
        UINT                      Width,
        UINT                      Height,
        UINT                      Depth,
        UINT                      MipLevels,
        DWORD                     Usage,
        D3DFORMAT                 Format,
        D3DPOOL                   Pool,
        DWORD                     Filter,
        DWORD                     MipFilter,
        D3DCOLOR                  ColorKey,
        D3DXIMAGE_INFO*           pSrcInfo,
        PALETTEENTRY*             pPalette,
        LPDIRECT3DVOLUMETEXTURE9* ppVolumeTexture);

#ifdef UNICODE
#define D3DXCreateVolumeTextureFromFileEx D3DXCreateVolumeTextureFromFileExW
#else
#define D3DXCreateVolumeTextureFromFileEx D3DXCreateVolumeTextureFromFileExA
#endif


// FromResourceEx

HRESULT WINAPI
    D3DXCreateTextureFromResourceExA(
        LPDIRECT3DDEVICE9         pDevice,
        HMODULE                   hSrcModule,
        LPCSTR                    pSrcResource,
        UINT                      Width,
        UINT                      Height,
        UINT                      MipLevels,
        DWORD                     Usage,
        D3DFORMAT                 Format,
        D3DPOOL                   Pool,
        DWORD                     Filter,
        DWORD                     MipFilter,
        D3DCOLOR                  ColorKey,
        D3DXIMAGE_INFO*           pSrcInfo,
        PALETTEENTRY*             pPalette,
        LPDIRECT3DTEXTURE9*       ppTexture);

HRESULT WINAPI
    D3DXCreateTextureFromResourceExW(
        LPDIRECT3DDEVICE9         pDevice,
        HMODULE                   hSrcModule,
        LPCWSTR                   pSrcResource,
        UINT                      Width,
        UINT                      Height,
        UINT                      MipLevels,
        DWORD                     Usage,
        D3DFORMAT                 Format,
        D3DPOOL                   Pool,
        DWORD                     Filter,
        DWORD                     MipFilter,
        D3DCOLOR                  ColorKey,
        D3DXIMAGE_INFO*           pSrcInfo,
        PALETTEENTRY*             pPalette,
        LPDIRECT3DTEXTURE9*       ppTexture);

#ifdef UNICODE
#define D3DXCreateTextureFromResourceEx D3DXCreateTextureFromResourceExW
#else
#define D3DXCreateTextureFromResourceEx D3DXCreateTextureFromResourceExA
#endif


HRESULT WINAPI
    D3DXCreateCubeTextureFromResourceExA(
        LPDIRECT3DDEVICE9         pDevice,
        HMODULE                   hSrcModule,
        LPCSTR                    pSrcResource,
        UINT                      Size,
        UINT                      MipLevels,
        DWORD                     Usage,
        D3DFORMAT                 Format,
        D3DPOOL                   Pool,
        DWORD                     Filter,
        DWORD                     MipFilter,
        D3DCOLOR                  ColorKey,
        D3DXIMAGE_INFO*           pSrcInfo,
        PALETTEENTRY*             pPalette,
        LPDIRECT3DCUBETEXTURE9*   ppCubeTexture);

HRESULT WINAPI
    D3DXCreateCubeTextureFromResourceExW(
        LPDIRECT3DDEVICE9         pDevice,
        HMODULE                   hSrcModule,
        LPCWSTR                   pSrcResource,
        UINT                      Size,
        UINT                      MipLevels,
        DWORD                     Usage,
        D3DFORMAT                 Format,
        D3DPOOL                   Pool,
        DWORD                     Filter,
        DWORD                     MipFilter,
        D3DCOLOR                  ColorKey,
        D3DXIMAGE_INFO*           pSrcInfo,
        PALETTEENTRY*             pPalette,
        LPDIRECT3DCUBETEXTURE9*   ppCubeTexture);

#ifdef UNICODE
#define D3DXCreateCubeTextureFromResourceEx D3DXCreateCubeTextureFromResourceExW
#else
#define D3DXCreateCubeTextureFromResourceEx D3DXCreateCubeTextureFromResourceExA
#endif


HRESULT WINAPI
    D3DXCreateVolumeTextureFromResourceExA(
        LPDIRECT3DDEVICE9         pDevice,
        HMODULE                   hSrcModule,
        LPCSTR                    pSrcResource,
        UINT                      Width,
        UINT                      Height,
        UINT                      Depth,
        UINT                      MipLevels,
        DWORD                     Usage,
        D3DFORMAT                 Format,
        D3DPOOL                   Pool,
        DWORD                     Filter,
        DWORD                     MipFilter,
        D3DCOLOR                  ColorKey,
        D3DXIMAGE_INFO*           pSrcInfo,
        PALETTEENTRY*             pPalette,
        LPDIRECT3DVOLUMETEXTURE9* ppVolumeTexture);

HRESULT WINAPI
    D3DXCreateVolumeTextureFromResourceExW(
        LPDIRECT3DDEVICE9         pDevice,
        HMODULE                   hSrcModule,
        LPCWSTR                   pSrcResource,
        UINT                      Width,
        UINT                      Height,
        UINT                      Depth,
        UINT                      MipLevels,
        DWORD                     Usage,
        D3DFORMAT                 Format,
        D3DPOOL                   Pool,
        DWORD                     Filter,
        DWORD                     MipFilter,
        D3DCOLOR                  ColorKey,
        D3DXIMAGE_INFO*           pSrcInfo,
        PALETTEENTRY*             pPalette,
        LPDIRECT3DVOLUMETEXTURE9* ppVolumeTexture);

#ifdef UNICODE
#define D3DXCreateVolumeTextureFromResourceEx D3DXCreateVolumeTextureFromResourceExW
#else
#define D3DXCreateVolumeTextureFromResourceEx D3DXCreateVolumeTextureFromResourceExA
#endif


// FromFileInMemory

HRESULT WINAPI
    D3DXCreateTextureFromFileInMemory(
        LPDIRECT3DDEVICE9         pDevice,
        LPCVOID                   pSrcData,
        UINT                      SrcDataSize,
        LPDIRECT3DTEXTURE9*       ppTexture);

HRESULT WINAPI
    D3DXCreateCubeTextureFromFileInMemory(
        LPDIRECT3DDEVICE9         pDevice,
        LPCVOID                   pSrcData,
        UINT                      SrcDataSize,
        LPDIRECT3DCUBETEXTURE9*   ppCubeTexture);

HRESULT WINAPI
    D3DXCreateVolumeTextureFromFileInMemory(
        LPDIRECT3DDEVICE9         pDevice,
        LPCVOID                   pSrcData,
        UINT                      SrcDataSize,
        LPDIRECT3DVOLUMETEXTURE9* ppVolumeTexture);


// FromFileInMemoryEx

HRESULT WINAPI
    D3DXCreateTextureFromFileInMemoryEx(
        LPDIRECT3DDEVICE9         pDevice,
        LPCVOID                   pSrcData,
        UINT                      SrcDataSize,
        UINT                      Width,
        UINT                      Height,
        UINT                      MipLevels,
        DWORD                     Usage,
        D3DFORMAT                 Format,
        D3DPOOL                   Pool,
        DWORD                     Filter,
        DWORD                     MipFilter,
        D3DCOLOR                  ColorKey,
        D3DXIMAGE_INFO*           pSrcInfo,
        PALETTEENTRY*             pPalette,
        LPDIRECT3DTEXTURE9*       ppTexture);

HRESULT WINAPI
    D3DXCreateCubeTextureFromFileInMemoryEx(
        LPDIRECT3DDEVICE9         pDevice,
        LPCVOID                   pSrcData,
        UINT                      SrcDataSize,
        UINT                      Size,
        UINT                      MipLevels,
        DWORD                     Usage,
        D3DFORMAT                 Format,
        D3DPOOL                   Pool,
        DWORD                     Filter,
        DWORD                     MipFilter,
        D3DCOLOR                  ColorKey,
        D3DXIMAGE_INFO*           pSrcInfo,
        PALETTEENTRY*             pPalette,
        LPDIRECT3DCUBETEXTURE9*   ppCubeTexture);

HRESULT WINAPI
    D3DXCreateVolumeTextureFromFileInMemoryEx(
        LPDIRECT3DDEVICE9         pDevice,
        LPCVOID                   pSrcData,
        UINT                      SrcDataSize,
        UINT                      Width,
        UINT                      Height,
        UINT                      Depth,
        UINT                      MipLevels,
        DWORD                     Usage,
        D3DFORMAT                 Format,
        D3DPOOL                   Pool,
        DWORD                     Filter,
        DWORD                     MipFilter,
        D3DCOLOR                  ColorKey,
        D3DXIMAGE_INFO*           pSrcInfo,
        PALETTEENTRY*             pPalette,
        LPDIRECT3DVOLUMETEXTURE9* ppVolumeTexture);



//----------------------------------------------------------------------------
// D3DXSaveTextureToFile:
// ----------------------
// Save a texture to a file.
//
// Parameters:
//  pDestFile
//      File name of the destination file
//  DestFormat
//      D3DXIMAGE_FILEFORMAT specifying file format to use when saving.
//  pSrcTexture
//      Source texture, containing the image to be saved
//  pSrcPalette
//      Source palette of 256 colors, or NULL
//
//----------------------------------------------------------------------------


HRESULT WINAPI
    D3DXSaveTextureToFileA(
        LPCSTR                    pDestFile,
        D3DXIMAGE_FILEFORMAT      DestFormat,
        LPDIRECT3DBASETEXTURE9    pSrcTexture,
        CONST PALETTEENTRY*       pSrcPalette);

HRESULT WINAPI
    D3DXSaveTextureToFileW(
        LPCWSTR                   pDestFile,
        D3DXIMAGE_FILEFORMAT      DestFormat,
        LPDIRECT3DBASETEXTURE9    pSrcTexture,
        CONST PALETTEENTRY*       pSrcPalette);

#ifdef UNICODE
#define D3DXSaveTextureToFile D3DXSaveTextureToFileW
#else
#define D3DXSaveTextureToFile D3DXSaveTextureToFileA
#endif


//----------------------------------------------------------------------------
// D3DXSaveTextureToFileInMemory:
// ----------------------
// Save a texture to a file.
//
// Parameters:
//  ppDestBuf
//      address of a d3dxbuffer pointer to return the image data
//  DestFormat
//      D3DXIMAGE_FILEFORMAT specifying file format to use when saving.
//  pSrcTexture
//      Source texture, containing the image to be saved
//  pSrcPalette
//      Source palette of 256 colors, or NULL
//
//----------------------------------------------------------------------------

HRESULT WINAPI
    D3DXSaveTextureToFileInMemory(
        LPD3DXBUFFER*             ppDestBuf,
        D3DXIMAGE_FILEFORMAT      DestFormat,
        LPDIRECT3DBASETEXTURE9    pSrcTexture,
        CONST PALETTEENTRY*       pSrcPalette);




//////////////////////////////////////////////////////////////////////////////
// Misc Texture APIs /////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
// D3DXFilterTexture:
// ------------------
// Filters mipmaps levels of a texture.
//
// Parameters:
//  pBaseTexture
//      The texture object to be filtered
//  pPalette
//      256 color palette to be used, or NULL for non-palettized formats
//  SrcLevel
//      The level whose image is used to generate the subsequent levels. 
//  Filter
//      D3DX_FILTER flags controlling how each miplevel is filtered.
//      Or D3DX_DEFAULT for D3DX_FILTER_BOX,
//
//----------------------------------------------------------------------------

HRESULT WINAPI
    D3DXFilterTexture(
        LPDIRECT3DBASETEXTURE9    pBaseTexture,
        CONST PALETTEENTRY*       pPalette,
        UINT                      SrcLevel,
        DWORD                     Filter);

#define D3DXFilterCubeTexture D3DXFilterTexture
#define D3DXFilterVolumeTexture D3DXFilterTexture



//----------------------------------------------------------------------------
// D3DXFillTexture:
// ----------------
// Uses a user provided function to fill each texel of each mip level of a
// given texture.
//
// Paramters:
//  pTexture, pCubeTexture, pVolumeTexture
//      Pointer to the texture to be filled.
//  pFunction
//      Pointer to user provided evalutor function which will be used to 
//      compute the value of each texel.
//  pData
//      Pointer to an arbitrary block of user defined data.  This pointer 
//      will be passed to the function provided in pFunction
//-----------------------------------------------------------------------------

HRESULT WINAPI
    D3DXFillTexture(
        LPDIRECT3DTEXTURE9        pTexture,
        LPD3DXFILL2D              pFunction,
        LPVOID                    pData);

HRESULT WINAPI
    D3DXFillCubeTexture(
        LPDIRECT3DCUBETEXTURE9    pCubeTexture,
        LPD3DXFILL3D              pFunction,
        LPVOID                    pData);

HRESULT WINAPI
    D3DXFillVolumeTexture(
        LPDIRECT3DVOLUMETEXTURE9  pVolumeTexture,
        LPD3DXFILL3D              pFunction,
        LPVOID                    pData);

//---------------------------------------------------------------------------
// D3DXFillTextureTX:
// ------------------
// Uses a TX Shader target to function to fill each texel of each mip level
// of a given texture. The TX Shader target should be a compiled function 
// taking 2 paramters and returning a float4 color.
//
// Paramters:
//  pTexture, pCubeTexture, pVolumeTexture
//      Pointer to the texture to be filled.
//  pTextureShader
//      Pointer to the texture shader to be used to fill in the texture
//----------------------------------------------------------------------------

HRESULT WINAPI 
    D3DXFillTextureTX(
        LPDIRECT3DTEXTURE9        pTexture,
        LPD3DXTEXTURESHADER       pTextureShader);


HRESULT WINAPI
    D3DXFillCubeTextureTX(
        LPDIRECT3DCUBETEXTURE9    pCubeTexture,
        LPD3DXTEXTURESHADER       pTextureShader);
                                                
                                                        
HRESULT WINAPI 
    D3DXFillVolumeTextureTX(
        LPDIRECT3DVOLUMETEXTURE9  pVolumeTexture,
        LPD3DXTEXTURESHADER       pTextureShader);



//----------------------------------------------------------------------------
// D3DXComputeNormalMap:
// ---------------------
// Converts a height map into a normal map.  The (x,y,z) components of each
// normal are mapped to the (r,g,b) channels of the output texture.
//
// Parameters
//  pTexture
//      Pointer to the destination texture
//  pSrcTexture
//      Pointer to the source heightmap texture 
//  pSrcPalette
//      Source palette of 256 colors, or NULL
//  Flags
//      D3DX_NORMALMAP flags
//  Channel
//      D3DX_CHANNEL specifying source of height information
//  Amplitude
//      The constant value which the height information is multiplied by.
//---------------------------------------------------------------------------

HRESULT WINAPI
    D3DXComputeNormalMap(
        LPDIRECT3DTEXTURE9        pTexture,
        LPDIRECT3DTEXTURE9        pSrcTexture,
        CONST PALETTEENTRY*       pSrcPalette,
        DWORD                     Flags,
        DWORD                     Channel,
        FLOAT                     Amplitude);




#ifdef __cplusplus
}
#endif //__cplusplus

#endif //__D3DX9TEX_H__

