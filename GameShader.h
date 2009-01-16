//-------------------------------------------------------------------------
//
// Required Includes
//
//-------------------------------------------------------------------------

#import <Cocoa/Cocoa.h>
#import <OpenGL/OpenGL.h>

//-------------------------------------------------------------------------
//
// GLSL Shader
//
//-------------------------------------------------------------------------

@interface GameShader : NSObject
{
	@private
		const GLcharARB    *fragmentShaderSource;		// the GLSL source for the fragment Shader
		const GLcharARB    *vertexShaderSource;			// the GLSL source for the vertex Shader
		GLhandleARB		    programObject;				// the program object
	
		NSBundle * bundleToLoadFrom;					// location of resource folder;
} // Shader

- (id) initWithShadersInAppBundle:(NSString *)theShadersName;
- (id) initWithShadersInBundle:(NSBundle*)bundle withName:(NSString *)theShadersName;

- (GLhandleARB) programObject;

- (GLint) getUniformLocation:(const GLcharARB *)theUniformName;

@end

//-------------------------------------------------------------------------

