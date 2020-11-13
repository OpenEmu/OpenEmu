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

@objc(OEControlsKeyButton)
final class ControlsKeyButton: NSButton {
    
    let isWood = UserDefaults.standard.integer(forKey: OEControlsPrefsAppearancePreferenceKey) == OEControlsPrefsAppearancePreferenceValue.wood.rawValue
    
    override var isFlipped: Bool {
        return false
    }
    
    override func draw(_ dirtyRect: NSRect) {
        
        var image: NSImage?
        if isWood {
            image = state == .on ? NSImage(named: "wood_textfield_focus") : NSImage(named: "wood_textfield")
        } else {
            image = state == .on ? NSImage(named: "controls_textfield_focus")?.withTintColor(.controlAccentColor) : NSImage(named: "controls_textfield")
        }
        image?.draw(in: dirtyRect)
        
        let p = NSPoint(x: bounds.origin.x + 4, y: bounds.origin.y + (isFlipped ? 4 : 6))
        title.draw(at: p, withAttributes: isWood ? ControlsKeyButton.attributesWood : ControlsKeyButton.attributes)
    }
    
    private static let attributes: [NSAttributedString.Key : Any]? = {
        
        let attributes: [NSAttributedString.Key : Any] =
                                          [.font: NSFont.systemFont(ofSize: 11),
                                .foregroundColor: NSColor.labelColor]
        
        return attributes
    }()
    
    private static let attributesWood: [NSAttributedString.Key : Any]? = {
        
        let attributes: [NSAttributedString.Key : Any] =
                                          [.font: NSFont.boldSystemFont(ofSize: 11),
                                .foregroundColor: NSColor.black,
                                         .shadow: NSShadow.oeControls]
        
        return attributes
    }()
    
    // MARK: - Accessibility
    
    @objc var label: String = ""
    
    override var title: String {
        didSet {
            let announcement = NSLocalizedString("Changed button for %1$@ to %2$@.", comment: "Controls preferences, accessibility notification (“Changed button for Up to W.”)")
            NSAccessibility.post(element: NSApp.mainWindow as Any,
                            notification: .announcementRequested,
                                userInfo: [.announcement: String(format: announcement, label, title),
                                               .priority: NSAccessibilityPriorityLevel.high.rawValue])
        }
    }
    
    override func accessibilityTitle() -> String? {
        guard title == "" else {
            return super.accessibilityTitle()
        }
        let announcement = NSLocalizedString("Not assigned", comment: "Controls preferences, accessibility title (key is not mapped, “empty”)")
        return announcement
    }
    
    override func accessibilityHelp() -> String? {
        let announcement = NSLocalizedString("To assign a new key or button to %@, press this button, then press the desired key or button on your keyboard or gamepad.", comment: "Controls preferences, accessibility help")
        return String(format: announcement, label)
    }
}
