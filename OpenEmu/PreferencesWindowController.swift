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
class PreferencesWindowController: NSWindowController, NSWindowDelegate {
    
    static let debugModeKey = "debug"
    static let selectedPreferencesTabKey = "selectedPreferencesTab"
    
    static let openPaneNotificationName = "OEPrefOpenPane"
    static let userInfoPanelNameKey = "panelName"
    static let userInfoSystemIdentifierKey = "systemIdentifier"
    
    static let toolbarItemIdentiftierDebug = "OEToolBarItemIdentifierDebug"
    static let toolbarItemIdentifierSeparator = "OEToolbarItemIdentifierSeparator"
    
    @IBOutlet var backgroundView: OEBackgroundColorView!
    
    let konamiCode = [NSUpArrowFunctionKey,
                      NSUpArrowFunctionKey,
                      NSDownArrowFunctionKey,
                      NSDownArrowFunctionKey,
                      NSLeftArrowFunctionKey,
                      NSRightArrowFunctionKey,
                      NSLeftArrowFunctionKey,
                      NSRightArrowFunctionKey,
                      98, // 'b'
                      97] // 'a'
    var konamiCodeIndex = 0
    var konamiCodeMonitor: AnyObject?
    
    var preferencesTabViewController: PreferencesTabViewController!
    
    override func windowDidLoad() {
        
        super.windowDidLoad()
        
        backgroundView.wantsLayer = true
        
        window!.backgroundColor = NSColor(deviceWhite: 0.149, alpha: 1)
    
        preferencesTabViewController = PreferencesTabViewController()
        
        preferencesTabViewController.view.frame = backgroundView.bounds
        preferencesTabViewController.view.autoresizingMask = [.ViewWidthSizable, .ViewHeightSizable]
        
        backgroundView.addSubview(preferencesTabViewController.view)
        
        window!.center()
    }
    
    // MARK: -
    
    func showWindowWithNotification(notification: NSNotification) {
        
        showWindow(nil)
        
        guard let identifier = notification.userInfo?[PreferencesWindowController.userInfoPanelNameKey] as? String else {
            return
        }
        
        selectPaneWithTabViewIdentifier(identifier)
        
        // If selected, the controls preference pane performs additional work using the notification.
        if let controlsPane = preferencesTabViewController.tabView.selectedTabViewItem?.viewController as? OEPrefControlsController {
            controlsPane.preparePaneWithNotification(notification)
        }
    }
    
    func selectPaneWithTabViewIdentifier(identifier: String) {
        
        let index = preferencesTabViewController.tabView.indexOfTabViewItemWithIdentifier(identifier)
        
        guard index != NSNotFound else {
            return
        }
        
        preferencesTabViewController.tabView.selectTabViewItemAtIndex(index)
    }
    
    // MARK: - NSWindowDelegate
    
    func windowDidBecomeKey(notification: NSNotification) {
        
        konamiCodeIndex = 0
        
        konamiCodeMonitor = NSEvent.addLocalMonitorForEventsMatchingMask(.KeyDownMask) { event in
            
            if Int((event.characters! as NSString).characterAtIndex(0)) == self.konamiCode[self.konamiCodeIndex] {
                
                self.konamiCodeIndex += 1
                
                if self.konamiCodeIndex == self.konamiCode.count {
                    
                    let defaults = NSUserDefaults.standardUserDefaults()
                    let debugModeActivated = !defaults.boolForKey(PreferencesWindowController.debugModeKey)
                    defaults.setBool(debugModeActivated, forKey: PreferencesWindowController.debugModeKey)
                    
                    NSSound(named: "secret")!.play()
                    
                    self.preferencesTabViewController.toggleDebugPaneVisibility()
                    
                    self.konamiCodeIndex = 0
                }
                
                return nil
            }
            
            self.konamiCodeIndex = 0
            
            return event
        }
    }
    
    func windowDidResignKey(notification: NSNotification) {
        
        if let konamiCodeMonitor = konamiCodeMonitor {
            NSEvent.removeMonitor(konamiCodeMonitor)
        }
        
        konamiCodeIndex = 0
        konamiCodeMonitor = nil
    }
}

