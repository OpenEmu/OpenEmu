//---------------------------------------------------------------------------------

#import "v002Shader.h"
#import <OpenGL/CGLMacro.h>

//---------------------------------------------------------------------------------

//---------------------------------------------------------------------------------

#pragma mark -- Compiling shaders & linking a program object --

//---------------------------------------------------------------------------------

static GLhandleARB LoadShader(GLenum theShaderType, 
							  const GLcharARB **theShader, 
							  GLint *theShaderCompiled,
							  CGLContextObj context) 
{
	CGLContextObj cgl_ctx = context;
		
	GLhandleARB shaderObject = NULL;
	
	if( theShader != NULL ) 
	{
		GLint infoLogLength = 0;
		
		shaderObject = glCreateShaderObjectARB(theShaderType);
		
		glShaderSourceARB(shaderObject, 1, theShader, NULL);
		glCompileShaderARB(shaderObject);
		
		glGetObjectParameterivARB(shaderObject, 
								  GL_OBJECT_INFO_LOG_LENGTH_ARB, 
								  &infoLogLength);
		
		if( infoLogLength > 0 ) 
		{
			GLcharARB *infoLog = (GLcharARB *)malloc(infoLogLength);
			
			if( infoLog != NULL )
			{
				glGetInfoLogARB(shaderObject, 
								infoLogLength, 
								&infoLogLength, 
								infoLog);
				
				NSLog(@">> Shader compile log:\n%s\n", infoLog);
				
				free(infoLog);
			} // if
		} // if
		
		glGetObjectParameterivARB(shaderObject, 
								  GL_OBJECT_COMPILE_STATUS_ARB, 
								  theShaderCompiled);
		
		if( *theShaderCompiled == 0 )
		{
			NSLog(@">> Failed to compile shader %s\n", theShader);
		} // if
	} // if
	else 
	{
		*theShaderCompiled = 1;
	} // else
	
	return shaderObject;
} // LoadShader

//---------------------------------------------------------------------------------

static void LinkProgram(GLhandleARB programObject, 
						GLint *theProgramLinked,
						CGLContextObj context) 
{
	CGLContextObj cgl_ctx = context;
	
	GLint  infoLogLength = 0;
	
	glLinkProgramARB(programObject);
	
	glGetObjectParameterivARB(programObject, 
							  GL_OBJECT_INFO_LOG_LENGTH_ARB, 
							  &infoLogLength);
	
	if( infoLogLength >  0 ) 
	{
		GLcharARB *infoLog = (GLcharARB *)malloc(infoLogLength);
		
		if( infoLog != NULL)
		{
			glGetInfoLogARB(programObject, 
							infoLogLength, 
							&infoLogLength, 
							infoLog);
			
			NSLog(@">> Program link log:\n%s\n", infoLog);
			
			free(infoLog);
		} // if
	} // if
	
	glGetObjectParameterivARB(programObject, 
							  GL_OBJECT_LINK_STATUS_ARB, 
							  theProgramLinked);
	
	if( *theProgramLinked == 0 )
	{
		NSLog(@">> Failed to link program 0x%lx\n", (GLubyte *)&programObject);
	} // if
} // LinkProgram

//---------------------------------------------------------------------------------

//---------------------------------------------------------------------------------

@implementation v002Shader

//---------------------------------------------------------------------------------

#pragma mark -- Get shaders from resource --

//---------------------------------------------------------------------------------

- (GLcharARB *) getShaderSourceFromResource:(NSString *)theShaderResourceName 
								  extension:(NSString *)theExtension
{	
	NSString  *shaderTempSource = [bundleToLoadFrom pathForResource:theShaderResourceName 
													  ofType:theExtension];	
	GLcharARB *shaderSource = NULL;
	
	shaderTempSource = [NSString stringWithContentsOfFile:shaderTempSource];
	
	shaderSource = (GLcharARB *)[shaderTempSource cStringUsingEncoding:NSASCIIStringEncoding];
	
	return  shaderSource;
} // getShaderSourceFromResource

//---------------------------------------------------------------------------------

- (void) getFragmentShaderSourceFromResource:(NSString *)theFragmentShaderResourceName
{
	fragmentShaderSource = [self getShaderSourceFromResource:theFragmentShaderResourceName 
												   extension:@"frag" ];
} // getFragmentShaderSourceFromResource

//---------------------------------------------------------------------------------

- (void) getVertexShaderSourceFromResource:(NSString *)theVertexShaderResourceName
{
	vertexShaderSource = [self getShaderSourceFromResource:theVertexShaderResourceName 
												 extension:@"vert" ];
} // getVertexShaderSourceFromResource

//---------------------------------------------------------------------------------

