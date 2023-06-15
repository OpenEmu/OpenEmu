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

class ImageCollectionViewController: NSViewController {
    
    var database: OELibraryDatabase!
    
    @IBOutlet weak var collectionView: CollectionView!
    @IBOutlet weak var flowLayout: NSCollectionViewFlowLayout!
    let blankSlateView = BlankSlateView()
    
    var shouldShowBlankSlate = false
    var searchKeys = ["rom.game.gameTitle", "rom.game.name", "rom.game.system.lastLocalizedName", "name"]
    var currentSearchTerm = ""
    var itemSize = CGSize(width: 168, height: 143)
    
    private var isSelected: Bool {
        return view.superview != nil
    }
    
    var toolbar: LibraryToolbar? {
        view.window?.toolbar as? LibraryToolbar
    }
    
    var dataSourceDelegate: ImageDataSourceDelegate!
    
    override var representedObject: Any? {
        didSet {
            _ = self.view
        }
    }
    
    override var nibName: NSNib.Name? { "ImageCollectionViewController" }
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        itemSize = flowLayout.itemSize
        
        collectionView.registerForDraggedTypes([.fileURL])
        collectionView.setDraggingSourceOperationMask([], forLocal: true)
        collectionView.setDraggingSourceOperationMask(.copy, forLocal: false)

        blankSlateView.delegate = self
        blankSlateView.registerForDraggedTypes([.fileURL])
        
        if let context = OELibraryDatabase.default?.mainThreadContext {
            NotificationCenter.default.addObserver(self,
                                                   selector: #selector(managedObjectContextDidUpdate(_:)),
                                                   name: .NSManagedObjectContextDidSave,
                                                   object: context)
        }
        
        NotificationCenter.default.addObserver(self,
                                               selector: #selector(libraryLocationDidChange(_:)),
                                               name: .libraryLocationDidChange,
                                               object: nil)
        
