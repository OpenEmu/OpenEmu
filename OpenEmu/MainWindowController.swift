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
import OpenEmuKit.OECorePlugin

let OEForcePopoutGameWindowKey = "forcePopout"
let OEFullScreenGameWindowKey = "fullScreen"

private extension NSUserInterfaceItemIdentifier {
    static let mainWindow = NSUserInterfaceItemIdentifier("LibraryWindow")
}

final class MainWindowController: NSWindowController {
    
    private var gameDocument: OEGameDocument?
    private var shouldExitFullScreenWhenGameFinishes = false
    private var shouldUndockGameWindowOnFullScreenExit = false
    private var resumePlayingAfterFullScreenTransition = false
    private var isLaunchingGame = false
    @objc var mainWindowRunsGame = false
    private lazy var libraryController = LibraryController()
    @IBOutlet var placeholderView: NSView!
    private var lastPlaceholderViewTopConstraint: NSLayoutConstraint!
    
    private var _currentContentController: NSViewController?
    private var currentContentController: NSViewController? {
        get {
            _currentContentController
        }
        set {
            let newController = newValue ?? libraryController
            
            if newController == _currentContentController {
                return
            }
            
            guard let window = window else { return assertionFailure("MainWindow is nil") }
            
            _currentContentController?.viewWillDisappear()
            _ = newController.view
            newController.viewWillAppear()
            
            let viewToReplace = currentContentController?.view
            viewToReplace?.removeFromSuperview()
            _currentContentController?.viewDidDisappear()
            
            // Adjust visual properties of the window.
            var overlapsTitleBar = false
            if newController == gameDocument?.gameViewController {
                window.toolbar = nil
                window.titleVisibility = .visible
                window.titlebarAppearsTransparent = false
                overlapsTitleBar = false
            }
            else if newController == libraryController {
                window.toolbar = libraryController.toolbar
                window.titleVisibility = .hidden
                window.titlebarAppearsTransparent = false
                window.styleMask.insert(.closable)
                overlapsTitleBar = true
            }
            else if newController is SetupAssistant {
                window.toolbar = nil
                window.titleVisibility = .hidden
                window.titlebarAppearsTransparent = true
                window.styleMask.remove(.closable)
                overlapsTitleBar = true
            }
            
            // Set the size of the new controller *before* it is added to the
            // view hierarchy. This is important because things like NSSplitter
            // sizes are loaded as soon as the view is added to a subview.
            newController.view.frame = placeholderView.bounds
            newController.view.autoresizingMask = [.width, .height]
            placeholderView.addSubview(newController.view)
            
            let windowGuide = window.contentLayoutGuide as! NSLayoutGuide
            lastPlaceholderViewTopConstraint.isActive = false
            if overlapsTitleBar {
                lastPlaceholderViewTopConstraint = placeholderView.topAnchor.constraint(equalTo: window.contentView!.topAnchor)
            } else {
                lastPlaceholderViewTopConstraint = placeholderView.topAnchor.constraint(equalTo: windowGuide.topAnchor)
            }
            lastPlaceholderViewTopConstraint.isActive = true
            
            window.makeFirstResponder(newController.view)
            
            _currentContentController = newController
            if newController == gameDocument?.gameViewController {
                gameDocument?.gameWindowController = self
            }
            
            newController.viewDidAppear()
            
            // If a game is playing in the library window, unpause emulation immediately.
            if newController == gameDocument?.gameViewController {
                gameDocument?.isEmulationPaused = false
            }
        }
    }
    
    deinit {
        currentContentController = nil
        placeholderView = nil
    }
    
    override func windowDidLoad() {
        super.windowDidLoad()
        
        setUpPlaceholderView()
        setUpLibraryController()
        setUpWindow()
        setUpCurrentContentController()
        setUpViewMenuItemBindings()
    }
    
    private func setUpPlaceholderView() {
        placeholderView.translatesAutoresizingMaskIntoConstraints = false
        
        let windowGuide = window!.contentLayoutGuide as! NSLayoutGuide
        lastPlaceholderViewTopConstraint = windowGuide.topAnchor.constraint(equalTo: placeholderView.topAnchor)
        
        NSLayoutConstraint.activate([
            windowGuide.leftAnchor.constraint(equalTo: placeholderView.leftAnchor),
            windowGuide.bottomAnchor.constraint(equalTo: placeholderView.bottomAnchor),
            windowGuide.rightAnchor.constraint(equalTo: placeholderView.rightAnchor),
            lastPlaceholderViewTopConstraint,
        ])
    }
    
