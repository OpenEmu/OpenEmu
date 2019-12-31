// Copyright (c) 2019, OpenEmu Team
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

fileprivate extension NSTouchBar.CustomizationIdentifier {
    static let saveStatesTouchBar = "org.openemu.SaveStateViewController.saveStatesTouchBar"
}

fileprivate extension NSTouchBarItem.Identifier {
    static let deleteSaveState = NSTouchBarItem.Identifier("org.openemu.SaveStateViewController.saveStatesTouchbar.delete")
    static let resumeSaveState = NSTouchBarItem.Identifier("org.openemu.SaveStateViewController.saveStatesTouchbar.resume")
}

extension SaveStateViewController {
    
    open override func makeTouchBar() -> NSTouchBar? {
        
        let touchBar = NSTouchBar()
        
        touchBar.delegate = self
        
        touchBar.customizationIdentifier = .saveStatesTouchBar
        touchBar.defaultItemIdentifiers = [.deleteSaveState,
                                           .flexibleSpace,
                                           .resumeSaveState,
                                           .otherItemsProxy]
        touchBar.customizationAllowedItemIdentifiers = [.deleteSaveState,
                                                        .resumeSaveState]
        touchBar.principalItemIdentifier = .resumeSaveState
        
        return touchBar
    }
    
    func updateButtonStatesForMediaViewControllerSelection() {
        guard let touchBar = self.touchBar else { return }
        let selected = self.collectionView.selectionIndexPaths
        
        if let deleteItem = touchBar.item(forIdentifier: .deleteSaveState) as? NSCustomTouchBarItem {
            let button = deleteItem.view as! NSButton
            button.isEnabled = !selected.isEmpty
        }
        
        if let playItem = touchBar.item(forIdentifier: .resumeSaveState) as? NSCustomTouchBarItem {
            let button = playItem.view as! NSButton
            button.isEnabled = selected.count == 1
        }
    }
}

extension SaveStateViewController: NSTouchBarDelegate {
    
    public func touchBar(_ touchBar: NSTouchBar, makeItemForIdentifier identifier: NSTouchBarItem.Identifier) -> NSTouchBarItem? {
        
        switch identifier {
            
        case .deleteSaveState:
            
            let item = NSCustomTouchBarItem(identifier: identifier)
            item.customizationLabel = NSLocalizedString("Delete", comment: "")
            
            let button = NSButton(image: NSImage(named: NSImage.touchBarDeleteTemplateName)!, target: nil, action: #selector(deleteSelectedItems(_:)))
            
            button.isEnabled = !collectionView.selectionIndexPaths.isEmpty
            button.bezelColor = #colorLiteral(red: 0.5665243268, green: 0.2167189717, blue: 0.2198875844, alpha: 1)
            
            item.view = button
            
            return item
            
        case .resumeSaveState:
            
            let item = NSCustomTouchBarItem(identifier: identifier)
            item.customizationLabel = NSLocalizedString("Resume Game", comment: "")
            
            let button = NSButton(image: NSImage(named: NSImage.touchBarPlayTemplateName)!, target: nil, action: #selector(OELibraryController.startSaveState(_:)))
            
            button.isEnabled = collectionView.selectionIndexPaths.count == 1
            
            item.view = button
            
            return item
            
        default:
            return nil
        }
    }
}
