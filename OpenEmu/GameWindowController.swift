// Copyright (c) 2023, OpenEmu Team
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
import OpenEmuBase.OEGeometry
import OpenEmuKit

let OEPopoutGameWindowAlwaysOnTopKey = "OEPopoutGameWindowAlwaysOnTop"
let OEPopoutGameWindowBackgroundColorKey = "gameViewBackgroundColor"
let OEPopoutGameWindowIntegerScalingOnlyKey = "OEPopoutGameWindowIntegerScalingOnly"
let OEPopoutGameWindowTreatScaleFactorAsPixels = "OEPopoutGameWindowTreatScaleFactorAsPixels"
let OEAdaptiveSyncEnabledKey = "OEAdaptiveSyncEnabled"

final class GameWindowController: NSWindowController {
    
    private enum FullScreenStatus {
        case nonFullScreen, fullScreen
        case entering, exiting
    }
    
    private static let systemIntegralScaleKeyFormat = "OEIntegralScale.%@" // Dictionary
    private static let windowedIntegralScaleKey = "integralScale" // Int
    private static let fullScreenIntegralScaleKey = "fullScreenIntegralScale" // Int
    private static let lastWindowSizeKey = "lastPopoutWindowSize" // String (NSSize)
    
    private static let windowMinSize = NSSize(width: 100, height: 100)
    private static let fitToWindowScale = 0
    
    var shouldShowWindowInFullScreen = false
    
    private var screenBeforeWindowMove: NSScreen?
    
    private var screenshotWindow: ScreenshotWindow
    private var fullScreenStatus: FullScreenStatus = .nonFullScreen
    private var fullScreenIntegralScale = 0
    private var resumePlayingAfterFullScreenTransition = false
    private var adaptiveSyncWasEnabled = false
    
    private var isSnapResizing = false
    private var snapDelegate: OEIntegralWindowResizingDelegate
    
    /// State prior to entering full screen
    private var windowedFrame = NSRect.zero
    private var windowedIntegralScale = 0
    
    private var eventMonitor: Any?
    
    override init(window: NSWindow?) {
        snapDelegate = OEIntegralWindowResizingDelegate()
        
        let screenFrame = NSScreen.screens.first?.frame ?? .zero
        screenshotWindow = ScreenshotWindow(screenFrame: screenFrame)
        
        super.init(window: window)
        
        if let window {
            window.delegate = self
            window.collectionBehavior = .fullScreenPrimary
            window.animationBehavior = .documentWindow
            window.minSize = Self.windowMinSize
            
            if UserDefaults.standard.bool(forKey: OEPopoutGameWindowAlwaysOnTopKey) {
                window.level = .floating
            }
        }
        
        let nc = NotificationCenter.default
        nc.addObserver(self,
                       selector: #selector(constrainIntegralScaleIfNeeded),
                       name: NSApplication.didChangeScreenParametersNotification,
                       object: nil)
        
        eventMonitor = NSEvent.addLocalMonitorForEvents(matching: [.flagsChanged, .keyDown]) { [weak self] event in
            guard let self = self else { return event }
            
            // shift key pressed
            if event.modifierFlags.contains(.shift) && event.keyCode == 56 {
                self.updateContentSizeConstraints()
                self.window?.resetCursorRects()
            }
            // shift key released
            else if event.keyCode == 56 {
                self.updateContentSizeConstraints()
                self.window?.resetCursorRects()
            }
            
            return event
        }
    }
    
    required init?(coder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }
    
    deinit {
        window?.delegate = nil
        window = nil
        
        if let eventMonitor {
            NSEvent.removeMonitor(eventMonitor)
            self.eventMonitor = nil
        }
        
        let nc = NotificationCenter.default
        nc.removeObserver(self,
                          name: NSApplication.didChangeScreenParametersNotification,
                          object: nil)
    }
    
    private var gameDocument: OEGameDocument! {
        return document as? OEGameDocument
    }
    
