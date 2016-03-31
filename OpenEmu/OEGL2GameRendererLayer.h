//
//  OEGL2GameRendererLayer.h
//  OpenEmu
//
//  Created by Alexander Strange on 5/15/16.
//
//

#import "OEGameRenderer.h"
#import <QuartzCore/QuartzCore.h>

/*!
 * @class OEGL2GameRendererLayer
 * @abstract It's a CAOpenGLLayer, but it's also an OEGameRenderer.
 * @description Does not update the IOSurface.
 */
@interface OEGL2GameRendererLayer : CAOpenGLLayer

@end
