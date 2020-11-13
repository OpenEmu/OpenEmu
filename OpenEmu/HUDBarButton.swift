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

final class HUDBarButton: HoverButton {
    
    @objc var backgroundColor = NSColor.clear
    
    override func draw(_ dirtyRect: NSRect) {
        
        var backgroundImageName = ""
        
        if backgroundColor == .black {
            backgroundImageName = isHighlighted ? "hud_button_black_pressed" : "hud_button_black"
        } else if backgroundColor == .red {
            backgroundImageName = isHighlighted ? "hud_button_red_pressed" : "hud_button_red"
        }
        
        NSImage(named: backgroundImageName)?.draw(in: dirtyRect)
        
        if var img = state == .on && alternateImage != nil ? alternateImage : image {
            if isHighlighted || !isHovering {
                img = img.withTintColor(.labelColor)
            } else {
                img = img.withTintColor(.white)
            }
            
            var imageRect = NSRect(x: frame.size.width/2 - img.size.width/2,
                                   y: frame.size.height/2 - img.size.height/2,
                               width: img.size.width,
                              height: img.size.height)
            imageRect = backingAlignedRect(imageRect, options:.alignAllEdgesNearest)
            
            img.draw(in: imageRect)
        }
    }
}
