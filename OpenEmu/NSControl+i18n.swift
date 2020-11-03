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

extension NSButton {
    @IBInspectable var localizeTitle: Bool {
        get { return false }
        set {
            if newValue == true {
                title = Bundle.main.preferredLocalizedString(forKey: title, value: "No translation", table: "OEControls")
            }
        }
   }
}

extension NSTextField {
    @IBInspectable var localizeTitle: Bool {
        get { return false }
        set {
            if newValue == true {
                stringValue = Bundle.main.preferredLocalizedString(forKey: stringValue, value: "No translation", table: "OEControls")
            }
        }
    }
}

extension NSMenu {
    @IBInspectable var localizeTitle: Bool {
        get { return false }
        set {
            if newValue == true {
                let localizedTitle = Bundle.main.preferredLocalizedString(forKey: title, value: "No translation", table: "MainMenu")
                if localizedTitle == "No translation" {
                    title = Bundle.main.preferredLocalizedString(forKey: title, value: "No translation", table: nil)
                }
                else {
                    title = localizedTitle
                }
            }
        }
    }
}

extension NSMenuItem {
    @IBInspectable var localizeTitle: Bool {
        get { return false }
        set {
            if newValue == true {
                let localizedTitle = Bundle.main.preferredLocalizedString(forKey: title, value: "No translation", table: "MainMenu")
                if localizedTitle == "No translation" {
                    title = Bundle.main.preferredLocalizedString(forKey: title, value: "No translation", table: nil)
                }
                else {
                    title = localizedTitle
                }
            }
        }
    }
}

extension Bundle {
    static let noTransKey = "no-trans"
    
    func preferredLocalizedString(forKey key: String, value: String?, table: String?) -> String {
        let new = localizedString(forKey: key, value: Self.noTransKey, table: table)
        if new != Self.noTransKey {
            return new
        }
        
        for lang in preferredLocalizations {
            guard
                let path = self.path(forResource: lang, ofType: "lproj"),
                let bundle = Bundle(path: path)
                else { continue }
            
            let new = bundle.localizedString(forKey: key, value: Self.noTransKey, table: table)
            if new != Self.noTransKey {
                return new
            }
        }
        
        return value ?? key
    }
}
