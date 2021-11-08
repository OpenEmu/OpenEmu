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

private extension NSUserInterfaceItemIdentifier {
    static let coreColumn    = NSUserInterfaceItemIdentifier("coreColumn")
    static let systemColumn  = NSUserInterfaceItemIdentifier("systemColumn")
    static let versionColumn = NSUserInterfaceItemIdentifier("versionColumn")
    
    static let coreNameCell        = NSUserInterfaceItemIdentifier("coreNameCell")
    static let systemListCell      = NSUserInterfaceItemIdentifier("systemListCell")
    static let versionCell         = NSUserInterfaceItemIdentifier("versionCell")
    static let installButtonCell   = NSUserInterfaceItemIdentifier("installBtnCell")
    static let installProgressCell = NSUserInterfaceItemIdentifier("installProgressCell")
}

final class PrefCoresController: NSViewController {
    
    @IBOutlet var coresTableView: NSTableView!
    
    var coreListObservation: NSKeyValueObservation?
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        coreListObservation = CoreUpdater.shared.observe(\CoreUpdater.coreList) {
            object, _ in
            self.coresTableView.reloadData()
        }
        CoreUpdater.shared.checkForNewCores()   // TODO: check error from completion handler
        CoreUpdater.shared.checkForUpdates()
        
        for column in coresTableView.tableColumns {
            switch column.identifier {
            case .coreColumn:
                column.headerCell.title = NSLocalizedString("Core", comment: "Cores preferences, column header")
            case .systemColumn:
                column.headerCell.title = NSLocalizedString("System", comment: "Cores preferences, column header")
            case .versionColumn:
                column.headerCell.title = NSLocalizedString("Version", comment: "Cores preferences, column header")
            default:
                break
            }
        }
    }
    
    @IBAction func updateOrInstall(_ sender: NSButton) {
        let cellView = sender.superview as! NSTableCellView
        let row = coresTableView.row(for: cellView)
        updateOrInstallItem(row)
    }
    
    private func updateOrInstallItem(_ row: Int) {
        CoreUpdater.shared.installCoreInBackgroundUserInitiated(coreDownload(row))
    }
    
    private func coreDownload(_ row: Int) -> CoreDownload {
        return CoreUpdater.shared.coreList[row]
    }
}

// MARK: - NSTableView DataSource

extension PrefCoresController: NSTableViewDataSource {
    
    func numberOfRows(in tableView: NSTableView) -> Int {
        return CoreUpdater.shared.coreList.count
    }
    
    func tableView(_ tableView: NSTableView, objectValueFor tableColumn: NSTableColumn?, row: Int) -> Any? {
        
        let plugin = coreDownload(row)
        let ident = tableColumn!.identifier
        
        if ident == .coreColumn {
            return plugin.name
            
        } else if ident == .systemColumn {
            return plugin.systemNames.joined(separator: ", ")
            
        } else if ident == .versionColumn {
            if plugin.isDownloading {
                return plugin
            } else if plugin.canBeInstalled {
                return NSLocalizedString("Install", comment: "Install Core")
            } else if plugin.hasUpdate {
                return NSLocalizedString("Update", comment: "Update Core")
            } else {
                return plugin.version
            }
        }
        return plugin
    }
}

// MARK: - NSTableView Delegate

extension PrefCoresController: NSTableViewDelegate {
    
    func tableView(_ tableView: NSTableView, viewFor tableColumn: NSTableColumn?, row: Int) -> NSView? {
        
        let ident = tableColumn!.identifier
        let plugin = coreDownload(row)
        
        if ident == .coreColumn {
            let view = tableView.makeView(withIdentifier: .coreNameCell, owner: self) as! NSTableCellView
            let color: NSColor = plugin.canBeInstalled ? .disabledControlTextColor : .labelColor
            view.textField!.textColor = color
            return view
            
        } else if ident == .systemColumn {
            let view = tableView.makeView(withIdentifier: .systemListCell, owner: self) as! NSTableCellView
            let color: NSColor = plugin.canBeInstalled ? .disabledControlTextColor : .labelColor
            view.textField!.textColor = color
            return view
            
        } else if ident == .versionColumn {
            if plugin.isDownloading {
                return tableView.makeView(withIdentifier: .installProgressCell, owner: self)
            } else if plugin.canBeInstalled || plugin.hasUpdate {
                return tableView.makeView(withIdentifier: .installButtonCell, owner: self)
            } else {
                return tableView.makeView(withIdentifier: .versionCell, owner: self)
            }
        }
        return nil
    }
    
    func tableView(_ tableView: NSTableView, shouldSelectRow row: Int) -> Bool {
        return false
    }
}

// MARK: - PreferencePane

extension PrefCoresController: PreferencePane {
    
    var icon: NSImage? { NSImage(named: "cores_tab_icon") }
    
    var panelTitle: String { "Cores" }
    
    var viewSize: NSSize { view.fittingSize }
}
