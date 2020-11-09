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

@objc(OEControlsLabel)
class ControlsLabel: NSTextField {
    
    required init?(coder: NSCoder) {
        super.init(coder: coder)
        commonInit()
    }
    
    override init(frame frameRect: NSRect) {
        super.init(frame: frameRect)
        commonInit()
    }
    
    let isWood = UserDefaults.standard.integer(forKey: OEControlsPrefsAppearancePreferenceKey) == OEControlsPrefsAppearancePreferenceValue.wood.rawValue
    
    private func commonInit() {
        
        isBezeled = false
        isEditable = false
        isSelectable = false
        drawsBackground = false
        
        if isWood {
            textColor = .black
            font = .boldSystemFont(ofSize: 11)
        } else {
            textColor = .labelColor
            font = .systemFont(ofSize: 11)
        }
        
        setupAttributes()
    }
    
    func setupAttributes() {
    }
    
    override func draw(_ dirtyRect: NSRect) {
        
        guard isWood else { return super.draw(dirtyRect) }
        
        let attributedString = attributedStringValue.mutableCopy() as! NSMutableAttributedString
        attributedString.addAttributes([.shadow : NSShadow.oeControls], range: NSRange(location: 0, length: attributedString.length))
        attributedStringValue = attributedString
        
        return super.draw(dirtyRect)
    }
}


@objc(OEControlsKeyHeadline)
final class ControlsKeyHeadline: ControlsLabel {
    
    override func setupAttributes() {
        if isWood {
            font = .boldSystemFont(ofSize: 11.5)
        } else {
            font = .systemFont(ofSize: 11.5)
        }
    }
}

@objc(OEControlsKeyLabel)
final class ControlsKeyLabel: ControlsLabel {
    
    override func setupAttributes() {
        alignment = .right
    }
}
