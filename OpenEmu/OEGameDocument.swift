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
import IOKit.pwr_mgt
import OpenEmuBase
import OpenEmuSystem
import OpenEmuKit

let OEGameVolumeKey = "volume"
let OEGameCoreDisplayModeKeyFormat = "displayMode.%@"
let OEBackgroundPauseKey = "backgroundPause"
let OEBackgroundControllerPlayKey = "backgroundControllerPlay"
let OETakeNativeScreenshots = "takeNativeScreenshots"

let OEScreenshotFileFormatKey = "screenshotFormat"
let OEScreenshotPropertiesKey = "screenshotProperties"
let OEScreenshotAspectRatioCorrectionDisabled = "disableScreenshotAspectRatioCorrection"

let OEGameCoreManagerModePreferenceKey = "OEGameCoreManagerModePreference"

@objc
final class OEGameDocument: NSDocument {
    
    private enum EmulationStatus: UInt {
        /// The current `OEGameCoreManager` has not been instantiated yet,
        /// or it has been deallocated because emulation has terminated
        case notSetup
        /// The `OEGameCoreManager` is ready, but the emulation was not started for
        /// the first time yet
        case setup
        /// The emulation has been requested to start
        case starting
        ///
        case playing
        ///
        case paused
        /// After emulation stops, but before `OEGameCoreManager` is deallocated
        case terminating
    }
    
    enum Errors: LocalizedError, CustomNSError {
        case fileDoesNotExist
        case noCore
        case noCoreForSystem
        case noCoreForSaveState
        case importRequired
        case couldNotLoadROM
        case gameCoreCrashed(OECorePlugin, String?, NSError)
        case invalidSaveState
        
        var errorDescription: String? {
            if case .fileDoesNotExist = self {
                return NSLocalizedString("The file you selected doesn't exist", comment: "Inexistent file error reason.")
            } else if case .noCore = self {
                return NSLocalizedString("OpenEmu could not find a Core to launch the game", comment: "No Core error reason.")
            } else {
                return nil
            }
        }
        
        var recoverySuggestion: String? {
            if case .fileDoesNotExist = self {
                return NSLocalizedString("Choose a valid file.", comment: "Inexistent file error recovery suggestion.")
            } else if case .noCore = self {
                return NSLocalizedString("Make sure your internet connection is active and download a suitable core.", comment: "No Core error recovery suggestion.")
            } else {
                return nil
            }
        }
        
        static var errorDomain: String { "org.openemu.OpenEmu.OEGameDocument" }
        
        var errorCode: Int {
            switch self {
            case .fileDoesNotExist:
                return 1
            case .noCore:
                return 2
            case .noCoreForSystem:
                return 7
            case .noCoreForSaveState:
                return 8
            case .importRequired:
                return 9
            case .couldNotLoadROM:
                return 10
            case .gameCoreCrashed(_, _, _):
                return 11
            case .invalidSaveState:
                return 12
            }
        }
    }
    
    private static let initializeDefaults: Void = {
        UserDefaults.standard.register(defaults: [
            OEScreenshotFileFormatKey : NSBitmapImageRep.FileType.png.rawValue,
            OEScreenshotPropertiesKey : [NSBitmapImageRep.PropertyKey : Any](),
        ])
    }()
    
    @objc //OEPopoutGameWindowController
    private(set) var rom: OEDBRom!
    private(set) var romFileURL: URL!
    private(set) var corePlugin: OECorePlugin!
    private(set) var systemPlugin: OESystemPlugin!
    
    @objc //OEPopoutGameWindowController
    private(set) var gameViewController: GameViewController!
    
    private var gameCoreManager: OEGameCoreManager!
    
    private var displaySleepAssertionID: IOPMAssertionID = 0
    
    private var emulationStatus: EmulationStatus = .notSetup
    private var saveStateForGameStart: OEDBSaveState?
    private var lastPlayStartDate: Date?
    private var lastSelectedDisplayModeOption: String?
    private var isMuted = false
    private var pausedByGoingToBackground = false
    private var coreDidTerminateSuddenly = false
    /// Indicates whether the document is currently moving from the main window into a separate popout window.
    private var isUndocking = false
    
    private var romPath: String?
    /// Track if ROM was decompressed.
    private var romDecompressed = false
    
    var coreIdentifier: String! {
        return gameCoreManager.plugin?.bundleIdentifier
    }
    
    var systemIdentifier: String! {
        return systemPlugin.systemIdentifier
    }
    
    var gameCoreHelper: OEGameCoreHelper! {
        return gameCoreManager
    }
    
    private var _gameWindowController: NSWindowController?
    var gameWindowController: NSWindowController? {
        get {
            return _gameWindowController
        }
        set {
            if _gameWindowController == newValue {
                return
            }
            
            if let gameWindowController = _gameWindowController {
                removeObservers(for: gameWindowController)
                removeWindowController(gameWindowController)
            }
            
            _gameWindowController = newValue
            
            if let gameWindowController = _gameWindowController {
                addWindowController(gameWindowController)
                addObservers(for: gameWindowController)
            }
        }
    }
    
    private var _handleEvents = false
    var handleEvents: Bool {
        get {
            return _handleEvents
        }
        set {
            if _handleEvents == newValue {
                return
            }
            
            _handleEvents = newValue
            gameCoreManager?.setHandleEvents(newValue)
        }
    }
    
    private var _handleKeyboardEvents = false
    var handleKeyboardEvents: Bool {
        get {
            return _handleKeyboardEvents
        }
        set {
            if _handleKeyboardEvents == newValue {
                return
            }
            
            _handleKeyboardEvents = newValue
            gameCoreManager?.setHandleKeyboardEvents(newValue)
        }
    }
    
    convenience init(rom: OEDBRom, core: OECorePlugin?) throws {
        self.init()
        do {
            try setUpDocument(with: rom, using: core)
        } catch {
            throw error
        }
    }
    
    convenience init(game: OEDBGame, core: OECorePlugin?) throws {
        self.init()
        do {
            try setUpDocument(with: game.defaultROM, using: core)
        } catch {
            throw error
        }
    }
    
    convenience init(saveState state: OEDBSaveState) throws {
        self.init()
        do {
            try setUpDocument(with: state)
        } catch {
            throw error
        }
    }
    
    private func setUpDocument(with saveState: OEDBSaveState) throws {
        do {
            try setUpDocument(with: saveState.rom!, using: OECorePlugin(bundleIdentifier: saveState.coreIdentifier))
            saveStateForGameStart = saveState
        } catch {
            throw error
        }
    }
    
    private func setUpDocument(with rom: OEDBRom, using core: OECorePlugin?) throws {
        Self.initializeDefaults
        
        var fileURL = rom.url
        
        // Check if local file is available
        let isReachable = try? fileURL?.checkResourceIsReachable()
        if isReachable != true {
            fileURL = nil
            let sourceURL = rom.sourceURL
            
            // try to fallback on external source
            if let sourceURL = sourceURL {
                let name = (rom.fileName != nil) ? rom.fileName! : (sourceURL.lastPathComponent as NSString).deletingPathExtension
                
                if OEAlert.romDownloadRequired(name: name).runModal() == .alertFirstButtonReturn {
                    
                    var destination: URL?
                    var error: NSError?
                    
                    let alert = OEAlert()
                    alert.messageText = String(format: NSLocalizedString("Downloading %@…", comment: "Downloading rom message text"), name)
                    alert.defaultButtonTitle = NSLocalizedString("Cancel", comment: "")
                    alert.showsProgressbar = true
                    alert.progress = -1
                    
                    alert.performBlockInModalSession {
                        let download = Download(url: sourceURL)
                        download.progressHandler = { progress in
                            alert.progress = Double(progress)
                            return true
                        }
                        
                        download.completionHandler = { dst, err in
                            destination = dst
                            if let err = err {
                                error = err as NSError
                            }
                            alert.close(withResult: .alertSecondButtonReturn)
                        }
                        
                        download.start()
                    }
                    
                    if alert.runModal() == .alertFirstButtonReturn || error?.code == NSUserCancelledError {
                        // User canceld
                        let error = NSError(domain: NSCocoaErrorDomain, code: NSUserCancelledError, userInfo: nil)
                        throw error
                    }
                    else {
                        if error != nil || destination == nil {
                            throw error!
                        }
                        
                        fileURL = destination
                        // make sure that rom's fileName is set
                        if rom.fileName == nil {
                            rom.fileName = destination?.lastPathComponent
                            rom.save()
                        }
                    }
                }
                else {
                    // User canceld
                    let error = NSError(domain: NSCocoaErrorDomain, code: NSUserCancelledError, userInfo: nil)
                    throw error
                }
            }
            
            // check if we have recovered
            let isReachable = try? fileURL?.checkResourceIsReachable()
            if fileURL == nil || isReachable != true {
                DLog("File does not exist")
                throw Errors.fileDoesNotExist
            }
        }
        
        self.rom = rom
        romFileURL = fileURL
        corePlugin = core
        systemPlugin = rom.game?.system?.plugin
        
        if corePlugin == nil {
            corePlugin = try? self.core(forSystem: systemPlugin)
        }
        
        if corePlugin == nil {
            
            var nsError: NSError?
            CoreUpdater.shared.installCore(for: rom.game!) { plugin, error in
                if error == nil,
                   let plugin = plugin {
                    self.corePlugin = plugin
                }
                else if let error = error as NSError?,
                        error.domain == NSCocoaErrorDomain,
                        error.code == NSUserCancelledError {
                    nsError = error
                }
            }
            
            if let error = nsError {
                throw error
            }
        }
        
        gameCoreManager = newGameCoreManager(with: corePlugin)
        gameViewController = GameViewController(document: self)
    }
    
