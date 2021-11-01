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

final class GameControlsBarView: NSView {
    
    private var slider: NSSlider!
    private var fullScreenButton: NSButton!
    private var pauseButton: NSButton!
    
    required init?(coder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }
    
    override init(frame frameRect: NSRect) {
        super.init(frame: frameRect)
        
        wantsLayer = true
        canDrawConcurrently = true
        canDrawSubviewsIntoLayer = true
        
        setUpControls()
    }
    
    override func draw(_ dirtyRect: NSRect) {
        if OEAppearance.hudBar == .dark {
            NSImage(named: "hud_bar")?.draw(in: bounds)
        } else {
            super.draw(dirtyRect)
        }
    }
    
    private func setUpControls() {
        
        let stop = HUDBarButton()
        stop.image = NSImage(named: "hud_power")
        stop.backgroundColor = .red
        stop.target = self
        stop.action = #selector(stopEmulation(_:))
        stop.toolTip = NSLocalizedString("Quit Game", comment: "HUD bar, tooltip")
        addSubview(stop)
        
        
        let playPause = HUDBarButton()
        playPause.image = NSImage(named: "hud_pause")
        playPause.alternateImage = NSImage(named: "hud_play")
        playPause.setButtonType(.toggle)
        playPause.action = #selector(OEGameDocument.toggleEmulationPaused(_:))
        playPause.toolTip = NSLocalizedString("Pause Game", comment: "HUD bar, tooltip")
        addSubview(playPause)
        pauseButton = playPause
        
        
        let restart = HUDBarButton()
        restart.image = NSImage(named: "hud_restart")
        restart.action = #selector(OEGameDocument.resetEmulation(_:))
        restart.toolTip = NSLocalizedString("Restart Game", comment: "HUD bar, tooltip")
        addSubview(restart)
        
        
        let saves = HUDBarButton()
        saves.image = NSImage(named: "hud_save")
        saves.target = self
        saves.action = #selector(showSaveMenu(_:))
        saves.toolTip = NSLocalizedString("Create or Load Save State", comment: "HUD bar, tooltip")
        addSubview(saves)
        
        
        let options = HUDBarButton()
        options.image = NSImage(named: "hud_options")
        options.target = self
        options.action = #selector(showOptionsMenu(_:))
        options.toolTip = NSLocalizedString("Options", comment: "HUD bar, tooltip")
        addSubview(options)
        
        
        let volumeDown = HUDBarButton()
        volumeDown.image = NSImage(named: "hud_volume_down")
        volumeDown.action = #selector(OEGameDocument.mute(_:))
        volumeDown.toolTip = NSLocalizedString("Mute Audio", comment: "HUD bar, tooltip")
        addSubview(volumeDown)
        
        
        let volumeUp = HUDBarButton()
        volumeUp.image = NSImage(named: "hud_volume_up")
        volumeUp.action = #selector(OEGameDocument.unmute(_:))
        volumeUp.toolTip = NSLocalizedString("Unmute Audio", comment: "HUD bar, tooltip")
        addSubview(volumeUp)
        
        
        let volume = NSSlider()
        if #available(macOS 11.0, *) {
            volume.controlSize = .mini
        } else {
            volume.controlSize = .small
        }
        volume.isContinuous = true
        volume.minValue = 0
        volume.maxValue = 1
        volume.floatValue = UserDefaults.standard.float(forKey: OEGameVolumeKey)
        volume.action = #selector(OEGameDocument.changeVolume(_:))
        volume.toolTip = NSLocalizedString("Change Volume", comment: "HUD bar, tooltip")
        addSubview(volume)
        slider = volume
        
        let animation = CABasicAnimation()
        animation.timingFunction = CAMediaTimingFunction(name: .easeOut)
        volume.animations = ["floatValue" : animation]
        
        
        let fullScreen = HUDBarButton()
        fullScreen.image = NSImage(named: "hud_fullscreen_enter")
        fullScreen.alternateImage = NSImage(named: "hud_fullscreen_exit")
        fullScreen.backgroundColor = .black
        fullScreen.setButtonType(.pushOnPushOff)
        fullScreen.target = self
        fullScreen.action = #selector(toggleFullScreen(_:))
        fullScreen.toolTip = NSLocalizedString("Toggle Fullscreen", comment: "HUD bar, tooltip")
        addSubview(fullScreen)
        fullScreenButton = fullScreen
        
        
        // MARK: - Auto Layout
        
