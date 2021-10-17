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

final class GameScannerButton: HoverButton {
    
    enum Icon {
        case cancel, `continue`, pause
        
        fileprivate var image: NSImage {
            switch self {
            case .cancel:
                return NSImage(named: "game_scanner_cancel")!
            case .continue:
                return NSImage(named: "game_scanner_continue")!
            case .pause:
                return NSImage(named: "game_scanner_pause")!
            }
        }
    }
    
    private var _icon: Icon = .cancel
    var icon: Icon {
        get {
            if NSEvent.modifierFlags.contains(.option) {
                return .cancel
            } else {
                return _icon
            }
        }
        set {
            _icon = newValue
            needsDisplay = true
        }
    }
    
    private var iconImage: NSImage {
        if isHighlighted {
            return icon.image.withTintColor(.labelColor.withSystemEffect(.pressed))
        }
        else if isHovering && isEnabled {
            return icon.image.withTintColor(.labelColor.withSystemEffect(.rollover))
        }
        else if let window = window, !window.isMainWindow {
            return icon.image.withTintColor(.labelColor.withSystemEffect(.disabled))
        }
        else {
            return icon.image.withTintColor(.labelColor)
        }
    }
    
    private var eventMonitor: Any?
    
    required init?(coder: NSCoder) {
        super.init(coder: coder)
        commonInit()
    }
    
    override init(frame frameRect: NSRect) {
        super.init(frame: frameRect)
        commonInit()
    }
    
    private func commonInit() {
        eventMonitor = NSEvent.addLocalMonitorForEvents(matching: [.flagsChanged]) { [weak self] event in
            self?.needsDisplay = true
            return event
        }
    }
    
    deinit {
        if let eventMonitor = eventMonitor {
            NSEvent.removeMonitor(eventMonitor)
            self.eventMonitor = nil
        }
    }
    
    override func draw(_ dirtyRect: NSRect) {
        image = iconImage
        super.draw(dirtyRect)
    }
    
    override func viewWillMove(toWindow newWindow: NSWindow?) {
        super.viewWillMove(toWindow: newWindow)
        
        if window != nil {
            NotificationCenter.default.removeObserver(self, name: NSWindow.didBecomeMainNotification, object: window)
            NotificationCenter.default.removeObserver(self, name: NSWindow.didResignMainNotification, object: window)
        }
        
        if newWindow != nil {
            NotificationCenter.default.addObserver(self, selector: #selector(windowKeyChanged), name: NSWindow.didBecomeMainNotification, object: newWindow)
            NotificationCenter.default.addObserver(self, selector: #selector(windowKeyChanged), name: NSWindow.didResignMainNotification, object: newWindow)
        }
    }
    
    @objc func windowKeyChanged() {
        needsDisplay = true
    }
}
