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

@objc
class OEDBCollection: OEDBItem {
    
    // MARK: - CoreDataProperties
    
    @NSManaged var name: String
    @NSManaged var games: Set<OEDBGame>
    
    // MARK: - Data Model Relationships
    
    @objc var mutableGames: NSMutableSet {
        mutableSetValue(forKeyPath: #keyPath(games))
    }
    
    override class var entityName: String { "Collection" }
}

// MARK: - SidebarItem

extension OEDBCollection: SidebarItem {
    var sidebarIcon: NSImage? { NSImage(named: "collection_simple") }
    var sidebarName: String { name }
    var sidebarID: String? { permanentIDURI.absoluteString }
    var isEditableInSidebar: Bool { true }
    var hasSubCollections: Bool { false }
}

// MARK: - GameCollectionViewItemProtocol

extension OEDBCollection: GameCollectionViewItemProtocol {
    public var collectionViewName: String? { name }
    public var isCollectionEditable: Bool { true }
    public var shouldShowSystemColumnInListView: Bool { true }
    
    public var fetchPredicate: NSPredicate? {
        NSPredicate(format: "ANY collections == %@", self)
    }
    public var fetchLimit: Int { 0 }
    public var fetchSortDescriptors: [NSSortDescriptor]? { [] }
}
