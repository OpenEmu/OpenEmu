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

fileprivate enum GameplaySegments: Int {
    case pauseGameplay = 0
    case restartSystem = 1
}

fileprivate enum SaveStatesSegments: Int {
    case quickSave = 0
    case quickLoad = 1
}

fileprivate enum VolumeSegments: Int {
    case down = 0
    case up   = 1
}

fileprivate extension NSTouchBar.CustomizationIdentifier {
    static let touchBar = "org.openemu.GameViewControllerTouchBar"
}

fileprivate extension NSTouchBarItem.Identifier {
    static let stop = NSTouchBarItem.Identifier("org.openemu.GameViewControllerTouchBar.stop")
    static let gameplay = NSTouchBarItem.Identifier("org.openemu.GameViewControllerTouchBar.gameplay")
    static let saveStates = NSTouchBarItem.Identifier("saveStateControls")
    static let volume = NSTouchBarItem.Identifier("org.openemu.GameViewControllerTouchBar.volume")
    static let toggleFullScreen = NSTouchBarItem.Identifier("org.openemu.GameViewControllerTouchBar.toggleFullScreen")
}

extension OEGameViewController {
    
    open override func makeTouchBar() -> NSTouchBar? {
        
        let touchBar = NSTouchBar()
        
        touchBar.delegate = self
        touchBar.customizationIdentifier = .touchBar
        touchBar.defaultItemIdentifiers = [.stop,
                                           .gameplay,
                                           .saveStates,
                                           .volume,
                                           .toggleFullScreen,
                                           .otherItemsProxy]
        touchBar.customizationAllowedItemIdentifiers = [.stop,
                                                        .gameplay,
                                                        .saveStates,
                                                        .volume,
                                                        .toggleFullScreen]
        
        return touchBar
    }
}

extension OEGameViewController: NSTouchBarDelegate {
    