    override var document: AnyObject? {
        willSet {
            assert(newValue == nil || newValue is OEGameDocument, "GameWindowController accepts OEGameDocument documents only")
        }
        didSet {
            if document != nil {
                var windowSize = NSSize.zero
                loadScaleDefaults(windowSize: &windowSize)
                // cache the size before setting the frame, as the windowWillResize(_:to:)
                // delegate method overwrites the value with fitToWindowScale
                let integralScale = windowedIntegralScale
                
                let windowRect = NSRect(origin: .zero, size: windowSize)
                
                let gameViewController = gameDocument.gameViewController
                gameViewController?.integralScalingDelegate = self
                
                if let window {
                    window.contentViewController = gameViewController
                    window.setFrame(windowRect, display: false, animate: false)
                    window.center()
                    window.contentAspectRatio = gameViewController?.defaultScreenSize ?? .zero
                    window.tabbingIdentifier = gameDocument.systemIdentifier
                }
                
                windowedIntegralScale = integralScale
                
                updateContentSizeConstraints()
            } else {
                window?.contentViewController = nil
            }
        }
    }
    
    override func cancelOperation(_ sender: Any?) {
        if let window, window.isFullScreen {
            window.toggleFullScreen(self)
        }
    }
    
    override func showWindow(_ sender: Any?) {
        guard let window else { return }
        
        let needsToggleFullScreen = window.isFullScreen != shouldShowWindowInFullScreen
        
        if !window.isVisible {
            // We disable window animation if we need to toggle full screen because two parallel animations
            // (window being ordered front and toggling full-screen) looks painfully ugly. The animation
            // behaviour is restored in windowDidExitFullScreen(_:). Note: Since 0bf417b, it’s not.
            if needsToggleFullScreen {
                window.animationBehavior = .none
            }
            
            window.makeKeyAndOrderFront(sender)
        }
        
        if needsToggleFullScreen {
            window.toggleFullScreen(self)
        }
    }
    
    private func hideScreenshotWindow() {
        screenshotWindow.orderOut(self)
        
        // Reduce the memory footprint of the screenshot window when it’s not visible
        screenshotWindow.setScreenshot(nil)
    }
    
    /// Load window defaults
    private func loadScaleDefaults(windowSize: inout NSSize) {
        let systemID = gameDocument.systemIdentifier
        let key = String(format: Self.systemIntegralScaleKeyFormat, systemID)
        let integralScaleInfo = UserDefaults.standard.dictionary(forKey: key)
        
        let maxScale = maximumIntegralScale
        let windowedScale = integralScaleInfo?[Self.windowedIntegralScaleKey] as? Int
        if let windowedScale {
            windowedIntegralScale = min(windowedScale, maxScale)
        } else {
            windowedIntegralScale = maxScale
        }
        
        if windowedIntegralScale == Self.fitToWindowScale {
            if let windowSizeString = integralScaleInfo?[Self.lastWindowSizeKey] as? String {
                windowSize = NSSizeFromString(windowSizeString)
            }
            if windowSize.width == 0 || windowSize.height == 0 {
                windowSize = self.windowSize(forGameViewIntegralScale: maxScale)
            }
        } else {
            windowSize = self.windowSize(forGameViewIntegralScale: windowedIntegralScale)
        }
        
        let fullScreenScale = integralScaleInfo?[Self.fullScreenIntegralScaleKey] as? Int
        fullScreenIntegralScale = fullScreenScale ?? Self.fitToWindowScale
    }
    
    private func saveScaleDefaults() {
        let windowSize: NSSize
        if let window, !window.isFullScreen {
            windowSize = window.frame.size
        } else {
            windowSize = windowedFrame.size
        }
        let integralScaleInfo: [String: Any] = [
            Self.windowedIntegralScaleKey: windowedIntegralScale,
            Self.lastWindowSizeKey: NSStringFromSize(windowSize),
            Self.fullScreenIntegralScaleKey: fullScreenIntegralScale,
        ]
        let systemID = gameDocument.systemIdentifier
        let key = String(format: Self.systemIntegralScaleKeyFormat, systemID)
        UserDefaults.standard.set(integralScaleInfo, forKey: key)
    }
    
    private var shouldSnapResize: Bool {
        let snapResizeEnabled = UserDefaults.standard.bool(forKey: OEPopoutGameWindowIntegerScalingOnlyKey)
        let shiftKeyPressed = NSEvent.modifierFlags.contains(.shift)
        
        return (snapResizeEnabled && !shiftKeyPressed) || (!snapResizeEnabled && shiftKeyPressed)
    }
    
