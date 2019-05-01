//
//  OEGameHelperLayer.m
//  OpenEmu
//
//  Created by Alexander Strange on 6/18/16.
//
//

@import OpenGL;

#import <OpenGL/gl.h>
#import "OEGameHelperOpenGLLayer.h"
#import "NSColor+OEAdditions.h"

static NSString *const OEGameViewBackgroundColorKey = @"gameViewBackgroundColor";

/*
 * OE game rendering from game texture to drawable.
 * Experimental version to be run in core process.
 */

/*
 * Inputs:
 * Outputs:
 * Features:
 * Todos:
 - Options for nearest and linear scale
 - NotificationRenderer? It can just be a subview though.
 - Syphon
 - Shaders (which GLSL ver?)
 - Blargg's NTSC software shader
 - Make a brightness/contrast/gamma controls shader
 - Gamma-correction during game pixels loading.
 (Can't use GL_SRGB 'cause slow with 15-bit RGB e.g. SNES9X)
 - Screenshots
 - Forwarding mouse events
 - The window should stick to the game's aspect ratio when resizing.
 - The layer should not change bounds during live resize.
 - Rewrite with Metal - https://github.com/aras-p/glsl-optimizer
 */

/*
 * Take the raw visible game rect and turn it into a smaller rect
 * which is centered inside 'bounds' and has aspect ratio 'aspectSize'.
 * ATM we try to fill the window, but maybe someday we'll support fixed zooms.
 */
static NSRect FitAspectRectIntoBounds(OEIntSize aspectSize, NSRect bounds)
{
    CGFloat wantAspect = aspectSize.width  / (CGFloat)aspectSize.height;
    CGFloat viewAspect = bounds.size.width / (CGFloat)bounds.size.height;

    CGFloat minFactor;
    NSRect outRect;

    if (viewAspect >= wantAspect) {
        // Raw image is too wide (normal case), squish inwards
        minFactor = wantAspect / viewAspect;

        outRect.size.height = bounds.size.height;
        outRect.size.width  = bounds.size.width * minFactor;
    } else {
        // Raw image is too tall, squish upwards
        minFactor = viewAspect / wantAspect;

        outRect.size.height = bounds.size.height * minFactor;
        outRect.size.width  = bounds.size.width;
    }

    outRect.origin.x = bounds.origin.x + (bounds.size.width  - outRect.size.width)/2;
    outRect.origin.y = bounds.origin.y + (bounds.size.height - outRect.size.height)/2;

    // This is going into a Nearest Neighbor, so the edges should be on pixels!
    outRect = NSIntegralRectWithOptions(outRect, NSAlignAllEdgesNearest);

    return outRect;
}

// -- state class for atomic params changing
@interface OEGameLayerGLState : NSObject
{
@public
    IOSurfaceRef ioSurface;

    OEIntSize    surfaceSize;
    GLuint       ioSurfaceTex;

    GLuint       quadVBO;
    GLuint       quadVAO;
}
@end

@implementation OEGameLayerGLState

- (void)dealloc {
    glDeleteTextures(1, &ioSurfaceTex);
    glDeleteBuffers(1, &quadVBO);
    glDeleteVertexArraysAPPLE(1, &quadVAO);
}

@end

// -- CALayer class

@interface OEGameHelperOpenGLLayer ()
@property (atomic) OEGameLayerGLState *state;
@end

@implementation OEGameHelperOpenGLLayer
{
    CGLContextObj _alternateCglCtx;
    BOOL _didChangeOutputSize;

    GLfloat _clearColors[4];
}