    public func touchBar(_ touchBar: NSTouchBar, makeItemForIdentifier identifier: NSTouchBarItem.Identifier) -> NSTouchBarItem? {
        
        switch identifier {
            
        case .stop:
            
            let item = NSCustomTouchBarItem(identifier: identifier)
            item.customizationLabel = NSLocalizedString("Quit Game", comment: "")
            
            let button = NSButton(image: NSImage(named: NSImage.touchBarRecordStopTemplateName)!, target: nil, action: #selector(OEGameDocument.stopEmulation(_:)))
            
            button.bezelColor = #colorLiteral(red: 0.5665243268, green: 0.2167189717, blue: 0.2198875844, alpha: 1)
            
            item.view = button
            
            return item
            
        case .gameplay:
            
            let item = NSCustomTouchBarItem(identifier: identifier)
            item.customizationLabel = NSLocalizedString("Pause/Restart", comment: "")
            
            let segmentImages = [NSImage(named: NSImage.touchBarPauseTemplateName)!,
                                 NSImage(named: NSImage.touchBarRefreshTemplateName)!]
            
            let segmentedControl = NSSegmentedControl(images: segmentImages, trackingMode: .momentary, target: nil, action: #selector(OEGameViewController.gameplayControlsTouched(_:)))
            
            segmentedControl.segmentStyle = .separated
            
            item.view = segmentedControl
            
            return item
            
        case .saveStates:

            guard supportsSaveStates else {
                return nil
            }
            
            let item = NSCustomTouchBarItem(identifier: identifier)
            item.customizationLabel = NSLocalizedString("Save States", comment: "")
            
            let segmentImages = [#imageLiteral(resourceName: "quick_save_touch_bar_template"),
                                 #imageLiteral(resourceName: "quick_load_touch_bar_template")]
            
            let segmentedControl = NSSegmentedControl(images: segmentImages, trackingMode: .momentary, target: nil, action: #selector(OEGameViewController.saveStatesControlsTouched(_:)))
            
            segmentedControl.segmentStyle = .separated
            
            item.view = segmentedControl
            
            return item
            
        case .volume:
            
            let item = NSCustomTouchBarItem(identifier: identifier)
            item.customizationLabel = NSLocalizedString("Volume", comment: "")
            
            let segmentImages = [NSImage(named: NSImage.touchBarVolumeDownTemplateName)!,
                                 NSImage(named: NSImage.touchBarVolumeUpTemplateName)!]
            
            let segmentedControl = NSSegmentedControl(images: segmentImages, trackingMode: .momentary, target: nil, action: #selector(OEGameViewController.volumeTouched(_:)))
            
            segmentedControl.segmentStyle = .separated
            
            let volume = document.volume
            
            segmentedControl.setEnabled(volume > 0, forSegment: VolumeSegments.down.rawValue)
            segmentedControl.setEnabled(volume < 1, forSegment: VolumeSegments.up.rawValue)
            
            item.view = segmentedControl
                        
            return item
            
        case .toggleFullScreen:
            
            let item = NSCustomTouchBarItem(identifier: identifier)
            item.customizationLabel = NSLocalizedString("Toggle Fullscreen", comment: "")
            
            let imageName: NSImage.Name = document.gameWindowController.window!.isFullScreen ? NSImage.touchBarExitFullScreenTemplateName : NSImage.touchBarEnterFullScreenTemplateName
            let image = NSImage(named: imageName)!
            let button = NSButton(image: image, target: nil, action: #selector(OEGameViewController.fullScreenTouched(_:)))
            
            item.view = button
            
            return item
            
        default:
            return nil
        }
    }
    
    @objc private func gameplayControlsTouched(_ sender: Any?) {
        
        guard let segmentedControl = sender as? NSSegmentedControl else {
            return
        }
        
        // Force-unwrap so that unhandled segments are noticed immediately.
        switch GameplaySegments(rawValue: segmentedControl.selectedSegment)! {
            
        case .pauseGameplay:
            
            document.toggleEmulationPaused(self)
            
            let imageName: NSImage.Name = document.isEmulationPaused ? NSImage.touchBarPlayTemplateName : NSImage.touchBarPauseTemplateName
            segmentedControl.setImage(NSImage(named: imageName)!, forSegment: 0)
            
        case .restartSystem:
            document.resetEmulation(self)
        }
    }
    
    @objc private func saveStatesControlsTouched(_ sender: Any?) {
        
        guard let segmentedControl = sender as? NSSegmentedControl else {
            return
        }
        
        // Force-unwrap so that unhandled segments are noticed immediately.
        switch SaveStatesSegments(rawValue: segmentedControl.selectedSegment)! {
            
        case .quickSave:
            document.quickSave(self)
            
        case .quickLoad:
            document.quickLoad(self)
        }
    }
    
    @objc private func volumeTouched(_ sender: Any?) {
        
        guard let segmentedControl = sender as? NSSegmentedControl else {
            return
        }
        
        // Force-unwrap so that unhandled segments are noticed immediately.
        switch VolumeSegments(rawValue: segmentedControl.selectedSegment)! {
            
        case .down:
            document.volumeDown(self)
            
        case .up:
            document.volumeUp(self)
        }
        
        let volume = document.volume
        
        segmentedControl.setEnabled(volume > 0, forSegment: VolumeSegments.down.rawValue)
        segmentedControl.setEnabled(volume < 1, forSegment: VolumeSegments.up.rawValue)
    }
    
    @objc private func fullScreenTouched(_ sender: Any?) {
        
        document.toggleFullScreen(self)
        
        let item = touchBar!.item(forIdentifier: .toggleFullScreen)!
        let button = item.view! as! NSButton
        
        let imageName: NSImage.Name = document.gameWindowController.window!.isFullScreen ? NSImage.touchBarExitFullScreenTemplateName : NSImage.touchBarEnterFullScreenTemplateName
        button.image = NSImage(named: imageName)!
    }
}
