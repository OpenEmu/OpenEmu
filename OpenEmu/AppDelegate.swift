/*
 Copyright (c) 2016, OpenEmu Team

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the OpenEmu Team nor the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY OpenEmu Team ''AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL OpenEmu Team BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

import Cocoa
import os

private var appearancePrefChangedKVOContext = 0

protocol CachedLastPlayedInfoItem {}
extension String: CachedLastPlayedInfoItem {}
extension OEDBRom: CachedLastPlayedInfoItem {}

@NSApplicationMain
@objc(OEApplicationDelegate)
@objcMembers
class AppDelegate: NSObject {
    
    static let websiteAddress = "http://openemu.org/"
    static let userGuideAddress = "https://github.com/OpenEmu/OpenEmu/wiki/User-guide"
    static let releaseNotesAddress = "https://github.com/OpenEmu/OpenEmu/wiki/Release-notes"
    static let feedbackAddress = "https://github.com/OpenEmu/OpenEmu/issues"
    static let localizationGuideAddress = "https://github.com/OpenEmu/OpenEmu/wiki/Developers:-Translation-Guide-(First-Draft)"
    
    @IBOutlet weak var fileMenu: NSMenu!
    @IBOutlet weak var localizationGuideMenuItem: NSMenuItem!
    
    lazy var mainWindowController = OEMainWindowController(windowNibName: "MainWindow")
    
    lazy var preferencesWindowController: PreferencesWindowController = PreferencesWindowController(windowNibName: "Preferences")
    
    var documentController = GameDocumentController.shared
    
    var restoreWindow = false
    var libraryDidLoadObserverForRestoreWindow: NSObjectProtocol?
    var libraryDidLoadObserverForRestoreWindow2: NSObjectProtocol?
    
    var hidEventsMonitor: Any?
    var keyboardEventsMonitor: Any?
    var unhandledEventsMonitor: Any?
        
    var cachedLastPlayedInfo = [CachedLastPlayedInfoItem]()
    
    var logHIDEvents = false {
        
        didSet {
            
            if let hidEventsMonitor = hidEventsMonitor {
                OEDeviceManager.shared.removeMonitor(hidEventsMonitor)
                self.hidEventsMonitor = nil
            }
            
            if logHIDEvents {
                hidEventsMonitor = OEDeviceManager.shared.addGlobalEventMonitorHandler { handler, event in
                    if event.type != .keyboard {
                        os_log(.info, log: OE_LOG_EVENT_HID, "%{public}@", event)
                    }
                    return true
                }
            }
        }
    }
    
    var logKeyboardEvents = false {
        
        didSet {
            
            if let keyboardEventsMonitor = keyboardEventsMonitor {
                OEDeviceManager.shared.removeMonitor(keyboardEventsMonitor)
                self.keyboardEventsMonitor = nil
            }
            
            if logKeyboardEvents {
                keyboardEventsMonitor = OEDeviceManager.shared.addGlobalEventMonitorHandler { handler, event in
                    if event.type == .keyboard {
                        os_log(.info, log: OE_LOG_EVENT_KEYBOARD, "%{public}@", event)
                    }
                    return true
                }
            }
        }
    }

    var backgroundControllerPlay = false {
        didSet {
            updateEventHandlers()
        }
    }
    
    var libraryLoaded = false
    var reviewingUnsavedDocuments = false
    
    typealias StartupQueueClosure = () -> Void
    var startupQueue = [StartupQueueClosure]()
    
    override init() {
        
        super.init()

        // Get the game library path.
        let supportDirectoryURL = FileManager.default.urls(for: .applicationSupportDirectory, in: .userDomainMask).last!
        let path = (supportDirectoryURL.appendingPathComponent("OpenEmu/Game Library").path as NSString).abbreviatingWithTildeInPath
        
        // Register defaults.
        UserDefaults.standard.register(defaults: [
            OEWiimoteSupportEnabled: true,
            OEDefaultDatabasePathKey: path,
            OEDatabasePathKey: path,
            OEAutomaticallyGetInfoKey: true,
            OEShadersModel.Preferences.global.key: "Pixellate",
            OEGameVolumeKey: 0.5,
            "defaultCore.openemu.system.nes": "org.openemu.Nestopia",
            "defaultCore.openemu.system.snes": "org.openemu.SNES9x",
            OEDisplayGameTitle: true,
            OEBackgroundPauseKey: true,
            OEBackgroundControllerPlayKey: true,
            "logsHIDEvents": false,
            "logsHIDEventsNoKeyboard": false,
            OEDBSavedGamesMediaShowsAutoSaves: true,
            OEDBSavedGamesMediaShowsQuickSaves: true,
            OEForcePopoutGameWindowKey: true,
            OEPopoutGameWindowIntegerScalingOnlyKey: true,
            OEAppearancePreferenceKey: OEAppearancePreferenceValue.dark.rawValue,
            OEHUDBarAppearancePreferenceKey: OEHUDBarAppearancePreferenceValue.dark.rawValue,
            OEControlsPrefsAppearancePreferenceKey: OEControlsPrefsAppearancePreferenceValue.wood.rawValue,
            ])
        
        #if !DEBUG_PRINT
            UserDefaults.standard.removeObject(forKey: OEGameCoreManagerModePreferenceKey)
        #endif

        // Don't let an old setting override automatically checking for app updates.
        if let automaticChecksEnabled = UserDefaults.standard.object(forKey: "SUEnableAutomaticChecks") as? Bool, automaticChecksEnabled == false {
            UserDefaults.standard.removeObject(forKey: "SUEnableAutomaticChecks")
        }

        // Trigger Objective-C +initialize methods in these classes.
        _ = OEControllerDescription.self
        
        // Reset preferences for default cores when migrating to 2.0.3. This is an attempt at cleanup after 9d5d696d07fe651f44f16f8bf8b98c87d90fe53f and d36e9ad4b7097f21ffbbe32d9cea3b72a390bc0f and for getting as many users as possible onto mGBA.
        OEVersionMigrationController.default.addMigratorTarget(self, selector: #selector(AppDelegate.migrationRemoveCoreDefaults), forVersion: "2.0.3")
    }
    
    required init?(coder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }
    
    deinit {
        OECorePlugin.self.removeObserver(self, forKeyPath: "values.".appending(OEAppearancePreferenceKey), context: &appearancePrefChangedKVOContext)
    }
    
    // MARK: - Library Database
    
    func loadDatabase() {
        
        let defaults = UserDefaults.standard
        
        let defaultDatabasePath = (defaults.string(forKey: OEDefaultDatabasePathKey)! as NSString).expandingTildeInPath
        let databasePath: String
        if let databasePathDefault = defaults.string(forKey: OEDatabasePathKey) {
            databasePath = (databasePathDefault as NSString).expandingTildeInPath
        } else {
            databasePath = defaultDatabasePath
        }
        
        let create = !FileManager.default.fileExists(atPath: databasePath) && databasePath == defaultDatabasePath
        
        let userDBSelectionRequest = NSEvent.modifierFlags.contains(.option)
        let databaseURL = URL(fileURLWithPath: databasePath)
        // If user holds down alt key.
        if userDBSelectionRequest {
            // We ask the user to either select/create one or quit OpenEmu.
            performDatabaseSelection()
        } else {
            loadDatabaseAsynchronously(from: databaseURL, createIfNecessary: create)
        }
    }
    
    fileprivate func loadDatabaseAsynchronously(from url: URL, createIfNecessary: Bool) {
        
        if createIfNecessary {
            try? FileManager.default.createDirectory(at: url, withIntermediateDirectories: true, attributes: nil)
        }
        
        do {
            
            try OELibraryDatabase.load(from: url)
            
            assert(OELibraryDatabase.default != nil, "No database available!")
            
            DispatchQueue.main.async {
                NotificationCenter.default.post(name: .OELibraryDidLoadNotificationName, object: OELibraryDatabase.default!)
            }
            
        } catch let error as NSError {
            
            if error.domain == NSCocoaErrorDomain && error.code == NSPersistentStoreIncompatibleVersionHashError {
                
                let migrator = OELibraryMigrator(store: url)
                
                DispatchQueue.global(qos: .default).async {
                    
                    do {
                        
                        try migrator.runMigration()
                        
                        self.loadDatabaseAsynchronously(from: url, createIfNecessary: createIfNecessary)
                        
                    } catch let error as NSError {
                                                
                        if error.domain != OEMigrationErrorDomain || error.code != OEMigrationErrorCode.cancelled.rawValue {
                            
                            DLog("Your library can't be opened with this version of OpenEmu.")
                            DLog("\(error)")
                            
                            DispatchQueue.main.async {
                                NSAlert(error: error).runModal()
                                NSApp.terminate(self)
                            }
                            
                        } else {
                            DLog("Migration cancelled.")
                            NSApp.terminate(self)
                        }
                    }
                }
                
            } else {
                DispatchQueue.main.async {
                    self.documentController.presentError(error)
                    self.performDatabaseSelection()
                }
            }
        }
    }
    
    fileprivate func performDatabaseSelection() {
        
        // Set up alert with "Quit", "Select", and "Create".
        let alert = OEAlert()
        alert.messageText = NSLocalizedString("Choose OpenEmu Library", comment: "")
        alert.informativeText = NSLocalizedString("OpenEmu needs a library to continue. You may choose an existing OpenEmu library or create a new one", comment: "")
        alert.defaultButtonTitle = NSLocalizedString("Choose Library…", comment: "")
        alert.alternateButtonTitle = NSLocalizedString("Create Library…", comment: "")
        alert.otherButtonTitle = NSLocalizedString("Quit", comment: "")
        
        switch alert.runModal() {
            
        case .alertThirdButtonReturn:
            NSApp.terminate(self)
            return
            
        case .alertFirstButtonReturn:
            
            let openPanel = NSOpenPanel()
            
            openPanel.canChooseFiles = true
            openPanel.allowedFileTypes = [(OEDatabaseFileName as NSString).pathExtension]
            openPanel.canChooseDirectories = true
            openPanel.allowsMultipleSelection = false
            
            openPanel.begin { result in
                
                if result == .OK {
                    
                    var databaseURL = openPanel.url!
                    let databasePath = databaseURL.path
                    
                    var isDir: ObjCBool = ObjCBool(false)
                    if FileManager.default.fileExists(atPath: databasePath, isDirectory: &isDir) && !isDir.boolValue {
                        databaseURL = databaseURL.deletingLastPathComponent()
                    }
                    
                    self.loadDatabaseAsynchronously(from: databaseURL, createIfNecessary: false)
                    
                } else {
                    self.performDatabaseSelection()
                }
            }
            
        case .alertSecondButtonReturn:
            
            let savePanel = NSSavePanel()
            
            savePanel.nameFieldStringValue = "OpenEmu Library"
            
            if savePanel.runModal() == .OK {
                
                let databaseURL = savePanel.url!
                try? FileManager.default.removeItem(at: databaseURL)
                try? FileManager.default.createDirectory(at: databaseURL, withIntermediateDirectories: true, attributes: nil)
                
                loadDatabaseAsynchronously(from: databaseURL, createIfNecessary: true)
                
            } else {
                performDatabaseSelection()
            }
            
        default:
            break
        }
    }
    
    // MARK: -

    fileprivate func validateDefaultPluginAssignments() {

        // Remove Higan WIP systems as defaults if found, since our core port does not support them.
        let defaults = UserDefaults.standard
        if defaults.string(forKey: "defaultCore.openemu.system.gb") == "org.openemu.Higan" {
            defaults.removeObject(forKey: "defaultCore.openemu.system.gb")
        }
        if defaults.string(forKey: "defaultCore.openemu.system.gba") == "org.openemu.Higan" {
            defaults.removeObject(forKey: "defaultCore.openemu.system.gba")
        }
        if defaults.string(forKey: "defaultCore.openemu.system.nes") == "org.openemu.Higan" {
            defaults.removeObject(forKey: "defaultCore.openemu.system.nes")
        }

        // Remove system defaults for deprecated core plugins.
        if defaults.string(forKey: "defaultCore.openemu.system.gba") == "org.openemu.VisualBoyAdvance" {
            defaults.removeObject(forKey: "defaultCore.openemu.system.gba")
        }
        if defaults.string(forKey: "defaultCore.openemu.system.gg") == "org.openemu.CrabEmu" ||
           defaults.string(forKey: "defaultCore.openemu.system.gg") == "org.openemu.TwoMbit" {
            defaults.removeObject(forKey: "defaultCore.openemu.system.gg")
        }
        if defaults.string(forKey: "defaultCore.openemu.system.ngp") == "org.openemu.NeoPop" {
            defaults.removeObject(forKey: "defaultCore.openemu.system.ngp")
        }
        if defaults.string(forKey: "defaultCore.openemu.system.saturn") == "org.openemu.Yabause" {
            defaults.removeObject(forKey: "defaultCore.openemu.system.saturn")
        }
        if defaults.string(forKey: "defaultCore.openemu.system.sms") == "org.openemu.CrabEmu" ||
           defaults.string(forKey: "defaultCore.openemu.system.sms") == "org.openemu.TwoMbit" {
            defaults.removeObject(forKey: "defaultCore.openemu.system.sms")
        }
        if defaults.string(forKey: "defaultCore.openemu.system.gc") == "org.openemu.Dolphin_Core" {
            defaults.removeObject(forKey: "defaultCore.openemu.system.gc")
        }
    }
    
    fileprivate func loadPlugins() {
        OEPlugin.registerPluginClass(OECorePlugin.self)
        OEPlugin.registerPluginClass(OESystemPlugin.self)
        
        // Register all system controllers with the bindings controller.
        for plugin in OESystemPlugin.allPlugins as! [OESystemPlugin] {
            OEBindingsController.register(plugin.controller)
        }
        
        let library = OELibraryDatabase.default!
        
        let context = library.mainThreadContext
        for plugin in OESystemPlugin.allPlugins as! [OESystemPlugin] {
            _ = OEDBSystem.init(for: plugin, in: context)
        }
        
        library.disableSystemsWithoutPlugin()
        try? library.mainThreadContext.save()
    }
    
    fileprivate func removeIncompatibleSaveStates() {
        
        let database = OELibraryDatabase.default!
        let context = database.mainThreadContext

        // Remove save states for deprecated core plugins.
        // Get incompatible save states by version.
        // Genesis Plus GX is especially known for breaking compatibility.
        let currentDesmumeCoreVersion = OECorePlugin(bundleIdentifier: "org.openemu.desmume")?.version
        let incompatibleSaveStates = (OEDBSaveState.allObjects(in: context) as! [OEDBSaveState]).filter {
            ($0.coreIdentifier == "org.openemu.CrabEmu" &&
                ($0.location!.contains("GameGear/") ||
                 $0.location!.contains("SegaMasterSystem/") ||
                 $0.location!.contains("SG-1000/"))) ||
            ($0.coreIdentifier == "org.openemu.desmume" && currentDesmumeCoreVersion == "0.9.12" &&
                ($0.coreVersion == "0.9.10" ||
                 $0.coreVersion == "0.9.10.1" ||
                 $0.coreVersion == "0.9.10.2" ||
                 $0.coreVersion == "0.9.11")) ||
            ($0.coreIdentifier == "org.openemu.GenesisPlus" &&
                ($0.coreVersion == "1.7.4" ||
                 $0.coreVersion == "1.7.4.1" ||
                 $0.coreVersion == "1.7.4.2" ||
                 $0.coreVersion == "1.7.4.3" ||
                 $0.coreVersion == "1.7.4.4" ||
                 $0.coreVersion == "1.7.4.5" ||
                 $0.coreVersion == "1.7.4.6")) ||
            ($0.coreIdentifier == "org.openemu.Mupen64Plus" &&
                ($0.coreVersion == "2.5.3" ||
                 $0.coreVersion == "2.5.2" ||
                 $0.coreVersion == "2.5.1" ||
                 $0.coreVersion == "2.5" ||
                 $0.coreVersion == "2.0.1" ||
                 $0.coreVersion == "2.0")) ||
            $0.coreIdentifier == "org.openemu.NeoPop" ||
            $0.coreIdentifier == "org.openemu.TwoMbit" ||
            $0.coreIdentifier == "org.openemu.VisualBoyAdvance" ||
            $0.coreIdentifier == "org.openemu.Dolphin_Core"
        }
        
        if !incompatibleSaveStates.isEmpty {
            
            os_log(.info, log: OE_LOG_DEFAULT, "Removing %d incompatible save states(s).", incompatibleSaveStates.count)
            
            for saveState in incompatibleSaveStates {
                saveState.deleteAndRemoveFiles()
            }
            
            try? context.save()
        }
    }
    
    fileprivate func setUpHIDSupport() {
        // Set up OEBindingsController.
        _ = OEBindingsController.self
        let dm = OEDeviceManager.shared
        if #available(macOS 10.15, *) {
            switch dm.accessType {
            case .unknown:
                dm.requestAccess()
                
            case .denied:
                DispatchQueue.main.async {
                    self.showInputMonitoringPermissionsAlert()
                }
                
            default:
                break
            }
        }
    }
    
    fileprivate func showInputMonitoringPermissionsAlert() {
        let alert = OEAlert()
        alert.messageText = NSLocalizedString("OpenEmu requires additional permissions", comment:"Headline for Input Monitoring permissions")
        alert.informativeText = NSLocalizedString("OpenEmu must be granted the Input Monitoring permission in order to use the keyboard as an input device.\n\nToggling the permission may also resolve keyboard input issues.", comment:"Message for Input Monitoring permissions")
        alert.defaultButtonTitle = NSLocalizedString("Open System Preferences", comment:"")
        alert.alternateButtonTitle = NSLocalizedString("Ignore", comment: "")

        let res = alert.runModal()
        if res == .alertFirstButtonReturn {
            NSWorkspace.shared.open(URL(string: "x-apple.systempreferences:com.apple.preference.security?Privacy_ListenEvent")!)
        }
    }
    
    // MARK: - Help Menu
    
    @IBAction func showOEHelp(_ sender: AnyObject?) {
        NSWorkspace.shared.open(URL(string: AppDelegate.userGuideAddress)!)
    }
    
    @IBAction func showOEReleaseNotes(_ sender: AnyObject?) {
        NSWorkspace.shared.open(URL(string: AppDelegate.releaseNotesAddress)!)
    }
    
    @IBAction func showOEWebSite(_ sender: AnyObject?) {
        NSWorkspace.shared.open(URL(string: AppDelegate.websiteAddress)!)
    }
    
    @IBAction func showOEIssues(_ sender: AnyObject?) {
        NSWorkspace.shared.open(URL(string: AppDelegate.feedbackAddress)!)
    }
    
    @IBAction func showOELocalizationGuide(_ sender: AnyObject?) {
        NSWorkspace.shared.open(URL(string: AppDelegate.localizationGuideAddress)!)
    }
    
    @IBAction func showOpenEmuWindow(_ sender: AnyObject?) {
        mainWindowController.showWindow(sender)
    }
    
    // MARK: - Preferences Window
    
    @IBAction func showPreferencesWindow(_ sender: AnyObject?) {
        preferencesWindowController.showWindow(nil)
    }
    
    // MARK: - Donation Link
    
    @IBAction func showDonationPage(_ sender: AnyObject?) {
        NSWorkspace.shared.open(URL(string: "http://openemu.org/donate/")!)
    }
    
    // MARK: - KVO
    
    override func observeValue(forKeyPath keyPath: String?, of object: Any?, change: [NSKeyValueChangeKey : Any]?, context: UnsafeMutableRawPointer?) {
        
        if context == &appearancePrefChangedKVOContext {
            let pref = OEAppearancePreferenceValue(rawValue: UserDefaults.standard.integer(forKey: OEAppearancePreferenceKey))
            switch pref {
                case .dark:
                    NSApp.appearance = NSAppearance(named: .darkAqua)
                case .light:
                    NSApp.appearance = NSAppearance(named: .aqua)
                default:
                    NSApp.appearance = nil
            }
        } else {
            super.observeValue(forKeyPath: keyPath, of: object, change: change, context: context)
        }
    }
    
    // MARK: - Migration
    
    @objc(migrationForceUpdateCores:)
    func migrationForceUpdateCores() throws {
        OECoreUpdater.shared.checkForUpdatesAndInstall()
    }
    
    @objc(migrationRemoveCoreDefaults:)
    func migrationRemoveCoreDefaults() throws {
        let defaults = UserDefaults.standard
        for key in defaults.dictionaryRepresentation().keys where key.contains("defaultCore.openemu.system.") {
            defaults.removeObject(forKey: key)
        }
    }
    
    // MARK: - Bindings Reset Message
    
    @objc(didRepairBindings:)
    func didRepairBindings(_ notif: NSNotification!) {
        let alert = OEAlert()
        alert.messageText = NSLocalizedString("An issue was detected with one of your controllers.", comment:"Headline for bindings repaired alert")
        alert.informativeText = NSLocalizedString("The button profile for one of your controllers does not match the profile detected the last time it was connected to OpenEmu. Some of the controls associated to the affected controller were reset.\n\nYou can go to the Controls preferences to check which associations were affected.", comment:"Message for bindings repaired alert")
        alert.defaultButtonTitle = NSLocalizedString("OK", comment:"")
        alert.runModal()
    }
    
    func didRegisterSystemPlugin(_ n: NSNotification!) {
        guard
            let plugin = n.object as? OESystemPlugin,
            let library = OELibraryDatabase.default
        else { return }
        
        _ = OEDBSystem.init(for: plugin, in: library.mainThreadContext)
    }
    
    // MARK: - Debug
    
    @IBAction func OEDebug_logResponderChain(_ sender: AnyObject?) {
        os_log(.info, log: OE_LOG_DEFAULT, "NSApp.KeyWindow: %{public}@", String(describing: NSApp.keyWindow))
        
        if let keyWindow = NSApp.keyWindow {
            
            var responderChain = [NSResponder]()
            var responder = keyWindow.firstResponder
            
            while let nextResponder = responder?.nextResponder {
                responderChain.append(nextResponder)
                responder = nextResponder
            }
            
            let output = responderChain.reduce("Responder Chain: ") { $0 + " -> \($1)" }
            
            os_log(.info, log: OE_LOG_DEFAULT, "%{public}@", output)
        }
    }
}

// MARK: - NSMenuDelegate

extension AppDelegate: NSMenuDelegate {
    
    func numberOfItems(in menu: NSMenu) -> Int {
        
        let database = OELibraryDatabase.default!
        
        guard let lastPlayedInfo = database.lastPlayedRomsBySystem, lastPlayedInfo.count > 0 else {
            cachedLastPlayedInfo.removeAll()
            return 1
        }
        
        let count = lastPlayedInfo.values.reduce(0) { $0 + $1.count }
        
        var lastPlayed = [CachedLastPlayedInfoItem]()
        lastPlayed.reserveCapacity(count)
        
        let sortedSystems = lastPlayedInfo.keys.sorted { return ($0 as NSString).localizedCaseInsensitiveCompare($1) == .orderedAscending }
        for system in sortedSystems {
            lastPlayed.append(system)
            lastPlayed = lastPlayed + lastPlayedInfo[system]!
        }
        
        cachedLastPlayedInfo = lastPlayed
        
        return count
    }
    
    func menu(_ menu: NSMenu, update item: NSMenuItem, at index: Int, shouldCancel: Bool) -> Bool {
        
        item.state = .off
        
        guard !cachedLastPlayedInfo.isEmpty else {
            item.title = NSLocalizedString("No game played yet!", comment: "")
            item.isEnabled = false
            item.indentationLevel = 0
            return true
        }
        
        switch cachedLastPlayedInfo[index] {
            
        case let string as String:
            item.title = string
            item.isEnabled = false
            item.indentationLevel = 0
            item.action = nil
            item.representedObject = nil
            
        case let rom as OEDBRom:
            guard let title = rom.game?.displayName else {
                return false
            }
            item.title = title
            item.isEnabled = true
            item.indentationLevel = 1
            item.target = mainWindowController
            item.action = #selector(OEMainWindowController.launchLastPlayedROM(_:))
            item.representedObject = rom
            
        default:
            break
        }
        
        return true
    }
    
    fileprivate func updateControlsMenu() {
        
        let saveStateSlotCount = 10
        
        let loadItemTag = 2
        let saveItemTag = 1
        
        let mainMenu = NSApp.mainMenu!
        let controlsItem = mainMenu.item(at: 4)
        let controlsMenu = controlsItem!.submenu!
        
        let currentLoadItem = controlsMenu.item(withTag: loadItemTag)!
        let currentSaveItem = controlsMenu.item(withTag: saveItemTag)!
        
        let useSlots = UserDefaults.standard.bool(forKey: OESaveStateUseQuickSaveSlotsKey)
        
        let newLoadItem: NSMenuItem
        let newSaveItem: NSMenuItem
        
        if useSlots {
            
            newLoadItem = NSMenuItem(title: NSLocalizedString("Quick Load", comment: "Quick Load Menu Item"), action: nil, keyEquivalent: "")
            newSaveItem = NSMenuItem(title: NSLocalizedString("Quick Save", comment: "Quick Save Menu Item"), action: nil, keyEquivalent: "")
            
            let loadMenu = NSMenu()
            let saveMenu = NSMenu()
            
            for i in 1..<saveStateSlotCount {
                
                let loadTitle = String(format: NSLocalizedString("Slot %ld", comment: "Slotted Quick Load Menu Item"), i)
                let saveTitle = String(format: NSLocalizedString("Slot %ld", comment: "Slotted Quick Save Menu Item"), i)
                
                let loadItem = NSMenuItem(title: loadTitle, action: #selector(OEGameDocument.quickLoad(_:)), keyEquivalent: "")
                let saveItem = NSMenuItem(title: saveTitle, action: #selector(OEGameDocument.quickSave(_:)), keyEquivalent: "")
                
                let keyEquivalent = "\(i)"
                loadItem.keyEquivalent = keyEquivalent
                loadItem.keyEquivalentModifierMask = [.shift, .command]
                saveItem.keyEquivalent = keyEquivalent
                saveItem.keyEquivalentModifierMask = [.command]
                
                loadItem.representedObject = keyEquivalent
                saveItem.representedObject = keyEquivalent
                
                loadMenu.addItem(loadItem)
                saveMenu.addItem(saveItem)
            }
            
            newLoadItem.submenu = loadMenu
            newSaveItem.submenu = saveMenu
            
        } else {
            
            newLoadItem = NSMenuItem(title: NSLocalizedString("Quick Load", comment: "Quick Load Menu Item"), action: #selector(OEGameDocument.quickLoad(_:)), keyEquivalent: "l")
            newLoadItem.representedObject = "0"
            newSaveItem = NSMenuItem(title: NSLocalizedString("Quick Save", comment: "Quick Save Menu Item"), action: #selector(OEGameDocument.quickSave(_:)), keyEquivalent: "s")
            newSaveItem.representedObject = "0"
        }
        
        newLoadItem.tag = loadItemTag
        newSaveItem.tag = saveItemTag
        
        var items = controlsMenu.items
        controlsMenu.removeAllItems()
        
        let loadItemIndex = items.firstIndex(of: currentLoadItem)!
        items[loadItemIndex] = newLoadItem
        
        let saveItemIndex = items.firstIndex(of: currentSaveItem)!
        items[saveItemIndex] = newSaveItem
        
        for item in items {
            controlsMenu.addItem(item)
        }
        
        controlsMenu.autoenablesItems = true
        controlsMenu.update()
    }
}

// MARK: - OpenEmuApplicationDelegateProtocol

@objc extension AppDelegate: OpenEmuApplicationDelegateProtocol {
    
    func applicationWillFinishLaunching(_ notification: Notification) {
        
        NSUserDefaultsController.shared.addObserver(self, forKeyPath: "values.".appending(OEAppearancePreferenceKey), options: [.initial], context: &appearancePrefChangedKVOContext)
        
        NotificationCenter.default.addObserver(self, selector: #selector(AppDelegate.removeLibraryDidLoadObserverForRestoreWindowFromNotificationCenter), name: NSApplication.didFinishRestoringWindowsNotification, object: nil)
    }
    
    func removeLibraryDidLoadObserverForRestoreWindowFromNotificationCenter(_ notification: Notification) {
        
        let notificationCenter = NotificationCenter.default
        
        if let observer = libraryDidLoadObserverForRestoreWindow {
            notificationCenter.removeObserver(observer)
            libraryDidLoadObserverForRestoreWindow = nil
        }
        
        if let observer = libraryDidLoadObserverForRestoreWindow2 {
            notificationCenter.removeObserver(observer)
            libraryDidLoadObserverForRestoreWindow2 = nil
        }
        
        notificationCenter.removeObserver(self, name: NSApplication.didFinishRestoringWindowsNotification, object: nil)
    }
    func applicationDidFinishLaunching(_ notification: Notification) {
        
        // Hide link to localization guide if localization for preferred language exists
        let preferredLanguage = NSLocale.preferredLanguages[0]
        for localization in Bundle.main.localizations {
            if preferredLanguage.contains(localization) && preferredLanguage.prefix(2) != "ru" {
                localizationGuideMenuItem.isHidden  = true
                break
            }
        }
        
        if NSClassFromString("NSTouchBar") != nil {
            // Get the “Customize Touch Bar…” menu to display in the View menu.
            NSApp.isAutomaticCustomizeTouchBarMenuItemEnabled = true
        }
        
        let notificationCenter = NotificationCenter.default
        
        notificationCenter.addObserver(self, selector: #selector(AppDelegate.libraryDatabaseDidLoad), name: .OELibraryDidLoadNotificationName, object: nil)
        notificationCenter.addObserver(self, selector: #selector(AppDelegate.openPreferencePane), name: PreferencesWindowController.openPaneNotificationName, object: nil)
        
        notificationCenter.addObserver(self, selector: #selector(AppDelegate.didRepairBindings), name: NSNotification.Name.OEBindingsRepaired, object: nil)
        
        notificationCenter.addObserver(self, selector: #selector(AppDelegate.didRegisterSystemPlugin), name: NSNotification.Name.OESystemPluginDidRegister, object: nil)
        
        NSDocumentController.shared.clearRecentDocuments(nil)
        
        validateDefaultPluginAssignments()
        
        DispatchQueue.main.async {
            self.loadDatabase()
        }
        
        /* Check if there is at least one metal-enabled device in the system.
         * Informs users that are using unsupported hacks for installing macOS on old machines,
         * so that we do not get issues like #4020 posted */
        let metalDevices = MTLCopyAllDevices()
        if metalDevices.count == 0 {
            let metalAlert = NSAlert()
            metalAlert.messageText = NSLocalizedString("Your Mac is too old for this version of OpenEmu", comment: "Message for the alert which appears when no Metal-enabled GPUs are detected")
            metalAlert.informativeText = NSLocalizedString("Since version 2.1, OpenEmu requires a GPU which supports Metal. Metal is available on all Macs that support macOS 10.14 or later.\n\nAs OpenEmu does not work with your current hardware/software configuration, it will now close.", comment: "Informative text for the alert which appears when no Metal-enabled GPUs are detected")
            metalAlert.addButton(withTitle: NSLocalizedString("Quit OpenEmu", comment: "Button title for the alert which appears when no Metal-enabled GPUs are detected"))
            metalAlert.runModal()
            NSApp.terminate(nil)
        }
    }
    
    func libraryDatabaseDidLoad(notification: Notification) {
        
        libraryLoaded = true

        loadPlugins()
        removeIncompatibleSaveStates()
        
        OECoreUpdater.shared.checkForUpdatesAndInstall()
        
        if !restoreWindow {
            _ = mainWindowController.window
        }
        
        // Remove the Open Recent menu item.
        let openDocumentMenuItemIndex = fileMenu.indexOfItem(withTarget: nil, andAction: #selector(NSDocumentController.openDocument(_:)))
        
        if openDocumentMenuItemIndex >= 0 && fileMenu.item(at: openDocumentMenuItemIndex + 1)!.hasSubmenu {
            fileMenu.removeItem(at: openDocumentMenuItemIndex + 1)
        }
        
        // Run Migration Manager.
        OEVersionMigrationController.default.runMigrationIfNeeded()
        
        // Set up HID support.
        setUpHIDSupport()
        
        // Replace quicksave/quickload items with menus if required.
        updateControlsMenu()
        
        // Preload shader plug-ins so HUD controls bar and gameplay preferences load faster.
        _ = OEShadersModel.shared
        
        if !restoreWindow {
            mainWindowController.showWindow(nil)
        }
        
        OECoreUpdater.shared.checkForNewCores(completionHandler: nil)   // TODO: check error from completion handler
        
        let userDefaultsController = NSUserDefaultsController.shared
        bind(.logHIDEvents, to: userDefaultsController, withKeyPath: "values.logsHIDEvents", options: nil)
        bind(.logKeyboardEvents, to: userDefaultsController, withKeyPath: "values.logsHIDEventsNoKeyboard", options: nil)
        bind(.backgroundControllerPlay, to: userDefaultsController, withKeyPath: "values.backgroundControllerPlay", options: nil)
        
        NotificationCenter.default.addObserver(self, selector: #selector(AppDelegate.windowDidBecomeKey), name: NSWindow.didBecomeKeyNotification, object: nil)

        NotificationCenter.default.addObserver(self, selector: #selector(AppDelegate.deviceManagerDidChangeGlobalEventMonitor(_:)), name: NSNotification.Name.OEDeviceManagerDidAddGlobalEventMonitorHandler, object: nil)
        NotificationCenter.default.addObserver(self, selector: #selector(AppDelegate.deviceManagerDidChangeGlobalEventMonitor(_:)), name: NSNotification.Name.OEDeviceManagerDidRemoveGlobalEventMonitorHandler, object: nil)
        NotificationCenter.default.addObserver(self, selector: #selector(AppDelegate.gameDocumentSetupDidFinish(_:)), name: .OEGameDocumentSetupDidFinish, object: nil)

        for startupQueueItem in startupQueue {
            startupQueueItem()
        }
        startupQueue.removeAll()
    }
    
    func openPreferencePane(_ notification: Notification) {
        preferencesWindowController.showWindow(with: notification)
    }
    
    func applicationShouldOpenUntitledFile(_ sender: NSApplication) -> Bool {
        if libraryLoaded {
            mainWindowController.showWindow(self)
        } else {
            startupQueue.append({ [unowned self] in
                self.mainWindowController.showWindow(self)
            })
        }
        return false
    }
    
    func application(_ sender: NSApplication, openFiles filenames: [String]) {
        
        guard UserDefaults.standard.bool(forKey: OESetupAssistantHasFinishedKey) else {
            NSApp.reply(toOpenOrPrint: .cancel)
            return
        }
        
        let block: StartupQueueClosure = {
            
            if filenames.count == 1 {
                
                let url = URL(fileURLWithPath: filenames.last!)
                self.documentController.openDocument(withContentsOf: url, display: true, completionHandler: { (document, documentWasAlreadyOpen, error) in
                    if error != nil {
                        self.documentController.presentError(error!)
                    }
                    NSApp.reply(toOpenOrPrint: document != nil ? .success : .failure)
                })
                
            } else {
                
                let reply: NSApplication.DelegateReply
                let importer = OELibraryDatabase.default!.importer
                if importer.importItems(atPaths: filenames) {
                    reply = .success
                } else {
                    reply = .failure
                }
                
                NSApp.reply(toOpenOrPrint: reply)
            }
        }
        
        if libraryLoaded {
            block()
        } else {
            startupQueue.append(block)
        }
    }
    
    func gameDocumentSetupDidFinish(_ notification: Notification) {
        updateEventHandlers()
    }
    
    @objc func deviceManagerDidChangeGlobalEventMonitor(_ notification: Notification) {
        updateEventHandlers()
    }
    
    func applicationDidResignActive(_ notification: Notification) {
        updateEventHandlers()
    }
    
    func applicationDidBecomeActive(_ notification: Notification) {
        updateEventHandlers()
    }
    
    @objc func windowDidBecomeKey(notification: Notification) {
        updateEventHandlers()
    }
    
    func spotlightStatusDidChange(for application: OpenEmuApplication) {
        updateEventHandlers()
    }
    
    func application(_ application: OpenEmuApplication, didBeginModalSessionForWindow window: NSWindow) {
        updateEventHandlers()
    }
    
    fileprivate var shouldHandleControllerEvents: Bool {
        if NSApp.modalWindow != nil {
            return false
        }

        if OEDeviceManager.shared.hasEventMonitor {
            return false
        }

        if OEApp.isSpotlightFrontmost {
            return false
        }

        if NSApp.isActive {
            return true
        }

        return backgroundControllerPlay
    }

    fileprivate var shouldHandleKeyboardEvents: Bool {
        if NSApp.modalWindow != nil {
            return false
        }

        if OEApp.isSpotlightFrontmost {
            return false
        }

        return NSApp.isActive
    }
    
    func updateEventHandlers() {
        let games = NSApp.orderedDocuments.compactMap({$0 as? OEGameDocument})
        guard games.count > 0 else { return }

        if let game = games.first {
            game.handleEvents = shouldHandleControllerEvents
            game.handleKeyboardEvents = shouldHandleKeyboardEvents
        }

        games.dropFirst().forEach {
            $0.handleEvents = false
            $0.handleKeyboardEvents = false
        }
    }
}
