/*
 Copyright (c) 2011, OpenEmu Team
 
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

#import "OESidebarOutlineView.h"
#import "OESidebarOutlineButtonCell.h"
#import <objc/runtime.h>
#import "NSViewController+OEAdditions.h"

@implementation OESidebarOutlineView (OEPrivate)

- (void)setupOutlineCell
{
    OESidebarOutlineButtonCell *sidebarOutlineCell = [[OESidebarOutlineButtonCell alloc] init];
    [sidebarOutlineCell retain];
    
    void *currentCell;
    object_getInstanceVariable(self, "_outlineCell", &currentCell);
    [(id)currentCell release];
    object_getInstanceVariable(self, "_trackingOutlineCell", &currentCell);
    [(id)currentCell release];
    
    object_setInstanceVariable(self, "_outlineCell", sidebarOutlineCell);
    object_setInstanceVariable(self, "_trackingOutlineCell", sidebarOutlineCell);
}

@end
