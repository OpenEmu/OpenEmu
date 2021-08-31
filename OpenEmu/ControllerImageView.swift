// Copyright (c) 2021, OpenEmu Team
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
import QuartzCore

final class ControllerImageView: NSView {
    
    static let drawControllerMaskKey = "drawControllerMask"
    
    private let overlayAlphaOn: CGFloat = 0.5
    private let overlayAlphaOff: CGFloat = 0
    private let ringRadius: CGFloat = 37
    
    @objc dynamic private var overlayAlpha: CGFloat = 0 {
        didSet {
            needsDisplay = true
        }
    }
    
    @objc dynamic private var ringAlpha: CGFloat = 0 {
        didSet {
            needsDisplay = true
        }
    }
    
    @objc dynamic private var ringPosition = NSPoint.zero {
        didSet {
            needsDisplay = true
        }
    }
    
    weak var target: AnyObject?
    var action: Selector?
    
    var image: NSImage? {
        didSet {
            ringPosition = .zero
            ringAlpha = 0
            needsDisplay = true
        }
    }
    var imageMask: NSImage?
    
    /// associates keys with NSPoint encapsulated in NSValue
    var keyPositions: [String : NSValue]?
    
    private var _selectedKey: String?
    var selectedKey: String? {
        get {
            _selectedKey
        }
        set {
            setSelectedKey(newValue, animated: true)
        }
    }
    
    func setSelectedKey(_ key: String?, animated: Bool) {
        var key = key
        if keyPositions?[key ?? ""] == nil {
            key = nil
        }
        
        if _selectedKey != key {
            _selectedKey = key
            
            setHighlightPoint(highlightPoint(for: _selectedKey), animated: animated)
        }
    }
    
    required init?(coder: NSCoder) {
        super.init(coder: coder)
        commonInit()
    }
    
    override init(frame frameRect: NSRect) {
        super.init(frame: frameRect)
        commonInit()
    }
    
    private func commonInit() {
        wantsLayer = true
        
        let overlayAnimation = CABasicAnimation()
        let ringAnimation = CABasicAnimation()
        ringAnimation.timingFunction = CAMediaTimingFunction(name: .easeOut)
        ringAnimation.delegate = self
        
        animations = [
            "ringAlpha": ringAnimation,
            "ringPosition": ringAnimation,
            "overlayAlpha": overlayAnimation,
        ]
    }
    
    override func draw(_ dirtyRect: NSRect) {
        guard let image = image else { return }
        
        var targetRect = NSRect()
        targetRect.size = image.size
        targetRect.origin = NSPoint(x: (frame.size.width-image.size.width)/2, y: 0)
        
        image.draw(in: targetRect, from: .zero, operation: .copy, fraction: 1, respectFlipped: false, hints: [.interpolation: NSNumber(value: NSImageInterpolation.none.rawValue)])
        
        if UserDefaults.standard.bool(forKey: Self.drawControllerMaskKey) {
            imageMask?.draw(in: targetRect, from: .zero, operation: .sourceOver, fraction: 1, respectFlipped: false, hints: [.interpolation: NSNumber(value: NSImageInterpolation.none.rawValue)])
        }
        
        if overlayAlpha != overlayAlphaOff {
            NSGraphicsContext.saveGraphicsState()
            let rect = NSRect(x: targetRect.origin.x + ringPosition.x - 33, y: targetRect.origin.y + ringPosition.y - 33, width: 66, height: 66)
            
            let path = NSBezierPath(rect: bounds)
            path.windingRule = .evenOdd
            path.appendOval(in: rect)
            path.setClip()
            
            NSColor(deviceWhite: 0, alpha: overlayAlpha).setFill()
            bounds.fill(using: .sourceAtop)
            NSGraphicsContext.restoreGraphicsState()
        }
        
        if ringAlpha != 0 {
            let highlightPoint = NSPoint(x: targetRect.origin.x + ringPosition.x - 38, y: targetRect.origin.y + ringPosition.y - 45)
            let highlightImage = NSImage(named: "controls_highlight")
            highlightImage?.draw(at: highlightPoint, from: .zero, operation: .sourceOver, fraction: ringAlpha)
        }
    }
    
