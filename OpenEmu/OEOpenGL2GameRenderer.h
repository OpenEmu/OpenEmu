//
//  OEOpenGL2DGameRenderer.h
//  OpenEmu
//
//  Created by Alexander Strange on 10/18/15.
//
//

#import "OEGameRenderer.h"

/*!
 * @class OEOpenGL2GameRenderer
 * @discussion
 * Renderer which provides an OpenGL 2.1/Compatibility context to 3D game cores
 * and uses GL2.1 to blit 2D games' video buffers to the IOSurface.
 *
 * The second is generally only used to convert pixel formats, so any faster
 * alternative is welcome.
 */
@interface OEOpenGL2GameRenderer : NSObject <OEGameRenderer>

@end
