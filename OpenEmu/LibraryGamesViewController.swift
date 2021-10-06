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

extension NSNotification.Name {
    static let OELibrarySplitViewResetSidebar = NSNotification.Name("OELibrarySplitViewResetSidebar")
}

final class LibraryGamesViewController: NSSplitViewController {
    
    private let OESkipDiscGuideMessageKey = "OESkipDiscGuideMessageKey"
    private lazy var discGuideMessageSystemIDs: [String?] = []
    
    private let sidebarMinWidth: CGFloat = 105
    private let sidebarDefaultWidth: CGFloat = 200
    private let sidebarMaxWidth: CGFloat = 450
    private let collectionViewMinWidth: CGFloat = 495
    
    private var sidebarController: SidebarController!
    private var collectionController: OEGameCollectionViewController!
    
    private var toolbar: LibraryToolbar? {
        view.window?.toolbar as? LibraryToolbar
    }
    
    var database: OELibraryDatabase?
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        sidebarController = SidebarController()
        collectionController = OEGameCollectionViewController()
        
        setupSplitView()
        assignDatabase()
        
        NotificationCenter.default.addObserver(self, selector: #selector(updateCollectionContentsFromSidebar), name: .OESidebarSelectionDidChange, object: sidebarController)
    }
    
    override func viewWillAppear() {
        super.viewWillAppear()
        
        setupSplitViewAutosave()
        updateCollectionContentsFromSidebar()
        
        view.needsDisplay = true
    }
    
