//
//  INAppStoreWindow.h
//
//  Copyright 2011 Indragie Karunaratne. All rights reserved.
//
//  Licensed under the BSD License <http://www.opensource.org/licenses/bsd-license>
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#import <Foundation/Foundation.h>

/** @class INTitlebarView
 Draws a default style Mac OS X title bar.
 **/
@interface INTitlebarView : NSView {
    
}
- (NSBezierPath*)clippingPathWithRect:(NSRect)aRect cornerRadius:(float)radius;
@end

/** @class INAppStoreWindow 
 Creates a window similar to the Mac App Store window, with centered traffic lights and an enlarged title bar. This does not handle creating the toolbar.
 **/
@interface INAppStoreWindow : NSWindow {
    float _titleBarHeight;
    NSView *_titleBarView;
	int _trafficLightAlignment;
	
	NSString* _titleBarString;
}
/** The height of the title bar. By default, this is set to the standard title bar height. **/
@property (nonatomic, assign) float titleBarHeight;
/** The title bar view itself. Add subviews to this view that you want to show in the title bar (e.g. buttons, a toolbar, etc.). This view can also be set if you want to use a different styled title bar aside from the default one (textured, etc.). **/
@property (nonatomic, retain) NSView *titleBarView;

/**	TitlebarString aka Window title ;) **/
@property (nonatomic, copy) NSString* titleBarString;

/** Position of traffic lights (0=top, 1=middle, 2=bottom) **/
@property (nonatomic, assign) int trafficLightAlignment;
@end
