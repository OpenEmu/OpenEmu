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

extension OEGameControlsBar {
    
    private static let fadeOutDelayKey = "fadeoutdelay"
    static let showsAutoSaveStateKey = "HUDBarShowAutosaveState"
    static let showsQuickSaveStateKey = "HUDBarShowQuicksaveState"
    static let showsAudioOutputKey = "HUDBarShowAudioOutput"
    
    
    open
    override var canBecomeKey: Bool {
        return false
    }
    
    open
    override var canBecomeMain: Bool {
        return false
    }
    
    
    // MARK: - Cheats
    
    private func loadCheats() {
        // In order to load cheats, we need the game core to be running and, consequently, the ROM to be set.
        // We use -reflectEmulationRunning:, which we receive from GameViewController when the emulation
        // starts or resumes
        if gameViewController.supportsCheats,
           let md5Hash = gameViewController.document.rom.md5Hash {
            let cheatsXML = Cheats(md5Hash: md5Hash)
            cheats = cheatsXML.allCheats
            cheatsLoaded = true
        }
    }
    
    
    // MARK: - Updating UI States
    
    func reflectVolume(_ volume: Float) {
        controlsView.reflectVolume(volume)
    }
    
    func reflectEmulationRunning(_ isEmulationRunning: Bool) {
        controlsView.reflectEmulationPaused(!isEmulationRunning)
        
        if isEmulationRunning && !cheatsLoaded {
            loadCheats()
        }
    }
    
    @objc private func gameWindowDidEnterFullScreen(_ notification: Notification) {
        controlsView.reflectFullScreen(true)
        // Show HUD because fullscreen animation makes the cursor appear
        performMouseMoved(nil)
    }
    
    @objc private func gameWindowWillExitFullScreen(_ notification: Notification) {
        controlsView.reflectFullScreen(false)
    }
    
    // MARK: - Menus
    