    private func setUpLibraryController() {
        libraryController.delegate = self
        _ = libraryController.view
        
        window?.toolbar = libraryController.toolbar
        if #available(macOS 11.0, *) {
            window?.toolbarStyle = .unified
            window?.titlebarSeparatorStyle = .line
        }
    }
    
    private func setUpWindow() {
        window?.delegate = self
        window?.isExcludedFromWindowsMenu = true
        window?.restorationClass = type(of: self)
        window?.backgroundColor = .controlBackgroundColor
        
        assert(window?.identifier == .mainWindow, "Main library window identifier does not match between nib and code")
    }
    
    private func setUpCurrentContentController() {
        if !UserDefaults.standard.bool(forKey: SetupAssistant.hasFinishedKey) {
            let setupAssistant = SetupAssistant()
            setupAssistant.completionBlock = {
                self.currentContentController = self.libraryController
                
                // FIXME: Hack, reloads sidebar to make systems show up
                NotificationCenter.default.post(name: .OELibraryLocationDidChange, object: nil)
            }
            
            window?.center()
            
            currentContentController = setupAssistant
        } else {
            currentContentController = libraryController
        }
    }
    
    private func setUpViewMenuItemBindings() {
        let viewMenu = NSApp.mainMenu?.item(at: 3)?.submenu
        let showLibraryNames = viewMenu?.item(withTag: 10)
        let showRomNames = viewMenu?.item(withTag: 11)
        let undockGameWindow = viewMenu?.item(withTag: 3)
        
        let negateOptions = [NSBindingOption.valueTransformerName: NSValueTransformerName.negateBooleanTransformerName]
        
        showLibraryNames?.bind(.enabled, to: self, withKeyPath: "mainWindowRunsGame", options: negateOptions)
        showRomNames?.bind(.enabled, to: self, withKeyPath: "mainWindowRunsGame", options: negateOptions)
        undockGameWindow?.bind(.enabled, to: self, withKeyPath: "mainWindowRunsGame", options: [:])
    }
    
    // MARK: - Actions
    
    @IBAction func undockGameWindow(_ sender: Any?) {
        mainWindowRunsGame = false
        
        if shouldExitFullScreenWhenGameFinishes && window?.isFullScreen ?? false {
            window?.toggleFullScreen(self)
            shouldExitFullScreenWhenGameFinishes = false
            shouldUndockGameWindowOnFullScreenExit = true
        } else {
            gameDocument?.isEmulationPaused = true
            currentContentController = nil
            gameDocument?.gameWindowController = nil
            
            gameDocument?.showInSeparateWindow(inFullScreen: false)
            gameDocument = nil
        }
    }
    
    @IBAction func launchLastPlayedROM(_ sender: NSMenuItem) {
        let game = (sender.representedObject as? OEDBRom)?.game
        openGameDocument(with: game, saveState: nil)
    }
    
    // TODO: ugly hack, remove
    @objc func startGame(_ game: OEDBGame?) {
        openGameDocument(with: game, saveState: game?.autosaveForLastPlayedRom)
    }
}

extension MainWindowController: LibraryControllerDelegate {
    
    func libraryController(_ controller: LibraryController, didSelectGame game: OEDBGame) {
        openGameDocument(with: game, saveState: nil)
    }
    
    func libraryController(_ controller: LibraryController, didSelectSaveState saveState: OEDBSaveState) {
        openGameDocument(with: nil, saveState: saveState)
    }
    
