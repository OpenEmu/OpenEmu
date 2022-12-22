// Copyright (c) 2022, OpenEmu Team
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
import OpenEmuKit

protocol NameShaderPresetDelegate {
    func setPresetName(_ name: String)
    func cancelSetPresetName()
}

class NameShaderPreset: NSViewController, NSControlTextEditingDelegate {
    
    @IBOutlet weak var textField: NSTextField!
    @IBOutlet weak var warning: NSTextField!
    @IBOutlet weak var warningView: NSView!
    
    @objc dynamic var isTextValid: Bool = true
    @objc dynamic var validationMessage: String = ""
    
    var existingName: String? // set if a preset is being renamed
    
    override func viewWillAppear() {
        super.viewWillAppear()
        
        if #available(macOS 11.0, *),
           let window = view.window {
            window.titleVisibility = .hidden
            window.titlebarAppearsTransparent = true
            window.styleMask.insert(.fullSizeContentView)
            window.styleMask.remove([.closable, .resizable])
            window.isMovableByWindowBackground = true
        } else {
            view.window?.styleMask.remove(.resizable)
            (view as? NSVisualEffectView)?.state = .inactive
        }
        
        if let name = existingName {
            textField.stringValue = name
        }
        validateView()
    }
    
    @IBAction func save(_ sender: Any?) {
        if isTextValid && textField.stringValue.count > 0 {
            if let delegate = presentingViewController as? NameShaderPresetDelegate {
                delegate.setPresetName(textField.stringValue)
            }
            presentingViewController?.dismiss(self)
        }
    }
    
    @IBAction override func cancelOperation(_ sender: Any?) {
        presentingViewController?.dismiss(self)
        if let delegate = presentingViewController as? NameShaderPresetDelegate {
            delegate.cancelSetPresetName()
        }
    }
    
    @objc func controlTextDidChange(_ obj: Notification) {
        validateView()
    }
    
    func validateView() {
        let invalidCharacters: [Character] = ["\"", "#"]
        if textField.stringValue.isEmpty {
            isTextValid = false
            validationMessage = NSLocalizedString("A name is required.", comment: "warning: Displayed when user enters a zero-length shader preset name")
        } else if let invalid = textField.stringValue.first(where: { invalidCharacters.contains($0) }) {
            isTextValid = false
            validationMessage = .localizedStringWithFormat(NSLocalizedString("Name must not contain “%@.”", comment: "warning: Displayed when user enters a shader preset name containing an invalid character"), String(invalid))
        } else {
            isTextValid = true
            validationMessage = ""
        }
    }
}
