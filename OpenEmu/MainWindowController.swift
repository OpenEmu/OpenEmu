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

class MainWindowController: NSWindowController {
    
    @IBOutlet weak var categoryControl: NSSegmentedControl!
    
    var tabViewController: NSTabViewController?
    var allowWindowResizing = true
    var isLaunchingGame = false
    
    var gameDocument: OEGameDocument?
    
    var observer: NSKeyValueObservation?
    
    override func awakeFromNib() {
        super.awakeFromNib()
        
        guard let window = window else { return }
        precondition(window.identifier! == .libraryWindow)
        
        window.isExcludedFromWindowsMenu = true
        
        tabViewController = self.contentViewController as? NSTabViewController
        
        categoryControl.bind(.selectedIndex, to: tabViewController!, withKeyPath: "selectedTabViewItemIndex", options: nil)
        observer = tabViewController?.observe(\.selectedTabViewItemIndex, options: [.new]) { (_, change) in
            print("\(change.newValue ?? -1)")
        }
        
        self.window?.restorationClass = MainWindowController.self
    }
    
    func setupWindow() {
        
    }

    override func windowDidLoad() {
        super.windowDidLoad()
        
        if let window = window {
            // https://stackoverflow.com/a/44140102/12606
            window.setFrameUsingName(.libraryWindow)
            windowFrameAutosaveName = .libraryWindow
        }
    }

}

extension MainWindowController: NSWindowRestoration {
    static func restoreWindow(withIdentifier identifier: NSUserInterfaceItemIdentifier, state: NSCoder, completionHandler: @escaping (NSWindow?, Error?) -> Void) {
        guard
            identifier == .libraryWindow,
            let delegate = NSApp.delegate as? AppDelegate
        else { return }
        
        NSApp.extendStateRestoration()
        delegate.restoreWindow = true
        
        let observer = NotificationCenter.default.addObserver(forName: .OELibraryDidLoadNotificationName, object: nil, queue: .main) { _ in
            defer { NSApp.completeStateRestoration() }
            completionHandler(delegate.mainWindowController2.window, nil)
        }
        delegate.libraryDidLoadObserverForRestoreWindow2 = observer
    }
}

extension MainWindowController: NSWindowDelegate {
    
    static let openEmuWindowMenuTag = 502
    
    var windowMenuItem: NSMenuItem? {
        NSApp.mainMenu?.item(at: 5)?.submenu?.item(withTag: Self.openEmuWindowMenuTag)
    }
    
    func windowWillResize(_ sender: NSWindow, to frameSize: NSSize) -> NSSize {
        allowWindowResizing ? frameSize : sender.frame.size
    }
    
    func windowDidBecomeMain(_ notification: Notification) {
        windowMenuItem?.state = .on
    }
    
    func windowDidResignMain(_ notification: Notification) {
        windowMenuItem?.state = .off
    }
}

extension NSWindow.FrameAutosaveName {
    static let libraryWindow = NSWindow.FrameAutosaveName("LibraryWindowV2")
}

extension NSUserInterfaceItemIdentifier {
    static let libraryWindow = NSUserInterfaceItemIdentifier("LibraryWindowV2")
}
