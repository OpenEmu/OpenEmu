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

extension OEGameCorePluginError: LocalizedError {
    
    public var failureReason: String? {
        switch self {
        case .alreadyLoaded:
            return NSLocalizedString("ERROR_PLUGIN_IMPORT_ALREADYLOADED", comment: "Error, another version of imported plugin is already loaded")
        case .outOfSupport:
            return NSLocalizedString("ERROR_PLUGIN_IMPORT_OUTOFSUPPORT", comment: "Error, plugin to be imported is out of support")
        case .invalid:
            return nil
        }
    }
    
    public var recoverySuggestion: String? {
        switch self {
        case .alreadyLoaded:
            return NSLocalizedString("You need to restart the application to commit the change", comment: "")
        case .outOfSupport,
             .invalid:
            return nil
        }
    }
}

final class PluginDocument: NSDocument {
    
    override func read(from url: URL, ofType typeName: String) throws {
        let pathExtension = url.pathExtension.lowercased()
        switch pathExtension {
        case "oeshaderplugin":
            ImportOperation.importShaderPlugin(at: url)
        case OECorePlugin.pluginExtension:
            try Self.importCorePlugin(at: url)
        case OESystemPlugin.pluginExtension:
            try Self.importSystemPlugin(at: url)
        default:
            return
        }
    }
    
    static func importSystemPlugin(at url: URL) throws {
        guard url.pathExtension.lowercased() == OESystemPlugin.pluginExtension else { return }
        // Only the built-in system plugins are supported.
        throw OEGameCorePluginError.outOfSupport
    }
    
    static func importCorePlugin(at url: URL) throws {
        guard url.pathExtension.lowercased() == OECorePlugin.pluginExtension else { return }
        
        let coresDir = URL.oeApplicationSupportDirectory
            .appendingPathComponent(OECorePlugin.pluginFolder, isDirectory: true)
        let newURL = coresDir.appendingPathComponent(url.lastPathComponent, isDirectory: true)
        
        // If the file isn’t already in the right place
        if newURL != url {
            let fm = FileManager.default
            
            if fm.fileExists(atPath: newURL.path) {
                try fm.trashItem(at: newURL, resultingItemURL: nil)
            }
            if !fm.fileExists(atPath: coresDir.path) {
                try fm.createDirectory(at: coresDir, withIntermediateDirectories: true)
            }
            try fm.copyItem(at: url, to: newURL)
            
            // Remove quarantine attribute to make Gatekeeper happy.
            // TODO: Remove once core plugins are code-signed?
            if try fm.hasExtendedAttribute("com.apple.quarantine", at: newURL, traverseLink: true) {
                try fm.removeExtendedAttribute("com.apple.quarantine", at: newURL, traverseLink: true)
            }
        }
        
        try _ = OECorePlugin.plugin(bundleAtURL: newURL, forceReload: true)
    }
}
