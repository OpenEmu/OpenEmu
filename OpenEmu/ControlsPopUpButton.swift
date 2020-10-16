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

@objc(OEControlsPopUpButton)
final class ControlsPopUpButton: NSPopUpButton {
    
    private var cachedIntrinsicWidth: CGFloat = 0
    
    required init?(coder aDecoder: NSCoder) {
        super.init(coder: aDecoder)
        
        frame.size.height = 23
        cachedIntrinsicWidth = -1
    }
    
    override var alignmentRectInsets: NSEdgeInsets {
        
        return NSEdgeInsets(top: 0, left: 0, bottom: 0, right: 0)
    }
    
    override var menu: NSMenu? {
        didSet {
            cachedIntrinsicWidth = -1
        }
    }
    
    override var intrinsicContentSize: NSSize {
        
        if cachedIntrinsicWidth < 0 {
            if menu != nil {
                cachedIntrinsicWidth = menu!.minimumWidth
            } else {
                cachedIntrinsicWidth = NSView.noIntrinsicMetric
            }
        }
        
        return NSSize(width: cachedIntrinsicWidth, height: frame.size.height)
    }
    
    override var isFlipped: Bool {
        return false
    }
}
