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
import QuartzCore

final class BlankSlateSpinnerView: NSView, CALayerDelegate, NSViewLayerContentScaleDelegate {
    
    private var spinnerLayer: CALayer?
    
    required init?(coder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }
    
    override init(frame: NSRect) {
        super.init(frame: frame)
        wantsLayer = true
    }
    
    override func makeBackingLayer() -> CALayer {
        
        let rootLayer = CALayer()
        rootLayer.delegate = self
        
        let spinnerLayer = CALayer()
        spinnerLayer.contentsGravity = .resizeAspect
        spinnerLayer.frame = rootLayer.bounds
        spinnerLayer.add(spinnerAnimation, forKey: nil)
        spinnerLayer.delegate = self
        self.spinnerLayer = spinnerLayer
        
        rootLayer.addSublayer(spinnerLayer)
        
        return rootLayer
    }
    
    // MARK: - Animation
    
    private var spinnerAnimation: CAAnimation {
        
        let stepCount = 12
        var spinnerValues = [Double]()
        spinnerValues.reserveCapacity(stepCount)
        
        for step in 0..<stepCount {
            spinnerValues.append(-1 * (.pi * 2) * Double(step) / Double(stepCount))
        }
        
        let animation = CAKeyframeAnimation(keyPath: "transform.rotation.z")
        animation.calculationMode = .discrete
        animation.duration = 1.0
        animation.repeatCount = .infinity
        animation.isRemovedOnCompletion = false
        animation.values = spinnerValues
        
        return animation
    }
    
    // MARK: - Layer Delegate
    
    func layoutSublayers(of layer: CALayer) {
        if layer == self.layer, spinnerLayer != nil {
            spinnerLayer!.frame = layer.bounds
            spinnerLayer!.frame.size.height = 124
            spinnerLayer!.frame.origin.y -= (spinnerLayer!.frame.height - layer.frame.height) / 2
            spinnerLayer!.contents = NSImage(named: "blank_slate_spinner")?.withTintColor(NSColor(named: "blank_slate_box_text")!)
        }
    }
    
    func action(for layer: CALayer, forKey event: String) -> CAAction? {
        return nil
    }
}
