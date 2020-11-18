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

@objc(OEHUDControlsBarView)
class HUDControlsBarView: NSView, CAAnimationDelegate, OEHUDBarView {
    
    @objc private(set) var slider: NSSlider?
    @objc private(set) var fullScreenButton: NSButton?
    @objc private(set) var pauseButton: NSButton?
    
    required init?(coder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }
    
    override init(frame frameRect: NSRect) {
        super.init(frame: frameRect)
        
        wantsLayer = true
        canDrawConcurrently = true
        canDrawSubviewsIntoLayer = true
    }
    
    @objc func stopEmulation(_ sender: Any?) {
        window?.parent?.performClose(self)
    }
    
    override func draw(_ dirtyRect: NSRect) {
        NSImage(named: "HUD/hud_bar")?.draw(in: bounds)
    }
    
    @objc func setupControls() {
        
        let stop = HUDButton(frame: NSRect(x: 10, y: 13, width: 51, height: 23))
        stop.imageName = "HUD/hud_power"
        stop.backgroundColor = .red
        stop.target = self
        stop.action = #selector(stopEmulation(_:))
        stop.toolTip = NSLocalizedString("Quit Game", comment: "HUD bar, tooltip")
        addSubview(stop)
        
        
        let playPause = HUDButton(frame: NSRect(x: 82, y: 9, width: 32, height: 32))
        playPause.imageName = "HUD/hud_pause"
        playPause.alternateImageName = "HUD/hud_play"
        playPause.setButtonType(.toggle)
        playPause.action = #selector(OEGameDocument.toggleEmulationPaused(_:))
        playPause.toolTip = NSLocalizedString("Pause Game", comment: "HUD bar, tooltip")
        addSubview(playPause)
        pauseButton = playPause
        
        
        let restart = HUDButton(frame: NSRect(x: 111, y: 9, width: 32, height: 32))
        restart.imageName = "HUD/hud_restart"
        restart.action = #selector(OEGameDocument.resetEmulation(_:))
        restart.toolTip = NSLocalizedString("Restart Game", comment: "HUD bar, tooltip")
        addSubview(restart)
        
        
        let saves = HUDButton(frame: NSRect(x: 162, y: 7, width: 32, height: 32))
        saves.imageName = "HUD/hud_save"
        saves.target = window
        saves.action = #selector(OEGameControlsBar.showSaveMenu(_:))
        saves.toolTip = NSLocalizedString("Create or Load Save State", comment: "HUD bar, tooltip")
        addSubview(saves)
        
        
        let hideOptions = UserDefaults.standard.bool(forKey: OEGameControlsBarHidesOptionButtonKey)
        let optionsOffset = hideOptions ? 0 : 50
        if !hideOptions {
            let options = HUDButton(frame: NSRect(x: 212, y: 7, width: 32, height: 32))
            options.imageName = "HUD/hud_options"
            options.target = window
            options.action = #selector(OEGameControlsBar.showOptionsMenu(_:))
            options.toolTip = NSLocalizedString("Options", comment: "HUD bar, tooltip")
            addSubview(options)
        }
        
        
        let volumeDown = HUDButton(frame: NSRect(x: 224 + optionsOffset, y: 15, width: 12, height: 18))
        volumeDown.imageName = "HUD/hud_volume_down"
        volumeDown.action = #selector(OEGameDocument.mute(_:))
        volumeDown.toolTip = NSLocalizedString("Mute Audio", comment: "HUD bar, tooltip")
        addSubview(volumeDown)
        
        
        let volumeUp = HUDButton(frame: NSRect(x: 320 + optionsOffset, y: 15, width: 17, height: 18))
        volumeUp.imageName = "HUD/hud_volume_up"
        volumeUp.action = #selector(OEGameDocument.unmute(_:))
        volumeUp.toolTip = NSLocalizedString("Unmute Audio", comment: "HUD bar, tooltip")
        addSubview(volumeUp)
        
        
        let volume = HUDSlider(frame: NSRect(x: 238 + optionsOffset, y: 12, width: 78, height: 23))
        volume.cell = HUDSliderCell()
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
        animation.delegate = self
        volume.animations = ["floatValue" : animation]
        
        
        let fullScreen = HUDButton(frame: NSRect(x: 370 + optionsOffset, y: 13, width: 51, height: 23))
        fullScreen.imageName = "HUD/hud_fullscreen_enter"
        fullScreen.alternateImageName = "HUD/hud_fullscreen_exit"
        fullScreen.backgroundColor = .black
        fullScreen.setButtonType(.pushOnPushOff)
        fullScreen.action = #selector(NSWindow.toggleFullScreen(_:))
        fullScreen.toolTip = NSLocalizedString("Toggle Fullscreen", comment: "HUD bar, tooltip")
        addSubview(fullScreen)
        fullScreenButton = fullScreen
    }
}
