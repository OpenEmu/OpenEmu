/*
 Copyright (c) 2016, OpenEmu Team
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the OpenEmu Team nor the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY OpenEmu Team ''AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL OpenEmu Team BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

import Foundation

class OESmartCollectionEditViewController : NSViewController
{
    @IBOutlet weak var confirmButton: NSButton!
    @IBOutlet weak var limitButton: NSButton!
    @IBOutlet weak var limitInput: NSTextField!
    @IBOutlet weak var limitOrderButton: NSPopUpButton!
    @IBOutlet weak var predicateEditor: NSPredicateEditor!

    override func viewDidLoad() {
        super.viewDidLoad()

        predicateEditor.addRow(self)
    }


    @IBAction func confirm (sender :AnyObject){
        NSApp.stopModalWithCode(NSAlertSecondButtonReturn)
    }

    @IBAction func cancel(sender :AnyObject){
        NSApp.stopModalWithCode(NSAlertFirstButtonReturn)
    }


    // MARK: - Accessing Configuration -
    func predicate() -> NSPredicate {
        if let predicate = predicateEditor.predicate {
            return predicate
        }

        return NSPredicate.init(value: true);
    }

    func fetchLimit() -> NSNumber? {
        return limitButton.enabled ? NSNumber(integer: limitInput.integerValue) : nil
    }

    func fetchOrder() -> String? {
        if let selectedItem = limitOrderButton.selectedItem,
                object = selectedItem.representedObject
        {
            return object as? String;
        }

        return nil
    }

    func fetchOrderIsAscending() -> Bool {
        return limitOrderButton.selectedItem?.tag != 0
    }
}