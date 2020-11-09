/*
 Copyright (c) 2014, OpenEmu Team

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

#import "OECollectionViewController.h"
#import "OEGameGridViewDelegate.h"

NS_ASSUME_NONNULL_BEGIN

extern NSNotificationName const OEGameCollectionViewControllerDidSetSelectionIndexesNotification;

@interface OEGameCollectionViewController : OECollectionViewController <OEGameGridViewDelegate, NSMenuItemValidation, NSViewToolTipOwner>
- (IBAction)showInFinder:(nullable id)sender;

- (void)performSearch:(NSString *)text;

- (void)deleteSaveState:(nullable id)stateItem;
- (void)deleteSelectedItems:(nullable id)sender;
- (void)addSelectedGamesToCollection:(nullable id)sender;
- (void)downloadCoverArt:(nullable id)sender;
- (void)cancelCoverArtDownload:(nullable id)sender;
- (void)addCoverArtFromFile:(nullable id)sender;
- (void)addSaveStateFromFile:(nullable id)sender;
- (void)consolidateFiles:(nullable id)sender;

/// The search term of the currently applied filter.
@property (copy, nullable) NSString *currentSearchTerm;

@end

NS_ASSUME_NONNULL_END
