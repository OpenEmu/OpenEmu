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

#import "OEGridViewCell.h"
#import "OECoverGridViewCellRatingLayer.h"
#import "OECoverGridViewCellIndicationLayer.h"

@interface OECoverGridViewCell : OEGridViewCell
{
@private
    NSTimer *_dropDelayedTimer;

    CGRect _titleFrame;
    CGRect _ratingFrame;
    CGRect _imageFrame;

    CGSize _cachedSize;

    NSURL   *_proposedImageURL; // optional; nil when the user is dropping an NSImage instead of a file
    NSImage *_proposedImage;

    OEGridLayer                    *_proposedImageLayer;
    OEGridLayer                    *_imageLayer;
    CATextLayer                    *_titleLayer;
    OECoverGridViewCellRatingLayer *_ratingLayer;

    OECoverGridViewCellIndicationLayer *_statusIndicatorLayer;
    OEGridLayer                        *_glossyOverlayLayer;
    OEGridLayer                        *_selectionIndicatorLayer;

    BOOL _needsLayoutImageAndSelection;
    BOOL _activeSelector;

    OECoverGridViewCellIndicationType _indicationType;
}

- (NSRect)toolTipRect;

#pragma mark - Properties
@property(nonatomic, strong) NSImage   *image;
@property(nonatomic, assign) NSSize     imageSize;
@property(nonatomic, copy)   NSString  *title;
@property(nonatomic, assign) NSUInteger rating;

@property(nonatomic, assign) OECoverGridViewCellIndicationType indicationType;

@property(nonatomic, readonly) CATextLayer *titleLayer;

@end
