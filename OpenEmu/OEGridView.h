/*
 Copyright (c) 2012, OpenEmu Team

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

@import Cocoa;
@import Quartz;

extern NSSize const defaultGridSize;
extern NSString * const OEImageBrowserGroupSubtitleKey;

typedef enum
{
    IKImageBrowserDropNone = 2
} ExtendedIKImageBrowserDropOperation;

@interface OEGridView : IKImageBrowserView <NSTextFieldDelegate, NSViewToolTipOwner>
@property NSImage *proposedImage;
@property (assign) IKImageBrowserDropOperation draggingOperation;

@property Class cellClass;
@property (nonatomic) BOOL automaticallyMinimizeRowMargin;

- (void)performSetup;

- (void)beginEditingWithSelectedItem:(id)sender;
- (void)beginEditingItemAtIndex:(NSInteger)index;
- (void)reloadCellDataAtIndex:(unsigned long long)arg1;
@end

@protocol OEGridViewMenuSource <NSObject>
- (NSMenu *)gridView:(OEGridView*)gridView menuForItemsAtIndexes:(NSIndexSet*)indexes;
@end

@protocol OEGridViewDelegate <NSObject>
- (void)gridView:(OEGridView*)gridView setTitle:(NSString*)title forItemAtIndex:(NSInteger)index;
@optional
- (BOOL)toggleQuickLook;
@end
