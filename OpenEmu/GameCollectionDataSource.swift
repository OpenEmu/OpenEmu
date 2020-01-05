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

import Foundation

class GameCollectionDataSource: ImageDataSourceDelegate {
    var searchPredicate = NSPredicate(value: true)
    let entityName: String = OEDBGame.entityName()
    var isEmpty: Bool = true
    var numberOfSections: Int = 1
    
    var collection: OEGameCollectionViewItemProtocol?
    
    let items = ArrayController()
    
    var games: [OEDBGame] {
        return items.arrangedObjects as? [OEDBGame] ?? []
    }
    
    func numberOfItems(in section: Int) -> Int {
        precondition(section == 0)
        
        return games.count
    }
    
    func loadItemView(_ view: ImageCollectionViewItem, at indexPath: IndexPath) {
        let item = games[indexPath.item]
        view.representedObject = item
        
        // TODO: add place holder image
        let img = item.imageRepresentation()
        let iid = item.imageUID()
        // if iid starts with ":", it is a missing image
        if let url = item.boxImage?.imageURL {
            view.imageURL = url
            ImageCacheService.shared.fetchImage(url) { (img) in
                if let existing = view.imageURL, existing != url {
                    // ignore, if the view was recycled
                    return
                }
                
                view.imageView?.image = img
                view.view.needsLayout = true
            }
        }
        
        view.textField?.stringValue = item.displayName ?? ""
        
        // TODO: add rating
    }
    
    func loadHeaderView(_ view: ImageCollectionHeaderView, at indexPath: IndexPath) {
        
    }
    
    func fetchItems() {
        guard let collection = collection else { return }
        
        items.fetchPredicate = collection.fetchPredicate() ?? NSPredicate(value: false)
        items.limit = collection.fetchLimit()
        items.fetchSortDescriptors = collection.fetchSortDescriptors
        
        try? items.fetch(with: nil, merge: false)
        
        isEmpty = games.count == 0
    }
    
    func indexPath(forID id: NSManagedObjectID) -> IndexPath? {
        return nil
    }
    
    func permanentIDURIs(forItemsAt indexPaths: Set<IndexPath>) -> [URL] {
        return []
    }
    
    func imageURL(forItemAt indexPath: IndexPath) -> URL? {
        return nil
    }
    
    func imageURLs(forItemsAt indexPaths: Set<IndexPath>) -> [URL] {
        return []
    }
    
    // MARK: - Extra
    
    func item(at row: Int) -> OEDBGame? {
        guard row < numberOfItems(in: 0) else { return nil }
        
        return games[row]
    }
}
