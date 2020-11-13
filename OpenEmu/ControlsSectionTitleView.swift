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

@objc(OEControlsSectionTitleView)
final class ControlsSectionTitleView: NSView {
    
    private lazy var topColor = NSColor(deviceRed: 85/255, green: 45/255, blue: 0, alpha: 1)
    private lazy var bottomColor = NSColor(deviceRed: 1, green: 1, blue: 0, alpha: 0.2)
    private let leftGap: CGFloat = 16
    
    private let isWood = UserDefaults.standard.integer(forKey: OEControlsPrefsAppearancePreferenceKey) == OEControlsPrefsAppearancePreferenceValue.wood.rawValue
    private let isWoodVibrant = UserDefaults.standard.integer(forKey: OEControlsPrefsAppearancePreferenceKey) == OEControlsPrefsAppearancePreferenceValue.woodVibrant.rawValue
    
    private lazy var visualEffectView: NSVisualEffectView = {
        let veView = NSVisualEffectView()
        
        if isWoodVibrant {
            veView.blendingMode = .withinWindow
            veView.state = .active
        }
        
        return veView
    }()
    
    private lazy var imageView: NSImageView = {
        let imageView = NSImageView()
        
        if isWoodVibrant {
            imageView.image = NSImage(named: "controls_background")
        } else {
            imageView.image = NSImage(named: "controls_bg")
        }
        imageView.imageScaling = .scaleAxesIndependently
        return imageView
    }()
    
    private lazy var textField: NSTextField = {
        let textField = NSTextField(labelWithString: "")
        textField.font = .systemFont(ofSize: 12, weight: .medium)
        textField.textColor = .labelColor
        return textField
    }()
    
    private lazy var string = NSAttributedString(string: stringValue, attributes: ControlsSectionTitleView.attributes)
    
    @objc var stringValue = ""
    
    @objc var pinned = false {
        didSet {
            if isWood {
                needsDisplay = true
            }
        }
    }
    
    required init?(coder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }
    
    override init(frame frameRect: NSRect) {
        super.init(frame: frameRect)
        
        if !isWood {
            if isWoodVibrant {
                addSubview(imageView)
                addSubview(visualEffectView)
            } else {
                addSubview(visualEffectView)
                addSubview(imageView)
            }
            addSubview(textField)
        }
    }
    
    override func draw(_ dirtyRect: NSRect) {
        
        guard isWood else {
            visualEffectView.frame = bounds
            imageView.frame = bounds
            textField.stringValue = stringValue
            textField.frame = titleRect
            return
        }
        
        NSColor.clear.setFill()
        bounds.fill()
        
        // hacky solution to get section headers to clip underlying views:
        // we let the wood background view draw to an image and then draw the portion we need here
        NSGraphicsContext.current?.saveGraphicsState()
        let woodBackground = superview?.superview?.superview?.superview?.superview
        
        let woodBgRect = bounds.insetBy(dx: 5, dy: 0)
        let portion = convert(woodBgRect, to: woodBackground)
        woodBgRect.clip()
        
        let image = NSImage(size: woodBackground?.bounds.size ?? .zero)
        image.lockFocus()
        woodBackground?.draw(portion)
        image.unlockFocus()
        
        image.draw(in: woodBgRect, from: portion, operation: .copy, fraction: 1)
        NSGraphicsContext.current?.restoreGraphicsState()
        
        // draw spearator style lines at the top and the bottom
        var lineRect = bounds
        lineRect.size.height = 1
        
        // draw bottom line
        if frame.origin.y != 0 {
            topColor.setFill()
            lineRect.origin.y = 0
            lineRect.fill()
            
            bottomColor.setFill()
            lineRect.origin.y = 1
            lineRect.fill(using: .sourceOver)
        }
        
        // draw title
        string.draw(in: titleRect)
        
        // draw top line if the view is not pinned
        if !pinned {
            topColor.setFill()
            lineRect.origin.y = bounds.size.height-1
            lineRect.fill()
            
            bottomColor.setFill()
            lineRect.origin.y = bounds.size.height-2
            lineRect.fill(using: .sourceOver)
         }
    }
    
    private var titleRect: NSRect {
        
        guard isWood else {
            var rect = bounds
            
            rect.origin.y -= (rect.size.height-textField.attributedStringValue.size().height)/2
            rect.origin.x += leftGap
            
            return backingAlignedRect(rect, options: .alignAllEdgesNearest)
        }
        
        var rect = bounds
        
        rect.origin.y -= (rect.size.height-string.size().height)/2
        rect.origin.x += leftGap
        
        return backingAlignedRect(rect, options: .alignAllEdgesNearest)
    }
    
    private static let attributes: [NSAttributedString.Key : Any]? = {
        
        let attributes: [NSAttributedString.Key : Any] =
                                          [.font: NSFont.boldSystemFont(ofSize: 11),
                                .foregroundColor: NSColor.black,
                                         .shadow: NSShadow.oeControls]
        
        return attributes
    }()
}
