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

extension OpenEmuTools.Shader {
    struct DumpMetalSource: ParsableCommand {
        static var configuration = CommandConfiguration(abstract: "Dump the transformed Metal source for a shader.")
        
        @Argument
        var shaderPath: String
        
        @Argument(help: "Specify the passes to dump. Omit to dump all passes.")
        var passes: [Int] = []
        
        func run() throws {
            let shaderURL = URL(fileURLWithPath: shaderPath).absoluteURL
            let shader: SlangShader
            do {
                shader = try SlangShader(fromURL: shaderURL)
            } catch {
                print("Failed to load shader: \(error.localizedDescription)")
                throw ExitCode.failure
            }
            
            let options = ShaderCompilerOptions()
            options.isCacheDisabled = true
            let compiler = OEShaderPassCompiler(shaderModel: shader)
            
            let p: AnySequence<Int>
            if passes.isEmpty {
                p = AnySequence(shader.passes.indices)
            } else {
                p = AnySequence(passes)
            }
            
            for pass in p {
                let (vert, frag) = try compiler.buildPass(pass, options: options, passSemantics: nil)
                let p = shader.passes[pass]
                
                print("\(p.url.path)")
                print(vert)
                print(frag)
            }
        }
    }
}
