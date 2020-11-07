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

@objc(OEGlossButton)
final class GlossButton: NSButton {
    
    private var subImageRect         = NSRect.zero
    private var subImageRectInactive = NSRect.zero
    
    private let subImageRectBlack    = NSRect(x: 0, y: 138, width: 103, height: 23)
    private let subImageRectPressed  = NSRect(x: 0, y: 115, width: 103, height: 23)
    private let subImageRectBlackAlt = NSRect(x: 0, y: 92, width: 103, height: 23)
    private let subImageRectBlue     = NSRect(x: 0, y: 69, width: 103, height: 23)
    private let subImageRectBlueAlt  = NSRect(x: 0, y: 46, width: 103, height: 23)
    private let subImageRectGreen    = NSRect(x: 0, y: 23, width: 103, height: 23)
    private let subImageRectGreenAlt = NSRect(x: 0, y: 0, width: 103, height: 23)
    
    private static let attributes: [NSAttributedString.Key : Any] = {
        
        let font = NSFont.boldSystemFont(ofSize: 11)
        let color = NSColor(white: 0.91, alpha: 1)
        
        let shadow = NSShadow()
        shadow.shadowColor = .black
        shadow.shadowOffset = NSMakeSize(0, 1)
        
        let style = NSMutableParagraphStyle()
        style.alignment = .center
        
        let attributes: [NSAttributedString.Key : Any] =
                                          [.font: font,
                                .foregroundColor: color,
                                         .shadow: shadow,
                                 .paragraphStyle: style]
        
        return attributes
    }()
    
    private static let attributesPressed: [NSAttributedString.Key : Any] = {
        
        let font = NSFont.boldSystemFont(ofSize: 11)
        let color = NSColor(white: 0.03, alpha: 1)
        
        let shadow = NSShadow()
        shadow.shadowColor = NSColor.white.withAlphaComponent(0.4)
        shadow.shadowOffset = NSMakeSize(0, -1)
        
        let style = NSMutableParagraphStyle()
        style.alignment = .center
        
        let attributes: [NSAttributedString.Key : Any] =
                                          [.font: font,
                                .foregroundColor: color,
                                         .shadow: shadow,
                                 .paragraphStyle: style]
        
        return attributes
    }()
    
    @objc var themeKey: String? {
        didSet {
            if themeKey == "gloss_button" {
                subImageRect = subImageRectBlack
                subImageRectInactive = subImageRectBlackAlt
            }
            else if themeKey == "gloss_button_blue" {
                subImageRect = subImageRectBlue
                subImageRectInactive = subImageRectBlueAlt
            }
            if themeKey == "gloss_button_green" {
                subImageRect = subImageRectGreen
                subImageRectInactive = subImageRectGreenAlt
            }
            needsDisplay = true
        }
    }
    
    override func draw(_ dirtyRect: NSRect) {
        
        var bgSubImageRect = NSRect.zero
        
        if isHighlighted {
            bgSubImageRect = subImageRectPressed
        }
        else if window?.isMainWindow == false {
            bgSubImageRect = subImageRectInactive
        }
        else {
            bgSubImageRect = subImageRect
        }
        
        NSImage(named: "gloss_button_")?.subImage(from: bgSubImageRect).draw(in: dirtyRect)
        
        attributedTitle = NSAttributedString(string: title, attributes: isHighlighted ? GlossButton.attributesPressed : GlossButton.attributes)
        
        super.draw(dirtyRect)
    }
    
    override func viewWillMove(toWindow newWindow: NSWindow?) {
        super.viewWillMove(toWindow: newWindow)
        
        if (window != nil) {
            
            NotificationCenter.default.removeObserver(self, name: NSWindow.didBecomeMainNotification, object: window)
            NotificationCenter.default.removeObserver(self, name: NSWindow.didResignMainNotification, object: window)
        }
        
        if (newWindow != nil) {
            
            NotificationCenter.default.addObserver(self, selector: #selector(windowKeyChanged), name: NSWindow.didBecomeMainNotification, object: newWindow)
            NotificationCenter.default.addObserver(self, selector: #selector(windowKeyChanged), name: NSWindow.didResignMainNotification, object: newWindow)
        }
    }
    
    @objc func windowKeyChanged() {
        needsDisplay = true
    }
}
