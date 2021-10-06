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
class OEDBSmartCollection: OEDBCollection {
    
    override class var entityName: String { "SmartCollection" }
    
    private var isRecentlyAddedCollection: Bool {
        return name == "Recently Added"
    }
    
    // MARK: - SidebarItem
    
    override var sidebarIcon: NSImage? { NSImage(named: "collection_smart") }
    override var sidebarName: String {
        isRecentlyAddedCollection ? NSLocalizedString("Recently Added", comment: "Smart Collection Name")
                                  : name
    }
    override var isEditableInSidebar: Bool { false }
    
    // MARK: - GameCollectionViewItemProtocol
    
    public override var collectionViewName: String? {
        isRecentlyAddedCollection ? NSLocalizedString("Recently Added", comment: "Smart Collection Name")
                                  : name
    }
    public override var isCollectionEditable: Bool { false }
    public override var shouldShowSystemColumnInListView: Bool { true }
    
    public override var fetchPredicate: NSPredicate? {
        isRecentlyAddedCollection ? NSPredicate(value: true)
                                  : NSPredicate(value: false)
    }
    public override var fetchLimit: Int { 30 }
    public override var fetchSortDescriptors: [NSSortDescriptor]? {
        isRecentlyAddedCollection ? [NSSortDescriptor(key: "importDate", ascending: false)]
                                  : []
    }
}
