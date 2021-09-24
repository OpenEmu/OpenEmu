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

import Foundation
import OpenEmuKit

struct ShaderPreset: Hashable, Identifiable {
    public let id: UUID
    public let name: String
    public let shader: String
    public var parameters: [String: Double]
    
//    static func makeFrom(params: [ShaderParamValue]) -> ShaderPreset {
//        ShaderPreset(
//            id: UUID(),
//            shader: nil,
//            parameters: Dictionary(uniqueKeysWithValues: params.compactMap { pv in
//                pv.isInitial ? nil : (pv.name, pv.value.doubleValue)
//            })
//        )
//    }
    
    static func makeFrom(shader: String, params: [ShaderParamValue]) -> ShaderPreset {
        ShaderPreset(
            id: UUID(),
            name: "Unnamed",
            shader: shader,
            parameters: Dictionary(uniqueKeysWithValues: params.compactMap { pv in
                pv.isInitial ? nil : (pv.name, pv.value.doubleValue)
            })
        )
    }
}

struct ShaderPresetTextWriter {
    struct Options: OptionSet {
        let rawValue: Int
        
        static let shader   = Options(rawValue: 1 << 0)
        static let hash     = Options(rawValue: 1 << 1)
        
        static let all: Options = [.shader, .hash]
    }
    
    func write(preset c: ShaderPreset, options: Options = .all) -> String {
        var s = ""
        if options.contains(.shader) {
            s.append("\"\(c.shader)\":")
        }
        
        // Sort the keys for a consistent output
        var first = true
        for key in c.parameters.keys.sorted() {
            if !first {
                s.append(";")
            }
            s.append("\(key)=\(c.parameters[key]!)")
            first = false
        }
        return s
    }
}

enum ShaderPresetReadError: Swift.Error {
    
}

/// Coding Errors
public enum ShaderPresetCodingError: Error {
    /// type or statement is not supported
    case unsupported
    /// required key is missing
    case missing
    case state
    /// statement is malformed
    case malformed
}

struct ShaderPresetTextReader {
    enum State {
        case key, value
    }
    
    func read(line: String) throws -> ShaderPreset {
        var header  = [String]()
        var params  = [String: Double]()
        
        var iter = line.makeIterator()
    outer:
        while let ch = iter.next() {
            
            switch ch {
            case "\"":
                // quoted string
                var s = ""
                while let ch = iter.next() {
                    if ch == "\"" {
                        header.append(s)
                        continue outer
                    }
                    s.append(ch)
                }
                throw ShaderPresetCodingError.malformed
            case ":":
                // parameters section
                var state: State = .key
                var key = ""
                var current = ""
                while let ch = iter.next() {
                    if ch == "=" {
                        if state == .key {
                            key     = current
                            current = ""
                            state   = .value
                            continue
                        }
                        throw ShaderPresetCodingError.malformed
                    }
                    
                    if ch == ";" {
                        if state == .value {
                            state = .key
                            params[key] = Double(current)
                            key = ""
                            current = ""
                            continue
                        }
                        throw ShaderPresetCodingError.malformed
                    }
                    
                    current.append(ch)
                }
                
                if state == .value {
                    params[key] = Double(current)
                } else {
                    throw ShaderPresetCodingError.malformed
                }
            default:
                throw ShaderPresetCodingError.malformed
            }
        }
        
        let shader = header.count > 0 ? header[0] : ""
        
        return ShaderPreset(id: UUID(), name: "Unnamed", shader: shader, parameters: params)
    }
}
