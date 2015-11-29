/*
 Copyright (c) 2015, OpenEmu Team
 
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

private let activeFillColor   = NSColor(calibratedRed: 0.243, green: 0.502, blue: 0.871, alpha: 1)
private let inactiveFillColor = NSColor(calibratedWhite: 0.651, alpha: 1)

extension NSImage {
    
    /// Creates an image that draws a grid selection ring in the default coordinate system.
    /// - Parameter size: The size of the selection ring.
    /// - Parameter activeState: Whether or not the selection ring should be drawn in an active state.
    /// - Returns: An image of the grid selection ring.
    class func gridSelectionRingWithSize(size: NSSize, activeState: Bool) -> NSImage {
        
        return NSImage(size: size, flipped: false) { rect in
            
            let bounds = CGRect(x: 0, y: 0, width: rect.size.width, height: rect.size.height)
            
            let selectionPath = NSBezierPath(roundedRect: CGRectInset(bounds, 1, 1), xRadius: 4, yRadius: 4)
            selectionPath.appendBezierPath(NSBezierPath(roundedRect: CGRectInset(bounds, 4, 4), xRadius: 1.5, yRadius: 1.5))
            selectionPath.windingRule = .EvenOddWindingRule
            
            let fillColor: NSColor = activeState ? activeFillColor : inactiveFillColor
            
            fillColor.set()
            selectionPath.fill()
            
            return true
        }
    }
}
