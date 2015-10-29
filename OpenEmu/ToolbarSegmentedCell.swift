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

@objc(OEToolbarSegmentedCell)
class ToolbarSegmentedCell: NSSegmentedCell {
    
    let cornerRadius: CGFloat = 3

    let fillGradient = NSGradient(colorsAndLocations: (NSColor(deviceWhite: 0.21, alpha: 1), 0),
                                                      (NSColor(deviceWhite: 0.14, alpha: 1), 0.95),
                                                      (NSColor(deviceWhite: 0.05, alpha: 1), 1))!
    
    let bezelGradient = NSGradient(startingColor: NSColor(deviceWhite: 0.35, alpha: 1),
                                     endingColor: NSColor(deviceWhite: 0.15, alpha: 1))!
    
    let topActiveGradient = NSGradient(colorsAndLocations:
        (NSColor(deviceWhite: 0, alpha: 0.95), 0),
        (NSColor(deviceWhite: 0, alpha: 0.6), 0.1),
        (NSColor(deviceWhite: 0, alpha: 0),   0.5))!
    
    let horizontalActiveGradient = NSGradient(colorsAndLocations:
        (NSColor(deviceWhite: 0, alpha: 0.4), 0),
        (NSColor(deviceWhite: 0, alpha: 0),   0.1))!
    
    let bottomActiveGradient = NSGradient(colorsAndLocations:
        (NSColor(deviceWhite: 0, alpha: 0.2), 0),
        (NSColor(deviceWhite: 0, alpha: 0.2), 0.05),
        (NSColor(deviceWhite: 0, alpha: 0),   0.3))!
    
    let borderColor = NSColor(deviceWhite: 0.01, alpha: 1)
    
    /// The segment currently being highlighted by the user.
    var highlightedSegment: Int?

    override func drawWithFrame(cellFrame: NSRect, inView controlView: NSView) {
        
        drawBezelLayerInFrame(cellFrame)
        drawBottomBorderLayerInFrame(cellFrame)
        drawFillLayerInFrame(cellFrame)
        
        let segmentWidth = NSWidth(cellFrame) / CGFloat(segmentCount)

        for segment in 0..<segmentCount {
            
            let segmentRect = NSRect(x: CGFloat(segment) * segmentWidth,
                y: NSMinY(cellFrame),
                width: segmentWidth,
                height: NSHeight(cellFrame))
            
            drawSegment(segment, inFrame: segmentRect, withView: controlView)
        }
    }
    
    // MARK: - Bezel Layer
    
    func drawBezelLayerInFrame(cellFrame: NSRect) {
        
        let path = NSBezierPath(roundedRect: cellFrame, xRadius: cornerRadius, yRadius: cornerRadius)
        
        bezelGradient.drawInBezierPath(path, angle: 90)
    }
    
    // MARK: - Fill Layer
    
    func fillLayerRectInFrame(frame: NSRect) -> NSRect {
        
        var rect = frame
        
        rect.size.height -= 2
        rect.origin.y += 1.2
        
        return rect
    }
    
    func fillLayerPathInFrame(frame: NSRect) -> NSBezierPath {
        
        let rect = fillLayerRectInFrame(frame)
        
        return NSBezierPath(roundedRect: rect, xRadius: cornerRadius, yRadius: cornerRadius)
    }
    
    func drawFillLayerInFrame(frame: NSRect) {
        
        let path = fillLayerPathInFrame(frame)
        
        fillGradient.drawInBezierPath(path, angle: 90)
    }
    
    // MARK: - Bottom Border Layer
    
    func bottomBorderLayerRectInFrame(frame: NSRect) -> NSRect {
        
        var rect = frame
        
        rect.size.height -= 3
        rect.origin.y += 2.5
        
        return rect
    }
    
    func drawBottomBorderLayerInFrame(frame: NSRect) {
        
        let rect = bottomBorderLayerRectInFrame(frame)
        
        borderColor.set()
        
        let path = NSBezierPath(roundedRect: rect, xRadius: cornerRadius, yRadius: cornerRadius)

        path.fill()
    }
    
    // MARK: - Segments
    
    func rectForSegment(segment: Int) -> NSRect {
        
        let bounds = controlView!.bounds

        let xOffset: CGFloat
        if segment > 0 {
            let segmentWidths = (0..<segment).map { return widthForSegment($0) }
            xOffset = segmentWidths.reduce(NSMinX(bounds), combine: +)
        } else {
            xOffset = NSMinX(bounds)
        }

        return NSRect(x: xOffset,
                      y: NSMinY(bounds),
                  width: widthForSegment(segment),
                 height: NSHeight(bounds))
    }
    
