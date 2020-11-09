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

@objc(OEPreferencesControlsBox)
final class PreferencesControlsBox: NSView {
    
    lazy var lineColor = NSColor(deviceRed: 0.45, green: 0.24, blue: 0, alpha: 1)
    lazy var topHighlightColor = NSColor(deviceRed: 1, green: 0.92, blue: 0, alpha: 0.14)
    lazy var bottomHighlightColor = NSColor(deviceRed: 1, green: 0.92, blue: 0, alpha: 0.3)
    
    override func draw(_ dirtyRect: NSRect) {
        
        guard UserDefaults.standard.integer(forKey: OEControlsPrefsAppearancePreferenceKey) == OEControlsPrefsAppearancePreferenceValue.wood.rawValue else {
            
            if #available(macOS 10.16, *) {
                NSImage(named: "controls_box_11")?.draw(in: bounds)
            } else {
                NSImage(named: "controls_box")?.draw(in: bounds)
            }
            
            var lineRect = bounds
            lineRect.size.height = 1
            lineRect.size.width -= 2
            lineRect.origin.x += 1
            
            // Draw top separator.
            lineRect.origin.y = frame.size.height - 49
            NSColor.darkGray.setFill()
            lineRect.fill(using: .sourceIn)
            
            // Draw bottom separator.
            lineRect.origin.y = 48
            NSColor.darkGray.setFill()
            lineRect.fill(using: .sourceIn)
            
            return
        }
        
        NSImage(named: "wood_inset_box")?.draw(in: bounds)
        
        var lineRect = bounds
        lineRect.size.height = 1
        lineRect.size.width -= 4
        lineRect.origin.x += 2
        
        // Draw top separator.
        
        lineRect.origin.y = 317
        
        topHighlightColor.setFill()
        lineRect.fill()
        
        lineRect.origin.y -= 1
        lineColor.setFill()
        lineRect.fill(using: .sourceOver)
        
        // Draw bottom separator.
        
        lineRect.origin.y = 46
        
        lineColor.setFill()
        lineRect.fill()
        
        lineRect.origin.y -= 1
        bottomHighlightColor.setFill()
        lineRect.fill(using: .sourceOver)
    }
}
