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
    
    let backgroundGradient = NSGradient(starting: NSColor(deviceWhite: 0.22, alpha: 1),
                                        ending: NSColor(deviceWhite: 0.14, alpha: 1))!
    
    let topHighlightColor = NSColor(deviceWhite: 0.31, alpha: 1)
    
    let borderColor = NSColor(deviceWhite: 0.08, alpha: 1)
    
    let topActiveGradient = NSGradient(colorsAndLocations:
        (NSColor(deviceWhite: 0, alpha: 0.75), 0),
        (NSColor(deviceWhite: 0, alpha: 0.5),  0.15),
        (NSColor(deviceWhite: 0, alpha: 0),    0.6))!
    
    let horizontalActiveGradient = NSGradient(colorsAndLocations:
        (NSColor(deviceWhite: 0, alpha: 0.8),  0),
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
    
    override init() {
        super.init(textCell: "")
    }
    
    required init(coder aDecoder: NSCoder) {
        super.init(coder: aDecoder)
    }
    
    override var cellSize: NSSize {
        return cellSize(forBounds: NSMakeRect(0, 0, 0, 24))
    }
    
    override func cellSize(forBounds aRect: NSRect) -> NSSize {
        var res: NSSize = NSSize(width: 0, height: 24)
        for i in 0..<segmentCount {
            res.width += width(forSegment: i)
        }
        if let cv = controlView {
            return cv.backingAlignedRect(
                NSRect(origin: NSZeroPoint, size: res),
                options: [.alignMinXNearest, .alignMaxXNearest,
                          .alignMinYNearest, .alignMaxYNearest]
                ).size
        } else {
            return res
        }
    }
    
    @objc(sizeSegmentsToFitWithMinimumWidth:)
    func sizeSegmentsToFit(minimumWidth minw: CGFloat) {
        for i in 0..<segmentCount {
            let fitw = bestWidth(forSegment: i)
            setWidth(max(fitw, minw), forSegment:i)
        }
    }
    
    func bestWidth(forSegment i: Int) -> CGFloat {
        guard let label = label(forSegment: i) else {
            return 40.0;
        }
        let attributes: [NSAttributedStringKey: Any] = [
            .font: NSFont.systemFont(ofSize: 11, weight: NSFont.Weight(rawValue: 0.1))]
        let attributedString = NSAttributedString(string: label, attributes: attributes)
        return attributedString.size().width + 20.0
    }

    override func draw(withFrame cellFrame: NSRect, in controlView: NSView) {
        
        let realWidth = NSRect(origin: cellFrame.origin, size: cellSize(forBounds: cellFrame))
        drawBackgroundLayer(inFrame: cellFrame)
        drawTopHighlightEdgeLayer(inFrame: cellFrame)
        
        for segment in 0..<segmentCount {
            drawSegment(segment, inFrame: rectForSegment(segment), with: controlView)
        }
        
        drawBorderLayer(inFrame: realWidth)
    }
    
    // MARK: - Layers
    
    func controlPath(inFrame cellFrame: NSRect) -> NSBezierPath {
        return NSBezierPath(roundedRect: cellFrame, xRadius: cornerRadius, yRadius: cornerRadius)
    }
    
    func drawBackgroundLayer(inFrame cellFrame: NSRect) {
        
        let path = controlPath(inFrame: cellFrame)
        
        backgroundGradient.draw(in: path, angle: 90)
    }
    
    func drawTopHighlightEdgeLayer(inFrame frame: NSRect) {
        
        NSGraphicsContext.saveGraphicsState()
        
        let path = controlPath(inFrame: frame)
        
        path.setClip()
        
        topHighlightColor.set()
        
        NSBezierPath.strokeLine(from: NSPoint(x: frame.minX, y: frame.minY + 1.5),
                                to: NSPoint(x: frame.maxX, y: frame.minY + 1.5))
        
        NSGraphicsContext.restoreGraphicsState()
    }
    
    func drawBorderLayer(inFrame cellFrame: NSRect) {
        
        let path = controlPath(inFrame: cellFrame.insetBy(dx: 0.5, dy: 0.5))
        
        borderColor.set()
        
        path.stroke()
    }
    
    // MARK: - Segments
    
    func rectForSegment(_ segment: Int) -> NSRect {
        let cv = controlView!
        let bounds = cv.bounds
        
        let xOffset: CGFloat
        if segment > 0 {
            let segmentWidths = (0..<segment).map { return width(forSegment: $0) }
            xOffset = segmentWidths.reduce(bounds.minX, +)
        } else {
            xOffset = bounds.minX
        }
        
        return cv.backingAlignedRect(
            NSRect(x: xOffset,
                   y: bounds.minY,
                   width: width(forSegment: segment),
                   height: bounds.height),
            options: [.alignMinXNearest, .alignMaxXNearest,
                      .alignMinYNearest, .alignMaxYNearest])
    }
    
    func pathForSegment(_ segment: Int, inFrame frame: NSRect) -> NSBezierPath {
        
        let firstSegment = 0
        let lastSegment = segmentCount - 1
        
        if segment == firstSegment {
            return NSBezierPath.roundedRect(in: frame,
                                            topLeftCornerRadius: cornerRadius,
                                            topRightCornerRadius: 0,
                                            bottomLeftCornerRadius: cornerRadius,
                                            bottomRightCornerRadius: 0)
        } else if segment == lastSegment {
            return NSBezierPath.roundedRect(in: frame,
                                            topLeftCornerRadius: 0,
                                            topRightCornerRadius: cornerRadius,
                                            bottomLeftCornerRadius: 0,
                                            bottomRightCornerRadius: cornerRadius)
        } else {
            return NSBezierPath(rect: frame)
        }
    }
    
    func mouseInSegment(_ segment: Int) -> Bool {
        
        guard let controlView = controlView, let window = controlView.window else {
            return false
        }
        
        let segmentRect = rectForSegment(segment)
        let pointInWindow = window.convertPointFromScreen(NSEvent.mouseLocation)
        let pointInControlView = controlView.convert(pointInWindow, from: nil)
        
        return NSPointInRect(pointInControlView, segmentRect)
    }
    
    override func drawSegment(_ segment: Int, inFrame frame: NSRect, with controlView: NSView) {
        
        let highlighted = segment == highlightedSegment && mouseInSegment(segment)
        let selected = segment == selectedSegment
        
        if highlighted || selected {
            
            // Draw active/highlighted state.
            
            // Make room for the bottom highlight.
            var segmentRect = frame
            segmentRect.size.height -= 1
            // Make room for the left and right borders.
            segmentRect.size.width -= 1
            if (segment == 0) {
                // Only the first segment has a right border (actually, it's
                // the cell's border).
                segmentRect.origin.x += 1
                segmentRect.size.width -= 1
            }
            
            let segmentPath = pathForSegment(segment, inFrame: segmentRect)
            
            topActiveGradient.draw(in: segmentPath, angle: 90)
            horizontalActiveGradient.draw(in: segmentPath, angle: 0)
            horizontalActiveGradient.draw(in: segmentPath, angle: 180)
            bottomActiveGradient.draw(in: segmentPath, angle: 270)
        }
        
        // Draw divider.
        if segment < segmentCount - 1 {
            
            let segmentRect = NSRect(x: frame.maxX - 1, y: frame.minY, width: 1, height: frame.height - 1) // Leave room for bottom bezel.
            
            NSColor.black.set()
            
            controlView.centerScanRect(segmentRect).fill()
        }
        
        drawTextForSegment(segment, inFrame: frame)
    }
    
    func drawTextForSegment(_ segment: Int, inFrame frame: NSRect) {
        
        guard let label = label(forSegment: segment) else {
            return
        }
        
        let windowIsKeyWindow = controlView!.window!.isKeyWindow
        let highlighted = segment == highlightedSegment && mouseInSegment(segment)
        let selected = segment == selectedSegment
        
        let textColor: NSColor
        if !windowIsKeyWindow || !isEnabled {
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
        
        let attributes: [NSAttributedStringKey: Any] = [
            .font: NSFont.systemFont(ofSize: 11, weight: NSFont.Weight(rawValue: 0.1)),
            .foregroundColor: textColor,
            .shadow: textShadow]
        
        let attributedString = NSAttributedString(string: label, attributes: attributes)
        
        var labelRect = frame
        labelRect.origin.y = (NSMidY(frame) - attributedString.size().height / 2)
        labelRect.origin.x = NSMidX(frame) - attributedString.size().width / 2
        labelRect.size.height = attributedString.size().height
        labelRect.size.width = attributedString.size().width
        
        attributedString.draw(in: labelRect)
    }
    
    // MARK: - Mouse Tracking
    
    override func startTracking(at startPoint: NSPoint, in controlView: NSView) -> Bool {
        
        for segment in 0..<segmentCount {
            if NSPointInRect(startPoint, rectForSegment(segment)) {
                highlightedSegment = segment
                break
            }
        }
        
        isHighlighted = true
        return highlightedSegment != nil
    }
    
    override func continueTracking(last lastPoint: NSPoint, current currentPoint: NSPoint, in controlView: NSView) -> Bool {
        
        controlView.needsDisplay = true
        
        return true
    }
    
    override func stopTracking(last lastPoint: NSPoint, current stopPoint: NSPoint, in controlView: NSView, mouseIsUp flag: Bool) {
        
        guard let segment = highlightedSegment else {
            return
        }
        if flag && NSPointInRect(stopPoint, rectForSegment(segment)) {
            selectedSegment = segment
            NSApp.sendAction(action!, to: target, from: self)
        }
        highlightedSegment = nil
        isHighlighted = false
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
    static func roundedRect(in rect: NSRect, topLeftCornerRadius: CGFloat, topRightCornerRadius: CGFloat, bottomLeftCornerRadius: CGFloat, bottomRightCornerRadius: CGFloat) -> NSBezierPath {
        
        let path = NSBezierPath()
        
        // Start near top-right corner.
        let startingPoint = NSPoint(x: rect.maxX - topRightCornerRadius,
                                    y: rect.minY)
        path.move(to: startingPoint)
        
        // Top-right corner.
        if topRightCornerRadius > 0.0 {
            let topRightArcCenter = NSPoint(x: rect.maxX - topRightCornerRadius,
                                            y: rect.minY + topRightCornerRadius)
            path.appendArc(withCenter: topRightArcCenter,
                           radius: topRightCornerRadius, startAngle: -90, endAngle: 0)
        }
        
        // Right edge.
        path.line(to: NSPoint(x: rect.maxX,
                              y: rect.maxY - bottomRightCornerRadius))
        
        // Bottom-right corner.
        if bottomRightCornerRadius > 0.0 {
            let bottomRightArcCenter = NSPoint(x: rect.maxX - bottomRightCornerRadius,
                                               y: rect.maxY - bottomRightCornerRadius)
            path.appendArc(withCenter: bottomRightArcCenter,
                           radius: bottomRightCornerRadius, startAngle: 0, endAngle: 90)
        }
        
        // Bottom edge.
        path.line(to: NSPoint(x: rect.minX + bottomLeftCornerRadius,
                              y: rect.maxY))
        
        // Bottom-left corner.
        if bottomLeftCornerRadius > 0.0 {
            let bottomLeftArcCenter = NSPoint(x: rect.minX + bottomLeftCornerRadius,
                                              y: rect.maxY - bottomLeftCornerRadius)
            path.appendArc(withCenter: bottomLeftArcCenter,
                           radius: bottomLeftCornerRadius, startAngle: 90, endAngle: 180)
        }
        
        // Left edge.
        path.line(to: NSPoint(x: rect.minX,
                              y: rect.minY + topLeftCornerRadius))
        
        // Top-left corner.
        if topLeftCornerRadius > 0.0 {
            let topLeftArcCenter = NSPoint(x: rect.minX + topLeftCornerRadius,
                                           y: rect.minY + topLeftCornerRadius)
            path.appendArc(withCenter: topLeftArcCenter,
                           radius: topLeftCornerRadius, startAngle: 180, endAngle: 270)
        }
        
        // Top edge and close path
        path.close()
        
        return path
    }
}

extension NSWindow {
    
    func convertPointFromScreen(_ point: NSPoint) -> NSPoint {
        
        let screenRect = NSRect(origin: point, size: NSSize.zero)
        
        let convertedRect = convertFromScreen(screenRect)
        
        return convertedRect.origin
    }
}
