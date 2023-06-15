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

final class LibraryController: NSTabViewController, NSMenuItemValidation {
    
    enum DefaultKeys: String {
        case lastCategory = "OELibraryLastCategoryKey"
    }
    
    enum Category: Int, CaseIterable {
        case games, saveStates, screenshots, homebrew
    }
    
    enum ViewMode: Int {
        case grid, list
    }
    
    var selectedCatagory: Category {
        get {
            Category(rawValue: selectedTabViewItemIndex) ?? .games
        }
        
        set {
            selectedTabViewItemIndex = newValue.rawValue
            toolbar.categorySelector.selectedSegment = newValue.rawValue
            
            UserDefaults.standard.set(newValue.rawValue, forKey: DefaultKeys.lastCategory.rawValue)
        }
    }
    
    override var nibName: NSNib.Name? { "LibraryController" }
    
    @objc weak var delegate: LibraryControllerDelegate?
    lazy var toolbar: LibraryToolbar = {
        let toolbar = LibraryToolbar(identifier: "OELibraryToolbar")
        toolbar.delegate = toolbarDelegate
        toolbar.displayMode = .iconOnly
        toolbar.centeredItemIdentifier = .oeCategory
        toolbar.allowsUserCustomization = true
        toolbar.autosavesConfiguration = true
        if #available(macOS 10.15, *) {} else {
            // avoids a crash on 10.14. (f0e12e8)
            // TODO: remove when 10.14 support goes away
            toolbar.allowsUserCustomization = false
        }
        return toolbar
    }()
    private lazy var toolbarDelegate: LibraryToolbarDelegate = {
        let toolbarDelegate = LibraryToolbarDelegate()
        toolbarDelegate.toolbarOwner = self
        return toolbarDelegate
    }()
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        //set initial zoom value
        let zoomValue = UserDefaults.standard.float(forKey: OELastGridSizeKey)
        toolbar.gridSizeSlider.floatValue = zoomValue
        
        tabView.autoresizingMask = [.width, .height]
        tabView.tabViewType = .noTabsNoBorder
        tabStyle = .unspecified
        transitionOptions = []
        setUpTabViews()
        // HACK: force NIB to load, so GameScannerViewController is initialized
        _ = tabViewItems[0].view
        
        if let category = Category(rawValue: UserDefaults.standard.integer(forKey: DefaultKeys.lastCategory.rawValue)) {
            selectedCatagory = category
        }
        
        newSaveStatesViewObserver = Self.$useNewSaveStatesViewController.observe { [weak self] _, newValue in
            guard
                let self = self,
                let useNew = newValue
            else { return }
            
            let idx = self.tabView.indexOfTabViewItem(withIdentifier: NSUserInterfaceItemIdentifier.savestatesViewController)
            let item = self.tabView.tabViewItem(at: idx)
            self.removeTabViewItem(item)
            if useNew {
                let ctrl = SaveStateViewController()
                ctrl.database = self.database
                ctrl.representedObject = OEDBSaveStatesMedia.shared
                let item = NSTabViewItem(identifier: NSUserInterfaceItemIdentifier.savestatesViewController)
                item.viewController = ctrl
                self.insertTabViewItem(item, at: idx)
            } else {
                let ctrl = OEMediaViewController()
                ctrl.database = self.database
                ctrl.saveStateMode = true
                ctrl.representedObject = OEDBSaveStatesMedia.shared
                let item = NSTabViewItem(identifier: NSUserInterfaceItemIdentifier.savestatesViewController)
                item.viewController = ctrl
                self.insertTabViewItem(item, at: idx)
            }
            self.selectedTabViewItemIndex = idx
        }
        
        newScreenshotsViewObserver = Self.$useNewScreenshotsViewController.observe { [weak self] _, newValue in
            guard
                let self = self,
                let useNew = newValue
            else { return }
            
            let idx = self.tabView.indexOfTabViewItem(withIdentifier: NSUserInterfaceItemIdentifier.screenshotsViewController)
            let item = self.tabView.tabViewItem(at: idx)
            self.removeTabViewItem(item)
            if useNew {
                let ctrl = ScreenshotViewController()
                ctrl.database = self.database
                ctrl.representedObject = OEDBScreenshotsMedia.shared
                let item = NSTabViewItem(identifier: NSUserInterfaceItemIdentifier.screenshotsViewController)
                item.viewController = ctrl
                self.insertTabViewItem(item, at: idx)
            } else {
                let ctrl = OEMediaViewController()
                ctrl.database = self.database
                ctrl.representedObject = OEDBScreenshotsMedia.shared
                let item = NSTabViewItem(identifier: NSUserInterfaceItemIdentifier.screenshotsViewController)
                item.viewController = ctrl
                self.insertTabViewItem(item, at: idx)
            }
            self.selectedTabViewItemIndex = idx
        }
    }
    
    var newSaveStatesViewObserver: Any?
    var newScreenshotsViewObserver: Any?
    
    override func tabView(_ tabView: NSTabView, didSelect tabViewItem: NSTabViewItem?) {
        super.tabView(tabView, didSelect: tabViewItem)
        
        toolbar.categorySelector.selectedSegment = selectedTabViewItemIndex
        view.window?.makeFirstResponder(tabViewItem?.view)
    }
    
    private func setUpTabViews() {
        do {
            let ctrl = LibraryGamesViewController()
            ctrl.database = database
            let item = NSTabViewItem(identifier: "org.openemu.category.library")
            item.viewController = ctrl
            addTabViewItem(item)
        }
        
        if Self.useNewSaveStatesViewController {
            do {
                let ctrl = SaveStateViewController()
                ctrl.database = database
                ctrl.representedObject = OEDBSaveStatesMedia.shared
                let item = NSTabViewItem(identifier: NSUserInterfaceItemIdentifier.screenshotsViewController)
                item.viewController = ctrl
                addTabViewItem(item)
            }
        } else {
            do {
                let ctrl = OEMediaViewController()
                ctrl.database = database
                ctrl.saveStateMode = true
                ctrl.representedObject = OEDBSaveStatesMedia.shared
                let item = NSTabViewItem(identifier: NSUserInterfaceItemIdentifier.savestatesViewController)
                item.viewController = ctrl
                addTabViewItem(item)
            }
        }
        
        if Self.useNewScreenshotsViewController {
            do {
                let ctrl = ScreenshotViewController()
                ctrl.database = database
                ctrl.representedObject = OEDBScreenshotsMedia.shared
                let item = NSTabViewItem(identifier: NSUserInterfaceItemIdentifier.screenshotsViewController)
                item.viewController = ctrl
                addTabViewItem(item)
            }
        } else {
            do {
                let ctrl = OEMediaViewController()
                ctrl.database = database
                ctrl.representedObject = OEDBScreenshotsMedia.shared
                let item = NSTabViewItem(identifier: NSUserInterfaceItemIdentifier.screenshotsViewController)
                item.viewController = ctrl
                addTabViewItem(item)
            }
        }
        do {
            let ctrl = HomebrewViewController()
            ctrl.database = database
            let item = NSTabViewItem(identifier: "org.openemu.category.homebrew")
            item.viewController = ctrl
            addTabViewItem(item)
        }
    }
    
    override func viewDidAppear() {
        super.viewDidAppear()
        
        toolbar.categorySelector.selectedSegment = selectedCatagory.rawValue
    }
    
    override func viewWillDisappear() {
        super.viewWillAppear()
        
        if let container = toolbar.searchField.superview {
            container.autoresizingMask = .width
        }
    }
    
    // MARK: - Toolbar Actions
    
    private func tryForwardAction(sel: Selector, with sender: Any?) {
        if let ctl = tabView.selectedTabViewItem?.viewController, ctl.responds(to: sel) {
            ctl.perform(sel, with: sender)
        }
    }
    
    @IBAction func switchToView(_ sender: Any?) {
        switch toolbar.viewModeSelector.selectedSegment {
        case 0:
            switchToGridView(sender)
        case 1:
            switchToListView(sender)
        default:
            break
        }
    }
    
    @IBAction func switchToGridView(_ sender: Any?) {
        tryForwardAction(sel: #selector(switchToGridView(_:)), with: sender)
    }
    
    @IBAction func switchToListView(_ sender: Any?) {
        tryForwardAction(sel: #selector(switchToListView(_:)), with: sender)
    }
    
    @IBAction func search(_ sender: Any?) {
        tryForwardAction(sel: #selector(search(_:)), with: sender)
    }
    
    @IBAction func changeGridSize(_ sender: Any?) {
        tryForwardAction(sel: #selector(changeGridSize(_:)), with: sender)
    }
    
    @IBAction func decreaseGridSize(_ sender: Any?) {
        tryForwardAction(sel: #selector(decreaseGridSize(_:)), with: sender)
    }
    
    @IBAction func increaseGridSize(_ sender: Any?) {
        tryForwardAction(sel: #selector(increaseGridSize(_:)), with: sender)
    }
    
    override func magnify(with event: NSEvent) {
        guard toolbar.gridSizeSlider.isEnabled else { return }
        
        if let ctl = tabView.selectedTabViewItem?.viewController, !ctl.responds(to: #selector(changeGridSize(_:))) {
            return
        }
        
        let zoomChange = Float(event.magnification)
        let zoomValue = toolbar.gridSizeSlider.floatValue
        
        toolbar.gridSizeSlider.floatValue = zoomValue + zoomChange
        changeGridSize(toolbar.gridSizeSlider)
    }
    
    // MARK: - File Menu Item Actions
    
    @IBAction func newCollection(_ sender: Any?) {
        tryForwardAction(sel: #selector(newCollection(_:)), with: sender)
    }
    
    @IBAction func newSmartCollection(_ sender: Any?) {
        // TODO: implement
    }
    
    @IBAction func newCollectionFolder(_ sender: Any?) {
        // TODO: implement
    }
    
    @IBAction func editSmartCollection(_ sender: Any?) {
        // TODO: implement
    }
    
    // MARK: - Edit Menu
    
    @IBAction func find(_ sender: Any?) {
        view.window?.makeFirstResponder(toolbar.searchField)
    }
    
    // MARK: - Categories
    
    @objc var currentSubviewController: NSViewController? {
        return tabView.selectedTabViewItem?.viewController
    }
    
    private func switchCategoryFromToolbar() {
        guard
            let category = Category(rawValue: toolbar.categorySelector.selectedSegment)
        else { return }
        
        if category == selectedCatagory {
            return
        }
        
        selectedCatagory = category
    }
    
    @IBAction func switchCategory(_ sender: Any?) {
        switchCategoryFromToolbar()
    }
    
    @IBAction func switchCategoryFromMenu(_ sender: NSMenuItem) {
        guard let category = Category(rawValue: sender.tag - 100) else { return }
        selectedCatagory = category
    }
    
    // MARK: - Validation
    
    func validateMenuItem(_ menuItem: NSMenuItem) -> Bool {
        guard let currentSubviewController = tabView.selectedTabViewItem?.viewController else { return false }
        switch menuItem.action {
        case #selector(startSelectedGame(_:)):
            if let ctl = currentSubviewController as? LibrarySubviewControllerGameSelection {
                return ctl.selectedGames.count == 1
            }
            return false
        case #selector(find(_:)):
            return toolbar.searchField.isEnabled
        case #selector(switchCategoryFromMenu(_:)):
            menuItem.state = (toolbar.categorySelector.selectedSegment == menuItem.tag - 100) ? .on : .off
            return true
        case #selector(decreaseGridSize(_:)),
             #selector(increaseGridSize(_:)):
            return self.toolbar.gridSizeSlider.isEnabled
        case #selector(switchToGridView(_:)):
            menuItem.state = toolbar.viewModeSelector.selectedSegment == 0 ? .on : .off
            return toolbar.viewModeSelector.isEnabled
        case #selector(switchToListView(_:)):
            menuItem.state = toolbar.viewModeSelector.selectedSegment == 1 ? .on : .off
            return toolbar.viewModeSelector.isEnabled
        default:
            return true
        }
    }
    
    // MARK: - Import
    
    @IBAction func addToLibrary(_ sender: Any?) {
        guard let win = view.window else { return }
        
        let openPanel = NSOpenPanel()
        openPanel.allowsMultipleSelection = true
        openPanel.canChooseFiles = true
        openPanel.canCreateDirectories = false
        openPanel.canChooseDirectories = true
        
        openPanel.beginSheetModal(for: win) { result in
            if result == .OK {
                self.database.importer.importItems(at: openPanel.urls)
            }
        }
    }
    
    // MARK: -
    
    @IBAction func startSelectedGame(_ sender: Any?) {
        var gamesToStart = [OEDBGame]()
        
        if let game = sender as? OEDBGame {
            gamesToStart.append(game)
        } else {
            guard
                let ctl = tabView.selectedTabViewItem?.viewController as? LibrarySubviewControllerGameSelection
            else { return }
            
            gamesToStart.append(contentsOf: ctl.selectedGames)
        }
        
        precondition(gamesToStart.count > 0, "Attempt to start a game while the selection is empty")
        
        guard let delegate = delegate else { return }
        
        for game in gamesToStart {
            delegate.libraryController?(self, didSelectGame: game)
        }
    }
    
    @objc(startGame:)
    func start(game: OEDBGame) {
        delegate?.libraryController?(self, didSelectGame: game)
    }
    
    @IBAction func startSaveState(_ sender: Any?) {
        guard
            let ctl = tabView.selectedTabViewItem?.viewController as? LibrarySubviewControllerSaveStateSelection,
            let game = ctl.selectedSaveStates.first
        else { return }
        
        start(saveState: game)
    }
    
    func start(saveState: OEDBSaveState) {
        delegate?.libraryController?(self, didSelectSaveState: saveState)
    }
    
    @objc(startSelectedGameWithSaveState:)
    @IBAction func startSelectedGame(saveState sender: NSMenuItem) {
        guard let saveState = sender.representedObject as? OEDBSaveState else {
            fatalError("Attempt to start a save state without valid item")
        }
        
        delegate?.libraryController?(self, didSelectSaveState: saveState)
    }
    
    // MARK: Properties
    @objc lazy var database: OELibraryDatabase = {
        return OELibraryDatabase.default!
    }()
}

private extension NSUserInterfaceItemIdentifier {
    static let savestatesViewController = NSUserInterfaceItemIdentifier("org.openemu.category.savestates")
    static let screenshotsViewController = NSUserInterfaceItemIdentifier("org.openemu.category.screenshots")
}

extension Key {
    static let useNewSaveStatesViewController: Key = "useNewSaveStatesViewController"
    static let useNewScreenshotsViewController: Key = "useNewScreenshotsViewController"
}

extension LibraryController {
    @UserDefault(.useNewSaveStatesViewController, defaultValue: false)
    static var useNewSaveStatesViewController: Bool
    @UserDefault(.useNewScreenshotsViewController, defaultValue: false)
    static var useNewScreenshotsViewController: Bool
}


// MARK: - Protocols

@objc(OELibraryControllerDelegate)
protocol LibraryControllerDelegate {
    @objc optional func libraryController(_ controller: LibraryController, didSelectGame: OEDBGame)
    @objc optional func libraryController(_ controller: LibraryController, didSelectRom: OEDBRom)
    @objc optional func libraryController(_ controller: LibraryController, didSelectSaveState: OEDBSaveState)
}

@objc(OELibrarySubviewControllerGameSelection)
protocol LibrarySubviewControllerGameSelection {
    var selectedGames: [OEDBGame] { get }
}

@objc(OELibrarySubviewControllerSaveStateSelection)
protocol LibrarySubviewControllerSaveStateSelection {
    var selectedSaveStates: [OEDBSaveState] { get }
}
