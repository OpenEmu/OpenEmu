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
import Carbon.HIToolbox.Events
import OpenEmuSystem
import OpenEmuKit

final class PrefControlsController: NSViewController {
    
    private let lastControlsPluginIdentifierKey = "lastControlsPlugin"
    private let lastControlsPlayerKey = "lastControlsPlayer"
    private let lastControlsDeviceTypeKey = "lastControlsDevice"
    private let keyboardBindingsIsSelectedKey = "OEKeyboardBindingsIsSelectedKey"
    private let keyboardMenuItemRepresentedObject = "org.openemu.Bindings.Keyboard"
    
    private var _selectedKey: String?
    var selectedKey: String? {
        get {
            return _selectedKey
        }
        set {
            _selectedKey = (selectedKey != newValue) ? newValue : nil
            
            CATransaction.begin()
            controlsSetupView.selectedKey = selectedKey
            controllerView?.setSelectedKey(selectedKey, animated: true)
            CATransaction.commit()
            
            if selectedKey != nil {
                view.window?.makeFirstResponder(view)
            }
        }
    }
    
    private var _selectedPlayer: UInt?
    @objc dynamic var selectedPlayer: UInt {
        get {
            return _selectedPlayer ?? 0
        }
        set {
            if selectedPlayer != newValue {
                _selectedPlayer = newValue
                UserDefaults.standard.set(selectedPlayer, forKey: lastControlsPlayerKey)
                playerPopupButton.selectItem(withTag: Int(selectedPlayer))
                updateInputPopupButtonSelection()
            }
        }
    }
    
    var controllerView: ControllerImageView!
    @IBOutlet weak var controllerContainerView: NSView!
    @IBOutlet weak var consolesPopupButton: NSPopUpButton!
    @IBOutlet weak var playerPopupButton: NSPopUpButton!
    @IBOutlet weak var inputPopupButton: NSPopUpButton!
    @IBOutlet weak var gradientOverlay: NSView!
    @IBOutlet weak var veView: NSVisualEffectView!
    @IBOutlet weak var controlsSetupView: ControlsButtonSetupView!
    
    private var selectedPlugin: OESystemPlugin?
    private var readingEvent: OEHIDEvent?
    private var ignoredEvents = Set<IgnoredEvent>()
    private var eventMonitor: Any?
    
    var currentSystemController: OESystemController? {
        return selectedPlugin?.controller
    }
    @objc dynamic private(set) var currentSystemBindings: OESystemBindings?
    private var _currentPlayerBindings: OEPlayerBindings?
    @objc dynamic private(set) var currentPlayerBindings: OEPlayerBindings? {
        get {
            if isKeyboardEventSelected {
                return currentSystemBindings?.keyboardPlayerBindings(forPlayer: selectedPlayer)
            } else {
                return currentSystemBindings?.devicePlayerBindings(forPlayer: selectedPlayer)
            }
        }
        set {
            _currentPlayerBindings = newValue
        }
    }
    
    override class func keyPathsForValuesAffectingValue(forKey key: String) -> Set<String> {
        if key == "currentPlayerBindings" {
            return Set<String>([
                "currentSystemBindings",
                "currentSystemBindings.devicePlayerBindings",
                "selectedPlayer",
            ])
        } else {
            return super.keyPathsForValuesAffectingValue(forKey: key)
        }
    }
    
    // MARK: - ViewController Overrides
    
    override var nibName: NSNib.Name? {
        if OEAppearance.controlsPrefs == .wood {
            return "PrefControlsController"
        } else {
            return "PrefControlsController2"
        }
    }
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        controlsSetupView.target = self
        controlsSetupView.action = #selector(changeInputControl(_:))
        
        controlsSetupView.bind(NSBindingName("bindingsProvider"), to: self, withKeyPath: "currentPlayerBindings", options: nil)
        
        // Setup controls popup console list
        rebuildSystemsMenu()
        setUpInputMenu()
        
        // Restore previous state.
        changeInputDevice(nil)
        
        let pluginName = UserDefaults.standard.string(forKey: lastControlsPluginIdentifierKey)
        consolesPopupButton.selectItem(at: 0)
        let itemIndex = consolesPopupButton.indexOfItem(withRepresentedObject: pluginName)
        if itemIndex != -1 {
            consolesPopupButton.selectItem(at: itemIndex)
        }
        
        CATransaction.setDisableActions(true)
        changeSystem(consolesPopupButton)
        CATransaction.commit()
        