    private func updateContentSizeConstraints() {
        // Set contentMin/MaxSize to get the mouse pointer to correctly indicate that the window size
        // cannot be further de-/increased at min/max scale if snap resizing is enabled.
        
        // Exempt NDS where changing the screen configuration from the display mode menu causes issues FIXME:
        if gameDocument?.systemIdentifier == "openemu.system.nds" {
            return
        }
        
        if shouldSnapResize && fullScreenStatus != .entering {
            let minSize = gameDocument?.gameViewController.defaultScreenSize ?? .zero
            let maxSize = NSSize(width: Int(minSize.width) * maximumIntegralScale,
                                 height: Int(minSize.height) * maximumIntegralScale)
            window?.contentMinSize = minSize
            window?.contentMaxSize = maxSize
        } else {
            window?.contentMinSize = Self.windowMinSize
            window?.contentMaxSize = NSSize(width: CGFloat.greatestFiniteMagnitude,
                                            height: CGFloat.greatestFiniteMagnitude)
        }
    }
    
    /// This constant is required as macOS does not report the correct frame size with safe area
    /// for full screen windows.
    ///
    /// For example, on a MacBook Pro, the built-in screen with integrated camera reports
    ///
    /// * a height of 1117 points
    /// * a safe area of top = 32 points, other margins 0 points
    ///
    /// That reduces the size to 1085 points, however, reading the ``NSWindow`` frame
    /// after transitioning to full-screen, the final size is 1080 points. Hence, a constant of 5 points.
    /// ⚠️ The 5 points remains constant across all tested resolutions.
    static let extraMarginForFullScreenWithSafeArea = 5.0
    