    func pathForSegment(segment: Int, inFrame frame: NSRect) -> NSBezierPath {
        
        let firstSegment = 0
        let lastSegment = segmentCount - 1
        
        if segment == firstSegment {
            
            return NSBezierPath.roundedRectInRect(frame,
                topLeftCornerRadius: cornerRadius,
                topRightCornerRadius: 0,
                bottomLeftCornerRadius: cornerRadius,
                bottomRightCornerRadius: 0)
            
        } else if segment == lastSegment {
            
            return NSBezierPath.roundedRectInRect(frame,
                topLeftCornerRadius: 0,
                topRightCornerRadius: cornerRadius,
                bottomLeftCornerRadius: 0,
                bottomRightCornerRadius: cornerRadius)
            
        } else {
            
            return NSBezierPath(rect: frame)
        }
    }
    
    func mouseInSegment(segment: Int) -> Bool {
        
        guard let controlView = controlView, window = controlView.window else {
            return false
        }
        
        let segmentRect = rectForSegment(segment)
        let pointInWindow = window.convertPointFromScreen(NSEvent.mouseLocation())
        let pointInControlView = controlView.convertPoint(pointInWindow, fromView: nil)
        
        return NSPointInRect(pointInControlView, segmentRect)
    }

    override func drawSegment(segment: Int, inFrame frame: NSRect, withView controlView: NSView) {
        
        let highlighted = segment == highlightedSegment && mouseInSegment(segment)
        let selected = segment == selectedSegment
        
        if highlighted || selected {
            
            // Draw active/highlighted state.
            
            // Make room for the bottom highlight.
            var segmentRect = frame
            segmentRect.size.height -= 1
            
            let segmentPath = pathForSegment(segment, inFrame: segmentRect)
            
            topActiveGradient.drawInBezierPath(segmentPath, angle: 90)
            horizontalActiveGradient.drawInBezierPath(segmentPath, angle: 0)
            horizontalActiveGradient.drawInBezierPath(segmentPath, angle: 180)
            bottomActiveGradient.drawInBezierPath(segmentPath, angle: 270)
        }
        
        // Draw divider.
        if segment < segmentCount - 1 {
            
            let firstPoint = NSPoint(x: NSMaxX(frame), y: NSMinY(frame))
            let secondPoint = NSPoint(x: NSMaxX(frame), y: NSMaxY(frame) - 1) // Leave room for bottom bezel.
            
            borderColor.set()
            
            NSBezierPath.strokeLineFromPoint(firstPoint, toPoint: secondPoint)
        }
        
        drawTextForSegment(segment, inFrame: frame)
    }
    
    func drawTextForSegment(segment: Int, inFrame frame: NSRect) {
        
        guard let label = labelForSegment(segment) else {
            return
        }
        
        let windowIsKeyWindow = controlView!.window!.keyWindow
        let highlighted = segment == highlightedSegment && mouseInSegment(segment)
        let selected = segment == selectedSegment
        
        let textColor: NSColor
        if !windowIsKeyWindow {
            textColor = NSColor(deviceWhite: 0.6, alpha: 0.8)
        } else if highlighted {
            textColor = NSColor(deviceWhite: 1, alpha: 0.9)
        } else if selected {
            textColor = NSColor(deviceRed: 0.39, green: 0.77, blue: 1, alpha: 1)
        } else {
            textColor = NSColor(deviceWhite: 1, alpha: 0.8)
        }
        
        let textShadow = NSShadow()
        
        if highlighted {
            
            textShadow.shadowBlurRadius = 5
            textShadow.shadowOffset = NSSize(width: 0, height: 0)
            textShadow.shadowColor = NSColor(deviceWhite: 1, alpha: 0.6)
            
        } else if selected {
            
            textShadow.shadowBlurRadius = 1.5
            textShadow.shadowOffset = NSSize(width: 0, height: -1.5)
            textShadow.shadowColor = NSColor(deviceWhite: 0, alpha: 0.6)
            
        } else {
            
            textShadow.shadowBlurRadius = 1
            textShadow.shadowOffset = NSSize(width: 0, height: -1)
            textShadow.shadowColor = NSColor(deviceWhite: 0, alpha: 0.4)
        }
        
        let attributes: [String: AnyObject] = [
            NSFontAttributeName: NSFont.systemFontOfSize(11, weight: 0.1),
            NSForegroundColorAttributeName: textColor,
            NSShadowAttributeName: textShadow]
        
        let attributedString = NSAttributedString(string: label, attributes: attributes)
        
        var labelRect = frame
        labelRect.origin.y = (NSMidY(frame) - attributedString.size().height / 2)
        labelRect.origin.x = NSMidX(frame) - attributedString.size().width / 2
        labelRect.size.height = attributedString.size().height
        labelRect.size.width = attributedString.size().width

        attributedString.drawInRect(labelRect)
    }
    
