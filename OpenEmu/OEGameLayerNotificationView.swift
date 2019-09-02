// Copyright (c) 2019, OpenEmu Team
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

@IBDesignable
@objc
class OEGameLayerNotificationView: NSImageView {
    
    public var disableNotifications: Bool = false
    
    lazy var quicksaveImage = NSImage(named: "hud_quicksave_notification")
    
    override var wantsUpdateLayer: Bool { return true }
    
    // MARK: - Initialization
    
    override public init(frame frameRect: NSRect) {
        super.init(frame: frameRect)
        self.setup()
    }
    
    required public init?(coder: NSCoder) {
        super.init(coder: coder)
        self.setup()
    }
    
    private func setup() {
        self.wantsLayer = true
        self.layerContentsRedrawPolicy = .onSetNeedsDisplay
    }
    
    // MARK: - Notifications
    
    @objc public func showQuickSave() {
        CATransaction.begin()
        CATransaction.disableActions()
        self.image = quicksaveImage
        self.layer?.opacity = 0.0
        CATransaction.commit()
        
        CATransaction.begin()
        CATransaction.setCompletionBlock {
            NSLog("FINISHED")
        }
        self.layer?.add(self.makeOpacityAnimation(), forKey: "opacityAnim")
        CATransaction.commit()
    }
    
    @objc public func showScreenShot() {
        
    }
    
    @objc public func showFastForward(enabled: Bool) {
        
    }
    
    @objc public func showRewind(enabled: Bool) {
        
    }
    
    @objc public func showStepForward() {
        
    }
    
    @objc public func showStepBackward() {
        
    }
    
    func makeOpacityAnimation() -> CAAnimation {
        let opacityAnimation = CAKeyframeAnimation()
        opacityAnimation.beginTime = self.layer!.convertTime(CACurrentMediaTime(), from: nil) + 0.005
        opacityAnimation.duration = 1.75
        opacityAnimation.fillMode = .forwards
        opacityAnimation.isRemovedOnCompletion = false
        opacityAnimation.timingFunction = CAMediaTimingFunction(name: .linear)
        opacityAnimation.keyPath = "opacity"
        opacityAnimation.values = [ 0, 1, 1, 0 ]
        opacityAnimation.keyTimes = [ 0, 0.15, 0.85, 1 ]
        opacityAnimation.calculationMode = .linear
        return opacityAnimation
    }
    
}
