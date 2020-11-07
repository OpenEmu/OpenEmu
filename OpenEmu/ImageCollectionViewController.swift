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
class ImageCollectionViewController: NSViewController {
    
    @objc var database: OELibraryDatabase!
    
    @IBOutlet weak var collectionView: CollectionView!
    @IBOutlet weak var flowLayout: NSCollectionViewFlowLayout!
    @IBOutlet weak var blankSlateView: OEBlankSlateView!
    
    var shouldShowBlankSlate = false
    var searchKeys = ["rom.game.gameTitle", "rom.game.name", "rom.game.system.lastLocalizedName", "name", "userDescription"]
    var currentSearchTerm = ""
    var itemSize = CGSize(width: 168, height: 143)
    
    private var isSelected: Bool {
        return view.superview != nil
    }
    
    private var toolbar: LibraryToolbar? {
        view.window?.toolbar as? LibraryToolbar
    }
    
    var dataSourceDelegate: ImageDataSourceDelegate!
    
    override var representedObject: Any? {
        didSet {
            let _ = self.view
        }
    }
    
    override var nibName: NSNib.Name? {
        return NSNib.Name("ImageCollectionViewController")
    }
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        itemSize = flowLayout.itemSize
        
        collectionView.registerForDraggedTypes([.fileURL])
        collectionView.setDraggingSourceOperationMask([], forLocal: true)
        collectionView.setDraggingSourceOperationMask(.copy, forLocal: false)

        blankSlateView.delegate = self
        blankSlateView.autoresizingMask = [.width, .height]
        blankSlateView.registerForDraggedTypes([.fileURL])
        blankSlateView.frame = view.bounds
        
        if let context = OELibraryDatabase.default?.mainThreadContext {
            NotificationCenter.default.addObserver(self,
                                                   selector: #selector(managedObjectContextDidUpdate(_:)),
                                                   name: .NSManagedObjectContextDidSave,
                                                   object: context)
        }
        
        NotificationCenter.default.addObserver(self,
                                               selector: #selector(libraryLocationDidChange(_:)),
                                               name: .OELibraryLocationDidChange,
                                               object: nil)
        
