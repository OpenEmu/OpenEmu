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

protocol ImageDataSourceDelegate {
    var searchPredicate: NSPredicate { get set }
    var entityName: String { get }
    var isEmpty: Bool { get }
    var numberOfSections: Int { get }
    
    func numberOfItems(in section: Int) -> Int
    func loadItemView(_ view: ImageCollectionViewItem, at indexPath: IndexPath)
    func loadHeaderView(_ view: ImageCollectionHeaderView, at indexPath: IndexPath)
    func fetchItems()
    func indexPath(forID id: NSManagedObjectID) -> IndexPath?
    
    func permanentIDURIs(forItemsAt indexPaths: Set<IndexPath>) -> [URL]
    func imageURL(forItemAt indexPath: IndexPath) -> URL?
    func imageURLs(forItemsAt indexPaths: Set<IndexPath>) -> [URL]
}

class ImagesDataSource<Model: OEDBItem>: ImageDataSourceDelegate {
    
    var searchPredicate = NSPredicate(value: true)
    let entityName: String

    private var items = [[Model]]()
    private var indexPathByItems = [URL: IndexPath]()
    private var formatter: DateFormatter = {
        let formatter = DateFormatter()
        formatter.doesRelativeDateFormatting = true
        formatter.dateStyle = .medium
        formatter.timeStyle = .medium
        return formatter
    }()
    
    private let gameKeyPath: KeyPath<Model, OEDBGame?>
    private let titleKeyPath: KeyPath<Model, String?>
    private let timestampKeyPath: KeyPath<Model, Date?>
    private let imageURLKeyPath: KeyPath<Model, URL>
    private let sortDescriptors: [NSSortDescriptor]
    
    init(gameKeyPath: KeyPath<Model, OEDBGame?>,
         titleKeyPath: KeyPath<Model, String?>,
         timestampKeyPath: KeyPath<Model, Date?>,
         imageURLKeyPath: KeyPath<Model, URL>,
         sortDescriptors: [NSSortDescriptor],
         entityName: String) {
        
        self.gameKeyPath        = gameKeyPath
        self.titleKeyPath       = titleKeyPath
        self.timestampKeyPath   = timestampKeyPath
        self.imageURLKeyPath    = imageURLKeyPath
        self.sortDescriptors    = sortDescriptors
        self.entityName         = entityName
    }
    
    var isEmpty: Bool = true
    
    var numberOfSections: Int { items.count }
    
    func numberOfItems(in section: Int) -> Int { items[section].count }
    
    func loadItemView(_ view: ImageCollectionViewItem, at indexPath: IndexPath) {
        let item = items[indexPath.section][indexPath.item]
        view.representedObject = item
        let url = item[keyPath: imageURLKeyPath]
        view.imageURL = url
        ImageCacheService.shared.fetchImage(url) { (img) in
            if let existing = view.imageURL, existing != url {
                // ignore, if the view was recycled
                return
            }
            
            view.imageView?.image = img
            view.view.needsLayout = true
        }
        view.textField?.stringValue = item[keyPath: titleKeyPath] ?? ""
        if let ts = item[keyPath: timestampKeyPath] {
            view.subtitleField?.stringValue = formatter.string(from: ts)
        }
    }
    
    func loadHeaderView(_ view: ImageCollectionHeaderView, at indexPath: IndexPath) {
        let item = items[indexPath.section].first!
        let game = item[keyPath: gameKeyPath]!
        view.sectionTitle.stringValue = game.displayName!
        view.imageCount.stringValue = game.system!.name
    }
    
    func fetchItems() {
        guard let ctx = OELibraryDatabase.default?.mainThreadContext else { return }
        
        items = []
        indexPathByItems = [:]
        
        let req = NSFetchRequest<Model>(entityName: entityName)
        req.entity = NSEntityDescription.entity(forEntityName: entityName, in: ctx)
        
        let count = try? ctx.count(for: req)
        if count == .none || count! == 0 {
            isEmpty = true
            return
        }
        isEmpty = false
        
        req.sortDescriptors = sortDescriptors
        req.predicate       = searchPredicate
        
        guard let res = try? ctx.fetch(req) else { return }
        var i = res.startIndex
        while i < res.endIndex {
            let start = i
            let currentID = res[i][keyPath: gameKeyPath]?.objectID
            i += 1
            while i < res.endIndex {
                if currentID != res[i][keyPath: gameKeyPath]?.objectID {
                    break
                }
                i += 1
            }
            let group = Array(res[start..<i])
            let section = items.count
            group.enumerated().forEach { (offset, item) in
                indexPathByItems[item.permanentIDURI] = IndexPath(item: offset, section: section)
            }
            items.append(group)
        }
    }
    
    func indexPath(forID id: NSManagedObjectID) -> IndexPath? {
        indexPathByItems[id.uriRepresentation()]
    }
    
    func items(at indexPaths: Set<IndexPath>) -> [Model] {
        return indexPaths.map { items[$0.section][$0.item] }
    }
    
    func item(at indexPath: IndexPath) -> Model? {
        if indexPath.section >= items.endIndex {
            return nil
        }
        
        let group = items[indexPath.section]
        if indexPath.item >= group.endIndex {
            return nil
        }
        
        return group[indexPath.item]
    }
    
    func permanentIDURIs(forItemsAt indexPaths: Set<IndexPath>) -> [URL] {
        items(at: indexPaths).compactMap { $0.permanentIDURI }
    }
    
    func imageURL(forItemAt indexPath: IndexPath) -> URL? {
        item(at: indexPath)?[keyPath: imageURLKeyPath].absoluteURL as URL?
    }
    
    func imageURLs(forItemsAt indexPaths: Set<IndexPath>) -> [URL] {
        items(at: indexPaths).compactMap { $0[keyPath: imageURLKeyPath].absoluteURL }
    }
}