    func fullScreenContentSizeForScreen(_ screen: NSScreen) -> CGSize {
        var screenSize = screen.frame.size
        if #available(macOS 12.0, *) {
            let safeAreaInsets = screen.safeAreaInsets
            if safeAreaInsets.top > 0 {
                // HACK(sgc): only adjust if there is an integrated camera inset
                let ss = screen.frame.size
                screenSize = CGSize(width: ss.width - (safeAreaInsets.left + safeAreaInsets.right),
                                    height: ss.height - (safeAreaInsets.top + safeAreaInsets.bottom + Self.extraMarginForFullScreenWithSafeArea))
            }
        }
        return screenSize
    }
    
    func fullScreenWindowFrameForScreen(_ screen: NSScreen) -> NSRect {
        var screenFrame = screen.frame
        if #available(macOS 12.0, *) {
            // adjust frame based on insets
            let safeArea = screen.safeAreaInsets;
            screenFrame.origin = screenFrame.origin.applying(.identity.translatedBy(x: safeArea.left, y: safeArea.top))
            screenFrame.size   = fullScreenContentSizeForScreen(screen)
        }
        return screenFrame
    }
    
    /// A size with the same aspect ratio as the game document, matching the screen size in one dimension.
    private var fillScreenContentSize: NSSize {
        guard let screen = window?.screen else {
            return .zero
        }
        
        // Exempt NDS because defaultScreenSize is not updated after changing the screen configuration from the display mode menu FIXME:
        if gameDocument.systemIdentifier == "openemu.system.nds" {
            return screen.frame.size
        }
        
        let screenSize = fullScreenContentSizeForScreen(screen)
        
        let gameSize = gameDocument.gameViewController.defaultScreenSize
        var newSize = OEIntSize()
        
        // pillarboxed or no border
        if screenSize.width / screenSize.height - gameSize.width / gameSize.height >= 0 {
            newSize.height = Int32(screenSize.height)
            newSize.width = Int32(round(gameSize.width * screenSize.height / gameSize.height))
        }
        // letterboxed
        else {
            newSize.width = Int32(screenSize.width)
            newSize.height = Int32(round(gameSize.height * screenSize.width / gameSize.width))
        }
        
        return CGSizeFromOEIntSize(newSize)
    }
    
    // MARK: - Actions
    
    @IBAction func changeIntegralScale(_ sender: AnyObject) {
        if let newScale = sender.representedObject as? Int,
           newScale <= maximumIntegralScale
        {
            changeGameViewIntegralScale(newScale)
        }
    }
    
    @IBAction func floatOnTop(_ sender: Any?) {
        if window?.level == .normal {
            window?.level = .floating
        } else {
            window?.level = .normal
        }
    }
    
    // MARK: - Integral Scaling
    
    private func windowContentSize(forGameViewIntegralScale integralScale: Int) -> NSSize {
        let defaultSize = gameDocument.gameViewController.defaultScreenSize
        var contentSize = defaultSize.scaled(by: CGFloat(integralScale))
        
        if UserDefaults.standard.bool(forKey: OEPopoutGameWindowTreatScaleFactorAsPixels),
           let backingScaleFactor = window?.backingScaleFactor
        {
            contentSize = NSSize(width: contentSize.width / backingScaleFactor,
                                 height: contentSize.height / backingScaleFactor)
        }
        return contentSize
    }
    
    private func windowSize(forGameViewIntegralScale integralScale: Int) -> NSSize {
        guard let window else { return .zero }
        
        let contentSize = windowContentSize(forGameViewIntegralScale: integralScale)
        return window.frameRect(forContentRect: NSRect(origin: .zero, size: contentSize)).size
    }
    
    private func changeGameViewIntegralScale(_ newScale: Int) {
        if fullScreenStatus == .nonFullScreen {
            if UserDefaults.standard.bool(forKey: OEPopoutGameWindowTreatScaleFactorAsPixels) == false,
               windowedIntegralScale == newScale
            {
                return
            }
            
            windowedIntegralScale = newScale
            
            if newScale == Self.fitToWindowScale {
                return
            }
            
            let screenFrame = window?.screen?.visibleFrame ?? .zero
            let currentWindowFrame = window?.frame ?? .zero
            var newWindowFrame = NSRect(origin: .zero,
                                        size: windowSize(forGameViewIntegralScale: newScale))
            
            newWindowFrame.origin.y = round(currentWindowFrame.midY - newWindowFrame.size.height / 2)
            newWindowFrame.origin.x = round(currentWindowFrame.midX - newWindowFrame.size.width / 2)
            
            // Make sure the entire window is visible, centering it in case it isn’t
            if newWindowFrame.minY < screenFrame.minY || newWindowFrame.maxY > screenFrame.maxY {
                newWindowFrame.origin.y = screenFrame.minY + ((screenFrame.size.height - newWindowFrame.size.height) / 2)
            }
            
            if newWindowFrame.minX < screenFrame.minX || newWindowFrame.maxX > screenFrame.maxX {
                newWindowFrame.origin.x = screenFrame.minX + ((screenFrame.size.width - newWindowFrame.size.width) / 2)
            }
            
            if NSWorkspace.shared.accessibilityDisplayShouldReduceMotion {
                window?.setFrame(newWindowFrame, display: true)
            } else {
                window?.animator().setFrame(newWindowFrame, display: true)
            }
        } else { // fullScreenStatus == .fullScreen
            if UserDefaults.standard.bool(forKey: OEPopoutGameWindowTreatScaleFactorAsPixels) == false,
               fullScreenIntegralScale == newScale
            {
                return
            }
            
            fullScreenIntegralScale = newScale
            
            let gv = gameDocument.gameViewController
            if newScale == Self.fitToWindowScale {
                gv?.gameViewSetIntegralSize(fillScreenContentSize, animated: true)
            } else {
                let newSize = windowSize(forGameViewIntegralScale: newScale)
                gv?.gameViewSetIntegralSize(newSize, animated: true)
            }
        }
    }
    
    @objc private func constrainIntegralScaleIfNeeded() {
        if fullScreenStatus != .nonFullScreen || windowedIntegralScale == Self.fitToWindowScale {
            return
        }
        
        let newMaxScale = maximumIntegralScale
        let newScreenFrame = window?.screen?.visibleFrame ?? .zero
        let currentFrame = window?.frame ?? .zero
        
        if newScreenFrame.size.width < currentFrame.size.width || newScreenFrame.size.height < currentFrame.size.height {
            changeGameViewIntegralScale(newMaxScale)
            updateContentSizeConstraints()
        }
    }
}

// MARK: - NSMenuItemValidation

extension GameWindowController: NSMenuItemValidation {
    
    func validateMenuItem(_ menuItem: NSMenuItem) -> Bool {
        switch menuItem.action {
        case #selector(floatOnTop(_:)):
            if window?.level == .floating {
                menuItem.state = .on
            } else {
                menuItem.state = .off
            }
            return true
        default:
            return true
        }
    }
}

// MARK: - GameIntegralScalingDelegate

