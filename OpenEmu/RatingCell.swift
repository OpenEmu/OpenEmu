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

@objc(OERatingCell)
final class RatingCell: NSCell {
    
    override func draw(withFrame cellFrame: NSRect, in controlView: NSView) {
        
        let color: NSColor = isHighlighted ? .white : .controlAccentColor
        let image = NSImage(named: "list_rating")?.withTintColor(color)
        let rating = objectValue as? CGFloat ?? 0
        
        var sourceRect = NSRect(x: 0, y: 55 - rating * 11, width: 55, height: 11)
        if isHighlighted {
            sourceRect.origin.x = 55
        }
        
        let targetRect = NSRect(x: floor(cellFrame.origin.x + (cellFrame.size.width - sourceRect.size.width) / 2),
                                y: floor(cellFrame.origin.y + (cellFrame.size.height - sourceRect.size.height) / 2),
                            width: sourceRect.size.width,
                           height: sourceRect.size.height)
        
        image?.draw(in: targetRect, from: sourceRect, operation: .sourceOver, fraction: 1, respectFlipped: true, hints: nil)
    }
    
    // MARK: -
    
    override func startTracking(at startPoint: NSPoint, in controlView: NSView) -> Bool {
        updateRating(with: startPoint, in: controlView as! NSTableView)
        return true
    }
    
    override func continueTracking(last lastPoint: NSPoint, current pos: NSPoint, in controlView: NSView) -> Bool {
        updateRating(with: pos, in: controlView as! NSTableView)
        return true
    }
    
    override func stopTracking(last lastPoint: NSPoint, current stopPoint: NSPoint, in controlView: NSView, mouseIsUp flag: Bool) {
        updateRating(with: stopPoint, in: controlView as! NSTableView)
    }
    
    func updateRating(with point: NSPoint, in view: NSTableView) {
        objectValue = rating(for: point, in: view)
    }
    
    func rating(for point: NSPoint, in view: NSTableView) -> Int {
        
        let row = view.row(at: point)
        let column = view.column(at: point)
        
        let frame = view.frameOfCell(atColumn: column, row: row)
        
        let minRatingX = (frame.size.width - 55) / 2
        let posX = point.x - frame.origin.x - minRatingX
        let stepWidth: CGFloat = 11
        
        return Int(max(0, min((posX / stepWidth) + 1, 5)))
    }
}
