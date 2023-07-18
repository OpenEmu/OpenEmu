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
import OpenEmuBase.OEGeometry
import OpenEmuSystem
import OpenEmuKit

// TODO: Messages to remote layer:
// - Change bounds
// - Start Syphon
// - Native screenshot
//
// Messages from remote layer:
// - Default screen size/aspect size - DONE?

@objc(OEGameViewController)
@objcMembers
final class GameViewController: NSViewController {
    
    /// arbitrary default screen size with 4:3 ratio
    private let defaultSize = CGSize(width: 400, height: 300)
    private(set) var defaultScreenSize = CGSize.zero
    private var aspectSize = OEIntSize()
    private var screenSize = OEIntSize()
    
    private var scaledView: OEScaledGameLayerView!
    private(set) var gameView: OEGameLayerView!
    private var notificationView: OEGameLayerNotificationView!
    
    var controlsWindow: GameControlsBar!
    weak var document: OEGameDocument!
    weak var integralScalingDelegate: GameIntegralScalingDelegate?
    
    var shaderControl: ShaderControl!
    private var shaderWindowController: ShaderParametersWindowController!
    
    private var token: NSObjectProtocol?
    
    required init?(coder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }
    
    init(document: OEGameDocument) {
        super.init(nibName: nil, bundle: nil)
        
        self.document = document
        
        defaultScreenSize = defaultSize
        
        controlsWindow = GameControlsBar(gameViewController: self)
        controlsWindow.isReleasedWhenClosed = false
        shaderControl = ShaderControl(document: document)
        shaderWindowController = ShaderParametersWindowController(control: shaderControl)
        
        scaledView = OEScaledGameLayerView(frame: NSRect(origin: .zero, size: NSSize(width: 1, height: 1)))
        view = scaledView
        
        gameView = OEGameLayerView(frame: view.bounds)
        gameView.delegate = self
        scaledView.contentView = gameView
        scaledView.setContentViewSizeFill(animated: false)
        
        notificationView = OEGameLayerNotificationView(frame: NSRect(x: 0, y: 0, width: 28, height: 28))
        notificationView.translatesAutoresizingMaskIntoConstraints = false
        notificationView.cell?.setAccessibilityElement(false)
        view.addSubview(notificationView)
        
        NSLayoutConstraint.activate([
            notificationView.widthAnchor.constraint(equalToConstant: 28),
            notificationView.heightAnchor.constraint(equalToConstant: 28),
            notificationView.leadingAnchor.constraint(equalTo: view.leadingAnchor, constant: 60),
            notificationView.topAnchor.constraint(equalTo: view.topAnchor, constant: 10)
        ])
        
        token = NotificationCenter.default.addObserver(forName: NSView.frameDidChangeNotification, object: gameView, queue: .main) { [weak self] _ in
            guard let self = self else { return }
            
            self.controlsWindow.repositionOnGameWindow()
        }
    }
    
    deinit {
        if let token = token {
            NotificationCenter.default.removeObserver(token)
            self.token = nil
        }

        shaderWindowController.close()
        shaderWindowController = nil

        controlsWindow.gameWindow = nil
        controlsWindow.close()
        controlsWindow = nil
    }
    
    override func viewDidAppear() {
        super.viewDidAppear()
        
        guard let window = rootWindow else { return }
        
        controlsWindow.gameWindow = window
        controlsWindow.repositionOnGameWindow()
        
        window.makeFirstResponder(gameView)
    }
    
    override func viewWillDisappear() {
        super.viewWillDisappear()
        
        controlsWindow.hide(animated: false, hideCursor: false)
        controlsWindow.gameWindow = nil
        rootWindow?.removeChildWindow(controlsWindow)
    }
    
    private var rootWindow: NSWindow? {
        var window = gameView.window
        while window?.parent != nil {
            window = window?.parent
        }
        return window
    }
    
    // MARK: - Game View Control
    
    func gameViewSetIntegralSize(_ size: NSSize, animated: Bool) {
        scaledView.setContentViewSize(size, animated: animated)
    }
    
    func gameViewFillSuperView() {
        scaledView.setContentViewSizeFill(animated: false)
    }
    
    override func viewDidLayout() {
        document.updateBounds(gameView.bounds)
    }
    
