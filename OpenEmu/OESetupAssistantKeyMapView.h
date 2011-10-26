//
//  OESetupAssistantKeyMapView.h
//  OpenEmu
//
//  Created by Carl Leimbrock on 26.10.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

enum _OESetupAssistantKeys 
{
    OESetupAssistantKeyUp,
    OESetupAssistantKeyDown,
    OESetupAssistantKeyLeft,
    OESetupAssistantKeyRight,
    
    OESetupAssistantKeyQuestionMark
}
typedef OESetupAssistantKey;
@interface OESetupAssistantKeyMapView : NSView
@property OESetupAssistantKey key;
@end
