// Copyright (c) 2019, OpenEmu Team
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
import OpenEmuBase

@objc
public class OEShadersModel : NSObject {
    enum Preferences {
        case global
        case system(String)
        
        var key: String {
            get {
                switch self {
                case .global:
                    return "videoShader"
                case .system(let identifier):
                    return "videoShader.\(identifier)"
                }
            }
        }
    }
    
    @objc(OEShaderModel)
    @objcMembers
    public class OEShaderModel : NSObject {
        enum Params {
            case global(String)
            case system(String, String)
            
            var key: String {
                get {
                    switch self {
                    case .global(let shader):
                        return "videoShader.\(shader).params"
                    case .system(let shader, let identifier):
                        return "videoShader.\(identifier).\(shader).params"
                    }
                }
            }
        }
        
        public var name: String
        public var path: String
        
        init(path: String) {
            self.name = ((path as NSString).lastPathComponent as NSString).deletingPathExtension
            self.path = path
        }
        
        @objc
        public func parameters(forIdentifier identifier: String) -> [String: Double]? {
            if let state = UserDefaults.oe_application().string(forKey: Params.system(self.name, identifier).key) {
                var res = [String:Double]()
                for param in state.split(separator: ";") {
                    let vals = param.split(separator: "=")
                    if let d = Double(vals[1]) {
                        res[String(vals[0])] = d
                    }
                }
                return res
            }
            
            return nil
        }
    }
    
    @objc
    public static var shared : OEShadersModel = {
        return OEShadersModel()
    }()
    
    @objc
    public lazy var shaders: [OEShaderModel] = {
        var shaders = Bundle.main.paths(forResourcesOfType: "slangp", inDirectory: "Shaders").map(OEShaderModel.init(path:))
        
        let openEmuSearchPath = ("OpenEmu" as NSString).appendingPathComponent("Shaders")
        let paths = NSSearchPathForDirectoriesInDomains(.applicationSupportDirectory, .userDomainMask, true)
        
        for path in paths {
            let subpath  = (path as NSString).appendingPathComponent(openEmuSearchPath)
            guard let subpaths = try? FileManager.default.contentsOfDirectory(atPath: subpath) else {
                continue
            }
            
            let subpath2 = subpath as NSString
            let models = subpaths .filter { ($0 as NSString).pathExtension == "slangp" }.map {
                return OEShaderModel(path: subpath2.appendingPathComponent($0))
            }
            shaders.append(contentsOf: models)
        }
        
        return shaders
    }()
    
    @objc
    public var shaderNames: [String] {
        return shaders.map { $0.name }
    }
    
    @objc
    public var defaultShader: OEShaderModel {
        get {
            if let name = UserDefaults.oe_application().string(forKey: Preferences.global.key),
                let shader = self[name] {
                return shader
            }
            
            return self["Pixellate"]!
        }
        
        set {
            UserDefaults.standard.set(newValue.name, forKey: Preferences.global.key)
        }
    }
    
    @objc
    public func shader(forSystem identifier: String) -> OEShaderModel? {
        guard let name = UserDefaults.oe_application().string(forKey: Preferences.system(identifier).key) else {
            return defaultShader
        }
        guard let shader = self[name] else {
            return nil
        }
        return shader
    }
    
    @objc
    public func shader(forURL url: URL) -> OEShaderModel? {
        return OEShaderModel(path: url.path)
    }
    
    subscript(name: String) -> OEShaderModel? {
        return shaders.first(where: { $0.name == name })
    }
}
