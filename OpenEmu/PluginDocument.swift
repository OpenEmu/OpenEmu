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
import OpenEmuKit

final class PluginDocument: NSDocument {
    
    override func read(from url: URL, ofType typeName: String) throws {
        let pathExtension = url.pathExtension.lowercased()
        if pathExtension == "oeshaderplugin" {
            ImportOperation.importShaderPlugin(at: url)
            return
        }
        else if pathExtension == OESystemPlugin.pluginExtension {
            let userInfo = [
                NSLocalizedFailureReasonErrorKey : NSLocalizedString("Only the built-in system plugins are supported.", comment: ""),
            ]
            throw NSError(domain: OEGameCorePluginError.errorDomain, code: -1002 /*OEGameCorePluginError.outOfSupport*/, userInfo: userInfo)
        }
        else if pathExtension == OECorePlugin.pluginExtension {
            let coresDir = FileManager.default.urls(for: .applicationSupportDirectory, in: .userDomainMask).first!
                .appendingPathComponent("OpenEmu", isDirectory: true)
                .appendingPathComponent(OECorePlugin.pluginFolder, isDirectory: true)
            let newURL = coresDir.appendingPathComponent(url.lastPathComponent, isDirectory: true)
            
            // If the file isn’t already in the right place
            if newURL != url {
                let fm = FileManager.default
                
                if fm.fileExists(atPath: newURL.path) {
                    try fm.removeItem(at: newURL)
                }
                if !fm.fileExists(atPath: coresDir.path) {
                    try fm.createDirectory(at: coresDir, withIntermediateDirectories: true)
                }
                try fm.copyItem(at: url, to: newURL)
            }
            
            do {
                try _ = OECorePlugin.plugin(bundleAtPath: newURL.path, forceReload: true)
            } catch let error as NSError {
                if error.domain == OEGameCorePluginError.errorDomain {
                    if error.code == OEGameCorePluginError.alreadyLoaded.errorCode {
                        let userInfo = [
                            NSLocalizedFailureReasonErrorKey : NSLocalizedString("A version of this plugin is already loaded", comment: ""),
                            NSLocalizedRecoverySuggestionErrorKey : NSLocalizedString("You need to restart the application to commit the change", comment: ""),
                        ]
                        throw NSError(domain: error.domain, code: error.code, userInfo: userInfo)
                    } else if error.code == OEGameCorePluginError.outOfSupport.errorCode {
                        let userInfo = [
                            NSLocalizedFailureReasonErrorKey : NSLocalizedString("This plugin is currently unsupported", comment: ""),
                        ]
                        throw NSError(domain: error.domain, code: error.code, userInfo: userInfo)
                    }
                }
                throw error
            }
        }
    }
}
