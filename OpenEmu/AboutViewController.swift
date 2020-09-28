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

@objcMembers
class AboutViewController: NSViewController {
    
    @IBOutlet weak var websiteLink: TextButton!
    @IBOutlet weak var gitHubLink: TextButton!
    
    override func viewDidLoad() {
        
        let color = NSColor(from: "#526ba0")
        let hoverColor = NSColor(from: "#9c2a32")
        
        let shadow = NSShadow()
        shadow.shadowColor = .black
        shadow.shadowOffset = NSMakeSize(0, -1)
        
        websiteLink.font = .boldSystemFont(ofSize: 12)
        websiteLink.textShadow = shadow
        websiteLink.textColor = color
        websiteLink.textColorHover = hoverColor
        
        shadow.shadowColor = NSColor.black.withAlphaComponent(0.4)
        
        gitHubLink.font = .boldSystemFont(ofSize: 11)
        gitHubLink.textShadow = shadow
        gitHubLink.textColor = color
        gitHubLink.textColorHover = hoverColor
    }
    
    dynamic var appVersion: String {
        return Bundle.main.infoDictionary!["CFBundleVersion"] as! String
    }
    
    dynamic var buildVersion: String {
        return OEBuildInfo.buildVersion
    }
    
    dynamic lazy var specialThanks: NSAttributedString = {
        let msg = NSLocalizedString("Special thanks to everyone that made\nOpenEmu possible. To find out more\nabout our contributors, emulator cores,\ndocumentation, licenses and to issue\nbugs please visit us on our GitHub.", comment: "Special thanks message (about window).")
        let paragraphStyle = NSParagraphStyle.default.mutableCopy() as! NSMutableParagraphStyle
        paragraphStyle.alignment = .center
        paragraphStyle.lineHeightMultiple = 1.225
        let attributes: [NSAttributedString.Key: Any] = [
            .font: NSFont.systemFont(ofSize: NSFont.smallSystemFontSize),
            .paragraphStyle: paragraphStyle,
            .foregroundColor: NSColor.white
        ]
        return NSAttributedString(string: msg, attributes: attributes)
    }()
    
    @IBAction func openWeblink(_ sender: AnyObject?) {
        if let button = sender as? NSButton {
            let url = URL(string: "http://" + button.title)!
            NSWorkspace.shared.open(url)
        }
    }
}