    // MARK: - Controlling Emulation
    
    var supportsCheats: Bool {
        document.supportsCheats
    }
    
    var supportsSaveStates: Bool {
        document.supportsSaveStates
    }
    
    var supportsMultipleDiscs: Bool {
        document.supportsMultipleDiscs
    }
    
    var supportsFileInsertion: Bool {
        document.supportsFileInsertion
    }
    
    var supportsDisplayModeChange: Bool {
        document.supportsDisplayModeChange
    }
    
    var coreIdentifier: String {
        document.coreIdentifier
    }
    
    var systemIdentifier: String {
        document.systemIdentifier
    }
    
    @IBAction func takeScreenshot(_ sender: Any?) {
        document.takeScreenshot(sender)
    }
    
    func reflectVolume(_ volume: Float) {
        controlsWindow.reflectVolume(volume)
    }
    
    func reflectEmulationPaused(_ isPaused: Bool) {
        controlsWindow.reflectEmulationPaused(isPaused)
    }
    
    func toggleControlsVisibility(_ sender: NSMenuItem) {
        sender.state = sender.state == .off ? .on : .off
        controlsWindow.canShow = sender.state == .off
        if sender.state == .on {
            controlsWindow.hide()
        }
    }
    
    // MARK: - HUD Bar Actions
    
    func selectShader(_ sender: NSMenuItem) {
        let shaderName = sender.title
        if let shader = OEShaderStore.shared.shader(withName: shaderName) {
            shaderControl.changeShader(shader)
        }
    }
    
    func configureShader(_ sender: Any?) {
        shaderWindowController.showWindow(sender)
    }
    
    // MARK: - OEGameCoreOwner Methods
    
    func setRemoteContextID(_ contextID: OEContextID) {
        gameView.remoteContextID = contextID
    }
    
    func setScreenSize(_ newScreenSize: OEIntSize, aspectSize newAspectSize: OEIntSize) {
        screenSize = newScreenSize
        aspectSize = newAspectSize
        // Should never happen
        if newScreenSize.isEmpty && newAspectSize.isEmpty {
            defaultScreenSize = defaultSize
        }
        else {
            // Some cores may initially report a 0x0 screenRect on launch, so use aspectSize instead.
            if newScreenSize.isEmpty {
                screenSize = aspectSize
            }
            let correct = screenSize.corrected(forAspectSize: aspectSize)
            defaultScreenSize = CGSize(width: Int(correct.width), height: Int(correct.height))
        }
        
        gameView.setScreenSize(screenSize, aspectSize: aspectSize)
    }
}

// MARK: - NSMenuItemValidation

extension GameViewController: NSMenuItemValidation {
    
    func validateMenuItem(_ menuItem: NSMenuItem) -> Bool {
        switch menuItem.action {
        case #selector(toggleControlsVisibility(_:)):
            if controlsWindow.canShow {
                menuItem.state = .off
            } else {
                menuItem.state = .on
            }
            return true
        default:
            return true
        }
    }
}

// MARK: - OEGameViewDelegate

extension GameViewController: OEGameViewDelegate {
    
    func gameView(_ gameView: OEGameLayerView, didReceiveMouseEvent event: OEEvent) {
        document.didReceiveMouseEvent(event)
    }
    
    func gameView(_ gameView: OEGameLayerView, updateBounds newBounds: CGRect) {
        document.updateBounds(newBounds)
    }
    
    func gameView(_ gameView: OEGameLayerView, updateBackingScaleFactor newScaleFactor: CGFloat) {
        document.updateBackingScaleFactor(newScaleFactor)
    }
}

// MARK: - Notifications

extension GameViewController {
    
    func showQuickSaveNotification() {
        notificationView.showQuickSave()
    }
    
    func showScreenShotNotification() {
        notificationView.showScreenShot()
    }
    
    func showFastForwardNotification(_ enable: Bool) {
        notificationView.showFastForward(enabled: enable)
    }
    
    func showRewindNotification(_ enable: Bool) {
        notificationView.showRewind(enabled: enable)
    }
    
    func showStepForwardNotification() {
        notificationView.showStepForward()
    }
    
    func showStepBackwardNotification() {
        notificationView.showStepBackward()
    }
}
