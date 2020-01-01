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
class LibraryGamesViewController: NSViewController {
    @objc
    weak public var libraryController: OELibraryController!
    
    @IBOutlet weak public var sidebarController: OESidebarController!
    @IBOutlet weak public var collectionController: OEGameCollectionViewController!
    @IBOutlet weak public var gameScannerController: GameScannerViewController!
    
    @IBOutlet weak public var collectionViewContainer: NSView!
    
    override var nibName: NSNib.Name? {
        return NSNib.Name("OELibraryGamesViewController")
    }
    
    override func awakeFromNib() {
        super.awakeFromNib()
        
        sidebarController.database = libraryController.database
        collectionController.libraryController = libraryController
        gameScannerController.libraryController = libraryController
    }
    
    @objc func showIssuesView(_ sender: Any?) {
        presentAsSheet(gameScannerController)
    }
    
    @objc func makeNewCollectionWithSelectedGames(_ sender: Any?) {
        precondition(Thread.isMainThread, "Only call on main thread")
        
        let collection = sidebarController.addCollection()
        collection.games = Set(selectedGames)
        collection.save()
        
        collectionController.setNeedsReload()
    }
    
    // MARK: - Overrides
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        let noc = NotificationCenter.default
        noc.addObserver(self, selector: #selector(updateCollectionContentsFromSidebar(_:)), name: Notification.Name(rawValue: OESidebarSelectionDidChangeNotificationName), object: sidebarController)
        
        let collectionView = collectionController.view
        collectionView.frame = collectionViewContainer.bounds
        collectionView.autoresizingMask = [.width, .height]
        collectionViewContainer.addSubview(collectionView)
        
        addChild(sidebarController)
        addChild(collectionController)
        
        updateCollectionContentsFromSidebar(nil)
    }
    
    override func viewWillAppear() {
        super.viewWillAppear()
        
        setupToolbar()
        
        view.needsDisplay = true
    }
    
    override func viewDidAppear() {
        super.viewDidAppear()
        collectionController.updateBlankSlate()
    }
    
    // MARK: - Private
    
    private func setupToolbar() {
        guard let toolbar = libraryController.toolbar else { return }
        
        toolbar.gridSizeSlider.isEnabled = true
        toolbar.viewSelector.isEnabled = true
        
        if collectionController.selectedViewTag == .gridViewTag {
            toolbar.viewSelector.selectedSegment = 0
        } else {
            toolbar.viewSelector.selectedSegment = 1
        }
        
        guard let field = toolbar.searchField else { return }
        field.searchMenuTemplate = nil
        field.isEnabled = false
        field.stringValue = ""
    }
    
    // MARK: - Toolbar
    
    @IBAction func addCollectionAction(_ sender: Any?) {
        sidebarController.addCollectionAction(sender)
    }
    
    enum MainMenuTag: Int {
        case grid = 301
        case list = 302
    }
    
    @IBAction func switchToGridView(_ sender: Any?) {
        collectionController.switchToGridView(sender)
        guard let viewMenu = NSApp.mainMenu?.item(at: 3)?.submenu else { return }
        viewMenu.item(withTag: MainMenuTag.grid.rawValue)?.state = .on
        viewMenu.item(withTag: MainMenuTag.list.rawValue)?.state = .off
    }
    
    @IBAction func switchToListView(_ sender: Any?) {
        collectionController.switchToListView(sender)
        guard let viewMenu = NSApp.mainMenu?.item(at: 3)?.submenu else { return }
        viewMenu.item(withTag: MainMenuTag.grid.rawValue)?.state = .off
        viewMenu.item(withTag: MainMenuTag.list.rawValue)?.state = .on
    }
    
    @IBAction func search(_ sender: Any?) {
        collectionController.search(sender)
    }
    
    @IBAction func changeGridSize(_ sender: Any?) {
        collectionController.changeGridSize(sender)
    }
    
    // MARK: - Sidebar Handling
    
    @objc func updateCollectionContentsFromSidebar(_ sender: Any?) {
        guard let selectedItem = sidebarController.selectedSidebarItem else { return }
        precondition(selectedItem is OECollectionViewItemProtocol, "does not implement protocol")
        collectionController.representedObject = (selectedItem as! OECollectionViewItemProtocol)
        
        if
            let system = selectedItem as? OEDBSystem,
            let plugin = system.plugin,
            let games = system.games,
            
            plugin.supportsDiscs && games.count == 0 && !Self.skipDiscGuideMessage {
            
            let alert = NSAlert()
            alert.messageText = NSLocalizedString("Have you read the guide?", comment: "")
            alert.informativeText = NSLocalizedString("Disc-based games have special requirements. Please read the disc importing guide.", comment: "")
            alert.alertStyle = .informational
            alert.addButton(withTitle: NSLocalizedString("View Guide in Browser", comment: ""))
            alert.addButton(withTitle: NSLocalizedString("Dismiss", comment: ""))
            
            alert.beginSheetModal(for: view.window!) { (returnCode) in
                guard returnCode == .alertFirstButtonReturn else { return }
                let guideURL = URL(string: OECDBasedGamesUserGuideURLString)!
                NSWorkspace.shared.open(guideURL)
            }
        }
    }
    
    // MARK: - Defaults
    
    @UserDefault("OESkipDiscGuideMessageKey", defaultValue: false)
    static var skipDiscGuideMessage: Bool
}

extension LibraryGamesViewController: OELibrarySubviewController {
    @objc var selectedGames: [OEDBGame]! {
        return collectionController.selectedGames
    }
}
