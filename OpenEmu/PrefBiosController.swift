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
import OpenEmuKit.OECorePlugin

private var PrefBiosCoreListKVOContext = 0

private extension NSUserInterfaceItemIdentifier {
    static let infoCell = NSUserInterfaceItemIdentifier("InfoCell")
    static let coreCell = NSUserInterfaceItemIdentifier("CoreCell")
    static let fileCell = NSUserInterfaceItemIdentifier("FileCell")
}

final class PrefBiosController: NSViewController {
    
    @IBOutlet var tableView: NSTableView!
    
    private var items: [AnyHashable]?
    private var token: NSObjectProtocol?
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        tableView.delegate = self
        tableView.dataSource = self
        tableView.usesAutomaticRowHeights = true
        tableView.floatsGroupRows = true
        
        tableView.registerForDraggedTypes([.fileURL])
        
        let menu = NSMenu()
        menu.autoenablesItems = false
        menu.delegate = self
        tableView.menu = menu
        
        token = NotificationCenter.default.addObserver(forName: .didImportBIOSFile, object: nil, queue: .main) { [weak self] notification in
            self?.biosFileWasImported(notification)
        }
        
        OECorePlugin.addObserver(self, forKeyPath: #keyPath(OECorePlugin.allPlugins), context: &PrefBiosCoreListKVOContext)
        
        reloadData()
    }
    
    deinit {
        if let token = token {
            NotificationCenter.default.removeObserver(token)
            self.token = nil
        }
        OECorePlugin.removeObserver(self, forKeyPath: #keyPath(OECorePlugin.allPlugins), context: &PrefBiosCoreListKVOContext)
    }
    
    override func observeValue(forKeyPath keyPath: String?, of object: Any?, change: [NSKeyValueChangeKey : Any]?, context: UnsafeMutableRawPointer?) {
        
        if context == &PrefBiosCoreListKVOContext {
            reloadData()
        } else {
            super.observeValue(forKeyPath: keyPath, of: object, change: change, context: context)
        }
    }
    
    private func reloadData() {
        
        guard let cores = OECorePlugin.allPlugins as? [OECorePlugin] else { return }
        var items: [AnyHashable] = []
        
        for core in cores {
            guard
                core.requiredFiles != nil
            else { continue }
            
            let requiredFiles = (core.requiredFiles as NSArray).sortedArray(using: [NSSortDescriptor(key: "Description", ascending: true)])
            if requiredFiles.count > 0,
               let requiredFiles = requiredFiles as? [AnyHashable] {
                items.append(core)
                items.append(contentsOf: requiredFiles)
            }
        }
        
        self.items = items
        
        tableView.reloadData()
    }
    
    private func importBIOSFile(_ url: URL) -> Bool {
        return BIOSFile.checkIfBIOSFileAndImport(at: url)
    }
    
    @objc private func deleteBIOSFile(_ sender: Any?) {
        guard
            let file = items?[tableView.clickedRow - 1] as? [String : Any],
            let fileName = file["Name"] as? String
        else { return }
            
        if BIOSFile.deleteBIOSFile(withFileName: fileName),
           let view = tableView.view(atColumn: 0, row: tableView.clickedRow, makeIfNecessary: false),
           view.identifier == .fileCell,
           let availabilityIndicator = view.viewWithTag(3) as? NSImageView {
            availabilityIndicator.image = NSImage(named: "bios_missing")
            availabilityIndicator.contentTintColor = .systemOrange
        }
    }
    
    @objc private func biosFileWasImported(_ notification: Notification) {
        let md5 = notification.userInfo?["MD5"] as! String
        for (index, item) in (items ?? []).enumerated() {
            guard
                let file = item as? [String : Any],
                let fileMD5 = file["MD5"] as? String,
                fileMD5.caseInsensitiveCompare(md5) == .orderedSame,
                let view = tableView.view(atColumn: 0, row: index + 1, makeIfNecessary: false),
                view.identifier == .fileCell,
                let availabilityIndicator = view.viewWithTag(3) as? NSImageView
            else { continue }
            
            availabilityIndicator.image = NSImage(named: "bios_found")
            availabilityIndicator.contentTintColor = .systemGreen
            break
        }
    }
}

// MARK: - NSTableView DataSource

extension PrefBiosController: NSTableViewDataSource {
    
    func numberOfRows(in tableView: NSTableView) -> Int {
        guard let items = items else { return 0 }
        
        return items.count + 1
    }
    
    func tableView(_ tableView: NSTableView, validateDrop info: NSDraggingInfo, proposedRow row: Int, proposedDropOperation dropOperation: NSTableView.DropOperation) -> NSDragOperation {
        
        tableView.setDropRow(-1, dropOperation: .on)
        return .copy
    }
    
