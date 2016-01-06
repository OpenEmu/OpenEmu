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


    // MARK: - Accessing Configuration -
    func predicate() -> NSPredicate {
        if let predicate = predicateEditor.predicate {
            return predicate
        }

        return NSPredicate.init(value: true);
    }

    func fetchLimit() -> UInt {
        return limitButton.enabled ? UInt(limitInput.integerValue) : UInt.max
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