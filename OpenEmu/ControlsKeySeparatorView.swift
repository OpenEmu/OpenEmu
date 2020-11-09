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

@objc(OEControlsKeySeparatorView)
final class ControlsKeySeparatorView: NSView {
    
    private let topColor = NSColor(red: 85/255, green: 45/255, blue: 0, alpha: 1)
    private let bottomColor = NSColor(red: 1, green: 1, blue: 0, alpha: 0.2)
    
    override var isOpaque: Bool {
        return false
    }
    
    override func draw(_ dirtyRect: NSRect) {
        
        var lineRect = bounds
        lineRect.size.height = 1
        lineRect.origin.y = bounds.size.height/2
        
        if UserDefaults.standard.integer(forKey: OEControlsPrefsAppearancePreferenceKey) == OEControlsPrefsAppearancePreferenceValue.wood.rawValue {
            
            topColor.setFill()
            lineRect.fill()
            
            lineRect.origin.y -= 1
            bottomColor.setFill()
            
        } else {
            NSColor.darkGray.setFill()
        }
        
        lineRect.fill(using: .sourceOver)
    }
}