    var optionsMenu: NSMenu {
        let menu = NSMenu()
        
        var item = NSMenuItem(title: NSLocalizedString("Edit Game Controls…", comment: ""), action: #selector(OEGameDocument.editControls(_:)), keyEquivalent: "")
        menu.addItem(item)
        
        // insert cart/disk/tape
        if gameViewController.supportsFileInsertion {
            item = NSMenuItem(title: NSLocalizedString("Insert Cart/Disk/Tape…", comment: ""), action: #selector(OEGameDocument.insertFile(_:)), keyEquivalent: "")
            menu.addItem(item)
        }
        
        // cheats
        if gameViewController.supportsCheats {
            item = NSMenuItem()
            item.title = NSLocalizedString("Select Cheat", comment: "")
            item.submenu = cheatsMenu
            menu.addItem(item)
        }
        
        // core selection
        if let coresMenu = coresMenu {
            item = NSMenuItem()
            item.title = NSLocalizedString("Select Core", comment: "")
            item.submenu = coresMenu
            menu.addItem(item)
        }
        
        // disc selection
        if gameViewController.supportsMultipleDiscs {
            let maxDiscs = gameViewController.discCount
            item = NSMenuItem()
            item.title = NSLocalizedString("Select Disc", comment: "")
            item.submenu = maxDiscs > 1 ? discsMenu : nil
            item.isEnabled = maxDiscs > 1 ? true : false
            menu.addItem(item)
        }
        
        // display mode
        if gameViewController.supportsDisplayModeChange,
           !gameViewController.displayModes.isEmpty {
            item = NSMenuItem()
            item.title = NSLocalizedString("Select Display Mode", comment: "")
            item.submenu = displayModesMenu
            menu.addItem(item)
        }
        
        // video shader
        item = NSMenuItem()
        item.title = NSLocalizedString("Select Shader", comment: "")
        item.submenu = shadersMenu
        menu.addItem(item)
        
        // integral scaling
        item = NSMenuItem()
        item.title = NSLocalizedString("Select Scale", comment: "")
        if let scaleMenu = scaleMenu {
            item.submenu = scaleMenu
        } else {
            item.isEnabled = false
        }
        menu.addItem(item)
        
        // audio output
        if UserDefaults.standard.bool(forKey: Self.showsAudioOutputKey) {
            item = NSMenuItem()
            item.title = NSLocalizedString("Select Audio Output Device", comment: "")
            if let audioOutputMenu = audioOutputMenu {
                item.submenu = audioOutputMenu
            } else {
                item.isEnabled = false
            }
            menu.addItem(item)
        }
        
        return menu
    }
    
    var cheatsMenu: NSMenu {
        let menu = NSMenu()
        
        let item = NSMenuItem(title: NSLocalizedString("Add Cheat…", comment: ""), action: #selector(OEGameDocument.addCheat(_:)), keyEquivalent: "")
        item.representedObject = cheats
        menu.addItem(item)
        
        if let cheats = cheats, !cheats.isEmpty {
            menu.addItem(.separator())
            
            for cheat in cheats {
                let description = cheat["description"] as? String ?? ""
                let enabled = cheat["enabled"] as? Bool ?? false
                
                let item = NSMenuItem(title: description, action: #selector(OEGameDocument.setCheat(_:)), keyEquivalent: "")
                item.representedObject = cheat
                item.state = enabled ? .on : .off
                
                menu.addItem(item)
            }
        }
        
        return menu
    }
    
    var coresMenu: NSMenu? {
        let systemIdentifier = gameViewController.systemIdentifier
        guard var corePlugins = OECorePlugin.corePlugins(forSystemIdentifier: systemIdentifier),
              corePlugins.count > 1
        else { return nil }
        
        let menu = NSMenu()
        
        corePlugins.sort { ($0.displayName).localizedStandardCompare($1.displayName) == .orderedAscending }
        
        for plugin in corePlugins {
            let item = NSMenuItem(title: plugin.displayName, action: #selector(OEGameDocument.switchCore(_:)), keyEquivalent: "")
            item.representedObject = plugin
            
            if plugin.bundleIdentifier == gameViewController.coreIdentifier {
                item.state = .on
            }
            
            menu.addItem(item)
        }
        
        return menu
    }
    
    var discsMenu: NSMenu {
        let menu = NSMenu()
        
        let maxDiscs = gameViewController.discCount
        for disc in 1...maxDiscs {
            let title = String(format: NSLocalizedString("Disc %u", comment: "Disc selection menu item title"), disc)
            let item = NSMenuItem(title: title, action: #selector(OEGameDocument.setDisc(_:)), keyEquivalent: "")
            item.representedObject = disc
            
            menu.addItem(item)
        }
        
        return menu
    }
    
    var displayModesMenu: NSMenu {
        let menu = NSMenu()
        menu.autoenablesItems = false
        
        var mode: String
        var selected: Bool
        var enabled: Bool
        var indentationLevel: Int
        
        for modeDict in gameViewController.displayModes {
            if modeDict[OEGameCoreDisplayModeSeparatorItemKey] != nil {
                menu.addItem(.separator())
                continue
            }
            
            mode             = modeDict[OEGameCoreDisplayModeNameKey] as? String ??
                               modeDict[OEGameCoreDisplayModeLabelKey] as? String ?? ""
            selected         = modeDict[OEGameCoreDisplayModeStateKey] as? Bool ?? false
            enabled          = modeDict[OEGameCoreDisplayModeLabelKey] != nil ? false : true
            indentationLevel = modeDict[OEGameCoreDisplayModeIndentationLevelKey] as? Int ?? 0
            
            // Submenu group
            if modeDict[OEGameCoreDisplayModeGroupNameKey] != nil {
                // Setup Submenu
                let submenu = NSMenu()
                submenu.autoenablesItems = false
                
                let item = NSMenuItem()
                item.title = modeDict[OEGameCoreDisplayModeGroupNameKey] as? String ?? ""
                item.submenu = submenu
                menu.addItem(item)
                
                // Submenu items
                for subModeDict in modeDict[OEGameCoreDisplayModeGroupItemsKey] as? [[String : AnyObject]] ?? [] {
                    // Disallow deeper submenus
                    if subModeDict[OEGameCoreDisplayModeGroupNameKey] != nil {
                        continue
                    }
                    
                    if subModeDict[OEGameCoreDisplayModeSeparatorItemKey] != nil {
                        menu.addItem(.separator())
                        continue
                    }
                    
                    mode             = subModeDict[OEGameCoreDisplayModeNameKey] as? String ??
                                       subModeDict[OEGameCoreDisplayModeLabelKey] as? String ?? ""
                    selected         = subModeDict[OEGameCoreDisplayModeStateKey] as? Bool ?? false
                    enabled          = subModeDict[OEGameCoreDisplayModeLabelKey] != nil ? false : true
                    indentationLevel = subModeDict[OEGameCoreDisplayModeIndentationLevelKey] as? Int ?? 0
                    
                    let submenuItem = NSMenuItem(title: mode, action: #selector(OEGameDocument.changeDisplayMode(_:)), keyEquivalent: "")
                    submenuItem.representedObject = subModeDict
                    submenuItem.state = selected ? .on : .off
                    submenuItem.isEnabled = enabled
                    submenuItem.indentationLevel = indentationLevel
                    submenu.addItem(submenuItem)
                }
                
                continue
            }
            
            let item = NSMenuItem(title: mode, action: #selector(OEGameDocument.changeDisplayMode(_:)), keyEquivalent: "")
            item.representedObject = modeDict
            item.state = selected ? .on : .off
            item.isEnabled = enabled
            item.indentationLevel = indentationLevel
            menu.addItem(item)
        }
        
        return menu
    }
    
    var shadersMenu: NSMenu {
        let menu = NSMenu()
        
        let item = NSMenuItem(title: NSLocalizedString("Configure Shader…", comment: ""), action: #selector(GameViewController.configureShader(_:)), keyEquivalent: "")
        menu.addItem(item)
        menu.addItem(.separator())
        
        let systemIdentifier = gameViewController.systemIdentifier
        let selectedShader = OEShadersModel.shared.shaderName(forSystem: systemIdentifier)
        
        // add system shaders first
        let sortedSystemShaders = OEShadersModel.shared.sortedSystemShaderNames
        sortedSystemShaders.forEach { shaderName in
            let item = NSMenuItem(title: shaderName, action: #selector(GameViewController.selectShader(_:)), keyEquivalent: "")
            
            if shaderName == selectedShader {
                item.state = .on
            }
            
            menu.addItem(item)
        }
        
        // add custom shaders
        let sortedCustomShaders = OEShadersModel.shared.sortedCustomShaderNames
        if !sortedCustomShaders.isEmpty {
            menu.addItem(.separator())
            
            sortedCustomShaders.forEach { shaderName in
                let item = NSMenuItem(title: shaderName, action: #selector(GameViewController.selectShader(_:)), keyEquivalent: "")
                
                if shaderName == selectedShader {
                    item.state = .on
                }
                
                menu.addItem(item)
            }
        }
        
        return menu
    }
    
    var scaleMenu: NSMenu? {
        guard let delegate = gameViewController.integralScalingDelegate,
              delegate.shouldAllowIntegralScaling,
              let maxScale = delegate.maximumIntegralScale,
              let currentScale = delegate.currentIntegralScale
        else { return nil }
        
        let menu = NSMenu()
        
        for scale in 1...maxScale {
            let title = String(format: NSLocalizedString("%ux", comment: "Integral scale menu item title"), scale)
            let item = NSMenuItem(title: title, action: #selector(OEPopoutGameWindowController.changeIntegralScale(_:)), keyEquivalent: "")
            item.representedObject = scale
            item.state = scale == currentScale ? .on : .off
            menu.addItem(item)
        }
        
        return menu
    }
    
    var audioOutputMenu: NSMenu? {
        let menu = NSMenu()
        
        let audioOutputDevices = OEAudioDeviceManager.shared.audioDevices.filter { $0.numberOfOutputChannels > 0 }
        
        guard !audioOutputDevices.isEmpty
        else { return nil }
        
        let item = NSMenuItem(title: NSLocalizedString("System Default", comment: "Default audio device setting"), action: #selector(OEGameDocument.changeAudioOutputDevice(_:)), keyEquivalent: "")
        item.representedObject = nil
        menu.addItem(item)
        
        menu.addItem(.separator())
        
        for device in audioOutputDevices {
            let item = NSMenuItem(title: device.deviceName, action: #selector(OEGameDocument.changeAudioOutputDevice(_:)), keyEquivalent: "")
            item.representedObject = device
            menu.addItem(item)
        }
        
        return menu
    }
    
    var saveMenu: NSMenu {
        let menu = NSMenu()
        menu.autoenablesItems = false
        
        let item = NSMenuItem(title: NSLocalizedString("Save Current Game…", comment: ""), action: #selector(OEGlobalEventsHandler.saveState(_:)), keyEquivalent: "")
        item.isEnabled = gameViewController.supportsSaveStates
        menu.addItem(item)
        
        guard gameViewController.supportsSaveStates,
              let rom = gameViewController.document.rom
        else { return menu }
        rom.removeMissingStates()
        
        let includeAutoSaveState = UserDefaults.standard.bool(forKey: Self.showsAutoSaveStateKey)
        let includeQuickSaveState = UserDefaults.standard.bool(forKey: Self.showsQuickSaveStateKey)
        let useQuickSaveSlots = UserDefaults.standard.bool(forKey: OESaveStateUseQuickSaveSlotsKey)
        var saveStates = rom.normalSaveStates(byTimestampAscending: true) ?? []
        
        if includeQuickSaveState && !useQuickSaveSlots, let quickSaveState = rom.quickSaveState(inSlot: 0) {
            saveStates = [quickSaveState] + saveStates
        }
        
        if includeAutoSaveState, let autosaveState = rom.autosaveState() {
            saveStates = [autosaveState] + saveStates
        }
        
        if !saveStates.isEmpty || (includeQuickSaveState && useQuickSaveSlots) {
            menu.addItem(.separator())
            
            var item = NSMenuItem(title: NSLocalizedString("Load", comment: ""), action: nil, keyEquivalent: "")
            item.isEnabled = false
            menu.addItem(item)
            
            item = NSMenuItem(title: NSLocalizedString("Delete", comment: ""), action: nil, keyEquivalent: "")
            item.isAlternate = true
            item.isEnabled = false
            item.keyEquivalentModifierMask = .option
            menu.addItem(item)
            
            // Build Quck Load item with submenu
            if includeQuickSaveState && useQuickSaveSlots {
                let title = NSLocalizedString("Quick Load", comment: "Quick load menu title")
                let item = NSMenuItem(title: title, action: nil, keyEquivalent: "")
                item.indentationLevel = 1
                
                let submenu = NSMenu(title: title)
                for i in 1...9 {
                    let state = rom.quickSaveState(inSlot: i)
                    
                    let title = String(format: NSLocalizedString("Slot %ld", comment: "Quick load menu item title"), i)
                    let item = NSMenuItem(title: title, action: #selector(OEGlobalEventsHandler.quickLoad(_:)), keyEquivalent: "")
                    item.isEnabled = state != nil
                    item.representedObject = i
                    submenu.addItem(item)
                }
                
                item.submenu = submenu
                menu.addItem(item)
            }
            
            // Add 'normal' save states
            for saveState in saveStates {
                let itemTitle = saveState.displayName ?? saveState.timestamp?.description ?? ""
                
                var item = NSMenuItem(title: itemTitle, action: #selector(OEGlobalEventsHandler.loadState(_:)), keyEquivalent: "")
                item.indentationLevel = 1
                item.representedObject = saveState
                menu.addItem(item)
                
                item = NSMenuItem(title: itemTitle, action: #selector(OEGameDocument.deleteSaveState(_:)), keyEquivalent: "")
                item.isAlternate = true
                item.keyEquivalentModifierMask = .option
                item.representedObject = saveState
                item.indentationLevel = 1
                menu.addItem(item)
            }
        }
        
        return menu
    }
}