        for view in subviews {
            view.translatesAutoresizingMaskIntoConstraints = false
        }
        
        var constraints = [NSLayoutConstraint]()
        constraints.reserveCapacity(subviews.count * 4)
        
        
        // MARK: Size
        for button in [stop, fullScreen] {
            constraints.append(button.widthAnchor.constraint(equalToConstant: 51))
            constraints.append(button.heightAnchor.constraint(equalToConstant: 22))
        }
        for button in [playPause, restart, saves, options] {
            constraints.append(button.widthAnchor.constraint(equalToConstant: 32))
            constraints.append(button.heightAnchor.constraint(equalToConstant: 32))
        }
        constraints.append(volume.widthAnchor.constraint(equalToConstant: 70))
        
        
        // MARK: X axis
        constraints += [
            stop.leadingAnchor.constraint(equalTo:        leadingAnchor,  constant:  10),
            playPause.leadingAnchor.constraint(equalTo:   stop.trailingAnchor,                 constant:  14),
            restart.leadingAnchor.constraint(equalTo:     playPause.trailingAnchor,            constant:   0),
            saves.leadingAnchor.constraint(equalTo:       restart.trailingAnchor,              constant:   15),
            options.leadingAnchor.constraint(equalTo:     saves.trailingAnchor,                constant:   8),
            volume.leadingAnchor.constraint(equalTo:      volumeDown.trailingAnchor,           constant:   3),
            volumeUp.leadingAnchor.constraint(equalTo:    volume.trailingAnchor,               constant:   3),
            fullScreen.leadingAnchor.constraint(equalTo:  volumeUp.trailingAnchor,             constant:  22),
            fullScreen.trailingAnchor.constraint(equalTo: trailingAnchor, constant: -10)
        ]
        
        
        // MARK: Y axis
        let constant: CGFloat = OEAppearance.hudBar == .vibrant ? 0 : -2
        for view in subviews {
            constraints.append(view.centerYAnchor.constraint(equalTo: centerYAnchor, constant: constant))
        }
        
        NSLayoutConstraint.activate(constraints)
    }
    
    // MARK: - Actions
    
    @objc private func stopEmulation(_ sender: Any?) {
        window?.parent?.performClose(self)
    }
    
    @objc private func toggleFullScreen(_ sender: Any?) {
        window?.parent?.toggleFullScreen(sender)
    }
    
    @objc private func showSaveMenu(_ sender: NSButton) {
        if let menu = (window as? GameControlsBar)?.saveMenu {
            let targetRect = sender.bounds.insetBy(dx: -2, dy: 1)
            let menuPosition = NSPoint(x: targetRect.minX, y: targetRect.maxY)
            menu.popUp(positioning: nil, at: menuPosition, in: sender)
        }
    }
    
    @objc private func showOptionsMenu(_ sender: NSButton) {
        if let menu = (window as? GameControlsBar)?.optionsMenu {
            let targetRect = sender.bounds.insetBy(dx: -2, dy: 1)
            let menuPosition = NSPoint(x: targetRect.minX, y: targetRect.maxY)
            menu.popUp(positioning: nil, at: menuPosition, in: sender)
        }
    }
    
    // MARK: - UI State
    
    func reflectEmulationPaused(_ isPaused: Bool) {
        if isPaused {
            pauseButton.state = .on
            pauseButton.toolTip = NSLocalizedString("Resume Game", comment: "HUD bar, tooltip")
        } else {
            pauseButton.state = .off
            pauseButton.toolTip = NSLocalizedString("Pause Game", comment: "HUD bar, tooltip")
        }
    }
    
    func reflectFullScreen(_ isFullScreen: Bool) {
        if isFullScreen {
            fullScreenButton.state = .on
        } else {
            fullScreenButton.state = .off
        }
    }
    
    func reflectVolume(_ volume: Float) {
        slider.animator().floatValue = volume
    }
}