        reloadData()
    }
    
    func validateToolbarItems() {
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
        
        if #available(macOS 11.0, *) {
            for item in toolbar.items {
                if item.itemIdentifier == .oeSearch {
                    item.isEnabled = !shouldShowBlankSlate
                }
            }
        }
    }
    
    @objc func libraryLocationDidChange(_ notification: Notification) {
        
    }
    
    @objc func managedObjectContextDidUpdate(_ notification: Notification) {
        DispatchQueue.main.async {
            self.scheduleUpdateViews()
        }
        if view.window == nil {
            collectionViewFrameSizeMightBeIncorrect = true
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
    
    private var collectionViewFrameSizeMightBeIncorrect = false
    
    override func viewWillAppear() {
        super.viewWillAppear()
        // Fixes mismatch between collectionViewContentSize and the collection view’s frame size.
        // If adding (or removing) items while another category is selected changes the number
        // of rows, the bottom area was clipped (or empty) because the size/height did not update.
        // _resizeToFitContentAndClipView() is called before viewWillAppear(), but is
        // "[…] returning without resize, due to empty clipView.bounds".
        if collectionViewFrameSizeMightBeIncorrect {
            let selector = #selector(NSCollectionView._resizeToFitContentAndClipView)
            if collectionView.responds(to: selector) {
                collectionView.perform(selector)
            }
            collectionViewFrameSizeMightBeIncorrect = false
        }
    }
    
    override func viewDidAppear() {
        super.viewDidAppear()
        
        if let slider = toolbar?.gridSizeSlider {
            slider.isContinuous = true
            slider.floatValue = Self.lastGridSize
            zoomGridView(zoomValue: CGFloat(slider.floatValue))
        }
        
        // update frame of the blank slate view (in viewDidLoad we didn't have a
        // window to check the toolbar height of).
        // TODO: change OE main window to not use full size content rect and remove
        blankSlateView.autoresizingMask = [.width, .height]
        let viewFrame = view.convert(view.bounds, to: nil)
        let nonOverlappingFrame = NSIntersectionRect(viewFrame, view.window!.contentLayoutRect)
        blankSlateView.frame = view.convert(nonOverlappingFrame, from: nil)
        
        validateToolbarItems()
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
    
    enum ViewTag {
        case collection
        case blankSlate
    }
    
    func updateBlankSlate() {
        if shouldShowBlankSlate {
            switchTo(tag: .blankSlate)
            blankSlateView.representedObject = representedObject
        } else {
            switchTo(tag: .collection)
        }
    }
    
    func switchTo(tag: ViewTag) {
        if isSelected {
//            validateToolbarItems()
        }
        
        show(tag: tag)
    }
    
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
        // required if implementing NSCollectionViewDelegateFlowLayout methods
        flowLayout.invalidateLayout()
    }
    
    private func searchMenuTemplate() -> NSMenu {
        let menu = NSMenu()
        
        let descriptors = [
            (
                "Everything",
                "Search field filter selection title",
                ["rom.game.gameTitle", "rom.game.name", "rom.game.system.lastLocalizedName", "name"]
            ),
            (
                "Name",
                "Search field filter selection title",
                ["name"]
            ),
//            (
//                "Description",
//                "Search field filter selection title",
//                ["userDescription"]
//            ),
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
            performSearch(currentSearchTerm)
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

private extension NSUserInterfaceItemIdentifier {
    static let imageCollectionViewItem       = NSUserInterfaceItemIdentifier("ImageCollectionViewItem")
    static let imageCollectionHeaderViewItem = NSUserInterfaceItemIdentifier("ImageCollectionHeaderView")
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
        let itemView = collectionView.makeItem(withIdentifier: .imageCollectionViewItem, for: indexPath) as! ImageCollectionViewItem
        dataSourceDelegate.loadItemView(itemView, at: indexPath)
        return itemView
    }
    
    func collectionView(_ collectionView: NSCollectionView, didEndDisplaying item: NSCollectionViewItem, forRepresentedObjectAt indexPath: IndexPath) {
        dataSourceDelegate.unloadItemView(at: indexPath)
    }
    
    func collectionView(_ collectionView: NSCollectionView, viewForSupplementaryElementOfKind kind: NSCollectionView.SupplementaryElementKind, at indexPath: IndexPath) -> NSView {
        if kind == NSCollectionView.elementKindInterItemGapIndicator {
            return NSView(frame: NSRect(x: 0, y: 0, width: 1, height: 1))
        }
        
        precondition(kind == NSCollectionView.elementKindSectionHeader, "only headers are supported")
        
        let view = collectionView.makeSupplementaryView(ofKind: kind, withIdentifier: .imageCollectionHeaderViewItem, for: indexPath)
        let headerView = view as! ImageCollectionHeaderView
        
        dataSourceDelegate.loadHeaderView(headerView, at: indexPath)
        
        return view
    }
    
    func reloadData() {
        precondition(Thread.isMainThread, "should only be called on main thread")
        
        dataSourceDelegate.fetchItems()
        shouldShowBlankSlate = dataSourceDelegate.isEmpty
        
        var selectedItemIDs = Set<NSManagedObjectID>()
        for indexPath in collectionView.selectionIndexPaths {
            if let item = dataSourceDelegate.item(at: indexPath) as? OEDBItem {
                selectedItemIDs.insert(item.objectID)
            }
        }
        
        collectionView.reloadData()
        
        var indexPaths = Set<IndexPath>()
        for s in 0..<collectionView.numberOfSections {
            for i in 0..<collectionView.numberOfItems(inSection: s) {
                indexPaths.insert(IndexPath(item: i, section: s))
            }
        }
        
        var selectedIndexPaths = Set<IndexPath>()
        for indexPath in indexPaths {
            if let item = dataSourceDelegate.item(at: indexPath) as? OEDBItem,
               selectedItemIDs.contains(item.objectID) {
                selectedIndexPaths.insert(indexPath)
            }
        }
        
        collectionView.selectItems(at: selectedIndexPaths, scrollPosition: [])
        
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
                let representation = try? NSKeyedUnarchiver.unarchivedObject(ofClass: NSURL.self, from: data),
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
    
    func collectionView(_ collectionView: NSCollectionView, draggingSession session: NSDraggingSession, willBeginAt screenPoint: NSPoint, forItemsAt indexPaths: Set<IndexPath>) {
        // Unhide the source views during drag operations per https://stackoverflow.com/a/59893117/12606
        indexPaths
            .compactMap(collectionView.item(at:))
            .forEach {
                $0.view.isHidden = false
        }
    }
    
    // MARK: - Handle Incoming Drag and Drop
    
    func collectionView(_ collectionView: NSCollectionView, acceptDrop draggingInfo: NSDraggingInfo, indexPath: IndexPath, dropOperation: NSCollectionView.DropOperation) -> Bool {
        let pboard = draggingInfo.draggingPasteboard
        if pboard.canReadObject(forClasses: [NSURL.self], options: nil) {
            guard let files = pboard.readObjects(forClasses: [NSURL.self], options: nil) as? [URL] else { return false }
            let romImporter = database.importer
            romImporter.importItems(at: files)
            return true
        }
        return false

    }
    
   
    func collectionView(_ collectionView: NSCollectionView, validateDrop draggingInfo: NSDraggingInfo, proposedIndexPath proposedDropIndexPath: AutoreleasingUnsafeMutablePointer<NSIndexPath>, dropOperation proposedDropOperation: UnsafeMutablePointer<NSCollectionView.DropOperation>) -> NSDragOperation {
        if let draggingSource = draggingInfo.draggingSource as? NSCollectionView,
           draggingSource == collectionView {
            // Drag came from self, so do nothing
            return []
        }
        
        let pboard = draggingInfo.draggingPasteboard
        if pboard.canReadObject(forClasses: [NSURL.self], options: nil) {
            return [.copy]
        }
        return []
    }
}

extension ImageCollectionViewController: NSCollectionViewDelegateFlowLayout {
    func collectionView(_ collectionView: NSCollectionView, layout collectionViewLayout: NSCollectionViewLayout, sizeForItemAt indexPath: IndexPath) -> NSSize {
        return flowLayout.itemSize
    }
    
    func collectionView(_ collectionView: NSCollectionView, layout collectionViewLayout: NSCollectionViewLayout, referenceSizeForHeaderInSection section: Int) -> NSSize {
        return NSSize(width: 0, height: 46)
    }
}

// MARK: - Blank Slate Delegate

extension ImageCollectionViewController: BlankSlateViewDelegate {
    func blankSlateView(_ blankSlateView: BlankSlateView, validateDrop sender: NSDraggingInfo) -> NSDragOperation {
        if let types = sender.draggingPasteboard.types, types.contains(.fileURL) {
            return .copy
        }
        
        return []
    }
    
    func blankSlateView(_ blankSlateView: BlankSlateView, acceptDrop sender: NSDraggingInfo) -> Bool {
        let pboard = sender.draggingPasteboard
        if pboard.canReadObject(forClasses: [NSURL.self], options: nil) {
            guard let files = pboard.readObjects(forClasses: [NSURL.self], options: nil) as? [URL] else { return false }
            let romImporter = database.importer
            romImporter.importItems(at: files)
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

