//
//  OEOpenGL3GameRenderer.h
//  OpenEmu
//
//  Created by Alexander Strange on 10/21/15.
//
//

#import "OEGameRenderer.h"

/*!
 * @class OEOpenGL3GameRenderer
 * @discussion
 * Renderer which provides an OpenGL 3.x/Core Profile context to 3D game cores.
 * Doesn't support 2D games unless someone can think of why it should.
 */
@interface OEOpenGL3GameRenderer : NSObject <OEGameRenderer>

@end
