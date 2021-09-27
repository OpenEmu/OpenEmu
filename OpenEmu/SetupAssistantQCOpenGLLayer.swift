// Copyright (c) 2021, OpenEmu Team
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the OpenEmu Team nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY OpenEmu Team ''AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL OpenEmu Team BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import Cocoa
import Quartz

final class SetupAssistantQCOpenGLLayer: CAOpenGLLayer {
    
    private var renderer: QCRenderer?
    private var runningTime: TimeInterval = 0
    weak var containingWindow: NSWindow?
    
    override func copyCGLPixelFormat(forDisplayMask mask: UInt32) -> CGLPixelFormatObj {
        var pixelFormatObj: CGLPixelFormatObj?
        var numPixelFormats: GLint = 0
        
        var attributes: [CGLPixelFormatAttribute] = [
            kCGLPFADisplayMask, CGLPixelFormatAttribute(mask),
            kCGLPFAAccelerated,
            kCGLPFAColorSize, CGLPixelFormatAttribute(24),
            kCGLPFAAlphaSize, CGLPixelFormatAttribute(8),
            kCGLPFADepthSize, CGLPixelFormatAttribute(16),
            kCGLPFAAcceleratedCompute,
            kCGLPFANoRecovery,
            kCGLPFAMultisample,
            //kCGLPFASupersample,
            kCGLPFASampleAlpha,
            kCGLPFASamples, CGLPixelFormatAttribute(2),
            kCGLPFASampleBuffers, CGLPixelFormatAttribute(1),
            CGLPixelFormatAttribute(0),
        ]
        
        var err = CGLChoosePixelFormat(attributes, &pixelFormatObj, &numPixelFormats)
        
        if pixelFormatObj == nil {
            print("Could not create pixel format, \(err), falling back")
            attributes = [
                kCGLPFADisplayMask, CGLPixelFormatAttribute(mask),
                kCGLPFAAccelerated,
                kCGLPFAColorSize, CGLPixelFormatAttribute(24),
                kCGLPFAAlphaSize, CGLPixelFormatAttribute(8),
                kCGLPFADepthSize, CGLPixelFormatAttribute(16),
                kCGLPFAAcceleratedCompute,
                kCGLPFANoRecovery,
                CGLPixelFormatAttribute(0),
            ]
            
            err = CGLChoosePixelFormat(attributes, &pixelFormatObj, &numPixelFormats)
            
            if pixelFormatObj == nil {
                print("failure to make pixel format with OpenCL:, \(err)")
                attributes = [
                    kCGLPFADisplayMask, CGLPixelFormatAttribute(mask),
                    kCGLPFAAccelerated,
                    kCGLPFAColorSize, CGLPixelFormatAttribute(24),
                    kCGLPFAAlphaSize, CGLPixelFormatAttribute(8),
                    kCGLPFADepthSize, CGLPixelFormatAttribute(16),
                    kCGLPFANoRecovery,
                    CGLPixelFormatAttribute(0),
                ]
                
                err = CGLChoosePixelFormat(attributes, &pixelFormatObj, &numPixelFormats)
                
                if pixelFormatObj == nil {
                    print("failure to make pixel format:, \(err)")
                }
            }
        }
        
        return pixelFormatObj!
    }
    
    override func copyCGLContext(forPixelFormat pf: CGLPixelFormatObj) -> CGLContextObj {
        let cgl_ctx = super.copyCGLContext(forPixelFormat: pf)
        
        // init our renderer
        let comp = QCComposition(file: Bundle.main.path(forResource: "OE Startup", ofType: "qtz"))
        
        let cspace = CGColorSpace(name: CGColorSpace.sRGB)
        
        renderer = QCRenderer(cglContext: cgl_ctx, pixelFormat: pf, colorSpace: cspace, composition: comp)
        
        // setup times
        runningTime = 0
        
        return cgl_ctx
    }
    
    override func releaseCGLContext(_ ctx: CGLContextObj) {
        renderer = nil
        containingWindow = nil
        
        super.releaseCGLContext(ctx)
    }
    
    override func canDraw(inCGLContext ctx: CGLContextObj, pixelFormat pf: CGLPixelFormatObj, forLayerTime t: CFTimeInterval, displayTime ts: UnsafePointer<CVTimeStamp>?) -> Bool {
        renderer != nil ? true : false
    }
    
    override func draw(inCGLContext ctx: CGLContextObj, pixelFormat pf: CGLPixelFormatObj, forLayerTime t: CFTimeInterval, displayTime ts: UnsafePointer<CVTimeStamp>?) {
        var time = Date.timeIntervalSinceReferenceDate
        
        // compute our local time
        if runningTime == 0 {
            runningTime = time
            time = 0
        } else {
            time -= runningTime
        }
        
        var arguments: [AnyHashable : Any]?
        
        if let containingWindow = containingWindow {
            var mouseLocation = containingWindow.mouseLocationOutsideOfEventStream
            mouseLocation.x /= containingWindow.frame.size.width
            mouseLocation.y /= containingWindow.frame.size.height
            arguments = [QCRendererMouseLocationKey : NSValue(point: mouseLocation)]
        }
        
        glMatrixMode(GLenum(GL_PROJECTION))
        glLoadIdentity()
        
        glMatrixMode(GLenum(GL_MODELVIEW))
        glLoadIdentity()
        
        renderer?.render(atTime: time, arguments: arguments)
        
        super.draw(inCGLContext: ctx, pixelFormat: pf, forLayerTime: t, displayTime: ts)
    }
}
