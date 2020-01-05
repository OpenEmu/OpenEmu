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

@objc
class GameCollectionViewController: ImageCollectionViewController {
    @IBOutlet var listView: GameTableView!
    var gamesController = ArrayController()
    var selectedViewTag: OECollectionViewControllerViewTag = .gridViewTag
    
    override var representedObject: Any? {
        willSet {
            precondition(newValue == nil || newValue is OEGameCollectionViewItemProtocol, "unexpected object")
            dataSource.collection = newValue as? OEGameCollectionViewItemProtocol
        }
    }
    
    var representedItem: OEGameCollectionViewItemProtocol? {
        return representedObject as? OEGameCollectionViewItemProtocol
    }
    
    var dataSource = GameCollectionDataSource()
    
    override func viewDidLoad() {
        Bundle(for: GameTableView.self).loadNibNamed("GameTableView", owner: self, topLevelObjects: nil)
        self.dataSourceDelegate = dataSource
        setupGamesController()
        
        listView.target         = self
        listView.delegate       = self
        listView.dataSource     = self
        listView.doubleAction   = #selector(tableViewWasDoubleClicked(_:))
        listView.rowSizeStyle   = .custom
        listView.rowHeight      = 20.0
        // TODO
        //listView.sortDescriptors = self.defaultSortDescriptors
        listView.allowsMultipleSelection = true
        listView.registerForDraggedTypes([.fileURL])
        
        super.viewDidLoad()
    }
    
    private func setupGamesController() {
        let db  = libraryController.database
        let ctx = db.mainThreadContext
        
        let controller = dataSource.items
        controller.automaticallyRearrangesObjects = true
        controller.automaticallyPreparesContent = true
        controller.usesLazyFetching = false
        controller.managedObjectContext = ctx
        controller.entityName = OEDBGame.entityName()
        controller.fetchPredicate = NSPredicate(value: false)
        controller.avoidsEmptySelection = false
    }
    
    override func reloadData() {
        super.reloadData()
        listView.reloadData()
    }
    
    // MARK: - View Selection
    func switchToGridView(_ sender: Any?) {
        
    }
    
    func switchToListView(_ sender: Any?) {
        
    }
    
    @objc func switchViewMode(_ sender: Any?) {
        
    }
}

extension GameCollectionViewController: NSTableViewDelegate {
    func tableView(_ tableView: NSTableView, willDisplayCell cell: Any, for tableColumn: NSTableColumn?, row: Int) {
        // for rating cell, can we use our own?
    }
    
    func tableView(_ tableView: NSTableView, shouldEdit tableColumn: NSTableColumn?, row: Int) -> Bool {
        return tableColumn?.identifier == .gameTableViewTitleColumn
    }
    
    func tableViewSelectionIsChanging(_ notification: Notification) {
        // TODO
    }
    
    @objc func tableViewWasDoubleClicked(_ sender: Any?) {
        let row = listView.clickedRow
        guard row > -1 else { return }
        
        
    }
}

extension GameCollectionViewController: NSTableViewDataSource {
    func numberOfRows(in tableView: NSTableView) -> Int {
        precondition(dataSource.numberOfSections <= 1)
        
        return dataSource.numberOfItems(in: 0)
    }
    
    func tableView(_ tableView: NSTableView, objectValueFor tableColumn: NSTableColumn?, row: Int) -> Any? {
        guard row < dataSource.numberOfItems(in: 0) else { return nil }
        
        let item = dataSource.item(at: row)
        guard let colId = tableColumn?.identifier else { return item }
        
        guard let lvi = item as? OEListViewDataSourceItem else { return item }
        
        if colId == .gameTableViewStatusColumn {
            let image = lvi.listViewStatus
            let state = listView.selectedRowIndexes.contains(row) ? OEThemeInputStateFocused : OEThemeStateDefault
            return image?.image(forState: UInt(state))
        }
            
        let sel = Selector(colId.rawValue)
        if lvi.responds(to: sel) {
            return lvi.perform(sel)
        }
        
        return item
    }
}

extension GameCollectionViewController {
    var selectedGames: [OEDBGame] {
        return []
    }
}

extension GameCollectionViewController {
    override func validateUserInterfaceItem(_ item: NSValidatedUserInterfaceItem) -> Bool {
        guard let action = item.action else { return true }
        
        if action == #selector(switchViewMode(_:)) {
            return self.shouldShowBlankSlate ? false : true
        }
        
        return super.validateUserInterfaceItem(item)
    }
}