        if OEAppearance.controlsPrefs == .wood {
            let gradient = CAGradientLayer()
            let topColor = NSColor(deviceWhite: 0, alpha: 0.3)
            let bottomColor = NSColor(deviceWhite: 0, alpha: 0)
            gradient.colors = [bottomColor.cgColor, topColor.cgColor]
            
            gradientOverlay.layer? = gradient
            
            controlsSetupView.enclosingScrollView?.appearance = NSAppearance(named: .aqua)
        }
        else if OEAppearance.controlsPrefs == .woodVibrant {
            veView.blendingMode = .withinWindow
            veView.state = .active
        }
        
        controllerView?.wantsLayer = true
        
        let nc = NotificationCenter.default
        nc.addObserver(self, selector: #selector(systemsChanged), name: .OEDBSystemAvailabilityDidChange, object: nil)
        
        nc.addObserver(self, selector: #selector(devicesDidUpdate(_:)), name: .OEDeviceManagerDidAddDeviceHandler, object: OEDeviceManager.shared)
        nc.addObserver(self, selector: #selector(devicesDidUpdate(_:)), name: .OEDeviceManagerDidRemoveDeviceHandler, object: OEDeviceManager.shared)
        
        nc.addObserver(self, selector: #selector(scrollerStyleDidChange), name: NSScroller.preferredScrollerStyleDidChangeNotification, object: nil)
    }
    
    @objc private func scrollerStyleDidChange() {
        controlsSetupView.layoutSubviews()
    }
    
    override func viewDidLayout() {
        super.viewDidLayout()
        
        // Fixes an issue where, if the controls pane isn't already the default selected pane on launch and the user manually selects the controls pane, the "Gameplay Buttons" ControlsSectionTitleView has a visible "highlight" artifact until the scroll view gets scrolled.
        controlsSetupView.layoutSubviews()
    }
    
    override func viewDidAppear() {
        super.viewDidAppear()
        
        if view.window?.isKeyWindow ?? false {
            setUpEventMonitor()
        }
        
        let nc = NotificationCenter.default
        nc.addObserver(self, selector: #selector(windowDidBecomeKey(_:)), name: NSWindow.didBecomeKeyNotification, object: nil)
        nc.addObserver(self, selector: #selector(windowDidResignKey(_:)), name: NSWindow.didResignKeyNotification, object: nil)
    }
    
    override func viewWillDisappear() {
        super.viewWillDisappear()
        
        selectedKey = nil
        
        OEBindingsController.default.synchronize()
        
        let nc = NotificationCenter.default
        nc.removeObserver(self, name: NSWindow.didBecomeKeyNotification, object: view.window)
        nc.removeObserver(self, name: NSWindow.didResignKeyNotification, object: view.window)
        
        tearDownEventMonitor()
    }
    
    @objc private func windowDidBecomeKey(_ notification: Notification) {
        if notification.object as? NSWindow == view.window {
            setUpEventMonitor()
        }
    }
    
    @objc private func windowDidResignKey(_ notification: Notification) {
        if notification.object as? NSWindow == view.window {
            selectedKey = nil
            tearDownEventMonitor()
        }
    }
    
    private func setUpEventMonitor() {
        if eventMonitor != nil {
            return
        }
        
        eventMonitor = OEDeviceManager.shared.addGlobalEventMonitorHandler { _, event in
            self.registerEventIfNeeded(event)
            return false
        }
    }
    
    private func tearDownEventMonitor() {
        guard let eventMonitor = eventMonitor else { return }
        
        OEDeviceManager.shared.removeMonitor(eventMonitor)
        self.eventMonitor = nil
    }
    
    @objc private func systemsChanged() {
        let menuItem = consolesPopupButton.selectedItem
        let selectedSystemIdentifier = menuItem?.representedObject as? String
        
        rebuildSystemsMenu()
        
        consolesPopupButton.selectItem(at: 0)
        let itemIndex = consolesPopupButton.indexOfItem(withRepresentedObject: selectedSystemIdentifier)
        if itemIndex != -1 {
            consolesPopupButton.selectItem(at: itemIndex)
        }
        
        CATransaction.setDisableActions(true)
        changeSystem(consolesPopupButton)
        CATransaction.commit()
    }
    
    @objc private func devicesDidUpdate(_ notification: Notification) {
        setUpInputMenu()
    }
    
    // MARK: - UI Setup
    
    private func rebuildSystemsMenu() {
        guard let context = OELibraryDatabase.default?.mainThreadContext
        else { return }
        
        let consolesMenu = NSMenu()
        
        let enabledSystems = OEDBSystem.enabledSystems(in: context)
        for system in enabledSystems {
            if let plugin = system.plugin {
                let item = NSMenuItem(title: plugin.systemName, action: #selector(changeSystem(_:)), keyEquivalent: "")
                item.target = self
                item.representedObject = plugin.systemIdentifier
                
                item.image = plugin.systemIcon
                
                consolesMenu.addItem(item)
            }
        }
        
        consolesPopupButton.menu = consolesMenu
    }
    
    private func setUpPlayerMenu(numberOfPlayers: UInt) {
        let playerMenu = NSMenu()
        for player in 0..<numberOfPlayers {
            let playerTitle = String(format: NSLocalizedString("Player %ld", comment: ""), player + 1)
            let playerItem = NSMenuItem(title: playerTitle, action: nil, keyEquivalent: "")
            playerItem.tag = Int(player + 1)
            playerMenu.addItem(playerItem)
        }
        
        playerPopupButton.menu = playerMenu
        
        playerPopupButton.selectItem(withTag: UserDefaults.standard.integer(forKey: lastControlsPlayerKey))
    }
    
    private func setUpInputMenu() {
        let inputMenu = NSMenu()
        inputMenu.autoenablesItems = false
        
        let inputItem = inputMenu.addItem(withTitle: NSLocalizedString("Keyboard", comment: "Keyboard bindings menu item."), action: nil, keyEquivalent: "")
        inputItem.representedObject = keyboardMenuItemRepresentedObject
        
        inputMenu.addItem(.separator())
        
        addControllers(to: inputMenu)
        
        if #available(macOS 12.0, *) {
            // kaput (issue #4612)
        } else {
            inputMenu.addItem(.separator())
            inputMenu.addItem(withTitle: NSLocalizedString("Add a Wiimote…", comment: "Wiimote bindings menu item."), action: #selector(searchForWiimote(_:)), keyEquivalent: "").target = self
        }
        
        inputPopupButton.menu = inputMenu
        updateInputPopupButtonSelection()
    }
    
    private func addControllers(to inputMenu: NSMenu) {
        var controllers = OEDeviceManager.shared.controllerDeviceHandlers
        if controllers.isEmpty {
            inputMenu.addItem(withTitle: NSLocalizedString("No available controllers", comment: "Menu item indicating that no controllers is plugged in"), action: nil, keyEquivalent: "").isEnabled = false
            return
        }
        
        controllers.sort { ($0.deviceDescription?.name ?? "").localizedStandardCompare($1.deviceDescription?.name ?? "") == .orderedAscending }
        for handler in controllers {
            let deviceName = handler.deviceDescription?.name ?? ""
            let item = inputMenu.addItem(withTitle: deviceName, action: nil, keyEquivalent: "")
            item.representedObject = handler
        }
    }
    
    private func updateInputPopupButtonSelection() {
        let keyboardIsSelected = UserDefaults.standard.bool(forKey: keyboardBindingsIsSelectedKey)
        
        let currentDeviceHandler = currentSystemBindings?.devicePlayerBindings(forPlayer: selectedPlayer)?.deviceHandler
        let representedObject = (keyboardIsSelected || currentDeviceHandler == nil) ? keyboardMenuItemRepresentedObject as NSString : currentDeviceHandler
        
        if !keyboardIsSelected && currentDeviceHandler == nil {
            UserDefaults.standard.set(true, forKey: keyboardBindingsIsSelectedKey)
        }
        
        for item in inputPopupButton.itemArray {
            if item.state == .on {
                continue
            }
            if item.representedObject != nil {
                item.state = (item.representedObject as? OEDeviceHandler) == currentDeviceHandler ? .mixed : .off
            }
        }
        inputPopupButton.selectItem(at: Int(max(0, inputPopupButton.indexOfItem(withRepresentedObject: representedObject))))
    }
    
    private func setUpControllerImageView() {
        let systemController = currentSystemController
        let imageViewLayer = controllerContainerView.layer
        
        let newControllerView = ControllerImageView(frame: controllerContainerView.bounds)
        newControllerView.image = systemController?.controllerImage
        newControllerView.imageMask = systemController?.controllerImageMask
        newControllerView.keyPositions = systemController?.controllerKeyPositions
        newControllerView.target = self
        newControllerView.action = #selector(changeInputControl(_:))
        
        // Setup animation that transitions the old controller image out
        let pathTransitionOut = CGMutablePath()
        pathTransitionOut.move(to: CGPoint(x: 0, y: 0))
        pathTransitionOut.addLine(to: CGPoint(x: 0, y: 450))
        
        let outTransition = CAKeyframeAnimation(keyPath: "position")
        outTransition.path = pathTransitionOut
        outTransition.timingFunction = CAMediaTimingFunction(name: .easeInEaseOut)
        outTransition.duration = 0.35
        
        // Setup animation that transitions the new controller image in
        let pathTransitionIn = CGMutablePath()
        pathTransitionIn.move(to: CGPoint(x: 0, y: 450))
        pathTransitionIn.addLine(to: CGPoint(x: 0, y: 0))
        
        let inTransition = CAKeyframeAnimation(keyPath: "position")
        inTransition.path = pathTransitionIn
        inTransition.timingFunction = CAMediaTimingFunction(name: .easeInEaseOut)
        inTransition.duration = 0.35
        
        let reduceMotion = NSWorkspace.shared.accessibilityDisplayShouldReduceMotion
        
        CATransaction.begin()
        CATransaction.setCompletionBlock {
            if self.controllerView != nil {
                self.controllerContainerView.replaceSubview(self.controllerView, with: newControllerView)
            } else {
                self.controllerContainerView.addSubview(newControllerView)
            }
            self.controllerView = newControllerView
            
            self.controllerContainerView.setFrameOrigin(.zero)
            if !reduceMotion {
                imageViewLayer?.add(inTransition, forKey: "animatePosition")
            }
        }
        
        controllerContainerView.setFrameOrigin(NSPoint(x: 0, y: 450))
        if !reduceMotion {
            imageViewLayer?.add(outTransition, forKey: "animatePosition")
        }
        
        CATransaction.commit()
        
        controlsSetupView.layoutSubviews()
    }
    
    // MARK: - Actions
    
    @IBAction func changeSystem(_ sender: Any?) {
        let menuItem = consolesPopupButton.selectedItem
        let systemIdentifier = menuItem?.representedObject as? String ?? UserDefaults.standard.string(forKey: lastControlsPluginIdentifierKey)
        
        var newPlugin = OESystemPlugin.systemPlugin(forIdentifier: systemIdentifier!)
        if newPlugin == nil {
            newPlugin = OESystemPlugin.allPlugins.first
        }
        
        if selectedPlugin != nil && newPlugin == selectedPlugin {
            return
        }
        selectedPlugin = newPlugin
        
        guard let systemController = currentSystemController
        else {
            assertionFailure("The systemController of the plugin \(String(describing: selectedPlugin)) with system identifier \(String(describing: selectedPlugin?.systemIdentifier)) is nil for some reason.")
            return
        }
        currentSystemBindings = OEBindingsController.default.systemBindings(for: systemController)
        
        // Rebuild player menu
        setUpPlayerMenu(numberOfPlayers: systemController.numberOfPlayers)
        
        // Make sure no key is selected before switching the system
        selectedKey = nil
        
        let preferenceView = controlsSetupView!
        preferenceView.bindingsProvider = currentPlayerBindings
        preferenceView.setup(withControlList: systemController.controlPageList as! [AnyHashable])
        preferenceView.autoresizingMask = [.maxXMargin, .maxYMargin]
        
        let rect = NSRect(x: .zero, y: .zero, width: controlsSetupView.bounds.size.width, height: preferenceView.frame.size.height)
        preferenceView.frame = rect
        
        if let scrollView = controlsSetupView.enclosingScrollView {
            controlsSetupView.setFrameOrigin(NSPoint(x: 0, y: scrollView.frame.size.height - rect.size.height))
            
            if controlsSetupView.frame.size.height <= scrollView.frame.size.height {
                scrollView.verticalScrollElasticity = .none
            } else {
                scrollView.verticalScrollElasticity = .automatic
                scrollView.flashScrollers()
            }
        }
        
        UserDefaults.standard.set(systemIdentifier, forKey: lastControlsPluginIdentifierKey)
        
        changePlayer(playerPopupButton)
        changeInputDevice(inputPopupButton)
        
        setUpControllerImageView()
    }
    
    @IBAction func changePlayer(_ sender: NSPopUpButton) {
        let player = sender.selectedTag()
        selectedPlayer = UInt(player)
    }
    
    @IBAction func changeInputDevice(_ sender: Any?) {
        guard let representedObject = inputPopupButton.selectedItem?.representedObject
        else {
            inputPopupButton.selectItem(at: 0)
            return
        }
        
        willChangeValue(forKey: "currentPlayerBindings")
        
        let isSelectingKeyboard = representedObject as? String == keyboardMenuItemRepresentedObject
        UserDefaults.standard.set(isSelectingKeyboard, forKey: keyboardBindingsIsSelectedKey)
        
        if !isSelectingKeyboard {
            guard let deviceHandler = representedObject as? OEDeviceHandler
            else {
                assertionFailure("Expecting instance of class OEDeviceHandler got: \(String(describing: representedObject))")
                return
            }
            
            let currentPlayerHandler = currentSystemBindings?.devicePlayerBindings(forPlayer: selectedPlayer)?.deviceHandler
            if deviceHandler != currentPlayerHandler {
                currentSystemBindings?.setDeviceHandler(deviceHandler, forPlayer: selectedPlayer)
            }
        }
        
        updateInputPopupButtonSelection()
        
        didChangeValue(forKey: "currentPlayerBindings")
    }
    
    @IBAction func changeInputControl(_ sender: NSView) {
        if let sender = sender as? ControllerImageView, sender == controllerView {
            selectedKey = sender.selectedKey
        }
        else if let sender = sender as? ControlsButtonSetupView, sender == controlsSetupView {
            selectedKey = sender.selectedKey
        }
    }
    
    @IBAction func searchForWiimote(_ sender: Any?) {
        updateInputPopupButtonSelection()
        
        let alert = OEAlert()
        
        if OEDeviceManager.shared.isBluetoothEnabled {
            alert.messageText = NSLocalizedString("Make your Wiimote discoverable", comment: "")
            alert.informativeText = NSLocalizedString("If there is a red button on the back battery cover, press it.\nIf not, hold down buttons ①+②.", comment: "")
            alert.defaultButtonTitle = NSLocalizedString("Start Scanning", comment: "")
            alert.alternateButtonTitle = NSLocalizedString("Cancel", comment: "")
            alert.otherButtonTitle = NSLocalizedString("Learn More", comment: "")
            
            let result = alert.runModal()
            if result == .alertFirstButtonReturn {
                OEDeviceManager.shared.startWiimoteSearch()
            }
            else if result == .alertThirdButtonReturn {
                NSWorkspace.shared.open(.userGuideWiimotePairing)
            }
        } else {
            alert.messageText = NSLocalizedString("Bluetooth Not Enabled", comment: "")
            alert.informativeText = NSLocalizedString("Bluetooth must be enabled to pair a Wii controller.", comment: "")
            alert.defaultButtonTitle = NSLocalizedString("OK", comment: "")
            alert.runModal()
        }
    }
    
    // MARK: - Input and Bindings Management
    
    private var isKeyboardEventSelected: Bool {
        return inputPopupButton.selectedItem?.representedObject as? String == keyboardMenuItemRepresentedObject
    }
    
    private func register(_ event: OEHIDEvent) {
        // Ignore any off state events
        guard !event.hasOffState, let selectedKey = selectedKey else { return }
        
        setCurrentBindings(for: event)
        
        let assignedKey = currentPlayerBindings?.assign(event, toKeyWithName: selectedKey)
        
        // automatically selecting the next button confuses VoiceOver
        if NSWorkspace.shared.isVoiceOverEnabled {
            changeInputControl(controlsSetupView)
            return
        }
        
        if let assignedKey = assignedKey as? OEKeyBindingGroupDescription {
            controlsSetupView.selectNextKeyAfterKeys(assignedKey.keyNames)
        } else {
            controlsSetupView.selectNextKeyButton()
        }
        
        changeInputControl(controlsSetupView)
    }
    
    private func setCurrentBindings(for event: OEHIDEvent) {
        willChangeValue(forKey: "currentPlayerBindings")
        let isKeyboardEvent = event.type == .keyboard
        UserDefaults.standard.set(isKeyboardEvent, forKey: keyboardBindingsIsSelectedKey)
        
        if !isKeyboardEvent {
            selectedPlayer = currentSystemBindings?.playerNumber(for: event) ?? 0
        }
        
        updateInputPopupButtonSelection()
        didChangeValue(forKey: "currentPlayerBindings")
    }
    
    private func registerEventIfNeeded(_ event: OEHIDEvent) {
        if shouldRegister(event) {
            register(event)
        }
    }
    
    // Only one event can be managed at a time, all events should be ignored until the currently read event went back to its null state
    // All ignored events are stored until they go back to the null state
    @discardableResult
    private func shouldRegister(_ event: OEHIDEvent) -> Bool {
        // The event is the currently read event,
        // if its state is off, nil the reading event,
        // in either case, this event shouldn't be registered.
        if readingEvent?.isUsageEqual(to: event) ?? false {
            if event.hasOffState {
                readingEvent = nil
            }
            
            return false
        }
        
        if selectedKey == nil && view != view.window?.firstResponder {
            view.window?.makeFirstResponder(view)
        }
        
        // Check if the event is ignored
        if ignoredEvents.contains(.init(event)) {
            // Ignored events going back to off-state are removed from the ignored events
            if event.hasOffState {
                ignoredEvents.remove(.init(event))
            }
            
            return false
        }
        
        // Esc-key events are handled through NSEvent
        if event.isEscapeKeyEvent {
            return false
        }
        
        // Ignore keyboard events if the user hasn’t explicitly chosen to configure
        // keyboard bindings. See https://github.com/OpenEmu/OpenEmu/issues/403
        if event.type == .keyboard && !isKeyboardEventSelected {
            return false
        }
        
        // No event currently read, if it's not off state, store it and read it
        if readingEvent == nil {
            // The event is not ignored but it's off, ignore it anyway
            if event.hasOffState {
                return false
            }
            
            readingEvent = event
            return true
        }
        
        if !event.hasOffState {
            ignoredEvents.insert(.init(event))
        }
        
        return false
    }
    
    override func axisMoved(_ event: OEHIDEvent) {
        if shouldRegister(event) {
            register(event)
        }
    }
    
    override func triggerPull(_ event: OEHIDEvent) {
        if shouldRegister(event) {
            register(event)
        }
    }
    
    override func triggerRelease(_ event: OEHIDEvent) {
        shouldRegister(event)
    }
    
    override func buttonDown(_ event: OEHIDEvent) {
        if shouldRegister(event) {
            register(event)
        }
    }
    
    override func buttonUp(_ event: OEHIDEvent) {
        shouldRegister(event)
    }
    
    override func hatSwitchChanged(_ event: OEHIDEvent) {
        if shouldRegister(event) {
            register(event)
        }
    }
    
    override func hidKeyDown(_ event: OEHIDEvent) {
        if shouldRegister(event) {
            register(event)
        }
    }
    
    override func hidKeyUp(_ event: OEHIDEvent) {
        shouldRegister(event)
    }
    
    override func mouseDown(with event: NSEvent) {
        if selectedKey != nil {
            selectedKey = selectedKey
        }
    }
    
    override func keyDown(with event: NSEvent) {
        guard let selectedKey = selectedKey else { return }
        
        if event.keyCode == kVK_Escape {
            currentPlayerBindings?.removeEventForKey(withName: selectedKey)
        }
    }
    
    override func keyUp(with event: NSEvent) {
    }
    
    // MARK: -
    
    func preparePane(with notification: Notification) {
        let userInfo = notification.userInfo
        let paneName = userInfo?[PreferencesWindowController.userInfoPanelNameKey] as? String
        
        guard paneName == panelTitle else { return }
        
        let systemIdentifier = userInfo?[PreferencesWindowController.userInfoSystemIdentifierKey] as? String
        let itemIndex = consolesPopupButton.indexOfItem(withRepresentedObject: systemIdentifier)
        if itemIndex != -1 {
            consolesPopupButton.selectItem(at: itemIndex)
            changeSystem(nil)
        }
    }
}

// MARK: - PreferencePane

extension PrefControlsController: PreferencePane {
    
    var icon: NSImage? { NSImage(named: "controls_tab_icon") }
    
    var panelTitle: String { "Controls" }
    
    var viewSize: NSSize {
        if OEAppearance.controlsPrefs == .wood {
            return NSSize(width: 755, height: 450)
        } else {
            return view.fittingSize
        }
    }
}

/// Wraps an ``OEHIDEvent`` to customise the ``Hashable`` and ``Equatable``
/// implementations.
private struct IgnoredEvent: Hashable {
    let event: OEHIDEvent
    
    init(_ event: OEHIDEvent) {
        self.event = event
    }
    
    static func == (lhs: IgnoredEvent, rhs: IgnoredEvent) -> Bool {
        lhs.event.isUsageEqual(to: rhs.event)
    }
    
    func hash(into hasher: inout Hasher) {
        hasher.combine(event.controlIdentifier)
    }
}
