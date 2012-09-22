//
//  INAppStoreWindow.h
//
//  Copyright 2011 Indragie Karunaratne. All rights reserved.
//
//  Licensed under the BSD License <http://www.opensource.org/licenses/bsd-license>
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
//  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
//  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
//  SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
//  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
//  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
//  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
//  THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

#import <Cocoa/Cocoa.h>

#if __has_feature(objc_arc)
#define INAppStoreWindowCopy nonatomic, strong
#define INAppStoreWindowRetain nonatomic, strong
#else
#define INAppStoreWindowCopy nonatomic, copy
#define INAppStoreWindowRetain nonatomic, retain
#endif

/** @class INTitlebarView
 Draws a default style Mac OS X title bar.
 */
@interface INTitlebarView : NSView
@end

/** @class INAppStoreWindow 
 Creates a window similar to the Mac App Store window, with centered traffic lights and an 
 enlarged title bar. This does not handle creating the toolbar.
 */
@interface INAppStoreWindow : NSWindow

/** The height of the title bar. By default, this is set to the standard title bar height. */
@property (nonatomic) CGFloat titleBarHeight;

/** The title bar view itself. Add subviews to this view that you want to show in the title bar 
 (e.g. buttons, a toolbar, etc.). This view can also be set if you want to use a different 
 styled title bar aside from the default one (textured, etc.). */
@property (INAppStoreWindowRetain) NSView *titleBarView;

/** Set whether the fullscreen or traffic light buttons are horizontally centered */
@property (nonatomic) BOOL centerFullScreenButton;
@property (nonatomic) BOOL centerTrafficLightButtons;

/** If you want to hide the title bar in fullscreen mode, set this boolean to YES */
@property (nonatomic) BOOL hideTitleBarInFullScreen;

/** Use this API to hide the baseline INAppStoreWindow draws between itself and the main window contents. */
@property (nonatomic) BOOL showsBaselineSeparator;

/** Adjust the left and right padding of the trafficlight and fullscreen buttons */
@property (nonatomic) CGFloat trafficLightButtonsLeftMargin;
@property (nonatomic) CGFloat fullScreenButtonRightMargin;


/** So much logic and work has gone into this window subclass to achieve a custom title bar,
 it would be a shame to have to re-invent that just to change the look. So this block can be used
 to override the default Mac App Store style titlebar drawing with your own drawing code!
 */
typedef void (^INAppStoreWindowTitleBarDrawingBlock)(BOOL drawsAsMainWindow, 
                                                     CGRect drawingRect, CGPathRef clippingPath);
@property (INAppStoreWindowCopy) INAppStoreWindowTitleBarDrawingBlock titleBarDrawingBlock;

@end