extension GameWindowController: GameIntegralScalingDelegate {
    
    var shouldAllowIntegralScaling: Bool {
        true
    }
    
    var maximumIntegralScale: Int {
        guard let screen = window?.screen ?? NSScreen.main,
              var maxContentSize = window?.contentRect(forFrameRect: screen.visibleFrame).size,
              let defaultSize = gameDocument?.gameViewController.defaultScreenSize
        else { return 1 }
        
        if UserDefaults.standard.bool(forKey: OEPopoutGameWindowTreatScaleFactorAsPixels),
           let backingScaleFactor = window?.backingScaleFactor
        {
            maxContentSize = NSSize(width: maxContentSize.width * backingScaleFactor,
                                    height: maxContentSize.height * backingScaleFactor)
        }
        let maxScale = max(min(floor(maxContentSize.height / defaultSize.height), floor(maxContentSize.width / defaultSize.width)), 1)
        
        return Int(maxScale)
    }
    
    var currentIntegralScale: Int {
        switch fullScreenStatus {
        case .nonFullScreen,
                .exiting:
            return windowedIntegralScale
        default:
            return fullScreenIntegralScale
        }
    }
}

// MARK: - NSWindowDelegate

extension GameWindowController: NSWindowDelegate {
    
    func windowShouldClose(_ sender: NSWindow) -> Bool {
        return document != nil
    }
    
    func windowWillClose(_ notification: Notification) {
        saveScaleDefaults()
    }
    
    func windowWillMove(_ notification: Notification) {
        if fullScreenStatus != .nonFullScreen {
            return
        }
        
        screenBeforeWindowMove = window?.screen
    }
    
    func windowDidMove(_ notification: Notification) {
        if fullScreenStatus != .nonFullScreen {
            return
        }
        
        if screenBeforeWindowMove != window?.screen {
            constrainIntegralScaleIfNeeded()
        }
        
        screenBeforeWindowMove = nil
    }
    
    func windowDidChangeScreen(_ notification: Notification) {
        if fullScreenStatus != .nonFullScreen {
            return
        }
        
        constrainIntegralScaleIfNeeded()
    }
    
    func windowWillStartLiveResize(_ notification: Notification) {
        if fullScreenStatus != .nonFullScreen {
            return
        }
        
        if shouldSnapResize {
            isSnapResizing = true
        }
        
        if isSnapResizing {
            window?.contentAspectRatio = .zero
            // Set resize increments is necessary to fix a crash / hang bug
            // on macOS 11.0+ (FB8943798) when setting (contentA|a)spectRatio = NSSize.zero.
            // Set aspect ratio and then resize increments *after* to work around the issue.
            window?.contentResizeIncrements = NSSize(width: 1, height: 1)
            snapDelegate.currentScale = windowedIntegralScale
            snapDelegate.screenSize = gameDocument.gameViewController.defaultScreenSize
            snapDelegate.windowWillStartLiveResize(notification)
        }
    }
    
    func windowWillResize(_ sender: NSWindow, to frameSize: NSSize) -> NSSize {
        if fullScreenStatus != .nonFullScreen {
            return frameSize
        }
        
        if isSnapResizing {
            return snapDelegate.windowWillResize(sender, to: frameSize)
        }
        
        windowedIntegralScale = Self.fitToWindowScale
        
        return frameSize
    }
    
    func windowDidEndLiveResize(_ notification: Notification) {
        if fullScreenStatus != .nonFullScreen {
            return
        }
        
        if isSnapResizing {
            window?.contentAspectRatio = gameDocument.gameViewController.defaultScreenSize
            snapDelegate.windowDidEndLiveResize(notification)
            windowedIntegralScale = snapDelegate.currentScale
            isSnapResizing = false
        }
        
        updateContentSizeConstraints()
    }
    
