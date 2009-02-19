//
//  v002BlurPlugIn.h
//  v002Blur
//
//  Created by vade on 7/10/08.
//  Copyright (c) 2008 __MyCompanyName__. All rights reserved.
//

#import <OpenGL/OpenGL.h>
@class GameShader;

@interface OpenEmuQCFiltersPlugin : QCPlugIn
{
	GameShader* Scale2xPlus;
	GameShader* Scale2xHQ;
	GameShader* Scale4x;	
	GameShader* Scale4xHQ;
	
	GLuint frameBuffer;
}

/*
Declare here the Obj-C 2.0 properties to be used as input and output ports for the plug-in e.g.
@property double inputFoo;
@property(assign) NSString* outputBar;
You can access their values in the appropriate plug-in methods using self.inputFoo or self.inputBar
*/

@property (assign) id<QCPlugInInputImageSource> inputImage;
@property (assign) NSUInteger inputScaler;
@property (assign) id<QCPlugInOutputImageProvider> outputImage;


@end