- (instancetype)init
{
    self = [super init];

    /*
     * Use the "HDTV" colorspace. This is right for TV (probably) and better
     * than nothing for e.g. Gameboy on DCI-P3 screens. But "no correction" might be
     * what people are used to.
     */
    self.colorspace      = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGBLinear);
    self.anchorPoint     = CGPointMake(0,0);
    self.contentsGravity = kCAGravityResizeAspect;

    NSString *backgroundColorName = [[NSUserDefaults standardUserDefaults] objectForKey:OEGameViewBackgroundColorKey];
    if(backgroundColorName != nil)
    {
        NSColor *color = [NSColor colorFromString:backgroundColorName];
        self.backgroundColor = color;
    }

    //    _filter.linearFilter = YES;

    return self;
}

- (void)dealloc
{
    if (_alternateCglCtx)
        CGLDestroyContext(_alternateCglCtx);
}

#pragma mark Properties

- (CALayer *)layer
{
    return self;
}

- (void)setInput:(struct OEGameLayerInputParams)input
{
    if (memcmp(&input, &_input, sizeof(input)) == 0) return;

    _input = input;
    [self reconfigure];
}

- (void)setFilter:(struct OEGameLayerFilterParams)filter
{
    if (memcmp(&filter, &_filter, sizeof(filter)) == 0) return;

    _filter = filter;
    [self reconfigure];
}

- (void)setBounds:(CGRect)bounds
{
    [super setBounds:bounds];
    [self reconfigure];

    _didChangeOutputSize = YES;
}

- (void)setBackgroundColor:(NSColor *)backgroundColor
{
    // Note this doesn't use CALayer colors.
    NSColorSpace *linearSpace = [[NSColorSpace alloc] initWithCGColorSpace:CGColorSpaceCreateWithName(kCGColorSpaceGenericRGBLinear)];
    NSColor *linearColor = [backgroundColor colorUsingColorSpace:linearSpace];

    _clearColors[0] = linearColor.redComponent;
    _clearColors[1] = linearColor.greenComponent;
    _clearColors[2] = linearColor.blueComponent;
    _clearColors[3] = 1.0;
}

- (BOOL)isOpaque
{
    return YES;
}

#pragma mark Methods

- (void)reconfigure
{
    // Recalculate everything.
    // It's not worth splitting this up, since any kind of resizing
    // will probably make us rebuild the whole shader setup.

    if (_alternateCglCtx == nil) return;
    if (self.input.ioSurfaceRef == nil) return;

    // Use an alternate context so this can be moved to another thread
    // if shader compiles take time. Note, texcoords need to be updated instantly.
    CGLSetCurrentContext(_alternateCglCtx);
    OEGameLayerGLState *state = [OEGameLayerGLState new];

    // Lookup the IOSurface.
    state->ioSurface = _input.ioSurfaceRef;
    NSParameterAssert(state->ioSurface);

    // Prepare IOSurface texture.
    {
        glEnable(GL_TEXTURE_RECTANGLE_ARB);
        glGenTextures(1, &state->ioSurfaceTex);
        glBindTexture(GL_TEXTURE_RECTANGLE_ARB, state->ioSurfaceTex);

        // Minification filter is always smooth.
        // We need this for "1x" size, since SNES might go to a highres mode and then we're actually scaling down...
        glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, _filter.linearFilter ? GL_LINEAR : GL_NEAREST);

        glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        state->surfaceSize = (OEIntSize){.width = (int)IOSurfaceGetWidth(state->ioSurface), .height = (int)IOSurfaceGetHeight(state->ioSurface)};

        CGLError err = CGLTexImageIOSurface2D(_alternateCglCtx, GL_TEXTURE_RECTANGLE_ARB, GL_SRGB8, state->surfaceSize.width, state->surfaceSize.height, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, state->ioSurface, 0);
        NSParameterAssert(err == kCGLNoError);
    }

    // Prepare filters.

    // Create VAO to hold game rect (GL2 doesn't need this but GL3 will)
    // Calculate input and output coordinates.
    {
        // Create VAO to store the two VBOs.
        glGenVertexArraysAPPLE(1, &state->quadVAO);
        glBindVertexArrayAPPLE(state->quadVAO);

        OEIntRect gameRect = OEIntRectMake(0, 0, _input.screenSize.width, _input.screenSize.height);
        NSRect bounds = self.bounds;

        NSRect vertsRect = FitAspectRectIntoBounds(_input.aspectSize, bounds);

        GLint iX1 = gameRect.origin.x, iX2 = iX1 + gameRect.size.width;
        GLint iY1 = gameRect.origin.y, iY2 = iY1 + gameRect.size.height;

        GLfloat oX1 = vertsRect.origin.x, oX2 = oX1 + vertsRect.size.width;
        GLfloat oY1 = vertsRect.origin.y, oY2 = oY1 + vertsRect.size.height;

        struct data {
            GLint outVerts[8];
            GLint inCoords[8];
        } __attribute__((packed)) data;

        data = (struct data){
                .outVerts ={
                        // x  y
                        oX1, oY1, // 1
                        oX2, oY1, // 2
                        oX2, oY2, // 3
                        oX1, oY2, // 4
                },
                .inCoords = {
                        // u  v
                        iX1, iY1, // 1
                        iX2, iY1, // 2
                        iX2, iY2, // 3
                        iX1, iY2, // 4
                }
        };

        // Create Vertex Buffer for both quad vertices and texcoords.
        glGenBuffers(1, &state->quadVBO);

        glBindBuffer(GL_ARRAY_BUFFER, state->quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(data), &data, GL_STATIC_DRAW);

        // This part isn't GL3 anymore!
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(2, GL_INT, 0, (GLvoid*)__offsetof(struct data, outVerts));

        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(2, GL_INT, 0, (GLvoid*)__offsetof(struct data, inCoords));
    }

    // Update.
    self.state = state;
}