    func windowDidChangeBackingProperties(_ notification: Notification) {
        // If a game started on an x2 screen is moved to an x1 screen, ensure that the size stays about the same
        // and that we don’t end up with an x.5 scaling if the scale is explicitly set to an integer.
        if UserDefaults.standard.bool(forKey: OEPopoutGameWindowTreatScaleFactorAsPixels),
           let oldScaleFactor = notification.userInfo?[NSWindow.oldScaleFactorUserInfoKey] as? NSNumber,
           let backingScaleFactor = window?.backingScaleFactor,
           oldScaleFactor.doubleValue != backingScaleFactor
        {
            let newIntegralScale: Int
            if oldScaleFactor.doubleValue > backingScaleFactor {
                newIntegralScale = Int(min(ceil(Double(currentIntegralScale) / backingScaleFactor), Double(maximumIntegralScale)))
            } else {
                newIntegralScale = Int(min(Double(currentIntegralScale) * backingScaleFactor, Double(maximumIntegralScale)))
            }
            changeGameViewIntegralScale(newIntegralScale)
        }
    }
    
    // MARK: - NSWindowDelegate - Full Screen
    
    // Since resizing OEGameView produces choppy animation, we do the following:
    //
    // - Take a screenshot of the game viewport inside OEGameView and build a borderless window from that
    // - The screenshot window is the one whose resizing to/from full screen is animated
    // - The actual window is faded out and resized to its final animation size
    // - When the animation ends, the actual window is faded in and the screenshot window is removed
    //
    // Emulation is paused when the animation begins and resumed when the animation ends (unless emulation
    // was already paused in the first place).
    
    func windowWillEnterFullScreen(_ notification: Notification) {
        guard let window = notification.object as? NSWindow else {
            if #available(macOS 11.0, *) {
                Logger.default.error("\(#function): Expected NSWindow")
            }
            
            return
        }
        
        fullScreenStatus = .entering
        windowedFrame = window.frame
        
        resumePlayingAfterFullScreenTransition = !gameDocument.isEmulationPaused
        gameDocument.isEmulationPaused = true
        
        // move the screenshot window to the same screen as the game window
        // otherwise it will be shown in the system full-screen animation
        screenshotWindow.setFrameOrigin(window.screen?.frame.origin ?? .zero)
        
        let gameViewController = gameDocument.gameViewController
        gameViewController?.controlsWindow.canShow = false
        gameViewController?.controlsWindow.hide(animated: true)
        screenshotWindow.setScreenshot(gameDocument.screenshot())
        