    deinit {
        if let url = romFileURL, url != rom.url {
            try? FileManager.default.removeItem(at: url)
        }
    }
    
    // MARK: - NSDocument
    
    override var description: String {
        return "<\(Self.self) \(Unmanaged.passUnretained(self).toOpaque()), ROM: '\(rom?.game?.displayName ?? "nil")', System: '\(systemPlugin?.systemIdentifier ?? "nil")', Core: '\(corePlugin?.bundleIdentifier ?? "nil")'>"
    }
    
    override var displayName: String! {
        get {
            // If we do not have a title yet, return an empty string instead of super.displayName.
            // The latter uses Cocoa document architecture and relies on documents having URLs,
            // including untitled (new) documents.
            var displayName = rom.game?.displayName ?? ""
            #if DEBUG
            displayName.append(" (DEBUG BUILD)")
            #endif
            return displayName
        }
        set {
            super.displayName = newValue
        }
    }
    
    /// Returns `true` if emulation is running or paused to prevent OpenEmu from quitting without warning/saving if the user attempts to quit the app during gameplay;
    /// returns `false` while undocking to prevent an ‘unsaved’ indicator from appearing inside the new popout window’s close button.
    override var isDocumentEdited: Bool {
        if isUndocking {
            return false
        }
        return emulationStatus == .playing || emulationStatus == .paused
    }
    
    override func data(ofType typeName: String) throws -> Data {
        DLog("\(typeName)")
        throw NSError(domain: NSOSStatusErrorDomain, code: unimpErr, userInfo: nil)
    }
    
    override func read(from url: URL, ofType typeName: String) throws {
        DLog("\(url)")
        DLog("\(typeName)")
        
        if typeName == "org.openemu.savestate" {
            let context = OELibraryDatabase.default!.mainThreadContext
            guard let state = OEDBSaveState.updateOrCreateState(with: url, in: context) else {
                // TODO: Specify failure reason and add recovery suggestion
                DLog("Save state is invalid")
                throw Errors.invalidSaveState
            }
            
            do {
                try setUpDocument(with: state)
            } catch {
                throw error
            }
            
            return
        }
        
        if !FileManager.default.fileExists(atPath: url.path) {
            DLog("File does not exist")
            throw Errors.fileDoesNotExist
        }
        
        if !url.isFileURL {
            // TODO: Handle URLs, by downloading to temp folder
            DLog("URLs that are not file urls are currently not supported!")
            return
        }
        
        let game: OEDBGame
        do {
            game = try OEDBGame(url: url, in: OELibraryDatabase.default!)
        } catch {
            // Could not find game in database. Try to import the file
            let importer = OELibraryDatabase.default!.importer
            let completion: OEImportItemCompletionBlock = { romID in
                guard let romID = romID else {
                    // import probably failed
                    return
                }
                
                let fileName = (url.lastPathComponent as NSString).deletingPathExtension
                let informativeText = String(format: NSLocalizedString("The game '%@' was imported.", comment: ""), fileName)
                
                let alert = OEAlert()
                alert.messageText = NSLocalizedString("Your game finished importing, do you want to play it now?", comment: "")
                alert.informativeText = informativeText
                alert.defaultButtonTitle = NSLocalizedString("Play Game", comment: "")
                alert.alternateButtonTitle = NSLocalizedString("Cancel", comment: "")
                
                if alert.runModal() == .alertFirstButtonReturn {
                    let context = OELibraryDatabase.default!.mainThreadContext
                    let rom = OEDBRom.object(with: romID, in: context)
                    
                    // Ugly hack to start imported games in main window
                    let mainWindowController = (NSApp.delegate as! AppDelegate).mainWindowController
                    if !mainWindowController.mainWindowRunsGame {
                        mainWindowController.startGame(rom.game)
                    } else {
                        if let url = rom.url {
                            NSDocumentController.shared.openDocument(withContentsOf: url, display: false) { document, documentWasAlreadyOpen, error in
                            }
                        }
                    }
                }
            }
            
            if importer.importItem(at: url, withCompletionHandler: completion) {
                throw Errors.importRequired
            }
            
            throw error
        }
        
        if let state = game.autosaveForLastPlayedRom,
           OEAlert.loadAutoSaveGame().runModal() == .alertFirstButtonReturn {
            do {
                try setUpDocument(with: state)
            } catch {
                throw error
            }
        } else {
            do {
                // TODO: Load rom that was just imported instead of the default one
                try setUpDocument(with: game.defaultROM, using: nil)
            } catch {
                throw error
            }
        }
    }
    
    override func canClose(withDelegate delegate: Any, shouldClose shouldCloseSelector: Selector?, contextInfo: UnsafeMutableRawPointer?) {
        if emulationStatus == .notSetup || emulationStatus == .terminating {
            super.canClose(withDelegate: delegate, shouldClose: shouldCloseSelector, contextInfo: contextInfo)
            return
        }
        
        pauseEmulationIfNeeded()
        
        if !shouldTerminateEmulation {
            let shouldClose = {
                guard let shouldCloseSelector = shouldCloseSelector else { return }
                let Class: AnyClass = type(of: delegate as AnyObject)
                let method = class_getMethodImplementation(Class, shouldCloseSelector)
                
                typealias Signature = @convention(c) (Any, Selector, AnyObject, Bool, UnsafeMutableRawPointer?) -> Void
                let function = unsafeBitCast(method, to: Signature.self)
                
                function(delegate, shouldCloseSelector, self, false, contextInfo)
            }
            shouldClose()
            return
        }
        
        saveState(name: OESaveStateAutosaveName) {
            self.emulationStatus = .terminating
            // TODO: #567 and #568 need to be fixed first
            //removeDeviceNotificationObservers()
            
            self.gameCoreManager.stopEmulation() {
                DLog("Emulation stopped")
                OEBindingsController.default.systemBindings(for: self.systemPlugin.controller).remove(self)
                
                self.emulationStatus = .notSetup
                
                self.gameCoreManager = nil
                
                if let lastPlayStartDate = self.lastPlayStartDate {
                    self.rom.addTimeInterval(toPlayTime: abs(lastPlayStartDate.timeIntervalSinceNow))
                }
                self.lastPlayStartDate = nil
                
                super.canClose(withDelegate: delegate, shouldClose: shouldCloseSelector, contextInfo: contextInfo)
            }
        }
    }
    
    // MARK: - Setup
    
