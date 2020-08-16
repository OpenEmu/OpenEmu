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

@objc(OEHUDSliderCell)
final class HUDSliderCell: NSSliderCell {
    
    let trackImage = NSImage(named: "hud_slider_track")
    let levelImage = NSImage(named: "hud_slider_level")
    let knobImage  = NSImage(named: "hud_slider_thumb")
    
    private var trackThickness: CGFloat {
        return trackImage?.size.height ?? trackRect.height
    }
    
    override func drawBar(inside aRect: NSRect, flipped: Bool) {
        
        var barRect = aRect.insetBy(dx: 2, dy: (aRect.height-trackThickness)/2)
        barRect = controlView?.backingAlignedRect(barRect, options: .alignAllEdgesNearest) ?? .zero
        
        trackImage?.draw(in: barRect)
        
        let knobRect = self.knobRect(flipped: flipped)
        
        var levelRect = barRect
        levelRect.size.width = max(knobRect.midX-barRect.minX, levelImage?.size.width ?? 0)
        levelRect = controlView?.backingAlignedRect(levelRect, options: .alignAllEdgesNearest) ?? .zero
        
        levelImage?.draw(in: levelRect)
    }
    
    override func drawKnob(_ knobRect: NSRect) {
        
        if let knobImage = self.knobImage {
            var knobRect = knobRect
            var knobOffset: CGFloat = 4 // knob shadow height
            if #available(macOS 11.0, *) {
                knobOffset += 1.5
            }
            knobRect.origin.y += knobRect.height-knobImage.size.height-knobOffset
            knobRect.origin.x += (knobRect.width-knobImage.size.width)/2
            knobRect.size = knobImage.size
            knobRect = controlView?.backingAlignedRect(knobRect, options: .alignAllEdgesNearest) ?? .zero
            
            knobImage.draw(in: knobRect)
        }
    }
}
