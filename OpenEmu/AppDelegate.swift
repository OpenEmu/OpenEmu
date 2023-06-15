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
import OpenEmuSystem
import OpenEmuKit

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
    
    @IBOutlet weak var fileMenu: NSMenu!
    @IBOutlet weak var helpMenu: NSMenu!
    
    lazy var mainWindowController = MainWindowController(windowNibName: "MainWindow")
    
    lazy var preferencesWindowController = PreferencesWindowController(windowNibName: "Preferences")
    
    var documentController = GameDocumentController.shared
    
    var restoreWindow = false
    var libraryDidLoadObserverForRestoreWindow: NSObjectProtocol?
    
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
                        os_log(.info, log: .event_hid, "%{public}@", event)
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
                        os_log(.info, log: .event_keyboard, "%{public}@", event)
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
        let libraryDirectory = URL.oeApplicationSupportDirectory
            .appendingPathComponent("Game Library", isDirectory: true)
        let path = (libraryDirectory.path as NSString).abbreviatingWithTildeInPath
        
        #if !DEBUG
            UserDefaults.standard.removeObject(forKey: OEGameCoreManagerModePreferenceKey)
        #endif
        
        // Register defaults.
        UserDefaults.standard.register(defaults: [
            OELibraryDatabase.defaultDatabasePathKey: path,
            OELibraryDatabase.databasePathKey: path,
            OECopyToLibraryKey: true,
            OEAutomaticallyGetInfoKey: true,
            OEGameVolumeKey: 0.5,
            "defaultCore.openemu.system.nes": "org.openemu.Nestopia",
            "defaultCore.openemu.system.snes": "org.openemu.SNES9x",
            OEDBGame.displayGameTitleKey: true,
            OEBackgroundPauseKey: true,
            OEBackgroundControllerPlayKey: true,
            "logsHIDEvents": false,
            "logsHIDEventsNoKeyboard": false,
            OEDBSaveStatesMedia.showsAutoSavesKey: true,
            OEDBSaveStatesMedia.showsQuickSavesKey: true,
            OEForcePopoutGameWindowKey: true,
            OEPopoutGameWindowIntegerScalingOnlyKey: true,
            OEGameLayerNotificationView.OEShowNotificationsKey : true,
            OEAppearance.Application.key: OEAppearance.Application.dark.rawValue,
            OEAppearance.HUDBar.key: OEAppearance.HUDBar.vibrant.rawValue,
            OEAppearance.ControlsPrefs.key: OEAppearance.ControlsPrefs.wood.rawValue,
            OEGameCoreManagerModePreferenceKey: NSStringFromClass(OEXPCGameCoreManager.self),
        ])
        
        // Don't let an old setting override automatically checking for app updates.
        if let automaticChecksEnabled = UserDefaults.standard.object(forKey: "SUEnableAutomaticChecks") as? Bool, automaticChecksEnabled == false {
            UserDefaults.standard.removeObject(forKey: "SUEnableAutomaticChecks")
        }

        // Trigger Objective-C +initialize methods in these classes.
        _ = OEControllerDescription.self
        
        OECorePlugin.registerClass()
        OESystemPlugin.registerClass()
        
        // Reset preferences for default cores when migrating to 2.0.3. This is an attempt at cleanup after 9d5d696d07fe651f44f16f8bf8b98c87d90fe53f and d36e9ad4b7097f21ffbbe32d9cea3b72a390bc0f and for getting as many users as possible onto mGBA.
        OEVersionMigrationController.default.addMigratorTarget(self, selector: #selector(migrationRemoveCoreDefaults), forVersion: "2.0.3")
    }
    
    required init?(coder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }
    
    deinit {
        NSUserDefaultsController.shared.removeObserver(self, forKeyPath: "values.\(OEAppearance.Application.key)", context: &appearancePrefChangedKVOContext)
    }
    
    // MARK: - Library Database
    
    func loadDatabase() {
        
        let defaults = UserDefaults.standard
        
        let defaultDatabasePath = (defaults.string(forKey: OELibraryDatabase.defaultDatabasePathKey)! as NSString).expandingTildeInPath
        let databasePath: String
        if let databasePathDefault = defaults.string(forKey: OELibraryDatabase.databasePathKey) {
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
                NotificationCenter.default.post(name: .libraryDidLoad, object: OELibraryDatabase.default!)
            }
            
        } catch {
            
            if (error as? CocoaError)?.code == .persistentStoreIncompatibleVersionHash {
                
                let migrator = LibraryMigrator(storeURL: url)
                
                DispatchQueue.global(qos: .default).async {
                    
                    do {
                        
                        try migrator.runMigration()
                        
                        self.loadDatabaseAsynchronously(from: url, createIfNecessary: createIfNecessary)
                        
                    } catch {
                        
                        DLog("Your library can't be opened with this version of OpenEmu.")
                        DLog("\(error)")
                        
                        DispatchQueue.main.async {
                            
                            if (error as NSError).code == 0 { // Foundation._GenericObjCError.nilError
                                let alert = NSAlert()
                                alert.alertStyle = .critical
                                alert.messageText = NSLocalizedString("Your library can’t be opened with this version of OpenEmu.", comment: "")
                                alert.runModal()
                            } else {
                                NSAlert(error: error).runModal()
                            }
                            
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
            openPanel.allowedFileTypes = [OELibraryDatabase.databaseFileExtension]
            openPanel.canChooseDirectories = false
            openPanel.allowsMultipleSelection = false
            
            openPanel.begin { result in
                
                if result == .OK {
                    
                    let databaseURL = openPanel.url!.deletingLastPathComponent()
                    
                    self.loadDatabaseAsynchronously(from: databaseURL, createIfNecessary: false)
                    
                } else {
                    openPanel.orderOut(nil)
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
        // Register all system controllers with the bindings controller.
        for plugin in OESystemPlugin.allPlugins {
            OEBindingsController.register(plugin.controller)
        }
        
        let library = OELibraryDatabase.default!
        
        let context = library.mainThreadContext
        for plugin in OESystemPlugin.allPlugins {
            _ = OEDBSystem.system(for: plugin, in: context)
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
        let currentDesmumeCoreVersion = OECorePlugin.corePlugin(bundleIdentifier: "org.openemu.desmume")?.version
        let incompatibleSaveStates = context.allObjects(ofType: OEDBSaveState.self).filter {
            ($0.coreIdentifier == "org.openemu.CrabEmu" &&
                ($0.location.contains("GameGear/") ||
                 $0.location.contains("SegaMasterSystem/") ||
                 $0.location.contains("SG-1000/"))) ||
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
            
            os_log(.info, log: .default, "Removing %d incompatible save states(s).", incompatibleSaveStates.count)
            
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
        #if DEBUG
        if UserDefaults.standard.bool(forKey: "pleaseDoNotAnnoyMeWithThePermissionsAlertEveryTimeIRunThisAppFromXcode") {
            return
        }
        #endif
        let alert = OEAlert()
        alert.messageText = NSLocalizedString("ALERT_INPUT_MONITORING_HEADLINE", comment:"Headline for Input Monitoring permissions")
        var informativeText = NSLocalizedString("ALERT_INPUT_MONITORING_PART1", comment:"Message for Input Monitoring permissions")
        informativeText += "\n\n"
        if #available(macOS 12.0, *) {
            informativeText += NSLocalizedString("ALERT_INPUT_MONITORING_PART2_MONTEREY", comment:"Message for Input Monitoring permissions")
        } else {
            informativeText += NSLocalizedString("ALERT_INPUT_MONITORING_PART2", comment:"Message for Input Monitoring permissions")
        }
        alert.informativeText = informativeText
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
    
    @IBAction func showAppSupportFolder(_ sender: AnyObject?) {
        NSWorkspace.shared.open(.oeApplicationSupportDirectory)
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
            switch OEAppearance.application {
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
        CoreUpdater.shared.checkForUpdatesAndInstall()
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
    
    // MARK: - Debug
    
    @IBAction func OEDebug_logResponderChain(_ sender: AnyObject?) {
        os_log(.info, log: .default, "NSApp.KeyWindow: %{public}@", String(describing: NSApp.keyWindow))
        
        if let keyWindow = NSApp.keyWindow {
            
            var responderChain = [NSResponder]()
            var responder = keyWindow.firstResponder
            
            while let nextResponder = responder?.nextResponder {
                responderChain.append(nextResponder)
                responder = nextResponder
            }
            
            let output = responderChain.reduce("Responder Chain: ") { $0 + " -> \($1)" }
            
            os_log(.info, log: .default, "%{public}@", output)
        }
    }
}

// MARK: - NSMenuDelegate

extension AppDelegate: NSMenuDelegate {
    
    func numberOfItems(in menu: NSMenu) -> Int {
        guard let database = OELibraryDatabase.default else {
            return 0
        }
        
        let lastPlayedInfo = database.lastPlayedRomsBySystem
        if lastPlayedInfo.isEmpty {
            cachedLastPlayedInfo.removeAll()
            return 1
        }
        
        let count = lastPlayedInfo.values.reduce(0) { $0 + 1 /*system name*/ + $1.count }
        
        var lastPlayed = [CachedLastPlayedInfoItem]()
        lastPlayed.reserveCapacity(count)
        
        let sortedSystems = lastPlayedInfo.keys.sorted { $0.localizedCaseInsensitiveCompare($1) == .orderedAscending }
        for system in sortedSystems {
            lastPlayed.append(system)
            lastPlayed += lastPlayedInfo[system]!
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
            item.action = #selector(MainWindowController.launchLastPlayedROM(_:))
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
        
        let useSlots = UserDefaults.standard.bool(forKey: OEDBSaveState.useQuickSaveSlotsKey)
        
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
                
                loadItem.representedObject = i
                saveItem.representedObject = i
                
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
        atexit {
            // Always remove the XPC broker registered with launchd.
            // If the app was executed under App Translocation,
            // the broker may refer to an invalid path, so this ensures
            // it is re-registered on next launch.
            try? LaunchControl.remove(service: "org.openemu.broker")
        }
        
        if AppMover.isAppQuarantined() {
            AppMover.removeQuarantineAttribute()
            // TODO: trigger a relaunch?
        }
        AppMover.moveIfNecessary()
        
        NSUserDefaultsController.shared.addObserver(self, forKeyPath: "values.\(OEAppearance.Application.key)", options: [.initial], context: &appearancePrefChangedKVOContext)
        
        NotificationCenter.default.addObserver(self, selector: #selector(removeLibraryDidLoadObserverForRestoreWindowFromNotificationCenter), name: NSApplication.didFinishRestoringWindowsNotification, object: nil)
    }
    
    func removeLibraryDidLoadObserverForRestoreWindowFromNotificationCenter(_ notification: Notification) {
        
        let notificationCenter = NotificationCenter.default
        
        if let observer = libraryDidLoadObserverForRestoreWindow {
            notificationCenter.removeObserver(observer)
            libraryDidLoadObserverForRestoreWindow = nil
        }
        
        notificationCenter.removeObserver(self, name: NSApplication.didFinishRestoringWindowsNotification, object: nil)
    }
    func applicationDidFinishLaunching(_ notification: Notification) {
        
        // Get the “Customize Touch Bar…” menu to display in the View menu.
        NSApp.isAutomaticCustomizeTouchBarMenuItemEnabled = true
        
        #if DEBUG
        helpMenu.addItem(withTitle: "Show Application Support Folder in Finder", action: #selector(showAppSupportFolder), keyEquivalent: "")
        #endif
        
        let notificationCenter = NotificationCenter.default
        
        notificationCenter.addObserver(self, selector: #selector(libraryDatabaseDidLoad), name: .libraryDidLoad, object: nil)
        notificationCenter.addObserver(self, selector: #selector(openPreferencePane), name: PreferencesWindowController.openPaneNotificationName, object: nil)
        
        notificationCenter.addObserver(self, selector: #selector(didRepairBindings), name: .OEBindingsRepaired, object: nil)
        
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
        
        CoreUpdater.shared.checkForUpdatesAndInstall()
        
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
        _ = OEShaderStore.shared
        
        if !restoreWindow {
            mainWindowController.showWindow(nil)
        }
        
        CoreUpdater.shared.checkForNewCores()   // TODO: check error from completion handler
        
        let userDefaultsController = NSUserDefaultsController.shared
        bind(.logHIDEvents, to: userDefaultsController, withKeyPath: "values.logsHIDEvents", options: nil)
        bind(.logKeyboardEvents, to: userDefaultsController, withKeyPath: "values.logsHIDEventsNoKeyboard", options: nil)
        bind(.backgroundControllerPlay, to: userDefaultsController, withKeyPath: "values.backgroundControllerPlay", options: nil)
        
        NotificationCenter.default.addObserver(self, selector: #selector(windowDidBecomeKey), name: NSWindow.didBecomeKeyNotification, object: nil)

        NotificationCenter.default.addObserver(self, selector: #selector(deviceManagerDidChangeGlobalEventMonitor(_:)), name: .OEDeviceManagerDidAddGlobalEventMonitorHandler, object: nil)
        NotificationCenter.default.addObserver(self, selector: #selector(deviceManagerDidChangeGlobalEventMonitor(_:)), name: .OEDeviceManagerDidRemoveGlobalEventMonitorHandler, object: nil)
        NotificationCenter.default.addObserver(self, selector: #selector(gameDocumentSetupDidFinish(_:)), name: .OEGameDocumentSetupDidFinish, object: nil)

        for startupQueueItem in startupQueue {
            startupQueueItem()
        }
        startupQueue.removeAll()
    }
    
    func openPreferencePane(_ notification: Notification) {
        preferencesWindowController.showWindow(with: notification)
    }
    
    func applicationShouldHandleReopen(_ sender: NSApplication, hasVisibleWindows flag: Bool) -> Bool {
        
        for window in sender.windows {
            
            if window.windowController is MainWindowController,
               !window.isVisible,
               sender.mainWindow == nil {
                
                window.makeKeyAndOrderFront(self)
                return false
            }
        }
        return true
    }
    
    func applicationShouldOpenUntitledFile(_ sender: NSApplication) -> Bool {
        if libraryLoaded {
            mainWindowController.showWindow(self)
        } else {
            startupQueue.append { [unowned self] in
                self.mainWindowController.showWindow(self)
            }
        }
        return false
    }
    
    // NOTE: When using ‘application:openURLs:’, “Document URLs that have an associated NSDocument class
    // will be opened through NSDocumentController.”, thereby bypassing the startupQueue.
    // FIXME: Handle ´PluginDocument´s
    func application(_ sender: NSApplication, openFiles filenames: [String]) {
        let urls = filenames.compactMap { URL(fileURLWithPath: $0) }
        
        guard UserDefaults.standard.bool(forKey: SetupAssistant.hasFinishedKey) else {
            NSApp.reply(toOpenOrPrint: .cancel)
            return
        }
        
        let block: StartupQueueClosure = {
            
            if urls.count == 1 {
                
                let url = urls.first!
                self.documentController.openDocument(withContentsOf: url, display: true) { (document, documentWasAlreadyOpen, error) in
                    if error != nil {
                        self.documentController.presentError(error!)
                    }
                    NSApp.reply(toOpenOrPrint: document != nil ? .success : .failure)
                }
                
            } else {
                
                let reply: NSApplication.DelegateReply
                let importer = OELibraryDatabase.default!.importer
                if importer.importItems(at: urls) {
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
        let block = {
            let games = NSApp.orderedDocuments.compactMap({$0 as? OEGameDocument})
            guard games.count > 0 else { return }
            
            if let game = games.first {
                game.handleEvents = self.shouldHandleControllerEvents
                game.handleKeyboardEvents = self.shouldHandleKeyboardEvents
            }
            
            games.dropFirst().forEach {
                $0.handleEvents = false
                $0.handleKeyboardEvents = false
            }
        }
        
        if #available(macOS 13.0, *) {
            // Workaround; the documents in NSApp.orderedDocuments are not yet in the correct order on Ventura,
            // so when switching between game documents, the previous one would be chosen to handle input.
            DispatchQueue.main.async(execute: block)
        } else {
            block()
        }
    }
}