        reloadData()
    }
    
    private func validateToolbarItems() {
        guard let toolbar = toolbar else { return }
        
        toolbar.viewModeSelector.isEnabled = false
        toolbar.viewModeSelector.selectedSegment = -1
        
        toolbar.gridSizeSlider.isEnabled = !shouldShowBlankSlate
        toolbar.decreaseGridSizeButton.isEnabled = !shouldShowBlankSlate
        toolbar.increaseGridSizeButton.isEnabled = !shouldShowBlankSlate

        toolbar.searchField.isEnabled = !shouldShowBlankSlate
        if toolbar.searchField.searchMenuTemplate == nil {
             toolbar.searchField.searchMenuTemplate = searchMenuTemplate()
        }
        toolbar.searchField.stringValue = currentSearchTerm
         
        toolbar.addButton.isEnabled = false
    }
    
    @objc func libraryLocationDidChange(_ notification: Notification) {
        
    }
    
    @objc func managedObjectContextDidUpdate(_ notification: Notification) {
        DispatchQueue.main.async {
            self.scheduleUpdateViews()
        }
    }
    
    func scheduleUpdateViews() {
        NSObject.cancelPreviousPerformRequests(withTarget: self, selector: #selector(updateViews), object: nil)
        perform(#selector(updateViews), with: nil, afterDelay: 0.5)
    }
    
    @objc func updateViews() {
        Self.cancelPreviousPerformRequests(withTarget: self, selector: #selector(updateViews), object: nil)
        reloadData()
    }
    
    override func viewDidAppear() {
        super.viewDidAppear()
        
        if let slider = toolbar?.gridSizeSlider {
            slider.isContinuous = true
            slider.floatValue = Self.lastGridSize
            zoomGridView(zoomValue: CGFloat(slider.floatValue))
        }
        
        validateToolbarItems();
        restoreSelectionFromDefaults()
    }
    
    // MARK: - Toolbar
    
    @IBAction func changeGridSize(_ sender: NSSlider?) {
        if let slider = sender {
            zoomGridView(zoomValue: CGFloat(slider.floatValue))
        }
    }
    
    @IBAction func decreaseGridSize(_ sender: AnyObject?) {
        if let slider = toolbar?.gridSizeSlider {
            slider.floatValue -= sender?.tag == 199 ? 0.25 : 0.5
            zoomGridView(zoomValue: CGFloat(slider.floatValue))
        }
    }
    
    @IBAction func increaseGridSize(_ sender: AnyObject?) {
        if let slider = toolbar?.gridSizeSlider {
            slider.floatValue += sender?.tag == 199 ? 0.25 : 0.5
            zoomGridView(zoomValue: CGFloat(slider.floatValue))
        }
    }
    
    @IBAction func showInFinder(_ sender: Any?) {
        let urls = dataSourceDelegate.imageURLs(forItemsAt: collectionView.selectionIndexPaths)
        NSWorkspace.shared.activateFileViewerSelecting(urls)
    }
    
    enum ViewTag {
        case collection
        case blankSlate
    }
    
    func updateBlankSlate() {
        if shouldShowBlankSlate {
            switchTo(tag: .blankSlate)
//            if let toolbar = libraryController.toolbar, isSelected {
//                toolbar.viewSelector.isEnabled = false
//                toolbar.gridSizeSlider.isEnabled = false
//                toolbar.searchField.isEnabled = false
//                toolbar.searchField.menu = nil
//            }
            
            blankSlateView.representedObject = representedObject
        } else {
            switchTo(tag: .collection)
//            if let toolbar = libraryController.toolbar, isSelected {
//                toolbar.viewSelector.isEnabled = true
//                toolbar.gridSizeSlider.isEnabled = true
//                toolbar.searchField.isEnabled = true
//                toolbar.searchField.menu = nil
//            }
        }
    }
    
    func switchTo(tag: ViewTag) {
        if isSelected {
//            setupToolbar(tag: tag)
        }
        
        show(tag: tag)
    }
    
//    func setupToolbar(tag: ViewTag) {
//        guard let toolbar = libraryController.toolbar else { return }
//
//        switch tag {
//        case .collection:
//            toolbar.viewSelector.selectedSegment = 0
//            toolbar.gridSizeSlider.isEnabled = true
//
//        case .blankSlate:
//            toolbar.viewSelector.isEnabled = false
//            toolbar.gridSizeSlider.isEnabled = false
//        }
//    }
    
    func show(tag: ViewTag) {
        var newView: NSView!
        switch tag {
        case .collection:
            newView = collectionView.enclosingScrollView
        case .blankSlate:
            newView = blankSlateView
        }
        
        if newView.superview == view {
            return
        }
        
        collectionView.reloadData()
        var makeFirstResponder = false
        if let firstResponder = view.window?.firstResponder as? NSView, firstResponder.isDescendant(of: view) {
            makeFirstResponder = true
        }
        
        while let sv = view.subviews.first {
            sv.removeFromSuperview()
        }
        
        view.addSubview(newView)
        newView.frame = view.bounds
        if makeFirstResponder {
            view.window?.makeFirstResponder(newView)
        }
    }
    
    func zoomGridView(zoomValue: CGFloat) {
        let finalZoom = zoomValue * 1.07 // multiplier replicates zoom level of IKImageBrowserView
        flowLayout.itemSize = itemSize.applying(CGAffineTransform(scaleX: finalZoom, y: finalZoom))
        Self.lastGridSize = Float(zoomValue)
    }
    
    private func searchMenuTemplate() -> NSMenu {
        let menu = NSMenu()
        
        let descriptors = [
            (
                "Everything",
                "Search field filter selection title",
                ["rom.game.gameTitle", "rom.game.name", "rom.game.system.lastLocalizedName", "name", "userDescription"]
            ),
            (
                "Name",
                "Search field filter selection title",
                ["name"]
            ),
            (
                "Description",
                "Search field filter selection title",
                ["userDescription"]
            ),
            (
                "Game Name",
                "",
                ["rom.game.gameTitle", "rom.game.name"]
            ),
            (
                "System",
                "",
                ["rom.game.system.lastLocalizedName"]
            )
        ]
        
        for desc in descriptors {
            let item = NSMenuItem(title: NSLocalizedString(desc.0, comment: desc.1),
                                  action: #selector(searchScopeDidChange(_:)),
                                  keyEquivalent: "")
            item.representedObject = desc.2
            item.target = self
            item.indentationLevel = 1
            menu.addItem(item)
        }
        
        menu.items.first!.state = .on

        do {
            let item = NSMenuItem(title: NSLocalizedString("Filter by:", comment: "Search field menu, first item, instructional"),
                                  action: nil,
                                  keyEquivalent: "")
            menu.insertItem(item, at: 0)
        }
        
        return menu
    }
    
    @objc func searchScopeDidChange(_ sender: NSMenuItem) {
        guard let menu = sender.menu else { return }
        menu.items.forEach { $0.state = .off }
        
        sender.state = .on
        if let keys = sender.representedObject as? [String] {
            searchKeys = keys
//            search(libraryController.toolbar?.searchField)
        }
    }
    
    @IBAction func search(_ sender: Any?) {
        guard let text = toolbar?.searchField.stringValue else { return }
        performSearch(text)
    }
    
    public func performSearch(_ text: String) {
        currentSearchTerm = text
        let tokens = currentSearchTerm.components(separatedBy: .whitespaces).filter { !$0.isEmpty }

        var searchKeyPredicates = [NSPredicate]()
        for key in searchKeys {
            let predicates = tokens.map {
                NSPredicate(format: "%K contains[cd] %@", key, $0)
            }
            searchKeyPredicates.append(NSCompoundPredicate(andPredicateWithSubpredicates: predicates))
        }

        if searchKeyPredicates.count > 0 {
            dataSourceDelegate.searchPredicate = NSCompoundPredicate(orPredicateWithSubpredicates: searchKeyPredicates)
        } else {
            dataSourceDelegate.searchPredicate = NSPredicate(value: true)
        }

        reloadData()
    }
}

extension NSUserInterfaceItemIdentifier {
    static let imageCollectionViewItemIdentifier        = NSUserInterfaceItemIdentifier(rawValue: "ImageCollectionViewItem")
    static let imageCollectionHeaderViewItemIdentifier  = NSUserInterfaceItemIdentifier(rawValue: "ImageCollectionHeaderView")
}

extension ImageCollectionViewController: NSCollectionViewDataSource {
    
    static var formatter: DateFormatter = {
        let formatter = DateFormatter()
        formatter.doesRelativeDateFormatting = true
        formatter.dateStyle = .medium
        formatter.timeStyle = .medium
        return formatter
    }()
    
    func numberOfSections(in collectionView: NSCollectionView) -> Int {
        dataSourceDelegate.numberOfSections
    }
    
    func collectionView(_ collectionView: NSCollectionView, numberOfItemsInSection section: Int) -> Int {
        dataSourceDelegate.numberOfItems(in: section)
    }
    
    func collectionView(_ collectionView: NSCollectionView, itemForRepresentedObjectAt indexPath: IndexPath) -> NSCollectionViewItem {
        let itemView = collectionView.makeItem(withIdentifier: .imageCollectionViewItemIdentifier, for: indexPath) as! ImageCollectionViewItem
        dataSourceDelegate.loadItemView(itemView, at: indexPath)
        return itemView
    }
    
    func collectionView(_ collectionView: NSCollectionView, viewForSupplementaryElementOfKind kind: NSCollectionView.SupplementaryElementKind, at indexPath: IndexPath) -> NSView {
        if kind == NSCollectionView.elementKindInterItemGapIndicator {
            return NSView(frame: NSRect(x: 0, y: 0, width: 1, height: 1))
        }
        
        precondition(kind == NSCollectionView.elementKindSectionHeader, "only headers are supported")
        
        let view = collectionView.makeSupplementaryView(ofKind: kind, withIdentifier: .imageCollectionHeaderViewItemIdentifier, for: indexPath)
        let headerView = view as! ImageCollectionHeaderView
        
        dataSourceDelegate.loadHeaderView(headerView, at: indexPath)
        
        return view
    }
    
    func reloadData() {
        precondition(Thread.isMainThread, "should only be called on main thread")
        
        // TODO: save selection,
        
        dataSourceDelegate.fetchItems()
        if dataSourceDelegate.isEmpty {
            shouldShowBlankSlate = true
        }
        
        collectionView.reloadData()
        updateBlankSlate()
    }
}

extension ImageCollectionViewController: NSCollectionViewDelegate {
    // MARK: - Handle selection

    private static let mediaKey = "_OESelectedMediaKey"
    
    func collectionView(_ collectionView: NSCollectionView, didSelectItemsAt indexPaths: Set<IndexPath>) {
        saveSelectionToDefaults()
        selectionDidChange()
        self.collectionView.refreshPreviewPanelIfNeeded()
    }
    
    func collectionView(_ collectionView: NSCollectionView, didDeselectItemsAt indexPaths: Set<IndexPath>) {
        saveSelectionToDefaults()
        selectionDidChange()
    }
    
    @objc func selectionDidChange() {}
    
    private func saveSelectionToDefaults() {
        var archivable = [Data]()
        
        for url in dataSourceDelegate.permanentIDURIs(forItemsAt: collectionView.selectionIndexPaths) {
            if let data = try? NSKeyedArchiver.archivedData(withRootObject: url, requiringSecureCoding: true) {
                archivable.append(data)
            }
        }
        
        let defaultsKey = "\(dataSourceDelegate.entityName)\(Self.mediaKey)"
        UserDefaults.standard.set(archivable, forKey: defaultsKey)
        
        collectionView.refreshPreviewPanelIfNeeded()
    }
    
    private func restoreSelectionFromDefaults() {
        let context = database.mainThreadContext
        guard let coordinator = context.persistentStoreCoordinator else { return }
        
        var set = Set<IndexPath>()
        let defaultsKey = "\(dataSourceDelegate.entityName)\(Self.mediaKey)"
        guard let archival = UserDefaults.standard.object(forKey: defaultsKey) as? [Data] else { return }
        
        for data in archival {
            guard
                let representation = try? NSKeyedUnarchiver.unarchiveTopLevelObjectWithData(data) as? NSURL,
                let objectID = coordinator.managedObjectID(forURIRepresentation: representation as URL)
                else { continue }
            
            guard let index = self.dataSourceDelegate.indexPath(forID: objectID) else { continue }
            set.insert(index)
        }
        
        collectionView.selectionIndexPaths = set
        if set.count > 0 {
            collectionView.scrollToItems(at: set, scrollPosition: .centeredVertically)
        }
    }
    
    // MARK: - Handle Outgoing Drag and Drop
    
    func collectionView(_ collectionView: NSCollectionView, canDragItemsAt indexPaths: Set<IndexPath>, with event: NSEvent) -> Bool {
        true
    }
    
    func collectionView(_ collectionView: NSCollectionView, pasteboardWriterForItemAt indexPath: IndexPath) -> NSPasteboardWriting? {
        dataSourceDelegate.imageURL(forItemAt: indexPath)?.absoluteURL as NSPasteboardWriting?
    }
    
    // MARK: - Handle Incoming Drag and Drop
    
    func collectionView(_ collectionView: NSCollectionView, acceptDrop draggingInfo: NSDraggingInfo, indexPath: IndexPath, dropOperation: NSCollectionView.DropOperation) -> Bool {
        let pboard = draggingInfo.draggingPasteboard
        if pboard.canReadObject(forClasses: [NSURL.self], options: nil) {
            guard let files = pboard.readObjects(forClasses: [NSURL.self], options: nil) as? [URL] else { return false }
            let romImporter = database.importer
            romImporter.importItems(at: files, intoCollectionWith: nil)
            return true
        }
        return false

    }
    
   
    func collectionView(_ collectionView: NSCollectionView, validateDrop draggingInfo: NSDraggingInfo, proposedIndexPath proposedDropIndexPath: AutoreleasingUnsafeMutablePointer<NSIndexPath>, dropOperation proposedDropOperation: UnsafeMutablePointer<NSCollectionView.DropOperation>) -> NSDragOperation {
        let pboard = draggingInfo.draggingPasteboard
        if pboard.canReadObject(forClasses: [NSURL.self], options: nil) {
            return .copy
        }
        return []
    }
}

// MARK: - Blank Slate Delegate

extension ImageCollectionViewController: OEBlankSlateViewDelegate {
    func blankSlateView(_ gridView: OEBlankSlateView!, validateDrop sender: NSDraggingInfo!) -> NSDragOperation {
        if let types = sender.draggingPasteboard.types, types.contains(.fileURL) {
            return .copy
        }
        
        return []
    }
    
    func blankSlateView(_ gridView: OEBlankSlateView!, acceptDrop sender: NSDraggingInfo!) -> Bool {
        let pboard = sender.draggingPasteboard
        if pboard.canReadObject(forClasses: [NSURL.self], options: nil) {
            guard let files = pboard.readObjects(forClasses: [NSURL.self], options: nil) as? [URL] else { return false }
            let romImporter = database.importer
            romImporter.importItems(at: files, intoCollectionWith: nil)
            return true
        }
        return false
    }
}

extension Key {
    static let lastGridSize: Key = "lastGridSize"
}

extension ImageCollectionViewController {
    @UserDefault(.lastGridSize, defaultValue: 1.0)
    static var lastGridSize: Float
}