    @available(macOS, deprecated: 10.15, message: "Remove 'or \"User Reports\"' from alert (~line 392) and localizations, the term is not used in Catalina and above.")
    private func openGameDocument(with game: OEDBGame?, saveState state: OEDBSaveState?, secondAttempt retry: Bool = false, disableAutoReload noAutoReload: Bool = false) {
        
        guard let window = window else { return assertionFailure("MainWindow is nil") }
        
        // make sure we don't launch a game multiple times
        if isLaunchingGame { return }
        isLaunchingGame = true
        
        let defaults = UserDefaults.standard
        let openInSeparateWindow = mainWindowRunsGame || defaults.bool(forKey: OEForcePopoutGameWindowKey)
        let fullScreen = defaults.bool(forKey: OEFullScreenGameWindowKey)
        
        var openWithSaveState = state != nil
        let state = state != nil ? state : game?.autosaveForLastPlayedRom
        if !noAutoReload {
            openWithSaveState = openWithSaveState || (state != nil && OEAlert.loadAutoSaveGame().runModal() == .alertFirstButtonReturn)
        }
        
        shouldExitFullScreenWhenGameFinishes = false
        let openDocument: ((OEGameDocument?, Error?) -> Void) = { document, error in
            self.isLaunchingGame = false
            if document == nil {
                
                if let error = error as? OEGameDocument.Errors,
                   case OEGameDocument.Errors.fileDoesNotExist = error,
                   let game = game {
                    game.status = OEDBGameStatus.alert.rawValue as NSNumber
                    game.save()
                    
                    let messageText = String(format: NSLocalizedString("The game '%@' could not be started because a rom file could not be found. Do you want to locate it?", comment: ""), game.name ?? "")
                    let alert = OEAlert()
                    alert.messageText = messageText
                    alert.defaultButtonTitle = NSLocalizedString("Locate…", comment: "")
                    alert.alternateButtonTitle = NSLocalizedString("Cancel", comment: "")
                    if alert.runModal() == .alertFirstButtonReturn {
                        let missingRom = game.roms?.first
                        let originalURL = missingRom?.url
                        let fileType = originalURL?.pathExtension
                        
                        let panelTitle = String(format: NSLocalizedString("Locate '%@'", comment: "Locate panel title"), originalURL?.pathComponents.last ?? "")
                        let panel = NSOpenPanel()
                        panel.message = panelTitle
                        panel.canChooseDirectories = false
                        panel.canChooseFiles = true
                        panel.directoryURL = originalURL?.deletingLastPathComponent()
                        panel.allowsOtherFileTypes = false
                        panel.allowedFileTypes = fileType != nil ? [fileType!] : nil
                        
                        if panel.runModal() == .OK {
                            missingRom?.url = panel.url
                            missingRom?.save()
                            self.openGameDocument(with: game, saveState: state)
                        }
                    }
                }
                // FIXME: make it possible to locate missing rom files when the game is started from a savestate
                else if let error = error as? OEGameDocument.Errors,
                        case OEGameDocument.Errors.fileDoesNotExist = error,
                        game == nil {
                    var messageText = NSLocalizedString("The game '%@' could not be started because a rom file could not be found. Do you want to locate it?", comment: "")
                    
                    let regex = try? NSRegularExpression(pattern: "[\"'“„« ]+%@[\"'”“» ]+[ を]?", options: .caseInsensitive)
                    messageText = regex?.stringByReplacingMatches(in: messageText, options: [], range: NSRange(location: 0, length: messageText.count), withTemplate: "") ?? ""
                    
                    var range = (messageText as NSString).range(of: ".")
                    if range.location == NSNotFound {
                        range = (messageText as NSString).range(of: "。")
                    }
                    if range.location != NSNotFound {
                        messageText = (messageText as NSString).substring(to: range.location + 1)
                    }
                    
                    messageText = messageText + NSLocalizedString(" Start the game from the library view if you want to locate it.", comment: "")
                    
                    let alert = OEAlert()
                    alert.messageText = messageText
                    alert.defaultButtonTitle = NSLocalizedString("OK", comment: "")
                    alert.runModal()
                }
                else if let error = error as? OEGameDocument.Errors,
                        case OEGameDocument.Errors.noCore = error,
                        !retry {
                    // Try downloading the core list before bailing out definitively
                    let alert = OEAlert()
                    alert.messageText = NSLocalizedString("Downloading core list...", comment: "")
                    alert.defaultButtonTitle = NSLocalizedString("Cancel", comment: "")
                    alert.performBlockInModalSession {
                        CoreUpdater.shared.checkForNewCores { error in
                            alert.close(withResult: .alertSecondButtonReturn)
                        }
                    }
                    if alert.runModal() == .alertFirstButtonReturn {
                        // user says no
                        CoreUpdater.shared.cancelCheckForNewCores()
                        DispatchQueue.main.async {
                            self.presentError(error)
                        }
                    } else {
                        // let's give it another try
                        self.openGameDocument(with: game, saveState: state, secondAttempt: true, disableAutoReload: false)
                    }
                }
                else if let error = error as? OEGameDocument.Errors,
                        case OEGameDocument.Errors.gameCoreCrashed(let core, let systemIdentifier, _) = error {
                    // TODO: the setup completion handler shouldn't be the place where non-setup-related errors are handled!
                    let coreName = core.displayName ?? ""
                    let glitchy = core.controller.hasGlitches(forSystemIdentifier: systemIdentifier) 
                    
                    let alert = OEAlert()
                    if openWithSaveState {
                        alert.messageText = String(format: NSLocalizedString("The %@ core has quit unexpectedly after loading a save state", comment: ""), coreName)
                        alert.informativeText = NSLocalizedString("Sometimes a crash may occur while loading a save state because of incompatibilities between different versions of the same core. Do you want to open the game without loading a save state?", comment: "")
                        alert.defaultButtonTitle = NSLocalizedString("Reopen without loading state", comment: "")
                        alert.alternateButtonTitle = NSLocalizedString("Cancel", comment: "")
                        let resp = alert.runModal()
                        if resp == .alertFirstButtonReturn {
                            self.openGameDocument(with: state?.rom?.game, saveState: nil, secondAttempt: true, disableAutoReload: true)
                        }
                    } else {
                        alert.messageText = String(format: NSLocalizedString("The %@ core has quit unexpectedly", comment: ""), coreName)
                        if glitchy {
                            alert.informativeText = String(format: NSLocalizedString("The %@ core has compatibility issues and some games may contain glitches or not play at all.\n\nPlease do not report problems as we are not responsible for the development of %@.", comment: ""), coreName, coreName)
                        } else {
                            alert.informativeText = NSLocalizedString(
                                """
                                <b>If and only if this issue persists</b>, please submit feedback including:<br><br>\
                                <ul>\
                                <li>The model of Mac you are using <b>and</b> the version of macOS you have installed\
                                <ul><li>This information is found in  > About this Mac</li></ul></li>\
                                <li>The <b>exact name</b> of the game you were playing</li>\
                                <li>The crash report of OpenEmuHelperApp\
                                <ul><li>Open Console.app, click on "Crash Reports" or "User Reports" in the sidebar, \
                                then look for the latest document with OpenEmuHelperApp in the name</ul></li></li>\
                                </ul><br>\
                                <b>Always search for similar feedback previously reported by other users!</b><br>\
                                If any of this information is omitted, or if similar feedback has already been issued, your issue report may be closed.
                                """,
                                comment: "Suggestion for crashed cores (HTML). Localizers: specify the report must be written in English"
                            )
                            alert.messageUsesHTML = true
                        }
                        alert.defaultButtonTitle = NSLocalizedString("OK", comment: "")
                        alert.runModal()
                    }
                }
                else if let error = error as NSError?, !(error.domain == NSCocoaErrorDomain && error.code == NSUserCancelledError) {
                    DispatchQueue.main.async {
                        self.presentError(error)
                    }
                }
                
                return
            }
            else if game?.status?.int16Value == OEDBGameStatus.alert.rawValue {
                game?.status = OEDBGameStatus.OK.rawValue as NSNumber
                game?.save()
            }
            
            if openInSeparateWindow {
                return
            }
            
            self.shouldExitFullScreenWhenGameFinishes = !window.isFullScreen
            self.gameDocument = document
            self.mainWindowRunsGame = true
            
            if fullScreen && !window.isFullScreen {
                NSApp.activate(ignoringOtherApps: true)
                
                self.currentContentController = document?.gameViewController
                document?.isEmulationPaused = false
                window.toggleFullScreen(self)
            } else {
                document?.gameWindowController = self
                self.currentContentController = document?.gameViewController
            }
        }
        
        if openWithSaveState {
            NSDocumentController.shared.openGameDocument(with: state!, display: openInSeparateWindow, fullScreen: fullScreen, completionHandler: openDocument)
        } else {
            NSDocumentController.shared.openGameDocument(with: game!, display: openInSeparateWindow, fullScreen: fullScreen, completionHandler: openDocument)
        }
    }
}

