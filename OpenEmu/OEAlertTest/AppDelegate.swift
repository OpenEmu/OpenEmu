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


class AlertStatus: NSObject
{
    @objc var defaultButtonText = "DEFAULT_BTN"
    @objc var alternateButtonText = ""
    @objc var otherButtonText = ""
    @objc var messageText = ""
    @objc var informativeText = ""
    @objc var showsInputField = false
    @objc var stringValue = ""
    @objc var showsOtherInputField = false
    @objc var otherStringValue = ""
    @objc var inputLabelText = ""
    @objc var otherInputLabelText = ""
    @objc var showsProgressbar = false
    @objc var progress = 0.0
    @objc var suppressionLabelText = ""
    @objc var showsSuppressionButton = false
    
    
    func update(_ alert: OEAlert)
    {
        alert.messageText = messageText
        alert.informativeText = informativeText
        alert.defaultButtonTitle = defaultButtonText
        alert.alternateButtonTitle = alternateButtonText
        alert.otherButtonTitle = otherButtonText
        alert.showsInputField = showsInputField
        alert.stringValue = stringValue
        alert.showsOtherInputField = showsOtherInputField
        alert.otherStringValue = otherStringValue
        alert.showsInputField = showsInputField
        alert.inputLabelText = inputLabelText
        alert.showsOtherInputField = showsOtherInputField
        alert.otherInputLabelText = otherInputLabelText
        alert.showsProgressbar = showsProgressbar
        alert.progress = progress
        alert.suppressionLabelText = suppressionLabelText
        alert.showsSuppressionButton = showsSuppressionButton
    }
}


@NSApplicationMain
class AppDelegate: NSObject, NSApplicationDelegate
{
    @IBOutlet weak var window: NSWindow!
    @IBOutlet weak var testBgProgressUpdateCheckbox: NSButton!
    @objc var currentStatus = AlertStatus()
    var nextStatuses = [AlertStatus]()
    
    var alert = OEAlert(messageText: "", defaultButton: "", alternateButton: "")


    func applicationWillFinishLaunching(_ aNotification: Notification)
    {
        if #available(OSX 10.14, *) {
            NSApp.appearance = NSAppearance(named: .darkAqua)
        }
    }
    
    func updateAlert()
    {
        currentStatus.update(alert)
    }
    
    @IBAction func pushStatus(_ sender: Any?)
    {
        nextStatuses.insert(currentStatus, at: 0)
        self.willChangeValue(for: \.currentStatus)
        currentStatus = AlertStatus()
        self.didChangeValue(for: \.currentStatus)
    }

    @IBAction func runModal(_ sender: Any?)
    {
        alert = OEAlert(messageText: "", defaultButton: "", alternateButton: "")
        updateAlert()
        if !nextStatuses.isEmpty {
            alert.setDefaultButtonAction(#selector(alertButtonPressed(_:)), andTarget: self)
        }
        if testBgProgressUpdateCheckbox.state == .on {
            alert.performBlock {
                DispatchQueue.global(qos:.background).async {
                    while self.alert.result.rawValue == 0 {
                        usleep(100000)
                        self.alert.performBlock {
                            self.alert.progress = self.alert.progress + 0.1
                            if self.alert.progress >= 0.95 {
                                self.alert.progress = 0.0
                            }
                        }
                    }
                }
            }
        }
        alert.runModal()
    }
    
    @IBAction func nsAlert(_ sender: Any?)
    {
        let nsalert = NSAlert()
        nsalert.messageText = currentStatus.messageText
        nsalert.informativeText = currentStatus.informativeText
        nsalert.addButton(withTitle: currentStatus.defaultButtonText)
        if (currentStatus.alternateButtonText.count != 0) {
            nsalert.addButton(withTitle: currentStatus.alternateButtonText)
        }
        if (currentStatus.otherButtonText.count != 0) {
            nsalert.addButton(withTitle: currentStatus.otherButtonText)
        }
        nsalert.runModal()
    }
    
    
    @objc func alertButtonPressed(_ sender: Any?)
    {
        if nextStatuses.isEmpty {
            NSApp.stopModal(withCode: .alertFirstButtonReturn)
        } else {
            currentStatus = nextStatuses[0]
            nextStatuses.remove(at: 0)
            updateAlert()
        }
    }
}