    func setUpGame(completionHandler handler: @escaping (_ success: Bool, _ error: Error?) -> Void) {
        guard
            emulationStatus == .notSetup,
            checkRequiredFiles(),
            !checkDeprecatedCore()
        else {
            handler(false, nil)
            return
        }
        
        checkGlitches()
        
        gameCoreManager.loadROM(completionHandler: {
            self.gameCoreManager.setupEmulation() { result in
                self.gameViewController.setScreenSize(result.screenSize, aspectSize: result.aspectSize)
                
                DLog("SETUP DONE.")
                self.emulationStatus = .setup
                
                // TODO: #567 and #568 need to be fixed first
                //self.addDeviceNotificationObservers()
                
                self.disableOSSleep()
                self.rom.incrementPlayCount()
                self.rom.markAsPlayedNow()
                self.lastPlayStartDate = Date()
                
                if let saveStateForGameStart = self.saveStateForGameStart {
                    self.loadState(state: saveStateForGameStart)
                    self.saveStateForGameStart = nil
                }
                
                // set initial volume
                self.setVolume(self.volume, asDefault: false)
                
                OEBindingsController.default.systemBindings(for: self.systemPlugin.controller).add(self)
                
                self.gameCoreManager.setHandleEvents(self.handleEvents)
                self.gameCoreManager.setHandleKeyboardEvents(self.handleKeyboardEvents)
                
                handler(true, nil)
            }
        }, errorHandler: { error in
            self.emulationStatus = .notSetup
            if self.romDecompressed,
               let romPath = self.romPath {
                try? FileManager.default.removeItem(atPath: romPath)
            }
            OEBindingsController.default.systemBindings(for: self.systemPlugin.controller).remove(self)
            self.gameCoreManager = nil
            self.stopEmulation(self)
            
            let rootError = error as NSError
            if rootError.domain == NSCocoaErrorDomain,
               rootError.code >= NSXPCConnectionErrorMinimum,
               rootError.code <= NSXPCConnectionErrorMaximum {
                let error = Errors.gameCoreCrashed(self.corePlugin, self.systemIdentifier, rootError)
                handler(false, error)
                return
            }
            
            // TODO: the setup completion handler shouldn't be the place where non-setup-related errors are handled!
            handler(false, error)
        })
    }
    
    private func setUpGameCoreManager(using core: OECorePlugin, completionHandler: @escaping () -> Void) {
        assert(core != gameCoreManager.plugin, "Do not attempt to run a new core using the same plug-in as the current one.")
        
        emulationStatus = .notSetup
        gameCoreManager.stopEmulation() {
            OEBindingsController.default.systemBindings(for: self.systemPlugin.controller).remove(self)
            
            self.gameCoreManager = self.newGameCoreManager(with: core)
            self.setUpGame { success, error in
                if !success {
                    if let error = error {
                        self.presentError(error)
                    }
                    return
                }
                
                completionHandler()
            }
        }
    }
    
    private func newGameCoreManager(with corePlugin: OECorePlugin) -> OEGameCoreManager {
        self.corePlugin = corePlugin
        
        var path = romFileURL.path
        let lastDisplayModeInfo = UserDefaults.standard.object(forKey: String(format: OEGameCoreDisplayModeKeyFormat, corePlugin.bundleIdentifier)) as? [String : Any]
        // if file is in an archive append :entryIndex to path, so the core manager can figure out which entry to load
        if let index = rom.archiveFileIndex as? Int {
            path.append(":\(index)")
        }
        
        // Never extract arcade roms and .md roms (XADMaster identifies some as LZMA archives)
        let ext = URL(fileURLWithPath: path).pathExtension.lowercased()
        if systemPlugin.systemIdentifier != "openemu.system.arcade",
           ext != "md", ext != "nds", ext != "iso" {
            var romDecompressed = ObjCBool(romDecompressed)
            path = OEDecompressFileInArchiveAtPathWithHash(path, rom.md5, &romDecompressed)
            self.romDecompressed = romDecompressed.boolValue
            romPath = path
        }
        
        let shader = OEShadersModel.shared.shader(forSystem: systemIdentifier)!
        let params = shader.parameters(forIdentifier: systemPlugin.systemIdentifier) ?? [:]
        
        let info = OEGameStartupInfo(romPath: path,
                                     romMD5: rom.md5 ?? "",
                                     romHeader: rom.header ?? "",
                                     romSerial: rom.serial ?? "",
                                     systemRegion: OELocalizationHelper.shared.regionName,
                                     displayModeInfo: lastDisplayModeInfo,
                                     shader: shader.url,
                                     shaderParameters: params as [String : NSNumber],
                                     corePluginPath: corePlugin.path,
                                     systemPluginPath: systemPlugin.path)
        
        if let managerClassName = UserDefaults.standard.string(forKey: OEGameCoreManagerModePreferenceKey),
           let managerClass = NSClassFromString(managerClassName),
           managerClass == OEThreadGameCoreManager.self {
            return OEThreadGameCoreManager(startupInfo: info, corePlugin: corePlugin, systemPlugin: systemPlugin, gameCoreOwner: self)
        } else {
            return OEXPCGameCoreManager(startupInfo: info, corePlugin: corePlugin, systemPlugin: systemPlugin, gameCoreOwner: self)
        }
    }
    
    private func core(forSystem system: OESystemPlugin) throws -> OECorePlugin {
        let systemIdentifier = system.systemIdentifier!
        var validPlugins = OECorePlugin.corePlugins(forSystemIdentifier: systemIdentifier)!
        
        if validPlugins.isEmpty {
            throw Errors.noCore
        }
        else if validPlugins.count == 1 {
            return validPlugins.first!
        }
        else {
            let defaults = UserDefaults.standard
            if let coreIdentifier = defaults.string(forKey: "defaultCore.\(systemIdentifier)"),
               let core = OECorePlugin(bundleIdentifier: coreIdentifier) {
                return core
            } else {
                validPlugins.sort { $0.displayName.caseInsensitiveCompare($1.displayName) == .orderedAscending }
                return validPlugins.first!
            }
        }
    }
    
    private func checkRequiredFiles() -> Bool {
        // Check current system plugin for OERequiredFiles and core plugin for OEGameCoreRequiresFiles opt-in
        if gameCoreManager.plugin?.controller.requiresFiles(forSystemIdentifier: systemPlugin.systemIdentifier) == false {
            return true
        }
        
        if let validRequiredFiles = gameCoreManager.plugin?.controller.requiredFiles(forSystemIdentifier: systemPlugin.systemIdentifier) {
            return BIOSFile.requiredFilesAvailable(forSystemIdentifier: validRequiredFiles)
        } else {
            return true
        }
    }
    
    @discardableResult
    private func checkGlitches() -> Bool {
        let OEGameCoreGlitchesKey = OEAlert.OEGameCoreGlitchesSuppressionKey
        let coreName = gameCoreManager.plugin?.controller.pluginName ?? ""
        let systemIdentifier = systemPlugin.systemIdentifier ?? ""
        let systemKey = "\(coreName).\(systemIdentifier)"
        let defaults = UserDefaults.standard
        
        let glitchInfo = defaults.object(forKey: OEGameCoreGlitchesKey) as? [String : Bool] ?? [:]
        let showAlert = !(glitchInfo[systemKey] ?? false)
        
        if (gameCoreManager.plugin?.controller.hasGlitches(forSystemIdentifier: systemPlugin.systemIdentifier) ?? false) && showAlert {
            
            let message = String(format: NSLocalizedString("The %@ core has compatibility issues and some games may contain glitches or not play at all.\n\nPlease do not report problems as we are not responsible for the development of %@.", comment: ""), coreName, coreName)
            let alert = OEAlert()
            alert.messageText = NSLocalizedString("Warning", comment: "")
            alert.informativeText = message
            alert.defaultButtonTitle = NSLocalizedString("OK", comment: "")
            alert.showsSuppressionButton = true
            alert.suppressionLabelText = NSLocalizedString("Do not show me again", comment: "Alert suppression label")
            
            if alert.runModal() == .alertFirstButtonReturn && alert.suppressionButtonState {
                var systemKeyGlitchInfo: [String : Bool] = [:]
                for (key, value) in glitchInfo {
                    systemKeyGlitchInfo[key] = value
                }
                systemKeyGlitchInfo[systemKey] = true
                
                defaults.set(systemKeyGlitchInfo, forKey: OEGameCoreGlitchesKey)
            }
            
            return true
        }
        return false
    }
    
