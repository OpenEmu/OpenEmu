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

/// An object that mediates access to shaders and presets in OpenEmuKit and provides
/// logic specific to OpenEmu.
///
/// Specific logic includes providing a transition from ``OESystemShaderModel`` to ``ShaderPreset``.
@objc public class ShaderControl: NSObject {
    
    @objc public let systemPlugin: OESystemPlugin
    @objc public var systemIdentifier: String { systemPlugin.systemIdentifier }
    @objc public dynamic var preset: ShaderPreset
    
    private var systemShader: OESystemShaderModel? { preset.systemShader }
    let helper: OEGameCoreHelper
    
    @objc public init(systemPlugin: OESystemPlugin, helper: OEGameCoreHelper) {
        self.systemPlugin   = systemPlugin
        self.helper         = helper
        self.preset         = Self.currentPreset(forSystemPlugin: systemPlugin)
    }
    
    public static func currentPreset(forSystemPlugin systemPlugin: OESystemPlugin) -> ShaderPreset {
        if let preset = SystemShaderPresetStore.shared.findPresetForSystem(systemPlugin.systemIdentifier) {
            return preset
        }
        let shader = OESystemShaderStore.shared.shader(forSystem: systemPlugin.systemIdentifier)
        return presetForSystemShader(shader, systemPlugin)
    }
    
    /// Returns an array of shader presets for the selected shader.
    public func shaderPresets(byShader name: String, sortedByName sort: Bool = false) -> [ShaderPreset] {
        guard let shader = OEShaderStore.shared.shader(withName: name) else { return [] }
        let systemShader = OESystemShaderStore.shared.shader(withShader: shader, forSystem: systemIdentifier)
        var presets = ShaderPresetStore.shared.findPresets(byShader: name)
        presets.append(Self.presetForSystemShader(systemShader, systemPlugin)) // add default system shader
        if sort {
            presets.sort { $0.name > $1.name }
        }
        return presets
    }
    
    /// Updates the parameters for the current preset.
    ///
    /// - Warning:
    /// This is a warning
    /// - Parameter params: An array of replacement parameters.
    public func writeParameters(_ params: [ShaderParamValue]) {
        if preset.isLegacy {
            systemShader?.write(parameters: params)
        } else {
            preset.parameters = Dictionary(changedParams: params)
            try? ShaderPresetStore.shared.savePreset(preset)
        }
    }
    
    public func setValue(_ value: CGFloat, forParameter name: String) {
        helper.setShaderParameterValue(value, forKey: name)
    }
    
    /// Change to the specified shader for the current core.
    /// - Parameter shader: The shader to use for the current core.
    public func changeShader(_ shader: OEShaderModel) {
        let systemShader = OESystemShaderStore.shared.shader(withShader: shader, forSystem: systemIdentifier)
        let preset = Self.presetForSystemShader(systemShader, systemPlugin)
        changePreset(preset)
    }
    
    public func changePreset(_ preset: ShaderPreset) {
        let params = preset.parameters as [String: NSNumber]?
        let shader = preset.shader
        
        helper.setShaderURL(shader.url, parameters: params) { error in
            if let error = error {
                let alert = NSAlert(error: error)
                alert.runModal()
                return
            }
            
            self.preset = preset
            
            // Alternates between
            if preset.isLegacy {
                OESystemShaderStore.shared.setShader(shader, forSystem: self.systemIdentifier)
                SystemShaderPresetStore.shared.resetPresetForSystem(self.systemIdentifier)
            } else {
                SystemShaderPresetStore.shared.setPreset(preset, forSystem: self.systemIdentifier)
                OESystemShaderStore.shared.resetShader(forSystem: self.systemIdentifier)
            }
        }
    }
    
    static func presetForSystemShader(_ systemShader: OESystemShaderModel, _ systemPlugin: OESystemPlugin) -> ShaderPreset {
        ShaderPreset(name: "Default \(systemShader.shader.name) preset for \(systemPlugin.name!)",
                     systemShader: systemShader)
    }
}

extension ShaderPreset {
    static let legacyID = "legacy-shader"
    static let associationKey = UnsafeRawPointer(UnsafeMutablePointer<UInt64>.allocate(capacity: 1))
    
    var isLegacy: Bool { id == Self.legacyID }
    
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
                  id: Self.legacyID)
        self.systemShader = systemShader
    }
}
