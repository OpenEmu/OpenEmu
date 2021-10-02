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
import OpenEmuKit


// MARK: -  NSMenuItemValidation

extension OEGameDocument {
    
    open
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
        case #selector(nextDisplayMode(_:)):
            return supportsDisplayModeChange
        case #selector(lastDisplayMode(_:)):
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
    
    public
    func systemBindings(_ sender: OESystemBindings, didSetEvent event: OEHIDEvent, forBinding bindingDescription: OEBindingDescription, playerNumber: UInt) {
        gameCoreManager.systemBindingsDidSetEvent(event, forBinding: bindingDescription, playerNumber: playerNumber)
    }
    
    public
    func systemBindings(_ sender: OESystemBindings, didUnsetEvent event: OEHIDEvent, forBinding bindingDescription: OEBindingDescription, playerNumber: UInt) {
        gameCoreManager.systemBindingsDidUnsetEvent(event, forBinding: bindingDescription, playerNumber: playerNumber)
    }
}

// MARK: - OEGameCoreOwner

extension OEGameDocument: OEGameCoreOwner {
    
    public
    func saveState() {
        NSApp.sendAction(#selector(saveState(_:)), to: nil, from: nil)
        //saveState(nil)
    }
    
    public
    func loadState() {
        // FIXME: This replaces a call from OESystemResponder which used to pass self, but passing OESystemResponder would yield the same result in -loadState: so I do not know whether this ever worked in this case. (6eeda41)
        loadState(nil)
    }
    
    public
    func quickSave() {
        quickSave(nil)
    }
    
    public
    func quickLoad() {
        quickLoad(nil)
    }
    
    public
    func toggleFullScreen() {
        toggleFullScreen(self)
    }
    
    public
    func toggleAudioMute() {
        if isMuted {
            unmute(self)
        } else {
            mute(self)
        }
    }
    
    public
    func volumeDown() {
        volumeDown(self)
    }
    
    public
    func volumeUp() {
        volumeUp(self)
    }
    
    public
    func stopEmulation() {
        stopEmulation(self)
    }
    
    public
    func resetEmulation() {
        resetEmulation(self)
    }
    
    public
    func toggleEmulationPaused() {
        toggleEmulationPaused(self)
    }
    
    public
    func takeScreenshot() {
        takeScreenshot(self)
    }
    
    public
    func fastForwardGameplay(_ enable: Bool) {
        if emulationStatus != .playing { return }
        gameViewController.showFastForwardNotification(enable)
    }
    
    public
    func rewindGameplay(_ enable: Bool) {
        if emulationStatus != .playing { return }
        gameViewController.showRewindNotification(enable)
    }
    
    public
    func stepGameplayFrameForward() {
        if emulationStatus == .playing {
            toggleEmulationPaused(self)
        }
        if emulationStatus == .paused {
            gameViewController.showStepForwardNotification()
        }
    }
    
    public
    func stepGameplayFrameBackward() {
        if emulationStatus == .playing {
            toggleEmulationPaused(self)
        }
        if emulationStatus == .paused {
            gameViewController.showStepBackwardNotification()
        }
    }
    
    public
    func nextDisplayMode() {
        nextDisplayMode(self)
    }
    
    public
    func lastDisplayMode() {
        lastDisplayMode(self)
    }
    
    public
    func setScreenSize(_ newScreenSize: OEIntSize, aspectSize newAspectSize: OEIntSize) {
        gameViewController.setScreenSize(newScreenSize, aspectSize: newAspectSize)
    }
    
    public func setDiscCount(_ discCount: UInt) {
        gameViewController.discCount = discCount
    }
    
    public
    func setDisplayModes(_ displayModes: [[String : Any]]) {
        gameViewController.displayModes = displayModes
    }
    
    public
    func setRemoteContextID(_ contextID: OEContextID) {
        gameViewController.setRemoteContextID(contextID)
    }
    
    public
    func gameCoreDidTerminate() {
        if !(emulationStatus == .starting || emulationStatus == .paused) {
            return
        }
        coreDidTerminateSuddenly = true
        stopEmulation(self)
    }
}