    private func checkDeprecatedCore() -> Bool {
        guard gameCoreManager.plugin?.isDeprecated == true else {
            return false
        }
        
        let coreName = gameCoreManager.plugin?.controller.pluginName ?? ""
        let systemIdentifier = systemPlugin.systemIdentifier ?? ""
        
        let removalDate = gameCoreManager.plugin?.infoDictionary?[OEGameCoreSupportDeadlineKey] as? Date
        var deadlineInMoreThanOneMonth = false
        let oneMonth: TimeInterval = 30 * 24 * 60 * 60
        if removalDate == nil || removalDate!.timeIntervalSinceNow > oneMonth {
            deadlineInMoreThanOneMonth = true
        }
        
        let replacements = gameCoreManager.plugin?.infoDictionary?[OEGameCoreSuggestedReplacement] as? [String : String]
        let replacement = replacements?[systemIdentifier]
        var replacementName: String?
        var download: CoreDownload?
        
        if let replacement = replacement {
            if let plugin = OECorePlugin(bundleIdentifier: replacement) {
                replacementName = plugin.controller.pluginName
            } else {
                let repl = CoreUpdater.shared.coreList.firstIndex(where: { $0.bundleIdentifier.caseInsensitiveCompare(replacement) == .orderedSame })
                if let repl = repl {
                    download = CoreUpdater.shared.coreList[repl]
                    replacementName = download?.name
                }
            }
        }
        
        let title: String
        if deadlineInMoreThanOneMonth {
            title = String(format: NSLocalizedString("The %@ core plugin is deprecated", comment: "Message title (removal far away)"), coreName)
        } else {
            title = String(format: NSLocalizedString("The %@ core plugin is deprecated, and will be removed soon", comment: "Message title (removal happening soon)"), coreName)
        }
        
        var infoMsg: String
        if deadlineInMoreThanOneMonth {
            infoMsg = NSLocalizedString("This core plugin will not be available in the future. Once it is removed, any save states created with it will stop working.", comment: "Message info, part 1 (removal far away)")
        } else {
            infoMsg = NSLocalizedString("In a few days, this core plugin is going to be automatically removed. Once it is removed, any save states created with it will stop working.", comment: "Message info, part 1 (removal happening soon)")
        }
        
        if let replacementName = replacementName {
            infoMsg += "\n\n"
            infoMsg += String(format: NSLocalizedString("We suggest you switch to the %@ core plugin as soon as possible.", comment: "Message info, part 2 (shown only if the replacement plugin is available)"), replacementName)
        }
        
        let alert = NSAlert()
        alert.messageText = title
        alert.informativeText = infoMsg
        
        if let download = download, gameWindowController == nil {
            let defaults = UserDefaults.standard
            let prefKey = "defaultCore." + systemIdentifier
            let currentCore = defaults.string(forKey: prefKey)
            let deprecatedIsDefault = currentCore == gameCoreManager.plugin?.bundleIdentifier
            
            if deprecatedIsDefault {
                alert.addButton(withTitle: String(format: NSLocalizedString("Install %@ and Set as Default", comment: ""), replacementName!))
            } else {
                alert.addButton(withTitle: String(format: NSLocalizedString("Install %@", comment: ""), replacementName!))
            }
            alert.addButton(withTitle: NSLocalizedString("Ignore", comment: ""))
            
            if alert.runModal() != .alertFirstButtonReturn {
                return false
            }
            
            CoreUpdater.shared.installCore(with: download) { plugin, error in
                if deprecatedIsDefault,
                   let plugin = plugin {
                    defaults.set(plugin.bundleIdentifier, forKey: prefKey)
                }
            }
            
            return true
        } else {
            alert.addButton(withTitle: NSLocalizedString("OK", comment: ""))
            alert.runModal()
        }
        
        return false
    }
    
    // MARK: - Game Window
    
    func showInSeparateWindow(inFullScreen fullScreen: Bool) {
        isUndocking = true
        let window = NSWindow(contentRect: .zero,
                              styleMask: [.titled, .closable, .miniaturizable, .resizable],
                              backing: .buffered,
                              defer: true)
        let windowController = OEPopoutGameWindowController(window: window)
        windowController.isWindowFullScreen = fullScreen
        gameWindowController = windowController
        showWindows()
        
        isEmulationPaused = false
        isUndocking = false
    }
    
    func toggleFullScreen(_ sender: Any?) {
        gameWindowController?.window?.toggleFullScreen(sender)
    }
    