class PreferencesTabViewController: NSTabViewController {
    
    /// Used to track when viewDidLoad has completed so that the default pane index selection of 0 doesn't get recorded in tabView(_:, didSelectTabViewItem:).
    private var viewDidLoadFinished = false
    
    override func viewDidLoad() {
        
        super.viewDidLoad()
        
        tabStyle = .Toolbar
        
        // The collection of preference panes to add.
        var preferencePanes: [OEPreferencePane] = [
            OEPrefLibraryController(),
            OEPrefGameplayController(),
            OEPrefControlsController(),
            OEPrefCoresController(),
            OEPrefBiosController()
        ]
        
        // Check if the debug pane should be included.
        let defaults = NSUserDefaults.standardUserDefaults()
        if defaults.boolForKey(PreferencesWindowController.debugModeKey) {
            preferencePanes.append(OEPrefDebugController())
        }
        
        // Add tab view items for each preference pane.
        for pane in preferencePanes {
            addTabViewItemWithPreferencePane(pane)
        }
        
        // Check defaults for a pane to select.
        var indexOfPaneToSelect = defaults.integerForKey(PreferencesWindowController.selectedPreferencesTabKey)
        if !(0..<childViewControllers.count ~= indexOfPaneToSelect) {
            indexOfPaneToSelect = 0
        }
        
        // Select a pane.
        tabView.selectTabViewItemAtIndex(indexOfPaneToSelect)
        
        viewDidLoadFinished = true
    }
    
    override func viewDidAppear() {
        
        super.viewDidAppear()
        
        updateWindowTitle()
        updateWindowFrameAnimated(false)
    }
    
    override func transitionFromViewController(fromViewController: NSViewController, toViewController: NSViewController, options: NSViewControllerTransitionOptions, completionHandler completion: (() -> Void)?) {
        
        NSAnimationContext.runAnimationGroup({ context in
            
            self.updateWindowTitle()
            self.updateWindowFrameAnimated(true)
            
            super.transitionFromViewController(fromViewController, toViewController: toViewController, options: [.Crossfade, .AllowUserInteraction], completionHandler: completion)
            
        }, completionHandler: nil)
    }
    
    override func tabView(tabView: NSTabView, didSelectTabViewItem tabViewItem: NSTabViewItem?) {
        
        super.tabView(tabView, didSelectTabViewItem: tabViewItem)
        
        guard viewDidLoadFinished else {
            return
        }
        
        // Record the selected pane index in user defaults.
        NSUserDefaults.standardUserDefaults().setInteger(selectedTabViewItemIndex, forKey: PreferencesWindowController.selectedPreferencesTabKey)
    }
    
    // MARK: -
    
    func addTabViewItemWithPreferencePane(pane: OEPreferencePane) {
        
        let item = NSTabViewItem(viewController: pane as! NSViewController)
        
        item.image = pane.icon
        item.identifier = pane.title
        item.label = pane.localizedTitle
        
        addTabViewItem(item)
    }
    
    func updateWindowTitle() {
        view.window?.title = tabView.selectedTabViewItem?.label ?? ""
    }
    
    func updateWindowFrameAnimated(animated: Bool) {
        
        guard let selectedItem = tabView.selectedTabViewItem, window = view.window else {
            return
        }
        
        let contentSize = (selectedItem.viewController! as! OEPreferencePane).viewSize
        let newWindowSize = window.frameRectForContentRect(NSRect(origin: NSPoint.zero, size: contentSize)).size
        
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
        
        if let debugPaneIndex = childViewControllers.indexOf({ $0 is OEPrefDebugController }) {
            
            if selectedTabViewItemIndex == debugPaneIndex {
                tabView.selectTabViewItemAtIndex(0)
            }
            
            removeChildViewControllerAtIndex(debugPaneIndex)
            
        } else {
            
            addTabViewItemWithPreferencePane(OEPrefDebugController())
        }
    }
}