    func tableView(_ tableView: NSTableView, acceptDrop info: NSDraggingInfo, row: Int, dropOperation: NSTableView.DropOperation) -> Bool {
        
        guard let files = info.draggingPasteboard.readObjects(forClasses: [NSURL.self], options: nil) as? [URL] else { return false }
        
        var importedSomething = false
        
        var recCheckURL: ((_ url: Any, _ idx: Int, _ stop: UnsafeMutablePointer<ObjCBool>) -> Void) = { url, idx, stop in }
        let checkURL: ((Any, Int, UnsafeMutablePointer<ObjCBool>) -> Void) = { url, idx, stop in
            let url = url as! URL
            if url.isDirectory {
                let dirEnum = FileManager.default.enumerator(at: url, includingPropertiesForKeys: nil, options: [.skipsHiddenFiles, .skipsPackageDescendants, .skipsSubdirectoryDescendants])
                
                while let suburl = dirEnum?.nextObject() {
                    recCheckURL(suburl, idx, stop)
                }
            }
            else if url.isFileURL {
                importedSomething = self.importBIOSFile(url) || importedSomething
            }
        }
        recCheckURL = checkURL
        
        (files as NSArray).enumerateObjects(checkURL)
        
        return importedSomething
    }
}

// MARK: - NSTableView Delegate

extension PrefBiosController: NSTableViewDelegate {
    
    func tableView(_ view: NSTableView, viewFor tableColumn: NSTableColumn?, row: Int) -> NSView? {
        
        if row == 0 {
            let groupCell = view.makeView(withIdentifier: .infoCell, owner: self) as? NSTableCellView
            let textField = groupCell?.textField
            
            let parStyle = NSMutableParagraphStyle()
            parStyle.alignment = .justified
            let attributes: [NSAttributedString.Key : Any] = [
                .font: NSFont.systemFont(ofSize: NSFont.smallSystemFontSize),
                .foregroundColor: NSColor.labelColor,
                .paragraphStyle: parStyle
            ]
            
            let linkAttributes: [NSAttributedString.Key : Any] = [
                .font: NSFont.systemFont(ofSize: NSFont.smallSystemFontSize),
                .paragraphStyle: parStyle,
                .link: URL.userGuideBIOSFiles
            ]
            
            let linkText = NSLocalizedString("User guide on BIOS files", comment: "Bios files introduction text, active region")
            let infoText = String(format: NSLocalizedString("In order to emulate some systems, BIOS files are needed due to increasing complexity of the hardware and software of modern gaming consoles. Please read our %@ for more information.", comment: "BIOS files preferences introduction text"), linkText)
            
            let attributedString = NSMutableAttributedString(string: infoText, attributes: attributes)
            
            let linkRange = (infoText as NSString).range(of: linkText)
            attributedString.setAttributes(linkAttributes, range: linkRange)
            
            textField?.attributedStringValue = attributedString
            
            return groupCell
        }
        
        let item = items?[row-1]
        if tableView(view, isGroupRow: row) {
            let core = item as? OECorePlugin
            let groupCell = view.makeView(withIdentifier: .coreCell, owner: self) as? NSTableCellView
            groupCell?.textField?.stringValue = core?.name ?? ""
            return groupCell
        }
        else {
            guard let file = item as? [String : Any] else { return nil }
            
            let fileCell = tableView.makeView(withIdentifier: .fileCell, owner: self) as? NSTableCellView
            
            let descriptionField = fileCell?.textField
            let fileNameField = fileCell?.viewWithTag(1) as? NSTextField
            let availabilityIndicator = fileCell?.viewWithTag(3) as? NSImageView
            
            let description = file["Description"] as? String ?? ""
            let md5 = file["MD5"] as? String ?? ""
            let name = file["Name"] as? String ?? ""
            let size = file["Size"] as AnyObject
            
            let available = BIOSFile.isBIOSFileAvailable(withFileInfo: file)
            let imageName = available ? "bios_found" : "bios_missing"
            let image = NSImage(named: imageName)
            
            descriptionField?.stringValue = description
            
            let sizeString = ByteCountFormatter.string(fromByteCount: size.int64Value ?? 0, countStyle: .file)
            fileNameField?.stringValue = "\(name) (\(sizeString))"
            fileNameField?.toolTip = "MD5: \(md5)"
            
            availabilityIndicator?.image = image
            availabilityIndicator?.contentTintColor = available ? .systemGreen : .systemOrange
            
            return fileCell
        }
    }
    
    func tableView(_ tableView: NSTableView, heightOfRow row: Int) -> CGFloat {
        if row == 0 {
            return 60
        } else if self.tableView(tableView, isGroupRow: row) {
            return 18
        } else {
            return 54
        }
    }
    
    func tableView(_ tableView: NSTableView, isGroupRow row: Int) -> Bool {
        
        if row == 0 {
            return false
        }
        
        return items?[row - 1] is OECorePlugin
    }
}

extension PrefBiosController: NSMenuDelegate {
    
    func menuNeedsUpdate(_ menu: NSMenu) {
        menu.removeAllItems()
        guard
            tableView.clickedRow >= 1,
            !tableView(tableView, isGroupRow: tableView.clickedRow)
        else { return }
        
        if let file = items?[tableView.clickedRow - 1] as? [String : Any] {
            let available = BIOSFile.isBIOSFileAvailable(withFileInfo: file)
            let item = NSMenuItem()
            item.title = NSLocalizedString("Delete", comment: "")
            item.action = #selector(deleteBIOSFile(_:))
            item.isEnabled = available ? true : false
            menu.addItem(item)
        }
    }
}

// MARK: - PreferencePane

extension PrefBiosController: PreferencePane {
    
    var icon: NSImage? { NSImage(named: "bios_tab_icon") }
    
    var panelTitle: String { "System Files" }
    
    var viewSize: NSSize { view.fittingSize }
}
