//
//  OESmartCollectionEditViewController.swift
//  OpenEmu
//
//  Created by Christoph Leimbrock on 1/6/16.
//
//

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
}