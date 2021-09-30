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

@objc enum OEGridViewCellIndicationType: Int {
    case none, fileMissing, processing, dropOn
}

@objc(OEGridViewCellIndicationLayer)
@objcMembers
final class GameCollectionViewItemIndicationLayer: CALayer {
    
    enum IndicationType {
        case none, fileMissing, processing, dropOn
    }
    
    private static let dropOnBackgroundColorRef = CGColor(red: 0.4, green: 0.361, blue: 0.871, alpha: 0.7)
    private static let indicationShadowColorRef = CGColor(red: 0.341, green: 0.0, blue: 0.012, alpha: 0.6)
    private static let missingFileBackgroundColorRef = CGColor(red: 0.992, green: 0.0, blue: 0.0, alpha: 0.4)
    private static let processingItemBackgroundColorRef = CGColor(red: 0.0, green: 0.0, blue: 0.0, alpha: 0.7)
    
    private static let rotationAnimation: CAKeyframeAnimation = {
        let stepCount = 12
        var spinnerValues = [Double]()
        spinnerValues.reserveCapacity(stepCount)
        
        for step in 0..<stepCount {
            spinnerValues.append(-1 * (.pi * 2) * Double(step) / Double(stepCount))
        }
        
        var animation = CAKeyframeAnimation(keyPath: "transform.rotation.z")
        animation.calculationMode = .discrete
        animation.duration = 1
        animation.repeatCount = .greatestFiniteMagnitude
        animation.isRemovedOnCompletion = false
        animation.values = spinnerValues
        
        return animation
    }()
    
    var _type: OEGridViewCellIndicationType = .none
    var type: OEGridViewCellIndicationType {
        get {
            return _type
        }
        set {
            guard _type != newValue else { return }
            _type = newValue
            
            if _type == .none {
                backgroundColor = nil
                sublayers?.forEach { $0.removeFromSuperlayer() }
            }
            else if type == .dropOn {
                sublayers?.forEach { $0.removeFromSuperlayer() }
                backgroundColor = Self.dropOnBackgroundColorRef
            }
            else {
                var sublayer: CALayer! = sublayers?.last
                if sublayer == nil {
                    sublayer = CALayer()
                    sublayer.actions = ["position" : NSNull()]
                    sublayer.shadowOffset = CGSize(width: 0, height: -1)
                    sublayer.shadowOpacity = 1
                    sublayer.shadowRadius = 1
                    sublayer.shadowColor = Self.indicationShadowColorRef
                    
                    addSublayer(sublayer)
                } else {
                    sublayer.removeAllAnimations()
                }
                
                if type == .fileMissing {
                    backgroundColor = Self.missingFileBackgroundColorRef
                    sublayer.contents = NSImage(named: "missing_rom")
                }
                else if type == .processing {
                    backgroundColor = Self.processingItemBackgroundColorRef
                    sublayer.contents = NSImage(named: "spinner")
                    sublayer.anchorPoint = CGPoint(x: 0.5, y: 0.5)
                    sublayer.anchorPointZ = 0
                    sublayer.add(Self.rotationAnimation, forKey: nil)
                }
                
                setNeedsLayout()
            }
        }
    }
    
    override func layoutSublayers() {
        guard let sublayer = sublayers?.last else { return }
        
        CATransaction.begin()
        defer { CATransaction.commit() }
        CATransaction.setDisableActions(true)
        
        if type == .fileMissing {
            let width = bounds.width * 0.45
            let height = width * 0.9
            
            sublayer.frame = CGRect(x: bounds.minX + (bounds.width - width) / 2,
                                    y: bounds.minY + (bounds.height - height) / 2,
                                    width: width,
                                    height: height).integral
        }
        else if type == .processing {
            let spinnerImage = NSImage(named: "spinner")!
            
            let spinnerImageSize = spinnerImage.size
            var frame = CGRect(x: (bounds.width - spinnerImageSize.width) / 2,
                               y: (bounds.height - spinnerImageSize.height) / 2,
                               width: spinnerImageSize.width,
                               height: spinnerImageSize.height).integral
            frame.size.height = frame.size.width
            sublayer.frame = frame
        }
        else {
            sublayer.frame = bounds
        }
    }
}
