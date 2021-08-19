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

@available(macOS, introduced: 10.14, deprecated: 11.0, message: "Remove everything but searchMenuTemplate.")
final class OESearchField: NSSearchField {
    
    // Force redraw of the search glyph after changing the menu to make sure the chevron is displayed.
    override var searchMenuTemplate: NSMenu? {
        didSet {
            let cell = self.cell as! NSSearchFieldCell
            cell.resetSearchButtonCell()
        }
    }
    
    // Due to a bug in AppKit, the background of a disabled search field is not dimmed when its window moves into the background.
    // As a workaround, enable the search field when it goes into background and restore its state once its window becomes key again
    // (unless the enabled state changed in the meantime).
    // Should be removed once support for Catalina is dropped.
    var wasEnabled: Bool?
    var isInBackground = false
    
    override var isEnabled: Bool {
        didSet {
            if isInBackground {
                wasEnabled = nil
            }
        }
    }
    
    override func viewWillMove(toWindow newWindow: NSWindow?) {
        super.viewWillMove(toWindow: newWindow)
        
        if #available(macOS 11.0, *) {
            return
        } else {
            
            if window != nil {
                NotificationCenter.default.removeObserver(self, name: NSWindow.didBecomeMainNotification, object: window)
                NotificationCenter.default.removeObserver(self, name: NSWindow.didResignMainNotification, object: window)
            }
            
            if newWindow != nil {
                NotificationCenter.default.addObserver(self, selector: #selector(windowDidBecomeMain), name: NSWindow.didBecomeMainNotification, object: newWindow)
                NotificationCenter.default.addObserver(self, selector: #selector(windowDidResignMain), name: NSWindow.didResignMainNotification, object: newWindow)
            }
        }
    }
    
    @objc func windowDidResignMain() {
        wasEnabled = isEnabled
        isEnabled = true
        isInBackground = true
    }
    
    @objc func windowDidBecomeMain() {
        isInBackground = false
        if wasEnabled != nil {
            isEnabled = wasEnabled!
            wasEnabled = nil
        }
    }
}