#pragma mark Overrides

// TODO pixel formats: Try and draw deeper than 8-bit

-(CGLContextObj)copyCGLContextForPixelFormat:(CGLPixelFormatObj)pixelFormat
{
    CGLContextObj ret = [super copyCGLContextForPixelFormat:pixelFormat];
    CGLContextObj alt;

    // Create a share context for recompiling stuff async.
    CGLCreateContext(pixelFormat, ret, &alt);

    if (_alternateCglCtx)
    {
        CGLDestroyContext(_alternateCglCtx);
    }
    _alternateCglCtx = alt;

    [self reconfigure];
    return ret;
}

-(BOOL)canDrawInCGLContext:(CGLContextObj)ctx pixelFormat:(CGLPixelFormatObj)pf forLayerTime:(CFTimeInterval)t displayTime:(const CVTimeStamp *)ts
{
    return YES;
}

-(void)drawInCGLContext:(CGLContextObj)glContext
            pixelFormat:(CGLPixelFormatObj)pixelFormat
           forLayerTime:(CFTimeInterval)timeInterval
            displayTime:(const CVTimeStamp *)timeStamp
{
    // Everything referenced in this method must be in 'state' to keep it consistent.

    OEGameLayerGLState *state = self.state;

    glClearColor(_clearColors[0], _clearColors[1], _clearColors[2], _clearColors[3]);
    glClear(GL_COLOR_BUFFER_BIT);

    if (state == nil) {
        return;
    }

    if (_didChangeOutputSize) {
        // This state change has to go in here because we don't have the right gl context otherwise.
        NSRect bounds = self.bounds;

        // By default OpenGLLayer has [-1,-1] to [1,1] coordinates.
        // We want to use actual pixel positions.
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(bounds.origin.x, bounds.origin.x + bounds.size.width,
                bounds.origin.y, bounds.origin.y + bounds.size.height,
                0, 1);
        _didChangeOutputSize = NO;
    }

    glEnable(GL_TEXTURE_RECTANGLE_ARB);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glColor4f(1.0, 1.0, 1.0, 1.0);
    glEnable(GL_FRAMEBUFFER_SRGB); // TODO: Turn off for 3D games?

    glBindVertexArrayAPPLE(state->quadVAO);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, state->ioSurfaceTex);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

@end