    private func addObservers(for windowController: NSWindowController) {
        let window = windowController.window
        let nc = NotificationCenter.default
        nc.addObserver(self, selector: #selector(windowDidBecomeMain(_:)), name: NSWindow.didBecomeMainNotification, object: window)
        nc.addObserver(self, selector: #selector(windowDidResignMain(_:)), name: NSWindow.didResignMainNotification, object: window)
    }
    
    private func removeObservers(for windowController: NSWindowController) {
        let window = windowController.window
        let nc = NotificationCenter.default
        nc.removeObserver(self, name: NSWindow.didBecomeMainNotification, object: window)
        nc.removeObserver(self, name: NSWindow.didResignMainNotification, object: window)
    }
    
    @objc private func windowDidResignMain(_ notification: Notification) {
        let backgroundPause = UserDefaults.standard.bool(forKey: OEBackgroundPauseKey)
        if backgroundPause && emulationStatus == .playing {
            isEmulationPaused = true
            pausedByGoingToBackground = true
        }
    }
    
    @objc private func windowDidBecomeMain(_ notification: Notification) {
        if pausedByGoingToBackground {
            isEmulationPaused = false
            pausedByGoingToBackground = false
        }
    }
    
    // MARK: - Device Notifications
    
    private func addDeviceNotificationObservers() {
        let nc = NotificationCenter.default
        nc.addObserver(self, selector: #selector(didReceiveLowBatteryWarning(_:)), name: .OEDeviceHandlerDidReceiveLowBatteryWarning, object: nil)
        nc.addObserver(self, selector: #selector(deviceDidDisconnect(_:)), name: .OEDeviceManagerDidRemoveDeviceHandler, object: nil)
    }
    
    private func removeDeviceNotificationObservers() {
        let nc = NotificationCenter.default
        nc.removeObserver(self, name: .OEDeviceHandlerDidReceiveLowBatteryWarning, object: nil)
        nc.removeObserver(self, name: .OEDeviceManagerDidRemoveDeviceHandler, object: nil)
    }
    
    @objc private func didReceiveLowBatteryWarning(_ notification: Notification) {
        let isRunning = !isEmulationPaused
        isEmulationPaused = true
        
        let devHandler = notification.object as? OEDeviceHandler
        let message = String(format: NSLocalizedString("The battery in device number %lu, %@, is low. Please charge or replace the battery.", comment: "Low battery alert detail message."), devHandler?.deviceNumber ?? 0, devHandler?.deviceDescription?.name ?? "")
        let alert = OEAlert()
        alert.messageText = NSLocalizedString("Low Controller Battery", comment: "Device battery level is low.")
        alert.informativeText = message
        alert.defaultButtonTitle = NSLocalizedString("Resume", comment: "")
        alert.runModal()
        
        if isRunning {
            isEmulationPaused = false
        }
    }
    
    @objc private func deviceDidDisconnect(_ notification: Notification) {
        let isRunning = !isEmulationPaused
        isEmulationPaused = true
        
        let devHandler = notification.userInfo?[OEDeviceManagerDeviceHandlerUserInfoKey] as? OEDeviceHandler
        let message = String(format: NSLocalizedString("Device number %lu, %@, has disconnected.", comment: "Device disconnection detail message."), devHandler?.deviceNumber ?? 0, devHandler?.deviceDescription?.name ?? "")
        let alert = OEAlert()
        alert.messageText = NSLocalizedString("Device Disconnected", comment: "A controller device has disconnected.")
        alert.informativeText = message
        alert.defaultButtonTitle = NSLocalizedString("Resume", comment: "Resume game after battery warning button label")
        alert.runModal()
        
        if isRunning {
            isEmulationPaused = false
        }
    }
    
    // MARK: - Display Sleep Handling
    
    private func enableOSSleep() {
        if displaySleepAssertionID == kIOPMNullAssertionID { return }
        IOPMAssertionRelease(displaySleepAssertionID)
        displaySleepAssertionID = IOPMAssertionID(kIOPMNullAssertionID)
    }
    
    private func disableOSSleep() {
        if displaySleepAssertionID != kIOPMNullAssertionID { return }
        IOPMAssertionCreateWithName(kIOPMAssertionTypePreventUserIdleDisplaySleep as CFString, IOPMAssertionLevel(kIOPMAssertionLevelOn), "OpenEmu playing game" as CFString, &displaySleepAssertionID)
    }
    
    // MARK: - Controlling Emulation
    
    private func startEmulation() {
        if emulationStatus != .setup {
            return
        }
        
        emulationStatus = .starting
        gameCoreManager.startEmulation() {
            self.emulationStatus = .playing
        }
        
        gameViewController.reflectEmulationPaused(false)
    }
    
    @objc(emulationPaused) //OEPopoutGameWindowController
    var isEmulationPaused: Bool {
        @objc(isEmulationPaused)
        get {
            return emulationStatus != .playing
        }
        @objc(setEmulationPaused:)
        set(pauseEmulation) {
            if emulationStatus == .setup {
                if !pauseEmulation {
                    startEmulation()
                    return
                }
            }
            if pauseEmulation {
                enableOSSleep()
                emulationStatus = .paused
                if let lastPlayStartDate = lastPlayStartDate {
                    rom.addTimeInterval(toPlayTime: abs(lastPlayStartDate.timeIntervalSinceNow))
                    self.lastPlayStartDate = nil
                }
            } else {
                disableOSSleep()
                rom.markAsPlayedNow()
                lastPlayStartDate = Date()
                emulationStatus = .playing
            }
            
            gameCoreManager?.setPauseEmulation(pauseEmulation)
            gameViewController.reflectEmulationPaused(pauseEmulation)
        }
    }
    
    @IBAction func performClose(_ sender: Any?) {
        stopEmulation(sender)
    }
    
    @IBAction func stopEmulation(_ sender: Any?) {
        // we can't just close the document here because proper shutdown is implemented in
        // method canClose(withDelegate:shouldClose:contextInfo:)
        windowControllers.forEach { $0.window?.performClose(sender) }
    }
    
    @objc func toggleEmulationPaused(_ sender: Any?) {
        isEmulationPaused.toggle()
    }
    
    @objc func resetEmulation(_ sender: Any?) {
        if OEAlert.resetSystem().runModal() == .alertFirstButtonReturn {
            gameCoreManager.resetEmulation() {
                self.isEmulationPaused = false
            }
        }
    }
    
    private var shouldTerminateEmulation: Bool {
        if coreDidTerminateSuddenly {
            return true
        }
        
        enableOSSleep()
        isEmulationPaused = true
        
        if OEAlert.stopEmulation().runModal() != .alertFirstButtonReturn {
            disableOSSleep()
            isEmulationPaused = false
            return false
        }
        
        return true
    }
    
    @discardableResult
    private func pauseEmulationIfNeeded() -> Bool {
        let pauseNeeded = emulationStatus == .playing
        
        if pauseNeeded {
            isEmulationPaused = true
        }
        
        return pauseNeeded
    }
    
    // MARK: - Actions
    
    @IBAction func editControls(_ sender: Any?) {
        let userInfo = [
            PreferencesWindowController.userInfoPanelNameKey: "Controls",
            PreferencesWindowController.userInfoSystemIdentifierKey: systemIdentifier ?? "",
        ]
        
        NotificationCenter.default.post(Notification(name: PreferencesWindowController.openPaneNotificationName, userInfo: userInfo))
    }
    
    /// expects `sender` or `sender.representedObject` to be an `OECorePlugin` object and prompts the user for confirmation
    @objc func switchCore(_ sender: AnyObject) {
        let plugin: OECorePlugin
        if let sender = sender as? OECorePlugin {
            plugin = sender
        } else if let obj = sender.representedObject as? OECorePlugin {
            plugin = obj
        } else {
            DLog("Invalid argument passed: \(String(describing: sender))")
            return
        }
        
        if plugin.bundleIdentifier == gameCoreManager.plugin?.bundleIdentifier {
            return
        }
        
        isEmulationPaused = true
        
        let alert = OEAlert()
        alert.messageText = NSLocalizedString("If you change the core you current progress will be lost and save states will not work anymore.", comment: "")
        alert.defaultButtonTitle = NSLocalizedString("Change Core", comment: "")
        alert.alternateButtonTitle = NSLocalizedString("Cancel", comment: "")
        alert.showSuppressionButton(forUDKey: OEAlert.OEAutoSwitchCoreAlertSuppressionKey)
        
        alert.callbackHandler = { alert, result in
            if result != .alertFirstButtonReturn {
                return
            }
            
            self.setUpGameCoreManager(using: plugin) {
                self.startEmulation()
            }
        }
        
        alert.runModal()
    }
    
    /// Returns a filtered screenshot of the currently running core.
    @objc //OEPopoutGameWindowController
    func screenshot() -> NSImage {
        let rep = gameCoreManager.captureOutputImage()
        let screenshot = NSImage(size: rep.size)
        screenshot.addRepresentation(rep)
        return screenshot
    }
    
    @objc func takeScreenshot(_ sender: Any?) {
        let defaults = UserDefaults.standard
        let type = NSBitmapImageRep.FileType(rawValue: UInt(defaults.integer(forKey: OEScreenshotFileFormatKey)))!
        let properties = defaults.dictionary(forKey: OEScreenshotPropertiesKey) as! [NSBitmapImageRep.PropertyKey : Any]
        let takeNativeScreenshots = defaults.bool(forKey: OETakeNativeScreenshots)
        let disableAspectRatioFix = defaults.bool(forKey: OEScreenshotAspectRatioCorrectionDisabled)
        
        if takeNativeScreenshots || ((sender as? NSMenuItem)?.tag == 1) {
            gameCoreManager.captureSourceImage() { image in
                var image = image
                if !disableAspectRatioFix {
                    let newSize = self.gameViewController.defaultScreenSize
                    image = image.resized(newSize) ?? image
                }
                if let imageData = image.representation(using: type, properties: properties) {
                    self.writeScreenshotImageData(imageData)
                }
            }
        } else {
            gameCoreManager.captureOutputImage() { image in
                if let imageData = image.representation(using: type, properties: properties) {
                    self.writeScreenshotImageData(imageData)
                }
            }
        }
    }
    
    private func writeScreenshotImageData(_ imageData: Data) {
        let dateFormatter = DateFormatter()
        dateFormatter.dateFormat = "yyyy-MM-dd HH.mm.ss"
        let timeStamp = dateFormatter.string(from: Date())
        
        // Replace forward slashes in the game title with underscores because forward slashes aren't allowed in filenames.
        var displayName = rom?.game?.displayName ?? ""
        displayName = displayName.replacingOccurrences(of: "/", with: "_")
        
        let fileName = "\(displayName) \(timeStamp).png"
        let temporaryURL = URL(fileURLWithPath: NSTemporaryDirectory()).appendingPathComponent(fileName)
        
        do {
            try imageData.write(to: temporaryURL, options: .atomic)
            if let context = rom.managedObjectContext {
                let screenshot = OEDBScreenshot.createObject(in: context, for: rom, with: temporaryURL)
                screenshot?.save()
                gameViewController.showScreenShotNotification()
            }
        } catch {
            NSLog("Could not save screenshot at URL: \(temporaryURL), with error: \(error)")
        }
    }
    
    // MARK: - Volume/Audio
    
    /// expects `sender` or `sender.representedObject` to be an `OEAudioDevice` object
    @IBAction func changeAudioOutputDevice(_ sender: AnyObject) {
        let device: OEAudioDevice?
        if let sender = sender as? OEAudioDevice {
            device = sender
        } else if let obj = sender.representedObject as? OEAudioDevice {
            device = obj
        } else {
            return
        }
        
        if let device = device {
            gameCoreManager.setAudioOutputDeviceID(device.deviceID)
        } else {
            gameCoreManager.setAudioOutputDeviceID(0)
        }
    }
    
    var volume: Float {
        return UserDefaults.standard.float(forKey: OEGameVolumeKey)
    }
    
    func setVolume(_ volume: Float, asDefault defaultFlag: Bool) {
        gameCoreManager.setVolume(volume)
        gameViewController.reflectVolume(volume)
        
        if defaultFlag {
            UserDefaults.standard.set(volume, forKey: OEGameVolumeKey)
        }
    }
    
    @IBAction func changeVolume(_ sender: AnyObject) {
        if let sender = sender as? NSSlider {
            setVolume(sender.floatValue, asDefault: true)
        } else {
            assertionFailure("Invalid argument passed: \(String(describing: sender))")
        }
    }
    
    @IBAction func mute(_ sender: Any?) {
        isMuted = true
        setVolume(0, asDefault: false)
    }
    
    @IBAction func unmute(_ sender: Any?) {
        isMuted = false
        setVolume(volume, asDefault: false)
    }
    
    func volumeDown(_ sender: Any?) {
        var volume = volume
        volume -= 0.1
        if volume < 0 {
            volume = 0
        }
        setVolume(volume, asDefault: true)
    }
    
    func volumeUp(_ sender: Any?) {
        var volume = volume
        volume += 0.1
        if volume > 1 {
            volume = 1
        }
        setVolume(volume, asDefault: true)
    }
    
    // MARK: - Cheats
    
    var supportsCheats: Bool {
        return gameCoreManager.plugin?.controller.supportsCheatCode(forSystemIdentifier: systemPlugin.systemIdentifier) == true
    }
    
    @IBAction func addCheat(_ sender: AnyObject) {
        let alert = OEAlert()
        
        alert.otherInputLabelText = NSLocalizedString("Title:", comment: "")
        alert.showsOtherInputField = true
        alert.otherInputPlaceholderText = NSLocalizedString("Cheat Description", comment: "")
        
        alert.inputLabelText = NSLocalizedString("Code:", comment: "")
        alert.showsInputField = true
        alert.inputPlaceholderText = NSLocalizedString("Join multi-line cheats with '+' e.g. 000-000+111-111", comment: "")
        
        alert.defaultButtonTitle = NSLocalizedString("Add Cheat", comment: "")
        alert.alternateButtonTitle = NSLocalizedString("Cancel", comment: "")
        
        alert.showsSuppressionButton = true
        alert.suppressionLabelText = NSLocalizedString("Enable now", comment: "Cheats button label")
        
        alert.inputLimit = 1000
        
        if alert.runModal() == .alertFirstButtonReturn {
            var enabled: Bool
            if alert.suppressionButtonState {
                enabled = true
                setCheat(alert.stringValue, withType: "Unknown", enabled: enabled)
            } else {
                enabled = false
            }
            
            //TODO: decide how to handle setting a cheat type from the modal and save added cheats to file
            (sender.representedObject as? NSMutableArray)?.add([
                "code": alert.stringValue,
                "type": "Unknown",
                "description": alert.otherStringValue,
                "enabled": enabled
            ] as NSMutableDictionary)
        }
    }
    
    @IBAction func toggleCheat(_ sender: AnyObject) {
        guard let dict = sender.representedObject as? [String : Any],
              let code = dict["code"] as? String,
              let type = dict["type"] as? String,
              var enabled = dict["enabled"] as? Bool
        else { return }
        
        enabled.toggle()
        (sender.representedObject as? NSMutableDictionary)?.setValue(enabled, forKey: "enabled")
        setCheat(code, withType: type, enabled: enabled)
    }
    
    func setCheat(_ cheatCode: String, withType type: String, enabled: Bool) {
        gameCoreManager.setCheat(cheatCode, withType: type, enabled: enabled)
    }
    
    // MARK: - Discs
    
    var supportsMultipleDiscs: Bool {
        return gameCoreManager.plugin?.controller.supportsMultipleDiscs(forSystemIdentifier: systemPlugin.systemIdentifier) == true
    }
    
    @IBAction func setDisc(_ sender: AnyObject) {
        if let sender = sender.representedObject as? UInt {
            gameCoreManager.setDisc(sender)
        }
    }
    
    // MARK: - File Insertion
    
    var supportsFileInsertion: Bool {
        return gameCoreManager.plugin?.controller.supportsFileInsertion(forSystemIdentifier: systemPlugin.systemIdentifier) == true
    }
    
    @IBAction func insertFile(_ sender: AnyObject?) {
        var archivedExtensions: [String] = []
        // The Archived Game document type lists all supported archive extensions, e.g. zip
        let bundleInfo = Bundle.main.infoDictionary
        let docTypes = bundleInfo?["CFBundleDocumentTypes"] as? [[String : Any]]
        for docType in docTypes ?? [] {
            if docType["CFBundleTypeName"] as? String == "Archived Game" {
                if let extensions = docType["CFBundleTypeExtensions"] as? [String] {
                    archivedExtensions.append(contentsOf: extensions)
                }
                break
            }
        }
        
        let validExtensions = archivedExtensions + systemPlugin.supportedTypeExtensions() as! [String]
        
        let system = rom.game!.system!
        let systemFolder = OELibraryDatabase.default!.romsFolderURL(for: system)
        // Seemed to need this to get NSOpenPanel to restrict to this directory on open for some reason
        let romsFolderURL = URL(fileURLWithPath: systemFolder.path, isDirectory: true)
        
        let panel = NSOpenPanel()
        panel.allowsMultipleSelection = false
        panel.directoryURL = romsFolderURL
        panel.canChooseFiles = true
        panel.canChooseDirectories = false
        panel.canCreateDirectories = false
        panel.allowedFileTypes = validExtensions
        
        panel.beginSheetModal(for: gameWindowController!.window!) { result in
            guard
                result == .OK,
                let url = panel.url
            else { return }
            
            let path = decompressedPathForRomAtPath(url.path)
            let fileURL = URL(fileURLWithPath: path)
            
            self.gameCoreManager.insertFile(at: fileURL) { success, error in
                if !success {
                    if let error = error {
                        self.presentError(error)
                    }
                    return
                }
                
                self.isEmulationPaused = false
            }
        }
    }
    
    // MARK: - Display Mode
    
    var supportsDisplayModeChange: Bool {
        return gameCoreManager.plugin?.controller.supportsDisplayModeChange(forSystemIdentifier: systemPlugin.systemIdentifier) == true
    }
    
    @IBAction func nextDisplayMode(_ sender: Any?) {
        changeDisplayMode(directionReversed: false)
    }
    
    @IBAction func lastDisplayMode(_ sender: Any?) {
        changeDisplayMode(directionReversed: true)
    }
    
    /// expects `sender` or `sender.representedObject` to be a `Dictionary<String, Any>` object
    @IBAction func changeDisplayMode(_ sender: Any) {
        let fromMenu: Bool
        let modeDict: [String : Any]
        if let obj = (sender as AnyObject).representedObject as? [String : Any] {
            fromMenu = true
            modeDict = obj
        } else if let sender = sender as? [String : Any] {
            fromMenu = false
            modeDict = sender
        } else {
            return
        }
        
        let isSelected   = modeDict[OEGameCoreDisplayModeStateKey] as? Bool ?? false
        let isToggleable = modeDict[OEGameCoreDisplayModeAllowsToggleKey] as? Bool ?? false
        let isPrefSaveDisallowed = modeDict[OEGameCoreDisplayModeDisallowPrefSaveKey] as? Bool ?? false
        let isManual     = modeDict[OEGameCoreDisplayModeManualOnlyKey] as? Bool ?? false
        
        // Mutually exclusive option is already selected, do nothing
        if isSelected && !isToggleable {
            return
        }
        
        let displayModeKeyForCore = String(format: OEGameCoreDisplayModeKeyFormat, corePlugin.bundleIdentifier)
        let prefKey  = modeDict[OEGameCoreDisplayModePrefKeyNameKey] as? String ?? ""
        let prefVal  = modeDict[OEGameCoreDisplayModePrefValueNameKey] as? String ?? ""
        let modeName = modeDict[OEGameCoreDisplayModeNameKey] as? String ?? ""
        let defaults = UserDefaults.standard
        var displayModeInfo: [String : Any]
        
        // Copy existing prefs
        displayModeInfo = defaults.dictionaryRepresentation()[displayModeKeyForCore] as? [String : Any] ?? [:]
        
        // Mutually exclusive option is unselected
        if !isToggleable {
            displayModeInfo[prefKey] = !prefVal.isEmpty ? prefVal : modeName
            if fromMenu && !isManual {
                lastSelectedDisplayModeOption = modeName
            }
        }
        // Toggleable option, swap YES/NO
        else if isToggleable {
            displayModeInfo[prefKey] = !isSelected
        }
        
        if !isPrefSaveDisallowed {
            defaults.set(displayModeInfo, forKey: displayModeKeyForCore)
        }
        
        gameCoreManager.changeDisplay(withMode: modeName)
    }
    
    private func changeDisplayMode(directionReversed reverse: Bool) {
        var availableOptions: [[String : Any]] = []
        var mode: String
        var isToggleable: Bool
        var isSelected: Bool
        var isManual: Bool
        
        for optionsDict in gameViewController.displayModes {
            mode         = optionsDict[OEGameCoreDisplayModeNameKey] as? String ?? ""
            isToggleable = optionsDict[OEGameCoreDisplayModeAllowsToggleKey] as? Bool ?? false
            isSelected   = optionsDict[OEGameCoreDisplayModeStateKey] as? Bool ?? false
            isManual     = optionsDict[OEGameCoreDisplayModeManualOnlyKey] as? Bool ?? false
            
            if optionsDict[OEGameCoreDisplayModeSeparatorItemKey] != nil ||
                optionsDict[OEGameCoreDisplayModeLabelKey] != nil ||
                isManual {
                continue
            }
            else if !mode.isEmpty && !isToggleable {
                availableOptions.append(optionsDict)
                
                // There may be multiple, but just take the first selected and start from the top
                if lastSelectedDisplayModeOption == nil && isSelected {
                    lastSelectedDisplayModeOption = mode
                }
            }
            else if optionsDict[OEGameCoreDisplayModeGroupNameKey] != nil {
                // Submenu Items
                for subOptionsDict in optionsDict[OEGameCoreDisplayModeGroupItemsKey] as? [[String : Any]] ?? [] {
                    mode         = subOptionsDict[OEGameCoreDisplayModeNameKey] as? String ?? ""
                    isToggleable = subOptionsDict[OEGameCoreDisplayModeAllowsToggleKey] as? Bool ?? false
                    isSelected   = subOptionsDict[OEGameCoreDisplayModeStateKey] as? Bool ?? false
                    isManual     = subOptionsDict[OEGameCoreDisplayModeManualOnlyKey] as? Bool ?? false
                    
                    if subOptionsDict[OEGameCoreDisplayModeSeparatorItemKey] != nil ||
                        subOptionsDict[OEGameCoreDisplayModeLabelKey] != nil ||
                        isManual {
                        continue
                    }
                    else if !mode.isEmpty && !isToggleable {
                        availableOptions.append(subOptionsDict)
                        
                        if lastSelectedDisplayModeOption == nil && isSelected {
                            lastSelectedDisplayModeOption = mode
                        }
                    }
                }
                
                continue
            }
        }
        
        // Reverse
        if reverse {
            availableOptions.reverse()
        }
        
        // If there are multiple mutually-exclusive groups of modes we want to enumerate
        // all the combinations.
        
        // List of pref keys used by each group of mutually exclusive modes
        var prefKeys: [String] = []
        // Index of the currently selected mode for each group
        var prefKeyToSelected: [String : Int] = [:]
        // Indexes of the modes that are part of the same group
        var prefKeyToOptions: [String : NSMutableIndexSet] = [:]
        
        var i = 0
        for optionsDict in availableOptions {
            let prefKey = optionsDict[OEGameCoreDisplayModePrefKeyNameKey] as? String ?? ""
            let name = optionsDict[OEGameCoreDisplayModeNameKey] as? String ?? ""
            
            if name == lastSelectedDisplayModeOption {
                // Put the group of the last mode manually selected by the user in front of the list
                // This way the options of this group will be cycled through first
                if let index = prefKeys.firstIndex(of: prefKey) {
                    prefKeys.remove(at: index)
                }
                prefKeys.insert(prefKey, at: 0)
            }
            else if !prefKeys.contains(prefKey) {
                // Prioritize cycling all other modes in the order that they appear
                prefKeys.append(prefKey)
            }
            
            if optionsDict[OEGameCoreDisplayModeStateKey] as? Bool ?? false {
                prefKeyToSelected[prefKey] = i
            }
            
            var optionsIndexes = prefKeyToOptions[prefKey]
            if optionsIndexes == nil {
                optionsIndexes = NSMutableIndexSet()
                prefKeyToOptions[prefKey] = optionsIndexes
            }
            optionsIndexes?.add(i)
            
            i += 1
        }
        
        for prefKey in prefKeys {
            let current = prefKeyToSelected[prefKey]!
            let options = prefKeyToOptions[prefKey]!
            
            var carry = false
            var next = options.indexGreaterThanIndex(current)
            if next == NSNotFound {
                // Finished cycling through this mode; advance to the next one
                carry = true
                next = options.firstIndex
            }
            
            let nextMode = availableOptions[next]
            changeDisplayMode(nextMode)
            
            if !carry {
                break
            }
        }
    }
    
    // MARK: - Saving States
    
    var supportsSaveStates: Bool {
        return gameCoreManager.plugin?.controller.saveStatesNotSupported(forSystemIdentifier: systemPlugin.systemIdentifier) == false
    }
    
    @objc private func saveState(_ sender: Any?) {
        if !supportsSaveStates {
            return
        }
        
        let didPauseEmulation = pauseEmulationIfNeeded()
        
        let saveGameNo = rom.saveStateCount + 1
        let date = Date()
        let formatter = DateFormatter()
        formatter.timeZone = NSTimeZone.local
        formatter.dateFormat = "yyyy-MM-dd HH:mm:ss ZZZ"
        
        let format = NSLocalizedString("Save-Game-%ld %@", comment: "default save state name")
        let proposedName = String(format: format, saveGameNo, formatter.string(from: date))
        let alert = OEAlert.saveGame(proposedName: proposedName)
        
        alert.performBlockInModalSession {
            let parentFrame = self.gameViewController.view.window?.frame ?? .zero
            let alertSize = alert.window.frame.size
            let alertX = (parentFrame.width - alertSize.width) / 2 + parentFrame.origin.x
            let alertY = (parentFrame.height - alertSize.height) / 2 + parentFrame.origin.y
            alert.window.setFrameOrigin(NSPoint(x: alertX, y: alertY))
        }
        
        if alert.runModal() == .alertFirstButtonReturn {
            if alert.stringValue == "" {
                saveState(name: proposedName, completionHandler: nil)
            } else {
                saveState(name: alert.stringValue, completionHandler: nil)
            }
        }
        
        if didPauseEmulation {
            isEmulationPaused = false
        }
    }
    
    @objc func quickSave(_ sender: AnyObject?) {
        var slot = 0
        if let obj = sender?.representedObject as? Int {
            slot = obj
        }
        
        let name = OEDBSaveState.nameOfQuickSave(inSlot: slot)
        let didPauseEmulation = pauseEmulationIfNeeded()
        
        saveState(name: name) {
            if didPauseEmulation {
                self.isEmulationPaused = false
            }
            self.gameViewController.showQuickSaveNotification()
        }
    }
    
    private func saveState(name stateName: String, completionHandler handler: (() -> Void)? = nil) {
        guard
            supportsSaveStates,
            emulationStatus.rawValue > EmulationStatus.starting.rawValue,
            let rom = rom
        else {
            handler?()
            return
        }
        
        let temporaryDirectoryURL = URL(fileURLWithPath: NSTemporaryDirectory())
        var temporaryStateFileURL = URL(string: UUID().uuidString, relativeTo: temporaryDirectoryURL)!
        let core = gameCoreManager.plugin!
        
        temporaryStateFileURL = (temporaryStateFileURL as NSURL).uniqueURL { triesCount in
            return NSURL(string: UUID().uuidString, relativeTo: temporaryDirectoryURL)!
        } as URL
        
        gameCoreManager.saveStateToFile(atPath: temporaryStateFileURL.path) { success, error in
            if !success {
                NSLog("Could not create save state file at url: \(temporaryStateFileURL)")
                
                handler?()
                return
            }
            
            var state: OEDBSaveState!
            if stateName.hasPrefix(OESaveStateSpecialNamePrefix) {
                state = rom.saveState(withName: stateName)
                state?.coreIdentifier = core.bundleIdentifier
                state?.coreVersion = core.version
            }
            
            if state == nil {
                let context = OELibraryDatabase.default!.mainThreadContext
                state = OEDBSaveState.createSaveStateNamed(stateName, for: rom, core: core, withFile: temporaryStateFileURL, in: context)
            } else {
                state.replaceFile(withFile: temporaryStateFileURL)
                state.timestamp = Date()
            }
            
            state.save()
            let mainContext = state?.managedObjectContext
            mainContext?.perform {
                try? mainContext?.save()
            }
            
            let defaults = UserDefaults.standard
            let type = NSBitmapImageRep.FileType(rawValue: UInt(defaults.integer(forKey: OEScreenshotFileFormatKey)))!
            let properties = defaults.dictionary(forKey: OEScreenshotPropertiesKey) as! [NSBitmapImageRep.PropertyKey : Any]
            
            self.gameCoreManager.captureSourceImage() { image in
                let newSize = self.gameViewController.defaultScreenSize
                let image = image.resized(newSize) ?? image
                let convertedData = image.representation(using: type, properties: properties)
                DispatchQueue.main.async {
                    do {
                        try convertedData?.write(to: state.screenshotURL, options: .atomic)
                    } catch {
                        NSLog("Could not create screenshot at url: \(state.screenshotURL) with error: \(error)")
                    }
                    handler?()
                }
            }
        }
    }
    
    // MARK: - Loading States
    
    /// expects `sender` or `sender.representedObject` to be an `OEDBSaveState` object
    @objc private func loadState(_ sender: AnyObject?) {
        // calling pauseGame here because it might need some time to execute
        pauseEmulationIfNeeded()
        
        let state: OEDBSaveState
        if let sender = sender as? OEDBSaveState {
            state = sender
        } else if let obj = sender?.representedObject as? OEDBSaveState {
            state = obj
        } else {
            DLog("Invalid argument passed: \(String(describing: sender))")
            return
        }
        
        loadState(state: state)
    }
    
    @objc func quickLoad(_ sender: AnyObject?) {
        var slot = 0
        if let obj = sender?.representedObject as? Int {
            slot = obj
        }
        let quicksaveState = rom.quickSaveState(inSlot: slot)
        if let quicksaveState = quicksaveState {
            loadState(quicksaveState)
        }
    }
    
    private func loadState(state: OEDBSaveState) {
        if state.rom != rom {
            DLog("Invalid save state for current rom")
            return
        }
        
        let loadState: (() -> Void) = {
            self.gameCoreManager.loadStateFromFile(atPath: state.dataFileURL.path) { success, error in
                if !success {
                    if let error = error {
                        self.presentError(error)
                    }
                    return
                }
                
                self.isEmulationPaused = false
            }
        }
        
        if gameCoreManager.plugin?.bundleIdentifier == state.coreIdentifier {
            loadState()
            return
        }
        
        let runWithCore: ((OECorePlugin?, Error?) -> Void) = { plugin, error in
            if let plugin = plugin {
                self.setUpGameCoreManager(using: plugin) {
                    loadState()
                }
            } else {
                if let error = error {
                    self.presentError(error)
                }
                return
            }
        }
        
        let alert = OEAlert()
        alert.messageText = NSLocalizedString("This save state was created with a different core. Do you want to switch to that core now?", comment: "")
        alert.defaultButtonTitle = NSLocalizedString("Change Core", comment: "")
        alert.alternateButtonTitle = NSLocalizedString("Cancel", comment: "")
        alert.showSuppressionButton(forUDKey: OEAlert.OEAutoSwitchCoreAlertSuppressionKey)
        
        if alert.runModal() == .alertFirstButtonReturn {
            if let core = OECorePlugin(bundleIdentifier: state.coreIdentifier) {
                runWithCore(core, nil)
            } else {
                CoreUpdater.shared.installCore(for: state, withCompletionHandler: runWithCore)
            }
        } else {
            startEmulation()
        }
    }
    
    // MARK: - Deleting States
    
    /// expects `sender` or `sender.representedObject` to be an `OEDBSaveState` object and prompts the user for confirmation
    @IBAction func deleteSaveState(_ sender: AnyObject) {
        let state: OEDBSaveState
        if let sender = sender as? OEDBSaveState {
            state = sender
        } else if let obj = sender.representedObject as? OEDBSaveState {
            state = obj
        } else {
            DLog("Invalid argument passed: \(String(describing: sender))")
            return
        }
        
        let stateName = state.name ?? ""
        let alert = OEAlert.deleteSaveState(name: stateName)
        
        if alert.runModal() == .alertFirstButtonReturn {
            state.deleteAndRemoveFiles()
        }
    }
}

// MARK: - NSMenuItemValidation

extension OEGameDocument {
    
    override func validateMenuItem(_ menuItem: NSMenuItem) -> Bool {
        switch menuItem.action {
        case #selector(quickLoad(_:)):
            let slot = menuItem.representedObject != nil ? (menuItem.representedObject as? Int) ?? 0 : menuItem.tag
            return rom.quickSaveState(inSlot: slot) != nil
        case #selector(quickSave(_:)):
            return supportsSaveStates
        case #selector(toggleEmulationPaused(_:)):
            if emulationStatus == .paused {
                menuItem.title = NSLocalizedString("Resume Game", comment: "")
                return true
            } else {
                menuItem.title = NSLocalizedString("Pause Game", comment: "")
                return emulationStatus == .playing
            }
        case #selector(nextDisplayMode(_:)),
             #selector(lastDisplayMode(_:)):
            return supportsDisplayModeChange
        default:
            return true
        }
    }
}

// MARK: - GameViewController Methods

extension OEGameDocument {
    
    func setOutputBounds(_ bounds: NSRect) {
        gameCoreManager.setOutputBounds(bounds)
    }
    
    func didReceiveMouseEvent(_ event: OEEvent) {
        gameCoreManager.handleMouseEvent(event)
    }
    
    func updateBackingScaleFactor(_ newScaleFactor: CGFloat) {
        gameCoreManager.setBackingScaleFactor(newScaleFactor)
    }
    
    func updateBounds(_ newBounds: CGRect) {
        gameCoreManager.setOutputBounds(newBounds)
    }
}

// MARK: - OESystemBindingsObserver

extension OEGameDocument: OESystemBindingsObserver {
    
    func systemBindings(_ sender: OESystemBindings, didSetEvent event: OEHIDEvent, forBinding bindingDescription: OEBindingDescription, playerNumber: UInt) {
        gameCoreManager.systemBindingsDidSetEvent(event, forBinding: bindingDescription, playerNumber: playerNumber)
    }
    
    func systemBindings(_ sender: OESystemBindings, didUnsetEvent event: OEHIDEvent, forBinding bindingDescription: OEBindingDescription, playerNumber: UInt) {
        gameCoreManager.systemBindingsDidUnsetEvent(event, forBinding: bindingDescription, playerNumber: playerNumber)
    }
}

// MARK: - OEGameCoreOwner

extension OEGameDocument: OEGameCoreOwner {
    
    func saveState() {
        NSApp.sendAction(#selector(saveState(_:)), to: nil, from: nil)
        //saveState(nil)
    }
    
    func loadState() {
        // FIXME: This replaces a call from OESystemResponder which used to pass self, but passing OESystemResponder would yield the same result in -loadState: so I do not know whether this ever worked in this case. (6eeda41)
        loadState(nil)
    }
    
    func quickSave() {
        quickSave(nil)
    }
    
    func quickLoad() {
        quickLoad(nil)
    }
    
    func toggleFullScreen() {
        toggleFullScreen(self)
    }
    
    func toggleAudioMute() {
        if isMuted {
            unmute(self)
        } else {
            mute(self)
        }
    }
    
    func volumeDown() {
        volumeDown(self)
    }
    
    func volumeUp() {
        volumeUp(self)
    }
    
    func stopEmulation() {
        stopEmulation(self)
    }
    
    func resetEmulation() {
        resetEmulation(self)
    }
    
    func toggleEmulationPaused() {
        toggleEmulationPaused(self)
    }
    
    func takeScreenshot() {
        takeScreenshot(self)
    }
    
    func fastForwardGameplay(_ enable: Bool) {
        if emulationStatus != .playing { return }
        gameViewController.showFastForwardNotification(enable)
    }
    
    func rewindGameplay(_ enable: Bool) {
        if emulationStatus != .playing { return }
        gameViewController.showRewindNotification(enable)
    }
    
    func stepGameplayFrameForward() {
        if emulationStatus == .playing {
            toggleEmulationPaused(self)
        }
        if emulationStatus == .paused {
            gameViewController.showStepForwardNotification()
        }
    }
    
    func stepGameplayFrameBackward() {
        if emulationStatus == .playing {
            toggleEmulationPaused(self)
        }
        if emulationStatus == .paused {
            gameViewController.showStepBackwardNotification()
        }
    }
    
    func nextDisplayMode() {
        nextDisplayMode(self)
    }
    
    func lastDisplayMode() {
        lastDisplayMode(self)
    }
    
    func setScreenSize(_ newScreenSize: OEIntSize, aspectSize newAspectSize: OEIntSize) {
        gameViewController.setScreenSize(newScreenSize, aspectSize: newAspectSize)
    }
    
    func setDiscCount(_ discCount: UInt) {
        gameViewController.discCount = discCount
    }
    
    func setDisplayModes(_ displayModes: [[String : Any]]) {
        gameViewController.displayModes = displayModes
    }
    
    func setRemoteContextID(_ contextID: OEContextID) {
        gameViewController.setRemoteContextID(contextID)
    }
    
    func gameCoreDidTerminate() {
        if !(emulationStatus == .starting || emulationStatus == .paused) {
            return
        }
        coreDidTerminateSuddenly = true
        stopEmulation(self)
    }
}
