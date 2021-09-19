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
import OpenEmuShaders
import OpenEmuKit
import CoreGraphics
import Metal

extension OpenEmuTools.Shader {
    struct Thumbnail: ParsableCommand {
        static var configuration = CommandConfiguration(
            abstract: "Generate thumbnail images of shaders.",
            discussion: """
This command generates a thumbnail image of a shader using a user-specified source image.
"""
        )
        
        @Argument
        var shaderPath: String
        
        @Option
        var imagePath: String
        
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
            
            let outRep = fi.captureOutputImage()
            let data = outRep.representation(using: .png, properties: [:])
            
            FileHandle.standardOutput.write(data!)
        }
    }
}
