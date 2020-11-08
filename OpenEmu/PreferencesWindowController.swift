/*
 Copyright (c) 2015, OpenEmu Team
 
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

@objc(OEPreferencesWindowController)
@objcMembers
class PreferencesWindowController: NSWindowController {
    
    static let debugModeKey = "debug"
    static let selectedPreferencesTabKey = "selectedPreferencesTab"
    
    static let openPaneNotificationName = Notification.Name("OEPrefOpenPane")
    static let userInfoPanelNameKey = "panelName"
    static let userInfoSystemIdentifierKey = "systemIdentifier"
    
    static let toolbarItemIdentiftierDebug = "OEToolBarItemIdentifierDebug"
    static let toolbarItemIdentifierSeparator = "OEToolbarItemIdentifierSeparator"
    
    let konamiCode = [NSEvent.SpecialKey.upArrow.rawValue,
                      NSEvent.SpecialKey.upArrow.rawValue,
                      NSEvent.SpecialKey.downArrow.rawValue,
                      NSEvent.SpecialKey.downArrow.rawValue,
                      NSEvent.SpecialKey.leftArrow.rawValue,
                      NSEvent.SpecialKey.rightArrow.rawValue,
                      NSEvent.SpecialKey.leftArrow.rawValue,
                      NSEvent.SpecialKey.rightArrow.rawValue,
                      98, // 'b'
                      97] // 'a'
    var konamiCodeIndex = 0
    var konamiCodeMonitor: AnyObject?
    
    var preferencesTabViewController: PreferencesTabViewController!
    
    override func windowDidLoad() {
        
        super.windowDidLoad()
        
        preferencesTabViewController = PreferencesTabViewController()
        self.window?.contentView = preferencesTabViewController.view
        
        if #available(macOS 11.0, *) {
            window?.titlebarSeparatorStyle = .line
        }
        
        window!.center()
    }
    
    // MARK: -
    
    @objc(showWindowWithNotification:)
    func showWindow(with notification: Notification) {
        
        showWindow(nil)
        
        guard let identifier = (notification as NSNotification).userInfo?[PreferencesWindowController.userInfoPanelNameKey] as? String else {
            return
        }
        
        selectPaneWithTabViewIdentifier(identifier)
        
        // If selected, the controls preference pane performs additional work using the notification.
        if let controlsPane = preferencesTabViewController.tabView.selectedTabViewItem?.viewController as? OEPrefControlsController {
            controlsPane.preparePane(with: notification)
        }
    }
    
    func selectPaneWithTabViewIdentifier(_ identifier: String) {
        
        let index = preferencesTabViewController.tabView.indexOfTabViewItem(withIdentifier: identifier)
        
        guard index != NSNotFound else {
            return
        }
        
        preferencesTabViewController.tabView.selectTabViewItem(at: index)
    }
}

extension PreferencesWindowController: NSWindowDelegate {
    
    func windowDidBecomeKey(_ notification: Notification) {
        
        konamiCodeIndex = 0
        
        konamiCodeMonitor = NSEvent.addLocalMonitorForEvents(matching: .keyDown) { event in
            if let char = event.characters?.unicodeScalars.first, char.value == self.konamiCode[self.konamiCodeIndex] {
                self.konamiCodeIndex += 1
                
                if self.konamiCodeIndex == self.konamiCode.count {
                    
                    let defaults = UserDefaults.standard
                    let debugModeActivated = !defaults.bool(forKey: PreferencesWindowController.debugModeKey)
                    defaults.set(debugModeActivated, forKey: PreferencesWindowController.debugModeKey)
                    
                    NSSound(named: "secret")!.play()
                    
                    self.preferencesTabViewController.toggleDebugPaneVisibility()
                    
                    self.konamiCodeIndex = 0
                }
                
                return nil
            }
            
            self.konamiCodeIndex = 0
            
            return event
        } as AnyObject?
    }
    
    func windowDidResignKey(_ notification: Notification) {
        
        if let konamiCodeMonitor = konamiCodeMonitor {
            NSEvent.removeMonitor(konamiCodeMonitor)
        }
        
        konamiCodeIndex = 0
        konamiCodeMonitor = nil
    }
}

class PreferencesTabViewController: NSTabViewController {
    
    typealias PreferencesViewController = (NSViewController & OEPreferencePane)
    
    /// Used to track when viewDidLoad has completed so that the default pane index selection of 0 doesn't get recorded in tabView(_:, didSelectTabViewItem:).
    fileprivate var viewDidLoadFinished = false
    
    override func viewDidLoad() {
        
        super.viewDidLoad()
        
        tabStyle = .toolbar
        
        // The collection of preference panes to add.
        var preferencePanes: [PreferencesViewController] = [
            OEPrefLibraryController(),
            OEPrefGameplayController(),
            OEPrefControlsController(),
            PrefCoresController(),
            OEPrefBiosController()
        ]
        
        // Check if the debug pane should be included.
        let defaults = UserDefaults.standard
        if defaults.bool(forKey: PreferencesWindowController.debugModeKey) {
            preferencePanes.append(OEPrefDebugController())
        }
        
        // Add tab view items for each preference pane.
        for pane in preferencePanes {
            addTabViewItem(with: pane)
        }
        
        // Check defaults for a pane to select.
        var indexOfPaneToSelect = defaults.integer(forKey: PreferencesWindowController.selectedPreferencesTabKey)
        if !(0..<children.count ~= indexOfPaneToSelect) {
            indexOfPaneToSelect = 0
        }
        
        // Select a pane.
        tabView.selectTabViewItem(at: indexOfPaneToSelect)
        
        viewDidLoadFinished = true
    }
    
    override func viewDidAppear() {
        
        super.viewDidAppear()
        
        updateWindowTitle()
        updateWindowFrame(animated: false)
    }
    
    override func transition(from fromViewController: NSViewController, to toViewController: NSViewController, options: NSViewController.TransitionOptions, completionHandler completion: (() -> Void)?) {
        
        NSAnimationContext.runAnimationGroup { _ in
            
            self.updateWindowTitle()
            self.updateWindowFrame(animated: true)
            
            super.transition(from: fromViewController, to: toViewController, options: [], completionHandler: completion)
            
        }
    }
    
    override func tabView(_ tabView: NSTabView, didSelect tabViewItem: NSTabViewItem?) {
        
        super.tabView(tabView, didSelect: tabViewItem)
        
        guard viewDidLoadFinished else {
            return
        }
        
        // Record the selected pane index in user defaults.
        UserDefaults.standard.set(selectedTabViewItemIndex, forKey: PreferencesWindowController.selectedPreferencesTabKey)
    }
    
    // MARK: -
    
    func addTabViewItem(with pane: PreferencesViewController) {
        
        let item = NSTabViewItem(viewController: pane)
        
        item.image = pane.icon
        item.identifier = pane.panelTitle
        item.label = pane.localizedPanelTitle
        
        addTabViewItem(item)
    }
    
    func updateWindowTitle() {
        view.window?.title = tabView.selectedTabViewItem?.label ?? ""
    }
    
    func updateWindowFrame(animated: Bool) {
        
        guard let selectedItem = tabView.selectedTabViewItem, let window = view.window else {
            return
        }
        
        let contentSize = (selectedItem.viewController! as! OEPreferencePane).viewSize
        let newWindowSize = window.frameRect(forContentRect: NSRect(origin: NSPoint.zero, size: contentSize)).size
        
        var frame = window.frame
        frame.origin.y += frame.height
        frame.origin.y -= newWindowSize.height
        frame.size = newWindowSize
        
        if animated {
            window.animator().setFrame(frame, display: false)
        } else {
            window.setFrame(frame, display: false)
        }
    }
    
    // MARK: - Debug Pane Visibility
    
    func toggleDebugPaneVisibility() {
        
        if let debugPaneIndex = children.firstIndex(where: { $0 is OEPrefDebugController }) {
            
            if selectedTabViewItemIndex == debugPaneIndex {
                tabView.selectTabViewItem(at: 0)
            }
            
            removeChild(at: debugPaneIndex)
            
        } else {
            
            addTabViewItem(with: OEPrefDebugController())
        }
    }
}
