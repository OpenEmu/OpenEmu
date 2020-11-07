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

@objc(OEControlsScroller)
final class ControlsScroller: NSScroller {
    
    lazy var knobImage  = NSImage(named: "wood_knob_vertical")
    
    override class var isCompatibleWithOverlayScrollers: Bool {
        return true
    }
    
    override class func scrollerWidth(for controlSize: NSControl.ControlSize, scrollerStyle: NSScroller.Style) -> CGFloat {
        guard scrollerStyle == .legacy else {
            return super.scrollerWidth(for: controlSize, scrollerStyle: scrollerStyle)
        }
        
        return 15
    }
    
    override func drawKnobSlot(in slotRect: NSRect, highlight flag: Bool) {
        
        let slotColor = NSColor(deviceRed: 0.45, green: 0.24, blue: 0, alpha: 0.3)
        slotColor.setFill()
        
        var slotRect = bounds
        slotRect.fill()
        
        let lineColor = NSColor(deviceRed: 0.45, green: 0.24, blue: 0, alpha: 1)
        lineColor.setFill()
        
        slotRect.size.width = 1
        slotRect.fill()
    }
    
    override func drawKnob() {
        guard scrollerStyle == .legacy else {
            return super.drawKnob()
        }
        
        let targetRect = rect(for: .knob)
        knobImage?.draw(in: targetRect)
    }
    
    override func rect(for aPart: NSScroller.Part) -> NSRect {
        guard scrollerStyle == .legacy,
              aPart == .knob else {
            return super.rect(for: aPart)
        }
        
        var knobRect = rect(for: .knobSlot)
        knobRect = knobRect.insetBy(dx: 0, dy: 2)
        
        guard let size = knobImage?.size else { return .zero }
        
        var knobHeight = round(knobRect.size.height*knobProportion)
        knobHeight = knobHeight < size.height ? size.height : knobHeight
        
        knobRect.size.width -= 2
        
        let knobY = knobRect.origin.y+round((knobRect.size.height-knobHeight)*CGFloat(floatValue))
        knobRect = NSRect(x: 0, y: knobY, width: knobRect.size.width, height: knobHeight)
        knobRect.origin.x += 1
        
        return knobRect
    }
}