    private func setHighlightPoint(_ point: CGPoint, animated: Bool) {
        if animated && !Thread.isMainThread {
            performSelector(onMainThread: #selector(animatedSetHighlightPointOnMainThread(value:)), with: NSValue(point: point), waitUntilDone: false)
            return
        }
        
        let animator = animated ? animator() : self
        
        if animated {
            NSAnimationContext.beginGrouping()
        }
        
        if point == .zero {
            animator.ringAlpha = 0
            animator.overlayAlpha = overlayAlphaOff
        } else if ringPosition == .zero {
            ringPosition = point
            animator.ringAlpha = 1
            animator.overlayAlpha = overlayAlphaOn
        } else {
            animator.ringPosition = point
            animator.ringAlpha = 1
            animator.overlayAlpha = overlayAlphaOn
        }
        
        if animated {
            NSAnimationContext.endGrouping()
        }
    }
    
    @objc private func animatedSetHighlightPointOnMainThread(value: NSValue) {
        setHighlightPoint(value.pointValue, animated: true)
    }
    
    private func highlightPoint(for key: String?) -> NSPoint {
        let value = keyPositions?[key ?? ""]
        return value?.pointValue ?? .zero
    }
    
    private func keyForHighlightPointClosestToPoint(_ point: NSPoint) -> String? {
        var ret: String?
        var distance = CGFloat.greatestFiniteMagnitude
        
        for (key, value) in keyPositions ?? [:] {
            let compared = value.pointValue
            
            let current = point.distance(to: compared)
            
            if current < distance {
                distance = current
                ret = key
            }
        }
        
        return ret
    }
    
    // MARK: - Interaction
    
    override func mouseDown(with event: NSEvent) {
        var selected = selectedKey
        
        let eventLocation = event.locationInWindow
        let localEventLocation = convert(eventLocation, from: nil)
        let ringLocation = ringPositionInView
        
        var targetRect = NSRect()
        targetRect.size = image?.size ?? .zero
        targetRect.origin = NSPoint(x: (bounds.size.width-(image?.size.width ?? 0))/2, y: 0)
        
        let selectAButton = imageMask?.hitTest(NSRect(origin: localEventLocation, size: .zero), withDestinationRect: targetRect, context: nil, hints: nil, flipped: false)
        
        if selectAButton ?? false {
            let locationOnController = localEventLocation.subtracting(targetRect.origin)
            selected = keyForHighlightPointClosestToPoint(locationOnController)
        }
        
        if selected == selectedKey && ringLocation != .zero {
            let distance = localEventLocation.distance(to: ringLocation)
            if distance > ringRadius && bounds.contains(localEventLocation) {
                selected = nil
            }
        }
        
        if selected != selectedKey {
            selectedKey = selected
            if let action = action {
                NSApp.sendAction(action, to: target, from: self)
            }
        }
    }
    
    var ringPositionInView: NSPoint {
        let ringLocation = ringPosition
        if ringLocation == .zero {
            return .zero
        }
        
        let offset = NSPoint(x: (frame.size.width-(image?.size.width ?? 0))/2, y: 0)
        return offset.adding(ringLocation)
    }
}

// MARK: - CAAnimationDelegate

extension ControllerImageView: CAAnimationDelegate {
    func animationDidStop(_ anim: CAAnimation, finished flag: Bool) {
        if ringAlpha == 0 {
            ringPosition = .zero
        }
    }
}

// MARK: - CGPoint Additions

private extension CGPoint {
    
    func distance(to point2: CGPoint) -> CGFloat {
        sqrt((self.x - point2.x) * (self.x - point2.x) + (point2.y - self.y) * (point2.y - self.y))
    }
    
    func adding(_ point2: CGPoint) -> CGPoint {
        CGPoint(x: self.x + point2.x, y: self.y + point2.y)
    }
    
    func subtracting(_ point2: CGPoint) -> CGPoint {
        CGPoint(x: self.x - point2.x, y: self.y - point2.y)
    }
}
