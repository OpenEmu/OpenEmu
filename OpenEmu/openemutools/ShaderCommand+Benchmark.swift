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

import ArgumentParser
import Metal
import OpenEmuShaders

extension OpenEmuTools.Shader {
    struct Benchmark: ParsableCommand {
        static var configuration = CommandConfiguration(
            abstract: "Benchmark a shader."
        )
        
        @Option
        var shaderPath: String
        
        @Option
        var imagePath: String
        
        @Option
        var count: Int = 50
        
        @Option
        var outputScale: Int = 3
        
        func run() throws {
            guard
                let dev = MTLCreateSystemDefaultDevice()
            else {
                print("error: unable to create default Metal device")
                throw ExitCode.failure
            }
            
            let options = ShaderCompilerOptions.makeOptions()
            let fi = OEFilterChain(device: dev)
            try fi.setShaderFrom(URL(fileURLWithPath: shaderPath), options: options)
            
            guard let ctx = CGContext.make(URL(fileURLWithPath: imagePath))
            else {
                print("unable to load image \(imagePath)")
                throw ExitCode.failure
            }
            
            let imgSize = CGSize(width: ctx.width, height: ctx.height)
            fi.setSourceRect(CGRect(x: 0, y: 0, width: ctx.width, height: ctx.height), aspect: imgSize)
            fi.drawableSize = imgSize.applying(.init(scaleX: CGFloat(outputScale), y: CGFloat(outputScale)))
            
            let buf = fi.newBuffer(with: .bgra8Unorm, height: UInt(ctx.height), bytesPerRow: UInt(ctx.bytesPerRow))
            buf.contents.copyMemory(from: ctx.data!, byteCount: ctx.height * ctx.bytesPerRow)
            
            let td = MTLTextureDescriptor.texture2DDescriptor(pixelFormat: .bgra8Unorm, width: Int(fi.drawableSize.width), height: Int(fi.drawableSize.height), mipmapped: false)
            td.storageMode = .private
            td.usage = .renderTarget
            
            let tex = dev.makeTexture(descriptor: td)
            
            let rpd = MTLRenderPassDescriptor()
            rpd.colorAttachments[0].clearColor = .init(red: 0, green: 0, blue: 0, alpha: 1)
            rpd.colorAttachments[0].loadAction = .clear
            rpd.colorAttachments[0].texture = tex
            
            let cq = dev.makeCommandQueue()!
            
            // warm up
            do {
                let cb = cq.makeCommandBuffer()!
                fi.render(with: cb, renderPassDescriptor: rpd)
                cb.commit()
                cb.waitUntilCompleted()
            }
            
            let scope = MTLCaptureManager.shared().makeCaptureScope(commandQueue: cq)
            scope.label = "Frames"
            MTLCaptureManager.shared().defaultCaptureScope = scope
            
            let start = CACurrentMediaTime()
            for _ in 0..<count {
                scope.begin()
                let cb = cq.makeCommandBuffer()!
                fi.render(with: cb, renderPassDescriptor: rpd)
                cb.commit()
                cb.waitUntilCompleted()
                scope.end()
            }
            
            let end = CACurrentMediaTime() - start
            print("Elapsed time for \(count, color: .white) frames was \(end) s")
            print("\tFPS: \(Double(count) / end, color: .white, style: [.bold])")
            
            let perFrameMs = (end / Double(count)) * 1000
            print("\tTime per Frame: \(perFrameMs, color: .white, style: [.bold]) ms")
        }
    }
}
