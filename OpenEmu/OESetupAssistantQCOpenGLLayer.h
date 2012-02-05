//
//  OESetupAssistantQCOpenGLLayer.h
//  OpenEmu
//
//  Created by Anton Marini on 2/4/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <Quartz/Quartz.h>

// Fuck QCCompositionLayer

@interface OESetupAssistantQCOpenGLLayer : CAOpenGLLayer

@property (readwrite, assign) NSTimeInterval runningTime;
@property (readwrite, retain) NSWindow* continaingWindow;
@property (readwrite, retain) QCRenderer* renderer;

@end