    override func viewDidAppear() {
        super.viewDidAppear()
        
        collectionController.updateBlankSlate()
        
        if #available(macOS 11.0, *) {
            view.window?.titlebarSeparatorStyle = .automatic
        }
    }
    
    override func viewWillDisappear() {
        super.viewWillDisappear()
        
        if #available(macOS 11.0, *) {
            view.window?.titlebarSeparatorStyle = .line
        }
    }
    
    private func assignDatabase() {
        sidebarController.database = database
        collectionController.database = database
    }
    
    // MARK: - Split View
    
    private func setupSplitView() {
        
        splitView.translatesAutoresizingMaskIntoConstraints = false
        
        let sidebarItem = NSSplitViewItem(sidebarWithViewController: sidebarController)
        sidebarItem.minimumThickness = sidebarMinWidth
        sidebarItem.maximumThickness = sidebarMaxWidth
        sidebarItem.canCollapse = false
        if #available(macOS 11.0, *) {
            sidebarItem.titlebarSeparatorStyle = .automatic
        }
        addSplitViewItem(sidebarItem)
        
        let collectionItem = NSSplitViewItem(viewController: collectionController)
        collectionItem.minimumThickness = collectionViewMinWidth
        if #available(macOS 11.0, *) {
            collectionItem.titlebarSeparatorStyle = .line
        }
        addSplitViewItem(collectionItem)
    }
    
    private func setupSplitViewAutosave() {
        
        if splitView.autosaveName != nil && !(splitView.autosaveName == "") {
            return
        }
        
        let autosaveName = "OELibraryGamesSplitView"
        
        if UserDefaults.standard.object(forKey: "NSSplitView Subview Frames \(autosaveName)") == nil {
            splitView.setPosition(sidebarDefaultWidth, ofDividerAt: 0)
        }
        
        splitView.autosaveName = autosaveName
        
        NotificationCenter.default.addObserver(self, selector: #selector(resetSidebar), name: .OELibrarySplitViewResetSidebar, object: nil)
    }
    
    @objc func resetSidebar() {
        splitView.setPosition(sidebarDefaultWidth, ofDividerAt: 0)
    }
    
    // MARK: - Actions
    
    @IBAction func newCollection(_ sender: AnyObject?) {
        sidebarController.newCollection(sender)
    }
    
    @IBAction func selectSystems(_ sender: Any?) {
        sidebarController.selectSystems(sender)
    }
    
    @IBAction func switchToGridView(_ sender: Any?) {
        collectionController.showGridView()
    }
    
    @IBAction func switchToListView(_ sender: Any?) {
        collectionController.showListView()
    }
    
    @IBAction func search(_ sender: NSSearchField?) {
        guard let searchField = sender else { return }
        collectionController.performSearch(searchField.stringValue)
    }
    
    @IBAction func changeGridSize(_ sender: NSSlider?) {
        guard let slider = toolbar?.gridSizeSlider else { return }
        collectionController.zoomGridView(withValue: CGFloat(slider.doubleValue))
    }
    
    @IBAction func decreaseGridSize(_ sender: AnyObject?) {
        guard let slider = toolbar?.gridSizeSlider else { return }
        slider.doubleValue -= sender?.tag == 199 ? 0.25 : 0.5
        collectionController.zoomGridView(withValue: CGFloat(slider.doubleValue))
    }
    
    @IBAction func increaseGridSize(_ sender: AnyObject?) {
        guard let slider = toolbar?.gridSizeSlider else { return }
        slider.doubleValue += sender?.tag == 199 ? 0.25 : 0.5
        collectionController.zoomGridView(withValue: CGFloat(slider.doubleValue))
    }
    
    @objc func updateCollectionContentsFromSidebar() {
        
        let selectedItem = sidebarController.selectedSidebarItem
        collectionController.representedObject = selectedItem as? GameCollectionViewItemProtocol
        
        // For empty collections of disc-based games, display an alert to compel the user to read the disc-importing guide.
        DispatchQueue.main.asyncAfter(deadline: .now() + .milliseconds(200)) {
            guard self.sidebarController.selectedSidebarItem === selectedItem else { return }
            
            if let system = selectedItem as? OEDBSystem,
               system.plugin?.supportsDiscsWithDescriptorFile ?? false,
               system.games?.count == 0,
               !self.discGuideMessageSystemIDs.contains(system.systemIdentifier),
               !UserDefaults.standard.bool(forKey: self.OESkipDiscGuideMessageKey),
               let window = self.view.window {
                
                let alert = NSAlert()
                alert.messageText = NSLocalizedString("Have you read the guide?", comment: "")
                alert.informativeText = NSLocalizedString("Disc-based games have special requirements. Please read the disc importing guide.", comment: "")
                alert.alertStyle = .informational
                alert.addButton(withTitle: NSLocalizedString("View Guide in Browser", comment: ""))
                alert.addButton(withTitle: NSLocalizedString("Dismiss", comment: ""))
                
                alert.beginSheetModal(for: window) { result in
                    if result == .alertFirstButtonReturn {
                        NSWorkspace.shared.open(.userGuideCDBasedGames)
                    }
                }
                
                self.discGuideMessageSystemIDs.append(system.systemIdentifier)
            }
        }
    }
    
    @objc func makeNewCollectionWithSelectedGames(_ sender: Any?) {
        assert(Thread.isMainThread, "Only call on main thread!")
        
        sidebarController.newCollection(games: selectedGames)
    }
}

extension LibraryGamesViewController: LibrarySubviewControllerGameSelection {
    
    var selectedGames: [OEDBGame] {
        collectionController.selectedGames
    }
}

// MARK: - Validation

extension LibraryGamesViewController: NSMenuItemValidation {
    
    func validateMenuItem(_ menuItem: NSMenuItem) -> Bool {
        let isGridView = collectionController.selectedViewTag == .gridViewTag
        let isBlankSlate = collectionController.shouldShowBlankSlate()
        
        switch menuItem.action {
        case #selector(switchToGridView):
            menuItem.state = isGridView ? .on : .off
            return !isBlankSlate
        case #selector(switchToListView) :
            menuItem.state = !isGridView ? .on : .off
            return !isBlankSlate
        default:
            return true
        }
    }
}