    // MARK: - Mouse Tracking
    
    override func startTrackingAt(startPoint: NSPoint, inView controlView: NSView) -> Bool {
        
        for segment in 0..<segmentCount {
            if NSPointInRect(startPoint, rectForSegment(segment)) {
                highlightedSegment = segment
                break
            }
        }
        
        return super.startTrackingAt(startPoint, inView: controlView)
    }
    
    override func continueTracking(lastPoint: NSPoint, at currentPoint: NSPoint, inView controlView: NSView) -> Bool {
        
        controlView.needsDisplay = true
        
        return super.continueTracking(lastPoint, at: currentPoint, inView: controlView)
    }
    
    override func stopTracking(lastPoint: NSPoint, at stopPoint: NSPoint, inView controlView: NSView, mouseIsUp flag: Bool) {
        
        highlightedSegment = nil
        
        return super.stopTracking(lastPoint, at: stopPoint, inView: controlView, mouseIsUp: flag)
    }
}

extension NSBezierPath {
    
    /// Returns a path with rounded corners according to the specified corner radiuses.
    /// - Parameter rect: The rect containing the path.
    /// - Parameter topLeftCornerRadius: The radius of the top-left corner.
    /// - Parameter topRightCornerRadius: The radius of the top-right corner.
    /// - Parameter bottomLeftCornerRadius: The radius of the bottom-left corner.
    /// - Parameter bottomRightCornerRadius: The radius of the bottom-right corner.
    /// - Returns: A path with rounded corners based on the given corner radiuses.
    static func roundedRectInRect(rect: NSRect, topLeftCornerRadius: CGFloat, topRightCornerRadius: CGFloat, bottomLeftCornerRadius: CGFloat, bottomRightCornerRadius: CGFloat) -> NSBezierPath {
        
        let path = NSBezierPath()
        
        // Start near top-left corner.
        let startingPoint = NSPoint(x: NSMinX(rect) + topLeftCornerRadius,
                                    y: NSMinY(rect))
        path.moveToPoint(startingPoint)
        
        // Top edge.
        path.lineToPoint(NSPoint(x: NSMaxX(rect) - topRightCornerRadius,
                                 y: NSMinY(rect)))
        
        // Top-right corner.
        let topRightCorner = NSPoint(x: NSMaxX(rect),
                                     y: NSMinY(rect))
        path.curveToPoint(NSPoint(x: NSMaxX(rect),
                                  y: NSMinY(rect) + topRightCornerRadius),
            controlPoint1: topRightCorner,
            controlPoint2: topRightCorner)
        
        // Right edge.
        path.lineToPoint(NSPoint(x: NSMaxX(rect),
                                 y: NSMaxY(rect) - bottomRightCornerRadius))
        
        // Bottom-right corner.
        let bottomRightCorner = NSPoint(x: NSMaxX(rect),
                                        y: NSMaxY(rect))
        path.curveToPoint(NSPoint(x: NSMaxX(rect) - bottomRightCornerRadius,
                                  y: NSMaxY(rect)),
            controlPoint1: bottomRightCorner,
            controlPoint2: bottomRightCorner)
        
        // Bottom edge.
        path.lineToPoint(NSPoint(x: NSMinX(rect) + bottomLeftCornerRadius,
                                 y: NSMaxY(rect)))
        
        // Bottom-left corner.
        let bottomLeftCorner = NSPoint(x: NSMinX(rect),
                                       y: NSMaxY(rect))
        path.curveToPoint(NSPoint(x: NSMinX(rect),
                                  y: NSMaxY(rect) - bottomLeftCornerRadius),
            controlPoint1: bottomLeftCorner,
            controlPoint2: bottomLeftCorner)
        
        // Left edge.
        path.lineToPoint(NSPoint(x: NSMinX(rect),
                                 y: NSMinY(rect) + topLeftCornerRadius))
        
        // Top-left corner.
        let topLeftCorner = NSPoint(x: NSMinX(rect),
                                    y: NSMinY(rect))
        path.curveToPoint(startingPoint,
            controlPoint1: topLeftCorner,
            controlPoint2: topLeftCorner)
        
        path.closePath()
        
        return path
    }
}

extension NSWindow {
    
    func convertPointFromScreen(point: NSPoint) -> NSPoint {
     
        let screenRect = NSRect(origin: point, size: NSZeroSize)
        
        let convertedRect = convertRectFromScreen(screenRect)
        
        return convertedRect.origin
    }
}