extension MainWindowController: NSWindowDelegate {
    
    func windowDidBecomeKey(_ notification: Notification) {
        currentContentController?.viewWillAppear()
        currentContentController?.viewDidAppear()
    }
    
    func windowDidBecomeMain(_ notification: Notification) {
        let windowMenu = NSApp.mainMenu?.item(at: 5)?.submenu
        let item = windowMenu?.item(withTag: 501)
        item?.state = .on
    }
    
    func windowDidResignMain(_ notification: Notification) {
        let windowMenu = NSApp.mainMenu?.item(at: 5)?.submenu
        let item = windowMenu?.item(withTag: 501)
        item?.state = .off
    }
    
    func windowWillClose(_ notification: Notification) {
        // Make sure the current content controller gets viewWillDisappear / viewDidAppear so it has a chance to store its state
        if currentContentController == libraryController {
            libraryController.viewWillDisappear()
            libraryController.viewDidDisappear()
            libraryController.viewWillAppear()
            libraryController.viewDidAppear()
        }
        
        currentContentController = nil
    }
    
    func windowWillEnterFullScreen(_ notification: Notification) {
        if let gameDocument = gameDocument, gameDocument.gameWindowController == self {
            resumePlayingAfterFullScreenTransition = !gameDocument.isEmulationPaused
            gameDocument.isEmulationPaused = true
        }
    }
    
