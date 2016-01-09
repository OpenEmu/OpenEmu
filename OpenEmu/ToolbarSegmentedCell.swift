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
    
    let cornerRadius: CGFloat = 2.5
    
    let backgroundGradient = NSGradient(startingColor: NSColor(deviceWhite: 0.22, alpha: 1), endingColor: NSColor(deviceWhite: 0.14, alpha: 1))!
    
    let topHighlightColor = NSColor(deviceWhite: 0.31, alpha: 1)
    
    let borderColor = NSColor(deviceWhite: 0.08, alpha: 1)
    
    let topActiveGradient = NSGradient(colorsAndLocations:
        (NSColor(deviceWhite: 0, alpha: 0.75),    0),
        (NSColor(deviceWhite: 0, alpha: 0.5),  0.15),
        (NSColor(deviceWhite: 0, alpha: 0),    0.6))!
    
    let horizontalActiveGradient = NSGradient(colorsAndLocations:
        (NSColor(deviceWhite: 0, alpha: 0.8),    0),
        (NSColor(deviceWhite: 0, alpha: 0.4),  0.01),
        (NSColor(deviceWhite: 0, alpha: 0.01), 0.04),
        (NSColor(deviceWhite: 0, alpha: 0),    0.5))!
    
    let bottomActiveGradient = NSGradient(colorsAndLocations:
        (NSColor(deviceWhite: 0, alpha: 0.15), 0),
        (NSColor(deviceWhite: 0, alpha: 0.15), 0.05),
        (NSColor(deviceWhite: 0, alpha: 0),    0.15))!
    
    let disabledTextColor = NSColor(deviceWhite: 0.6, alpha: 0.8)
    let highlightedTextColor = NSColor(deviceWhite: 1, alpha: 0.9)
    let selectedTextColor = NSColor(deviceRed: 0.39, green: 0.77, blue: 1, alpha: 1)
    let normalTextColor = NSColor(deviceWhite: 1, alpha: 0.8)
    
    /// The segment currently being highlighted by the user.
    var highlightedSegment: Int?

    override func drawWithFrame(cellFrame: NSRect, inView controlView: NSView) {
        
        let segmentWidth = cellFrame.width / CGFloat(segmentCount)
        
        drawBackgroundLayerInFrame(cellFrame)
        drawTopHighlightEdgeLayerInFrame(cellFrame)

        for segment in 0..<segmentCount {
            
            let segmentRect = NSRect(x: CGFloat(segment) * segmentWidth,
                y: cellFrame.minY,
                width: segmentWidth,
                height: cellFrame.height)
            
            drawSegment(segment, inFrame: segmentRect, withView: controlView)
        }
        
        drawBorderLayerInFrame(cellFrame)
    }
    
    // MARK: - Layers
    
    func controlPathInFrame(cellFrame: NSRect) -> NSBezierPath {
        return NSBezierPath(roundedRect: cellFrame, xRadius: cornerRadius, yRadius: cornerRadius)
    }
    
    func drawBackgroundLayerInFrame(cellFrame: NSRect) {
        
        let path = controlPathInFrame(cellFrame)
        
        backgroundGradient.drawInBezierPath(path, angle: 90)
    }
    
    func drawTopHighlightEdgeLayerInFrame(cellFrame: NSRect) {
        
        NSGraphicsContext.saveGraphicsState()
        
        let path = controlPathInFrame(cellFrame)
        
        path.setClip()
        
        topHighlightColor.set()

        NSBezierPath.strokeLineFromPoint(NSPoint(x: cellFrame.minX, y: cellFrame.minY + 1.5), toPoint: NSPoint(x: cellFrame.maxX, y: cellFrame.minY + 1.5))

        NSGraphicsContext.restoreGraphicsState()
    }
    
    func drawBorderLayerInFrame(cellFrame: NSRect) {
        
        let path = controlPathInFrame(cellFrame.insetBy(dx: 0.5, dy: 0.5))
        
        borderColor.set()
        
        path.stroke()
    }
    
    // MARK: - Segments
    
    func rectForSegment(segment: Int) -> NSRect {
        
        let bounds = controlView!.bounds

        let xOffset: CGFloat
        if segment > 0 {
            let segmentWidths = (0..<segment).map { return widthForSegment($0) }
            xOffset = segmentWidths.reduce(bounds.minX, combine: +)
        } else {
            xOffset = bounds.minX
        }

        return NSRect(x: xOffset,
                      y: bounds.minY,
                  width: widthForSegment(segment),
                 height: bounds.height)
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
            
            NSGraphicsContext.saveGraphicsState()
            
            controlPathInFrame(frame).setClip()
            
            topActiveGradient.drawInBezierPath(segmentPath, angle: 90)
            horizontalActiveGradient.drawInBezierPath(segmentPath, angle: 0)
            horizontalActiveGradient.drawInBezierPath(segmentPath, angle: 180)
            bottomActiveGradient.drawInBezierPath(segmentPath, angle: 270)
            
            NSGraphicsContext.restoreGraphicsState()
        }
        
        // Draw divider.
        if segment < segmentCount - 1 {
            
            let segmentRect = NSRect(x: frame.maxX - 1, y: frame.minY, width: 1, height: frame.height - 1) // Leave room for bottom bezel.
            
            NSColor.blackColor().set()
            
            NSRectFill(controlView.centerScanRect(segmentRect))
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
        if !windowIsKeyWindow || !enabled {
            textColor = disabledTextColor
        } else if highlighted {
            textColor = highlightedTextColor
        } else if selected {
            textColor = selectedTextColor
        } else {
            textColor = normalTextColor
        }
        
        let textShadow = NSShadow()
        
        if highlighted {
            
            textShadow.shadowBlurRadius = 5
            textShadow.shadowOffset = NSSize(width: 0, height: 0)
            textShadow.shadowColor = NSColor(deviceWhite: 1, alpha: 0.6)
            
        } else if selected {
            
            textShadow.shadowBlurRadius = 1.5
            textShadow.shadowOffset = NSSize(width: 0, height: -1.5)
            textShadow.shadowColor = NSColor(deviceWhite: 0, alpha: 0.7)
            
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
        let startingPoint = NSPoint(x: rect.minX + topLeftCornerRadius,
                                    y: rect.minY)
        path.moveToPoint(startingPoint)
        
        // Top edge.
        path.lineToPoint(NSPoint(x: rect.maxX - topRightCornerRadius,
                                 y: rect.minY))
        
        // Top-right corner.
        let topRightCorner = NSPoint(x: rect.maxX,
                                     y: rect.minY)
        path.curveToPoint(NSPoint(x: rect.maxX,
                                  y: rect.minY + topRightCornerRadius),
            controlPoint1: topRightCorner,
            controlPoint2: topRightCorner)
        
        // Right edge.
        path.lineToPoint(NSPoint(x: rect.maxX,
                                 y: rect.maxY - bottomRightCornerRadius))
        
        // Bottom-right corner.
        let bottomRightCorner = NSPoint(x: rect.maxX,
                                        y: rect.maxY)
        path.curveToPoint(NSPoint(x: rect.maxX - bottomRightCornerRadius,
                                  y: rect.maxY),
            controlPoint1: bottomRightCorner,
            controlPoint2: bottomRightCorner)
        
        // Bottom edge.
        path.lineToPoint(NSPoint(x: rect.minX + bottomLeftCornerRadius,
                                 y: rect.maxY))
        
        // Bottom-left corner.
        let bottomLeftCorner = NSPoint(x: rect.minX,
                                       y: rect.maxY)
        path.curveToPoint(NSPoint(x: rect.minX,
                                  y: rect.maxY - bottomLeftCornerRadius),
            controlPoint1: bottomLeftCorner,
            controlPoint2: bottomLeftCorner)
        
        // Left edge.
        path.lineToPoint(NSPoint(x: rect.minX,
                                 y: rect.minY + topLeftCornerRadius))
        
        // Top-left corner.
        let topLeftCorner = NSPoint(x: rect.minX,
                                    y: rect.minY)
        path.curveToPoint(startingPoint,
            controlPoint1: topLeftCorner,
            controlPoint2: topLeftCorner)
        
        path.closePath()
        
        return path
    }
}

extension NSWindow {
    
    func convertPointFromScreen(point: NSPoint) -> NSPoint {
     
        let screenRect = NSRect(origin: point, size: NSSize.zero)
        
        let convertedRect = convertRectFromScreen(screenRect)
        
        return convertedRect.origin
    }
}
