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

    @IBAction func confirm (sender :AnyObject){
        NSApp.stopModalWithCode(NSAlertSecondButtonReturn)
    }

    @IBAction func cancel(sender :AnyObject){
        NSApp.stopModalWithCode(NSAlertFirstButtonReturn)
    }
}