- (GLhandleARB) loadShader:(GLenum)theShaderType 
			  shaderSource:(const GLcharARB **)theShaderSource
{
	CGLContextObj cgl_ctx = shaderContext;
	
	GLint       shaderCompiled = 0;
	GLhandleARB shaderHandle   = LoadShader(theShaderType, 
											theShaderSource, 
											&shaderCompiled, shaderContext);
	
	if( !shaderCompiled ) 
	{
		if( shaderHandle ) 
		{
			glDeleteObjectARB(shaderHandle);
			shaderHandle = NULL;
		} // if
	} // if
	
	return shaderHandle;
} // loadShader

//---------------------------------------------------------------------------------

- (BOOL) newProgramObject:(GLhandleARB)theVertexShader  
	 fragmentShaderHandle:(GLhandleARB)theFragmentShader
{
	CGLContextObj cgl_ctx = shaderContext;
	
	GLint programLinked = 0;
	
	// Create a program object and link both shaders
	
	programObject = glCreateProgramObjectARB();
	
	glAttachObjectARB(programObject, theVertexShader);
	glDeleteObjectARB(theVertexShader);   // Release
	
	glAttachObjectARB(programObject, theFragmentShader);
	glDeleteObjectARB(theFragmentShader); // Release
	
	LinkProgram(programObject, &programLinked, cgl_ctx);
	
	if( !programLinked ) 
	{
		glDeleteObjectARB(programObject);
		
		programObject = NULL;
		
		return NO;
	} // if
	
	return YES;
} // newProgramObject

//---------------------------------------------------------------------------------

- (BOOL) setProgramObject
{	
	BOOL  programObjectSet = NO;
	
	// Load and compile both shaders
	
	GLhandleARB vertexShader = [self loadShader:GL_VERTEX_SHADER_ARB 
								   shaderSource:&vertexShaderSource];
	
	// Ensure vertex shader compiled
	
	if( vertexShader != NULL )
	{
		GLhandleARB fragmentShader = [self loadShader:GL_FRAGMENT_SHADER_ARB 
										 shaderSource:&fragmentShaderSource];
		
		// Ensure fragment shader compiled
		
		if( fragmentShader != NULL ) 
		{
			// Create a program object and link both shaders
			
			programObjectSet = [self newProgramObject:vertexShader 
								 fragmentShaderHandle:fragmentShader];
		} // if
	} // if
	
	return  programObjectSet;
} // setProgramObject

//---------------------------------------------------------------------------------

#pragma mark -- Designated Initializer --

//---------------------------------------------------------------------------------

- (id) initWithShadersInBundle:(NSBundle*)bundle withName:(NSString *)theShadersName forContext:(CGLContextObj) context
{
	if(self = [super init])
	{
		bundleToLoadFrom = [bundle retain];
		shaderContext = context; 

		BOOL  loadedShaders = NO;
		
		// Load vertex and fragment shader
		
		[self getVertexShaderSourceFromResource:theShadersName];
		
		if( vertexShaderSource != NULL )
		{
			[self getFragmentShaderSourceFromResource:theShadersName];
			
			if( fragmentShaderSource != NULL )
			{
				loadedShaders = [self setProgramObject];
				
				if( !loadedShaders)
				{
					NSLog(@">> WARNING: Failed to load GLSL \"%@\" fragment & vertex shaders!\n", 
						  theShadersName);
				} // if
			} // if
		} // if
	}
	return self;
}

- (id) initWithShadersInAppBundle:(NSString *)theShadersName forContext:(CGLContextObj)context;
{
	return [self initWithShadersInBundle:[NSBundle mainBundle] withName:theShadersName forContext:context];
} // initWithShadersInAppBundle

//---------------------------------------------------------------------------------

#pragma mark -- Deallocating Resources --

//---------------------------------------------------------------------------------

- (void) dealloc
{
	// Delete OpenGL resources
	// FIXME: Where is the deallocation ?!
	CGLContextObj cgl_ctx = shaderContext;
	
	if( programObject )
	{
		glDeleteObjectARB(programObject);
		
		programObject = NULL;
	} // if
	
	[bundleToLoadFrom release];
	
	//Dealloc the superclass
	
	[super dealloc];
} // dealloc

//---------------------------------------------------------------------------------

#pragma mark -- Accessors --

//---------------------------------------------------------------------------------

- (GLhandleARB) programObject
{
	return  programObject;
} // programObject

//---------------------------------------------------------------------------------

#pragma mark -- Utilities --

//---------------------------------------------------------------------------------

- (GLint) getUniformLocation:(const GLcharARB *)theUniformName
{
	CGLContextObj cgl_ctx = shaderContext;
	
	GLint uniformLoacation = glGetUniformLocationARB(programObject, 
													 theUniformName);
		
	if( uniformLoacation == -1 ) 
	{
		NSLog( @">> WARNING: No such uniform named \"%s\"\n", theUniformName );
	} // if
	
	return uniformLoacation;
} // getUniformLocation

//---------------------------------------------------------------------------------

@end

//---------------------------------------------------------------------------------

//---------------------------------------------------------------------------------

