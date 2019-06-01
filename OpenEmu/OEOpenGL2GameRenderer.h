//
//  OEOpenGL2DGameRenderer.h
//  OpenEmu
//
//  Created by Alexander Strange on 10/18/15.
//
//

#import "OEGameRenderer.h"

@class OECoreVideoTexture;

/*!
 * @class OEOpenGL2GameRenderer
 * @discussion
 * Renderer which provides an OpenGL 2.1/Compatibility context to 3D game cores.
 */
@interface OEOpenGL2GameRenderer : NSObject <OEGameRenderer>

@property (nonatomic) OEIntSize surfaceSize;

- (nonnull instancetype)initWithInteropTexture:(OECoreVideoTexture * _Nonnull )texture;

@end
