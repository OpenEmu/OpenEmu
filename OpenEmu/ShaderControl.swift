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

extension Notification.Name {
    // The notification raised whenenever the preset changes for a ShaderControl object.
    public static let shaderControlPresetDidChange = NSNotification.Name("shaderControlPresetDidChange")
}

/// An object that mediates access to shaders and presets in OpenEmuKit and provides
/// logic specific to OpenEmu.
///
/// Specific logic includes providing a transition from ``OESystemShaderModel`` to ``ShaderPreset``.
public class ShaderControl: NSObject {
    
    enum Errors: Error, LocalizedError {
        case deleteDefault
        case deleteCurrent
        
        var errorDescription: String? {
            switch self {
            case .deleteDefault:
                return NSLocalizedString("Unable to delete default shader preset", comment: "error: Occurs when attempting to delete the default shader preset")
            case .deleteCurrent:
                return NSLocalizedString("Unable to delete the active shader", comment: "error: Occurs when attempting to delete the currently selected shader")
            }
        }
    }
    
    private(set) weak var document: OEGameDocument?
    public let systemPlugin: OESystemPlugin
    public var systemIdentifier: String { systemPlugin.systemIdentifier }
    @objc dynamic public private(set) var preset: ShaderPreset
    @objc dynamic public private(set) var presets: [ShaderPreset]
    
    private var systemShader: OESystemShaderModel? { preset.systemShader }
    var helper: OEGameCoreHelper? { document?.gameCoreHelper }
    
    init(document: OEGameDocument) {
        self.document = document
        self.systemPlugin = document.systemPlugin
        self.preset         = Self.currentPreset(forSystemPlugin: systemPlugin)
        self.presets        = Self.shaderPresets(byShader: preset.shader.name, systemPlugin: systemPlugin)
    }
    
    /// Returns the preset that is assigned to the specified system plugin.
    /// - Parameter systemPlugin: The ``OESystemPlugin`` to locate the preset for.
    /// - Returns: The shader preset assigned to the system plugin.
    public static func currentPreset(forSystemPlugin systemPlugin: OESystemPlugin) -> ShaderPreset {
        if let preset = SystemShaderPresetStore.shared.findPresetForSystem(systemPlugin.systemIdentifier) {
            return preset
        }
        let shader = OESystemShaderStore.shared.shader(forSystem: systemPlugin.systemIdentifier)
        return presetForSystemShader(shader, systemPlugin)
    }
    
    /// Returns an array of shader presets for the specified shader.
    static func shaderPresets(byShader name: String, systemPlugin: OESystemPlugin) -> [ShaderPreset] {
        guard let shader = OEShaderStore.shared.shader(withName: name) else { return [] }
        let systemShader = OESystemShaderStore.shared.shader(withShader: shader, forSystem: systemPlugin.systemIdentifier)
        var presets = ShaderPresetStore.shared.findPresets(byShader: name)
        presets.append(Self.presetForSystemShader(systemShader, systemPlugin)) // add default system shader
        
        return presets
    }
    
    /// Updates the parameters for the current ``preset``.
    ///
    /// - Parameter params: An array of replacement parameter values.
    public func writeParameters(_ params: [ShaderParamValue]) {
        preset.parameters = Dictionary(changedParams: params)
        if preset.isDefault {
            systemShader?.write(parameters: params)
        } else {
            try? ShaderPresetStore.shared.savePreset(preset)
        }
    }
    
    public func savePreset(_ preset: ShaderPreset) throws {
        try ShaderPresetStore.shared.savePreset(preset)
        if self.preset.shader.name == preset.shader.name {
            presets = Self.shaderPresets(byShader: preset.shader.name, systemPlugin: systemPlugin)
        }
    }
    
    public func deletePreset(_ preset: ShaderPreset) throws {
        guard !preset.isDefault else { throw Errors.deleteDefault }
        guard self.preset.id != preset.id else { throw Errors.deleteCurrent }
        
        ShaderPresetStore.shared.removePreset(preset)
        
        var presets = presets
        if let i = presets.firstIndex(of: preset) {
            presets.remove(at: i)
            self.presets = presets
        }
    }
    
    public func setValue(_ value: CGFloat, forParameter name: String) {
        helper?.setShaderParameterValue(value, forKey: name)
    }
    
    /// Change to the specified shader for the current core.
    /// - Parameter shader: The shader to use for the current core.
    public func changeShader(_ shader: OEShaderModel) {
        let systemShader = OESystemShaderStore.shared.shader(withShader: shader, forSystem: systemIdentifier)
        let preset = Self.presetForSystemShader(systemShader, systemPlugin)
        changePreset(preset)
    }
    
    @available(macOS 10.15, *)
    public func changePreset(_ preset: ShaderPreset) async throws {
        try await withCheckedThrowingContinuation { (cont: CheckedContinuation<Void, Error>) in
            changePreset(preset) { error in
                if let error = error {
                    cont.resume(throwing: error)
                } else {
                    cont.resume()
                }
            }
        }
    }
    
    public func changePreset(_ preset: ShaderPreset, completionHandler handler: ((Error?) -> Void)? = nil) {
        let params = preset.parameters as [String: NSNumber]?
        let shader = preset.shader
        
        helper?.setShaderURL(shader.url, parameters: params) { error in
            if let error = error {
                handler?(error)
                return
            }
            
            let changedShader = preset.shader.name != self.preset.shader.name
            
            self.preset = preset
            
            // Alternates between
            if preset.isDefault {
                OESystemShaderStore.shared.setShader(shader, forSystem: self.systemIdentifier)
                SystemShaderPresetStore.shared.resetPresetForSystem(self.systemIdentifier)
            } else {
                SystemShaderPresetStore.shared.setPreset(preset, forSystem: self.systemIdentifier)
                OESystemShaderStore.shared.resetShader(forSystem: self.systemIdentifier)
            }
            
            if changedShader {
                self.presets = Self.shaderPresets(byShader: preset.shader.name, systemPlugin: self.systemPlugin)
            }
            
            handler?(nil)
            NotificationCenter.default.post(name: .shaderControlPresetDidChange, object: self)
        }
    }
    
    static func presetForSystemShader(_ systemShader: OESystemShaderModel, _ systemPlugin: OESystemPlugin) -> ShaderPreset {
        ShaderPreset(name: .localizedStringWithFormat(NSLocalizedString("Default %@ preset for %@", comment: "1st parameter: shader name; 2nd parameter: system name"), systemShader.shader.name, systemPlugin.systemName),
                     systemShader: systemShader)
    }
}

extension ShaderPreset {
    static let defaultID = "default-shader"
    static let associationKey = UnsafeRawPointer(UnsafeMutablePointer<UInt64>.allocate(capacity: 1))
    
    var isDefault: Bool { id == Self.defaultID }
    
    var systemShader: OESystemShaderModel? {
        get {
            objc_getAssociatedObject(self, Self.associationKey) as? OESystemShaderModel
        }
        set {
            objc_setAssociatedObject(self, Self.associationKey, newValue, .OBJC_ASSOCIATION_RETAIN_NONATOMIC)
        }
    }
    
    convenience init(name: String, systemShader: OESystemShaderModel) {
        self.init(name: name,
                  shader: systemShader.shader,
                  parameters: systemShader.parameters,
                  id: Self.defaultID)
        self.systemShader = systemShader
    }
}