    func windowDidEnterFullScreen(_ notification: Notification) {
        guard let gameDocument = gameDocument else { return }
        
        if shouldExitFullScreenWhenGameFinishes {
            gameDocument.gameWindowController = self
            currentContentController = gameDocument.gameViewController
        }
        
        if resumePlayingAfterFullScreenTransition {
            gameDocument.isEmulationPaused = false
        }
    }
    
    func windowWillExitFullScreen(_ notification: Notification) {
        if let gameDocument = gameDocument {
            resumePlayingAfterFullScreenTransition = !gameDocument.isEmulationPaused
            gameDocument.isEmulationPaused = true
        }
    }
    
    func windowDidExitFullScreen(_ notification: Notification) {
        if shouldUndockGameWindowOnFullScreenExit {
            shouldUndockGameWindowOnFullScreenExit = false
            
            currentContentController = nil
            
            gameDocument?.showInSeparateWindow(inFullScreen: false)
            
            if resumePlayingAfterFullScreenTransition {
                gameDocument?.isEmulationPaused = false
            }
            
            gameDocument = nil
            mainWindowRunsGame = false
        }
        else if let gameDocument = gameDocument, resumePlayingAfterFullScreenTransition {
            gameDocument.isEmulationPaused = false
        }
    }
    
    func windowShouldClose(_ sender: NSWindow) -> Bool {
        if currentContentController == libraryController {
            return true
        } else {
            gameDocument?.canClose { document, shouldClose in
                self.gameDocument?.gameWindowController = nil
                self.gameDocument?.close()
                self.gameDocument = nil
                self.mainWindowRunsGame = false
                
                let exitFullScreen = self.shouldExitFullScreenWhenGameFinishes && self.window?.isFullScreen ?? false
                if exitFullScreen {
                    self.window?.toggleFullScreen(self)
                    self.shouldExitFullScreenWhenGameFinishes = false
                }
                
                self.currentContentController = nil
            }
            
            return false
        }
    }
}

extension MainWindowController: NSWindowRestoration {
    
    static func restoreWindow(withIdentifier identifier: NSUserInterfaceItemIdentifier, state: NSCoder, completionHandler: @escaping (NSWindow?, Error?) -> Void) {
        
        if identifier == .mainWindow,
           let appDelegate = NSApp.delegate as? AppDelegate {
            
            NSApp.extendStateRestoration()
            appDelegate.restoreWindow = true
            
            let completionHandlerCopy = completionHandler
            let observerOfLibraryDidLoad = NotificationCenter.default.addObserver(forName: .OELibraryDidLoadNotificationName, object: nil, queue: .main) { _ in
                let mainWindowController = self.init(windowNibName: "MainWindow")
                appDelegate.mainWindowController = mainWindowController
                let mainWindow = mainWindowController.window
                
                completionHandlerCopy(mainWindow, nil)
                
                NSApp.completeStateRestoration()
            }
            
            appDelegate.libraryDidLoadObserverForRestoreWindow = observerOfLibraryDidLoad
            return
        }
        completionHandler(nil, nil)
    }
}