        window.contentMinSize = Self.windowMinSize
        window.contentMaxSize = NSSize(width: CGFloat.greatestFiniteMagnitude,
                                       height: CGFloat.greatestFiniteMagnitude)
    }
    
    func customWindowsToEnterFullScreen(for window: NSWindow) -> [NSWindow]? {
        return [window, screenshotWindow]
    }
    
    func window(_ window: NSWindow, startCustomAnimationToEnterFullScreenWithDuration duration: TimeInterval) {
        let screenFrame = fullScreenWindowFrameForScreen(window.screen!)
        let hideBorderDuration = duration / 4
        let resizeDuration = duration - hideBorderDuration
        let sourceScreenshotFrame = window.contentRect(forFrameRect: window.frame)
        
        screenshotWindow.setFrame(sourceScreenshotFrame, display: true)
        screenshotWindow.orderFront(self)
        
        var targetScreenshotFrame = screenFrame
        if fullScreenIntegralScale != Self.fitToWindowScale {
            let targetContentSize = windowContentSize(forGameViewIntegralScale: fullScreenIntegralScale)
            let origin = NSPoint(x: (screenFrame.size.width - targetContentSize.width) / 2,
                                 y: (screenFrame.size.height - targetContentSize.height) / 2)
            targetScreenshotFrame = NSRect(origin: origin + screenFrame.origin, size: targetContentSize)
            targetScreenshotFrame = targetScreenshotFrame.integral
        }
        
        // Fade the real window out
        CATransaction.begin()
        defer { CATransaction.commit() }
        
        CATransaction.setAnimationDuration(hideBorderDuration)
        CATransaction.setAnimationTimingFunction(CAMediaTimingFunction(name: .easeInEaseOut))
        CATransaction.setCompletionBlock {
            window.setFrame(screenFrame, display: false)
            
            // Resize the screenshot window to fullscreen
            CATransaction.begin()
            defer { CATransaction.commit() }
            
            CATransaction.setAnimationDuration(resizeDuration)
            CATransaction.setAnimationTimingFunction(CAMediaTimingFunction(name: .easeInEaseOut))
            CATransaction.setCompletionBlock {
                window.alphaValue = 1
                self.hideScreenshotWindow()
            }
            
            self.screenshotWindow.animator().setFrame(targetScreenshotFrame, display: true)
        }
        
        window.animator().alphaValue = 0
    }
    
    func windowDidEnterFullScreen(_ notification: Notification) {
        guard let window = notification.object as? NSWindow else {
            if #available(macOS 11.0, *) {
                Logger.default.error("windowDidEnterFullScreen: Expected NSWindow")
            }
            
            return
        }
        
        fullScreenStatus = .fullScreen
        
        if resumePlayingAfterFullScreenTransition {
            gameDocument.isEmulationPaused = false
        }
        
        let gameViewController = gameDocument.gameViewController
        gameViewController?.controlsWindow.hide(animated: true)
        gameViewController?.controlsWindow.canShow = true
        
        let bgColorString = UserDefaults.standard.string(forKey: OEPopoutGameWindowBackgroundColorKey)
        var bgColor: NSColor?
        if let bgColorString {
            bgColor = NSColor(from: bgColorString)
        }
        window.backgroundColor = bgColor ?? .black
        
        if fullScreenIntegralScale != Self.fitToWindowScale {
            let size = windowContentSize(forGameViewIntegralScale: fullScreenIntegralScale)
            gameViewController?.gameViewSetIntegralSize(size, animated: false)
        } else {
            gameViewController?.gameViewSetIntegralSize(fillScreenContentSize, animated: false)
        }
        
        if window.isAdaptiveSyncSchedulingAvailable {
            if #available(macOS 11.0, *) {
                Logger.default.debug("Enabling adaptive sync.")
            }
            gameDocument.setAdaptiveSyncEnabled(true)
            adaptiveSyncWasEnabled = true
        }
    }
    
    func windowWillExitFullScreen(_ notification: Notification) {
        fullScreenStatus = .exiting
        resumePlayingAfterFullScreenTransition = !gameDocument.isEmulationPaused
        
        gameDocument.isEmulationPaused = true
        
        let gameViewController = gameDocument.gameViewController
        gameViewController?.controlsWindow.canShow = false
        gameViewController?.controlsWindow.hide(animated: true)
        
        if adaptiveSyncWasEnabled {
            gameDocument.setAdaptiveSyncEnabled(false)
            adaptiveSyncWasEnabled = false
        }
    }
    
    func customWindowsToExitFullScreen(for window: NSWindow) -> [NSWindow]? {
        return [window, screenshotWindow]
    }
    
    func window(_ window: NSWindow, startCustomAnimationToExitFullScreenWithDuration duration: TimeInterval) {
        let gameViewController = gameDocument.gameViewController
        let screenFrame = fullScreenWindowFrameForScreen(window.screen!)
        let showBorderDuration: TimeInterval = duration / 4
        let resizeDuration: TimeInterval = duration - showBorderDuration
        
        // a window in full-screen mode does not have a title bar, thus we have to
        // explicitly specify the style mask to compute the correct content frame
        var styleMask = window.styleMask
        styleMask.remove(.fullScreen)
        let contentFrame = NSWindow.contentRect(forFrameRect: windowedFrame, styleMask: styleMask)
        
        let fullScreenContentSize = gameViewController?.gameView.frame.size ?? .zero
        let screenshotWindowedFrame = contentFrame
        let targetWindowOrigin = windowedFrame.origin
        
        var screenshotFrame = screenFrame
        if fullScreenIntegralScale != Self.fitToWindowScale {
            let origin = NSPoint(x: (screenFrame.size.width - fullScreenContentSize.width) / 2,
                                 y: (screenFrame.size.height - fullScreenContentSize.height) / 2)
            screenshotFrame = NSRect(origin: origin + screenFrame.origin, size: fullScreenContentSize)
            screenshotFrame = screenshotFrame.integral
        }
        
        screenshotWindow.setScreenshot(gameDocument.screenshot())
        screenshotWindow.setFrame(screenshotFrame, display: true)
        screenshotWindow.orderFront(self)
        window.alphaValue = 0
        // Scale the screenshot window down
        CATransaction.begin()
        defer { CATransaction.commit() }
        
        CATransaction.setAnimationDuration(resizeDuration)
        CATransaction.setAnimationTimingFunction(CAMediaTimingFunction(name: .easeInEaseOut))
        CATransaction.setCompletionBlock {
            // Restore the window to its original frame.
            // We do it using setContentSize(_:) instead of setFrame(_:display:)
            // because setFrame(_:display:) does not obey the new style mask correctly
            // for some reason
            window.styleMask.remove(.fullScreen)
            window.setContentSize(contentFrame.size)
            window.setFrameOrigin(targetWindowOrigin)
            
            // Fade the real window back in after the scaling is done
            CATransaction.begin()
            defer { CATransaction.commit() }
            
            CATransaction.setAnimationDuration(showBorderDuration)
            CATransaction.setAnimationTimingFunction(CAMediaTimingFunction(name: .easeInEaseOut))
            CATransaction.setCompletionBlock {
                self.hideScreenshotWindow()
            }
            
            window.animator().alphaValue = 1
        }
        
        screenshotWindow.animator().setFrame(screenshotWindowedFrame, display: true)
    }
    
    func windowDidExitFullScreen(_ notification: Notification) {
        guard let window = notification.object as? NSWindow else {
            if #available(macOS 11.0, *) {
                Logger.default.error("windowDidExitFullScreen: Expected NSWindow")
            }
            
            return
        }
        
        fullScreenStatus = .nonFullScreen
        
        if resumePlayingAfterFullScreenTransition {
            gameDocument.isEmulationPaused = false
        }
        
        let gameViewController = gameDocument?.gameViewController
        gameViewController?.controlsWindow.hide(animated: true)
        gameViewController?.controlsWindow.canShow = true
        gameViewController?.gameViewFillSuperView()
        updateContentSizeConstraints()
        
        window.backgroundColor = nil
    }
    
    func windowDidFailToEnterFullScreen(_ window: NSWindow) {
        fullScreenStatus = .nonFullScreen
        
        if resumePlayingAfterFullScreenTransition {
            gameDocument.isEmulationPaused = false
        }
        
        let gameViewController = gameDocument.gameViewController
        gameViewController?.controlsWindow.canShow = true
    }
    
    func windowDidFailToExitFullScreen(_ window: NSWindow) {
        fullScreenStatus = .fullScreen
        
        if resumePlayingAfterFullScreenTransition {
            gameDocument.isEmulationPaused = false
        }
        
        let gameViewController = gameDocument.gameViewController
        gameViewController?.controlsWindow.canShow = true
    }
}

