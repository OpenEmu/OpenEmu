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
import OpenEmuBase
import OpenEmuSystem
import OpenEmuKit

final class GameControlsBar: NSWindow {
    
    static let showsAutoSaveStateKey = "HUDBarShowAutosaveState"
    static let showsQuickSaveStateKey = "HUDBarShowQuicksaveState"
    static let showsAudioOutputKey = "HUDBarShowAudioOutput"
    private static let fadeOutDelayKey = "fadeoutdelay"
    private static let initializeDefaults: Void = {
        UserDefaults.standard.register(defaults: [
            // Time until hud controls bar fades out
            fadeOutDelayKey : 1.5,
            showsAutoSaveStateKey : false,
            showsQuickSaveStateKey : false,
            showsAudioOutputKey : false,
        ])
    }()
    
    @objc var canShow = true
    private var eventMonitor: Any?
    private var fadeTimer: Timer?
    private var controlsView: GameControlsBarView!
    weak var gameViewController: GameViewController!
    private var lastGameWindowFrame = NSRect.zero
    private var lastMouseMovement: Date! {
        willSet {
            if fadeTimer == nil {
                let interval = TimeInterval(UserDefaults.standard.double(forKey: Self.fadeOutDelayKey))
                fadeTimer = Timer.scheduledTimer(timeInterval: interval, target: self, selector: #selector(timerDidFire(_:)), userInfo: nil, repeats: true)
            }
        }
    }
    
    var gameWindow: NSWindow? {
        willSet {
            // un-register notifications for parent window
            if parent != nil {
                let nc = NotificationCenter.default
                nc.removeObserver(self, name: NSWindow.didEnterFullScreenNotification, object: gameWindow)
                nc.removeObserver(self, name: NSWindow.willExitFullScreenNotification, object: gameWindow)
                nc.removeObserver(self, name: NSWindow.didChangeScreenNotification, object: gameWindow)
            }
            // remove from parent window if there was one, and attach to to the new game window
            if (gameWindow == nil || parent != nil) && newValue != parent {
                parent?.removeChildWindow(self)
                newValue?.addChildWindow(self, ordered: .above)
            }
        }
        didSet {
            // register notifications and update state of the fullscreen button
            if let gameWindow = gameWindow {
                let nc = NotificationCenter.default
                nc.addObserver(self, selector: #selector(gameWindowDidEnterFullScreen(_:)), name: NSWindow.didEnterFullScreenNotification, object: gameWindow)
                nc.addObserver(self, selector: #selector(gameWindowWillExitFullScreen(_:)), name: NSWindow.willExitFullScreenNotification, object: gameWindow)
                
                controlsView.reflectFullScreen(gameWindow.isFullScreen)
            }
        }
    }
    
    init(gameViewController controller: GameViewController) {
        let useNew = OEAppearance.hudBar == .vibrant
        
        var barRect: NSRect
        if useNew {
            barRect = NSRect(x: 0, y: 0, width: 442, height: 42)
        } else {
            barRect = NSRect(x: 0, y: 0, width: 442, height: 45)
        }
        
        super.init(contentRect: barRect, styleMask: useNew ? .titled : .borderless, backing: .buffered, defer: true)
        
        isMovableByWindowBackground = true
        animationBehavior = .none
        
        gameViewController = controller
        
        if useNew {
            titlebarAppearsTransparent = true
            titleVisibility = .hidden
            styleMask.insert(.fullSizeContentView)
            appearance = NSAppearance(named: .vibrantDark)
            
            let veView = NSVisualEffectView()
            veView.material = .hudWindow
            veView.state = .active
            contentView = veView
        } else {
            backgroundColor = .clear
        }
        alphaValue = 0
        
        let barView = GameControlsBarView(frame: barRect)
        contentView?.addSubview(barView)
        controlsView = barView
        
        eventMonitor = NSEvent.addLocalMonitorForEvents(matching: .mouseMoved) { [weak self] event in
            if NSApp.isActive, let self = self, let gameWindow = self.gameWindow, gameWindow.isMainWindow {
                self.performSelector(onMainThread: #selector(self.mouseMoved(with:)), with: event, waitUntilDone: false)
            }
            return event
        }
        
        NSCursor.setHiddenUntilMouseMoves(true)
        
        let nc = NotificationCenter.default
        // Show HUD when switching back from other applications
        nc.addObserver(self, selector: #selector(mouseMoved(with:)), name: NSApplication.didBecomeActiveNotification, object: nil)
        nc.addObserver(self, selector: #selector(willMove(_:)), name: NSWindow.willMoveNotification, object: self)
        nc.addObserver(self, selector: #selector(didMove(_:)), name: NSWindow.didMoveNotification, object: self)
        
        Self.initializeDefaults
    }
    
    deinit {
        fadeTimer?.invalidate()
        fadeTimer = nil
        gameViewController = nil
        
        if let eventMonitor = eventMonitor {
            NSEvent.removeMonitor(eventMonitor)
        }
        
        gameWindow = nil
    }
    
    override var canBecomeKey: Bool {
        return false
    }
    
    override var canBecomeMain: Bool {
        return false
    }
    
    private var bounds: NSRect {
        var bounds = frame
        bounds.origin = NSPoint(x: 0, y: 0)
        return bounds
    }
    
    // MARK: - Manage Visibility
    
    func show() {
        if canShow {
            animator().alphaValue = 1
        }
    }
    
    func hide(animated: Bool = true, hideCursor: Bool = true) {
        NSCursor.setHiddenUntilMouseMoves(hideCursor)
        
        // only hide if 'docked' to game window (aka on the same screen)
        if parent != nil {
            if animated {
                DispatchQueue.main.async {
                    self.animator().alphaValue = 0
                }
            } else {
                alphaValue = 0
            }
        }
        
        fadeTimer?.invalidate()
        fadeTimer = nil
    }
    
    override func mouseMoved(with event: NSEvent) {
        performMouseMoved()
    }
    
    private func performMouseMoved() {
        guard let gameWindow = gameWindow else { return }
        
        let gameView = gameViewController.view
        let viewFrame = gameView.frame
        let mouseLoc = NSEvent.mouseLocation
        
        let viewFrameOnScreen = gameWindow.convertToScreen(viewFrame)
        if !viewFrameOnScreen.contains(mouseLoc) {
            return
        }
        
        if alphaValue == 0 {
            lastMouseMovement = Date()
            show()
        }
        
        lastMouseMovement = Date()
    }
    
    @objc private func timerDidFire(_ timer: Timer) {
        let interval = TimeInterval(UserDefaults.standard.double(forKey: Self.fadeOutDelayKey))
        let hideDate = lastMouseMovement.addingTimeInterval(interval)
        
        if hideDate.timeIntervalSinceNow <= 0 {
            if canFadeOut {
                fadeTimer?.invalidate()
                fadeTimer = nil
                
                hide()
            } else {
                let interval = TimeInterval(UserDefaults.standard.double(forKey: Self.fadeOutDelayKey))
                let nextTime = Date(timeIntervalSinceNow: interval)
                
                fadeTimer?.fireDate = nextTime
            }
        } else {
            fadeTimer?.fireDate = hideDate
        }
    }
    
    private var canFadeOut: Bool {
        return !bounds.contains(mouseLocationOutsideOfEventStream)
    }
    
    func repositionOnGameWindow() {
        guard let gameWindow = gameWindow, parent != nil else { return }
        
        let controlsMargin: CGFloat = 19
        let gameView = gameViewController.view
        let gameViewFrame = gameView.frame
        let gameViewFrameInWindow = gameView.convert(gameViewFrame, to: nil)
        var origin = gameWindow.convertToScreen(gameViewFrameInWindow).origin
        
        origin.x += (gameViewFrame.width - frame.width) / 2
        
        // If the controls bar fits, it sits over the window
        if gameViewFrame.width >= frame.width {
            origin.y += controlsMargin
        } else {
            // Otherwise, it sits below the window
            origin.y -= (frame.height + controlsMargin)
            
            // Unless below the window means it being off-screen, in which case it sits above the window
            if origin.y < gameWindow.screen?.visibleFrame.minY ?? 0 {
                origin.y = gameWindow.frame.maxY + controlsMargin
            }
        }
        
        setFrameOrigin(origin)
    }
    
    // MARK: -
    
    @objc private func willMove(_ notification: Notification) {
        if let parentWindow = parent {
            lastGameWindowFrame = parentWindow.frame
        }
    }
    
    @objc private func didMove(_ notification: Notification) {
        var userMoved = false
        if let parentWindow = parent {
            userMoved = parentWindow.frame.equalTo(lastGameWindowFrame)
        } else {
            userMoved = true
        }
        adjustWindowAttachment(userMoved)
    }
    
    private func adjustWindowAttachment(_ userMovesGameWindow: Bool) {
        let barScreen = screen
        let gameScreen = gameWindow?.screen
        let screensDiffer = barScreen != gameScreen
        
        if userMovesGameWindow && screensDiffer && parent != nil && barScreen != nil {
            let frame = frame
            orderOut(nil)
            setFrame(.zero, display: false)
            setFrame(frame, display: false)
            orderFront(self)
        }
        else if !screensDiffer && parent == nil {
            // attach to window and center the controls bar
            gameWindow?.addChildWindow(self, ordered: .above)
            repositionOnGameWindow()
        }
    }
    
    override func mouseUp(with event: NSEvent) {
        super.mouseUp(with: event)
        adjustWindowAttachment(false)
    }
    
    // MARK: - Updating UI States
    
    func reflectVolume(_ volume: Float) {
        controlsView.reflectVolume(volume)
    }
    
    func reflectEmulationPaused(_ isPaused: Bool) {
        controlsView.reflectEmulationPaused(isPaused)
    }
    
    @objc private func gameWindowDidEnterFullScreen(_ notification: Notification) {
        controlsView.reflectFullScreen(true)
        // Show HUD because fullscreen animation makes the cursor appear
        performMouseMoved()
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
            let maxDiscs = gameViewController.document.discCount
            item = NSMenuItem()
            item.title = NSLocalizedString("Select Disc", comment: "")
            item.submenu = maxDiscs > 1 ? discsMenu : nil
            item.isEnabled = maxDiscs > 1 ? true : false
            menu.addItem(item)
        }
        
        // display mode
        if gameViewController.supportsDisplayModeChange,
           !gameViewController.document.displayModes.isEmpty {
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
        menu.addItem(item)
        
        let cheats = gameViewController.document.cheats
        if !cheats.isEmpty {
            menu.addItem(.separator())
            
            for cheat in cheats {
                let item = NSMenuItem(title: cheat.name, action: #selector(OEGameDocument.toggleCheat(_:)), keyEquivalent: "")
                item.representedObject = cheat
                item.state = cheat.isEnabled ? .on : .off
                
                menu.addItem(item)
            }
        }
        
        return menu
    }
    
    var coresMenu: NSMenu? {
        let systemIdentifier = gameViewController.systemIdentifier
        var corePlugins = OECorePlugin.corePlugins(forSystemIdentifier: systemIdentifier)
        guard corePlugins.count > 1
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
        
        let maxDiscs = gameViewController.document.discCount
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
        
        for modeDict in gameViewController.document.displayModes {
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
                        submenu.addItem(.separator())
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
        
        let selectedShader = gameViewController.shaderControl.preset.shader.name
        
        // add system shaders first
        let sortedSystemShaders = OEShaderStore.shared.sortedSystemShaderNames
        for shaderName in sortedSystemShaders {
            let item = NSMenuItem(title: shaderName, action: #selector(GameViewController.selectShader(_:)), keyEquivalent: "")
            
            if shaderName == selectedShader {
                item.state = .on
            }
            
            menu.addItem(item)
        }
        
        // add custom shaders
        let sortedCustomShaders = OEShaderStore.shared.sortedCustomShaderNames
        if !sortedCustomShaders.isEmpty {
            menu.addItem(.separator())
            
            for shaderName in sortedCustomShaders {
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
              delegate.shouldAllowIntegralScaling
        else { return nil }
        
        let maxScale = delegate.maximumIntegralScale
        let currentScale = delegate.currentIntegralScale
        
        let menu = NSMenu()
        
        for scale in 1...maxScale {
            let title = String(format: NSLocalizedString("%ux", comment: "Integral scale menu item title"), scale)
            let item = NSMenuItem(title: title, action: #selector(GameWindowController.changeIntegralScale(_:)), keyEquivalent: "")
            item.representedObject = scale
            item.state = scale == currentScale ? .on : .off
            menu.addItem(item)
        }
        
        if gameWindow?.isFullScreen ?? false {
            let item = NSMenuItem(title: NSLocalizedString("Fill Screen", comment: "Integral scale menu item title"), action: #selector(GameWindowController.changeIntegralScale), keyEquivalent: "")
            item.representedObject = 0
            item.state = currentScale == 0 ? .on : .off
            menu.addItem(item)
        }
        
        return menu
    }
    
    var audioOutputMenu: NSMenu? {
        let menu = NSMenu()
        
        let audioOutputDevices = OEAudioDeviceManager.shared.audioDevices.filter { $0.numberOfOutputChannels > 0 }
        
        if audioOutputDevices.isEmpty {
            return nil
        }
        
        let item = NSMenuItem(title: NSLocalizedString("System Default", comment: "Default audio device setting"), action: #selector(OEGameDocument.changeAudioOutputDeviceToSystemDefault(_:)), keyEquivalent: "")
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
        let useQuickSaveSlots = UserDefaults.standard.bool(forKey: OEDBSaveState.useQuickSaveSlotsKey)
        var saveStates = rom.normalSaveStatesByTimestamp(ascending: true)
        
        if includeQuickSaveState && !useQuickSaveSlots, let quickSaveState = rom.quickSaveState(inSlot: 0) {
            saveStates.insert(quickSaveState, at: 0)
        }
        
        if includeAutoSaveState, let autosaveState = rom.autosaveState {
            saveStates.insert(autosaveState, at: 0)
        }
        
        if !saveStates.isEmpty || (includeQuickSaveState && useQuickSaveSlots) {
            menu.addItem(.separator())
            
            var item = NSMenuItem(title: NSLocalizedString("Load", comment: ""), action: nil, keyEquivalent: "")
            item.isEnabled = false
            menu.addItem(item)
            
            item = NSMenuItem(title: NSLocalizedString("Delete", comment: ""), action: nil, keyEquivalent: "")
            item.isEnabled = false
            item.isAlternate = true
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
                let itemTitle = saveState.displayName
                
                var item = NSMenuItem(title: itemTitle, action: #selector(OEGlobalEventsHandler.loadState(_:)), keyEquivalent: "")
                item.representedObject = saveState
                item.indentationLevel = 1
                menu.addItem(item)
                
                item = NSMenuItem(title: itemTitle, action: #selector(OEGameDocument.deleteSaveState(_:)), keyEquivalent: "")
                item.representedObject = saveState
                item.isAlternate = true
                item.keyEquivalentModifierMask = .option
                item.indentationLevel = 1
                menu.addItem(item)
            }
        }
        
        return menu
    }
}
