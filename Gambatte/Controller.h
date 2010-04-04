/*
 Copyright (c) 2009, OpenEmu Team
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the OpenEmu Team nor the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY OpenEmu Team ''AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL OpenEmu Team BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#import <Cocoa/Cocoa.h>
#import "MyOpenGLView.h"
#import "PrefController.h"


@interface Controller : NSObject {

	IBOutlet NSWindow* windowEmu;
	IBOutlet MyOpenGLView* viewEmu;
	IBOutlet NSMenuItem* stateS;
	IBOutlet NSMenuItem* stateL;
	IBOutlet NSMenuItem* pausePlay;
	IBOutlet NSMenuItem* resetItem;
	IBOutlet NSMenuItem* screenshotItem;
	IBOutlet NSMenuItem* closeItem;
	IBOutlet NSMenuItem* filtNoneItem;
	IBOutlet NSMenuItem* filt2xItem;
	IBOutlet NSMenuItem* filt3xItem;
	IBOutlet NSMenuItem* filtSai2xItem;
	IBOutlet NSMenuItem* filtHq2xItem;
	IBOutlet NSMenu* filtMenu;
	PrefController* windowPref;
	
}

-(IBAction) startLoad: (id) sender;
-(IBAction) saveSaveState: (id) sender;
-(IBAction) loadSaveState: (id) sender;
-(IBAction) pauseEmu: (id) sender;
-(IBAction) resetEmu: (id) sender;
-(IBAction) closeWindow: (id) sender;
-(IBAction) filterNone: (id) sender;
-(IBAction) filter2x: (id) sender;
-(IBAction) filter3x: (id) sender;
-(IBAction) filterSai2x: (id) sender;
-(IBAction) filterHq2x: (id) sender;
-(IBAction) screenshot: (id) sender;
-(IBAction) displayPref: (id) sender;
- (void) changeFilter: (int) newFilt;
- (void) changeVolume: (float) newVolume;
- (NSBitmapImageRep*) getRawData;
- (void) activateMenus: (bool) will;

@end
