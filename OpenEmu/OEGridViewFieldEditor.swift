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

@objc
@objcMembers
final class OEGridViewFieldEditor: NSView {
    
    private var textField = NSTextField()
    
    override init(frame frameRect: NSRect) {
        super.init(frame: frameRect)
        
        textField.usesSingleLineMode = true
        textField.cell?.isScrollable = true
        textField.isBezeled = false
        textField.allowsEditingTextAttributes = false
        textField.backgroundColor = .textBackgroundColor
        textField.textColor = .textColor
        textField.alignment = .center
        addSubview(textField)
        
        autoresizesSubviews = false
        isHidden = true
    }
    
    required init?(coder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }
    
    override func setFrameSize(_ newSize: NSSize) {
        super.setFrameSize(newSize)
        
        var frameSize = newSize
        if frameSize.width >= 2 {
            frameSize.width -= 2
        }
        if frameSize.height >= 2 {
            frameSize.height -= 2
        }
        
        textField.setFrameSize(newSize)
        textField.setFrameOrigin(NSPoint(x: 1, y: 1))
    }
    
    var string: String {
        get {
            textField.stringValue
        }
        set {
            textField.stringValue = newValue
        }
    }
    
    var font: NSFont? {
        get {
            textField.font
        }
        set {
            textField.font = newValue
        }
    }
    
    var delegate: NSTextFieldDelegate? {
        get {
            textField.delegate
        }
        set {
            textField.delegate = newValue
        }
    }
}