private class ScreenshotWindow: NSWindow {
    
    init(screenFrame: NSRect) {
        let imageView = NSImageView(frame: NSRect(origin: .zero, size: screenFrame.size))
        imageView.imageAlignment = .alignCenter
        imageView.imageScaling = .scaleProportionallyUpOrDown
        imageView.autoresizingMask = [.width, .height]
        screenshotView = imageView
        
        super.init(contentRect: screenFrame, styleMask: .borderless, backing: .buffered, defer: false)
        backgroundColor = .black
        isOpaque = true
        animationBehavior = .none
        contentView = imageView
    }
    
    private var screenshotView: NSImageView
    
    func setScreenshot(_ screenshot: NSImage?) {
        var screenshot = screenshot
        
        // The game view uses the ITU 709 color space, while the
        // screenshots we get use a sRGB color space for exactly the same pixel
        // data. We must fix this mismatch to make the screenshot window look
        // exactly the same as the game view.
        if let cgimg = screenshot?.cgImage(forProposedRect: nil, context: nil, hints: nil),
           let cgcs = CGColorSpace(name: CGColorSpace.itur_709),
           let cs = NSColorSpace(cgColorSpace: cgcs),
           let rep = NSBitmapImageRep(cgImage: cgimg).retagging(with: cs)
        {
            let img = NSImage()
            img.addRepresentation(rep)
            screenshot = img
        }
        
        screenshotView.image = screenshot
    }
}

// MARK: - Adaptive Sync

extension NSScreen {
    var isAdaptiveSyncSupported: Bool {
        guard #available(macOS 12.0, *) else {
            return false
        }
        // Source: https://developer.apple.com/wwdc21/10147?time=357
        return minimumRefreshInterval != maximumRefreshInterval
    }
}

extension NSWindow {
    var isAdaptiveSyncSchedulingAvailable: Bool {
        guard UserDefaults.standard.bool(forKey: OEAdaptiveSyncEnabledKey),
              let screen
        else { return false }
        
        return isFullScreen && screen.isAdaptiveSyncSupported
    }
}
