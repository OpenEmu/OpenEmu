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
    @IBOutlet var volumeSlider: NSSlider!
    @IBOutlet var volumeValueLabel: NSTextField!
    @IBOutlet var gameplayRateSlider: NSSlider!
    @IBOutlet var gameplayRateValueLabel: NSTextField!
    @IBOutlet var fastForwardRateSlider: NSSlider!
    @IBOutlet var fastForwardRateValueLabel: NSTextField!
    
    private var token: NSObjectProtocol?
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        loadShaderMenu()
        loadPlaybackSettings()
        
        token = NotificationCenter.default.addObserver(forName: .shaderModelCustomShadersDidChange, object: nil, queue: .main) { [weak self] _ in
            guard let self = self else { return }
            
            self.loadShaderMenu()
        }
    }
    
    deinit {
        if let token = token {
            NotificationCenter.default.removeObserver(token)
            self.token = nil
        }
    }
    
    private func loadShaderMenu() {
        
        let globalShaderMenu = NSMenu()
        
        let systemShaders = OEShaderStore.shared.sortedSystemShaderNames
        systemShaders.forEach { shaderName in
            globalShaderMenu.addItem(withTitle: shaderName, action: nil, keyEquivalent: "")
        }
        
        let customShaders = OEShaderStore.shared.sortedCustomShaderNames
        if !customShaders.isEmpty {
            globalShaderMenu.addItem(.separator())
            
            customShaders.forEach { shaderName in
                globalShaderMenu.addItem(withTitle: shaderName, action: nil, keyEquivalent: "")
            }
        }
        
        globalDefaultShaderSelection.menu = globalShaderMenu
        
        let selectedShaderName = OEShaderStore.shared.defaultShaderName
        
        if globalDefaultShaderSelection.item(withTitle: selectedShaderName) != nil {
            globalDefaultShaderSelection.selectItem(withTitle: selectedShaderName)
        } else {
            globalDefaultShaderSelection.selectItem(at: 0)
        }
    }

    private func loadPlaybackSettings() {
        let defaults = UserDefaults.standard
        volumeSlider.floatValue = defaults.float(forKey: OEGameVolumeKey)
        gameplayRateSlider.floatValue = defaults.float(forKey: OEGameDefaultRateKey)
        fastForwardRateSlider.floatValue = defaults.float(forKey: OEGameFastForwardRateKey)
        updatePlaybackValueLabels()
    }

    private func updatePlaybackValueLabels() {
        volumeValueLabel.stringValue = volumeSlider.formattedPercentage
        gameplayRateValueLabel.stringValue = gameplayRateSlider.formattedMultiplier
        fastForwardRateValueLabel.stringValue = fastForwardRateSlider.formattedMultiplier
    }

    private var gameDocuments: [OEGameDocument] {
        NSDocumentController.shared.documents.compactMap { $0 as? OEGameDocument }
    }

    @IBAction func changeVolume(_ sender: NSSlider) {
        UserDefaults.standard.set(sender.floatValue, forKey: OEGameVolumeKey)
        volumeValueLabel.stringValue = sender.formattedPercentage
        for document in gameDocuments {
            document.applyDefaultVolume()
        }
    }

    @IBAction func changeGameplayRate(_ sender: NSSlider) {
        sender.floatValue = (sender.floatValue * 5).rounded() / 5
        UserDefaults.standard.set(sender.floatValue, forKey: OEGameDefaultRateKey)
        gameplayRateValueLabel.stringValue = sender.formattedMultiplier
        for document in gameDocuments {
            document.applyGameplayRates()
        }
    }

    @IBAction func changeFastForwardRate(_ sender: NSSlider) {
        sender.floatValue = sender.floatValue.rounded()
        UserDefaults.standard.set(sender.floatValue, forKey: OEGameFastForwardRateKey)
        fastForwardRateValueLabel.stringValue = sender.formattedMultiplier
        for document in gameDocuments {
            document.applyGameplayRates()
        }
    }
    
    @IBAction func changeGlobalDefaultShader(_ sender: Any?) {
        guard let context = OELibraryDatabase.default?.mainThreadContext else { return }
        
        guard let shaderName = globalDefaultShaderSelection.selectedItem?.title else { return }
        
        let allSystemIdentifiers = OEDBSystem.allSystemIdentifiers(in: context)
        allSystemIdentifiers.forEach(OESystemShaderStore.shared.resetShader(forSystem:))
        OEShaderStore.shared.defaultShaderName = shaderName
    }
}

private extension NSSlider {
    var formattedPercentage: String {
        String(format: "%.0f%%", doubleValue * 100)
    }

    var formattedMultiplier: String {
        String(format: "%.2g×", doubleValue)
    }
}

// MARK: - PreferencePane

extension PrefGameplayController: PreferencePane {
    
    var icon: NSImage? { NSImage(named: "gameplay_tab_icon") }
    
    var panelTitle: String { "Gameplay" }
    
    var viewSize: NSSize { view.fittingSize }
}
