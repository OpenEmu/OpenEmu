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
 * Fragment_NoSpotlight.glsl
 *
 * Fragment shader for 3D rendering. Spotlight effect removed. Fixes fragment
 * shader link errors on older ATI Radeon GPUs.
 *
 * To load external fragment shaders, use the -frag-shader=<file> option when
 * starting Supermodel.
 */

#version 120

// Global uniforms
uniform sampler2D	textureMap;		// complete texture map, 2048x2048 texels
uniform vec4	spotEllipse;		// spotlight ellipse position: .x=X position (screen coordinates), .y=Y position, .z=half-width, .w=half-height)
uniform vec2	spotRange;			// spotlight Z range: .x=start (viewspace coordinates), .y=limit
uniform vec3	spotColor;			// spotlight RGB color

// Inputs from vertex shader 
varying vec4		fsSubTexture;	// .x=texture X, .y=texture Y, .z=texture width, .w=texture height (all in texels)
varying vec4		fsTexParams;	// .x=texture enable (if 1, else 0), .y=use transparency (if > 0), .z=U wrap mode (1=mirror, 0=repeat), .w=V wrap mode
varying float		fsTexFormat;	// .x=T1RGB5 contour texture (if > 0)
varying float		fsTransLevel;	// translucence level, 0.0 (transparent) to 1.0 (opaque)
varying vec3		fsLightIntensity;	// lighting intensity 
varying float		fsFogFactor;		// fog factor
varying float		fsViewZ;		// Z distance to fragment from viewpoint at origin

/*
 * WrapTexelCoords():
 *
 * Computes the normalized OpenGL S,T coordinates within the 2048x2048 texture
 * sheet, taking into account wrapping behavior.
 *
 * Computing normalized OpenGL texture coordinates (0 to 1) within the 
 * Real3D texture sheet:
 *
 * If the texture is not mirrored, we simply have to clamp the
 * coordinates to fit within the texture dimensions, add the texture
 * X, Y position to select the appropriate one, and normalize by 2048
 * (the dimensions of the Real3D texture sheet).
 *
 *		= [(u,v)%(w,h)+(x,y)]/(2048,2048)
 *
 * If mirroring is enabled, textures are mirrored every odd multiple of
 * the original texture. To detect whether we are in an odd multiple, 
 * simply divide the coordinate by the texture dimension and check 
 * whether the result is odd. Then, clamp the coordinates as before but
 * subtract from the last texel to mirror them:
 *
 * 		= [M*((w-1,h-1)-(u,v)%(w,h)) + (1-M)*(u,v)%(w,h) + (x,y)]/(2048,2048)
 *		where M is 1.0 if the texture must be mirrored.
 *
 * As an optimization, this function computes TWO texture coordinates
 * simultaneously. The first is texCoord.xy, the second is in .zw. The other
 * parameters must have .xy = .zw.
 */
vec4 WrapTexelCoords(vec4 texCoord, vec4 texOffset, vec4 texSize, vec4 mirrorEnable)
{
	vec4	clampedCoord, mirror, glTexCoord;
	
	clampedCoord = mod(texCoord,texSize);						// clamp coordinates to within texture size
	mirror = mirrorEnable * mod(floor(texCoord/texSize),2.0);	// whether this texel needs to be mirrored

	glTexCoord = (	mirror*(texSize-clampedCoord) +
					(vec4(1.0,1.0,1.0,1.0)-mirror)*clampedCoord +
					texOffset
				 ) / 2048.0;
/*
	glTexCoord = (	mirror*(texSize-vec4(1.0,1.0,1.0,1.0)-clampedCoord) +
					(vec4(1.0,1.0,1.0,1.0)-mirror)*clampedCoord +
					texOffset
				 ) / 2048.0;
*/
	return glTexCoord;
}

/*
 * main():
 *
 * Fragment shader entry point.
 */

void main(void)
{	
	vec4	uv_top, uv_bot, c[4];
	vec2	r;
	vec4	fragColor;
	vec2	ellipse;
	vec3	lightIntensity;
	float	insideSpot;
	
	// Get polygon color for untextured polygons (textured polygons will overwrite)
	if (fsTexParams.x==0.0)
		fragColor = gl_Color;
	else
	// Textured polygons: set fragment color to texel value
	{			
		fragColor = texture2D(textureMap,(fsSubTexture.xy+fsSubTexture.zw/2.0)/2048.0);
		//fragColor += texture2D(textureMap,(fsSubTexture.xy+fsSubTexture.zw))/2048.0);
	
	}

	// Compute spotlight and apply lighting
	ellipse = (gl_FragCoord.xy-spotEllipse.xy)/spotEllipse.zw;
	insideSpot = dot(ellipse,ellipse);
	if ((insideSpot <= 1.0) &&  (fsViewZ>=spotRange.x) && (fsViewZ<spotRange.y))
		lightIntensity = min(fsLightIntensity+(1.0-insideSpot)*spotColor,1.0);
	else
		lightIntensity = fsLightIntensity;
	fragColor.rgb *= lightIntensity;
	fragColor.rgb *= fsLightIntensity;

	// Translucency (modulates existing alpha channel for RGBA4 texels)
	fragColor.a *= fsTransLevel;

	// Apply fog under the control of fog factor setting from polygon header
	fragColor.rgb = mix(gl_Fog.color.rgb, fragColor.rgb, fsFogFactor );

	// Store final color
	gl_FragColor = fragColor;
}
