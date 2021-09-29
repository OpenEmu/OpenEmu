// Copyright (c) 2020, OpenEmu Team
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

final class PrefGameplayController: NSViewController {
    
    @IBOutlet var globalDefaultShaderSelection: NSPopUpButton!
    
    private var token: NSObjectProtocol?
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        loadShaderMenu()
        
        token = NotificationCenter.default.addObserver(forName: .shaderModelCustomShadersDidChange, object: nil, queue: .main) { [weak self] _ in
            guard let self = self else { return }
            
            self.loadShaderMenu()
        }
    }
    
    deinit {
        if token != nil {
            NotificationCenter.default.removeObserver(token!)
            token = nil
        }
    }
    
    private func loadShaderMenu() {
        
        let globalShaderMenu = NSMenu()
        
        let systemShaders = OEShadersModel.shared.sortedSystemShaderNames
        systemShaders.forEach { shaderName in
            globalShaderMenu.addItem(withTitle: shaderName, action: nil, keyEquivalent: "")
        }
        
        let customShaders = OEShadersModel.shared.sortedCustomShaderNames
        if !customShaders.isEmpty {
            globalShaderMenu.addItem(.separator())
            
            customShaders.forEach { shaderName in
                globalShaderMenu.addItem(withTitle: shaderName, action: nil, keyEquivalent: "")
            }
        }
        
        globalDefaultShaderSelection.menu = globalShaderMenu
        
        let selectedShaderName = OEShadersModel.shared.defaultShaderName
        
        if globalDefaultShaderSelection.item(withTitle: selectedShaderName) != nil {
            globalDefaultShaderSelection.selectItem(withTitle: selectedShaderName)
        } else {
            globalDefaultShaderSelection.selectItem(at: 0)
        }
    }
    
    @IBAction func changeGlobalDefaultShader(_ sender: Any?) {
        guard let context = OELibraryDatabase.default?.mainThreadContext else { return }
        
        guard let shaderName = globalDefaultShaderSelection.selectedItem?.title else { return }
        
        let allSystemIdentifiers = OEDBSystem.allSystemIdentifiers(in: context)
        allSystemIdentifiers.forEach(OEShadersModel.shared.resetShader(forSystem:))
        OEShadersModel.shared.defaultShaderName = shaderName
    }
}

// MARK: - PreferencePane

extension PrefGameplayController: PreferencePane {
    
    var icon: NSImage? { NSImage(named: "gameplay_tab_icon") }
    
    var panelTitle: String { "Gameplay" }
    
    var viewSize: NSSize { view.fittingSize }
}
