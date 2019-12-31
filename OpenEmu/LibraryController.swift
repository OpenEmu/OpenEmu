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

import Foundation

@objc(OELibraryController)
class OELibraryController: NSViewController {
    
    enum DefaultKeys: String {
        case libraryStates = "Library States"
        case lastCategory = "OELibraryLastCategoryKey"
    }
    
    enum Category: Int, CaseIterable {
        case games, saveStates, screenshots, homebrew
    }
    
    enum ViewMode: Int {
        case grid, list
    }
    
    @objc weak var delegate: LibraryControllerDelegate?
    
    override var nibName: NSNib.Name? {
        return NSNib.Name("OELibraryController")
    }
    
    override func viewDidLoad() {
        super.viewDidLoad()
        setUpCategoryViewControllers()
        let last = UserDefaults.standard.integer(forKey: DefaultKeys.lastCategory.rawValue)
        selectedCatagory = Category(rawValue: last) ?? .games
        showSubviewController(category: selectedCatagory)
    }
    
    var selectedCatagory: Category = .games
    
    func setUpCategoryViewControllers()
    {
        libraryGamesViewController = OELibraryGamesViewController()
        libraryGamesViewController.libraryController = self
        
        #if !USE_NEW
        saveStatesViewController = OEMediaViewController()
        saveStatesViewController.libraryController = self
        saveStatesViewController.representedObject = OEDBSavedGamesMedia.shared

        screenshotsViewController = OEMediaViewController()
        screenshotsViewController.libraryController = self
        screenshotsViewController.representedObject = OEDBScreenshotsMedia.shared
        #else
        saveStatesViewController2 = SaveStateViewController()
        saveStatesViewController2.libraryController = self;
        saveStatesViewController2.representedObject = OEDBSavedGamesMedia.shared;

        screenshotsViewController2 = ScreenshotViewController()
        screenshotsViewController2.libraryController = self;
        screenshotsViewController2.representedObject = OEDBScreenshotsMedia.shared;
        #endif
        
        homebrewViewController = OEHomebrewViewController();
        homebrewViewController.libraryController = self;
        
        addChild(libraryGamesViewController)
        #if !USE_NEW
        addChild(saveStatesViewController)
        addChild(screenshotsViewController)
        #else
        addChild(saveStatesViewController2)
        addChild(screenshotsViewController2)
        #endif
        addChild(homebrewViewController)
    }
    
    override func viewDidAppear() {
        super.viewDidAppear()
        
        toolbar?.categorySelector.selectedSegment = selectedCatagory.rawValue
    }
    
    override func viewWillDisappear() {
        super.viewWillDisappear()
        
        if let container = toolbar?.searchField.superview {
            container.autoresizingMask = .width
        }
    }
    
    // MARK: - Toolbar Actions
    @IBAction func addCollectionAction(_ sender: Any?) {
        if let ctl = currentSubviewController, ctl.responds(to: #selector(addCollectionAction(_:))) {
            ctl.perform(#selector(addCollectionAction(_:)), with: sender)
        }
    }
    
    @IBAction func switchViewMode(_ sender: Any?) {
        switch ViewMode(rawValue: toolbar!.viewSelector.selectedSegment) {
        case .grid:
            if let ctl = currentSubviewController, ctl.responds(to: #selector(OECollectionViewController.switchToGridView(_:))) {
                ctl.perform(#selector(OECollectionViewController.switchToGridView(_:)), with: sender)
            }
        case .list:
            if let ctl = currentSubviewController, ctl.responds(to: #selector(OECollectionViewController.switchToListView(_:))) {
                ctl.perform(#selector(OECollectionViewController.switchToListView(_:)), with: sender)
            }
        default:
            return
        }
    }
    
    @IBAction func search(_ sender: Any?) {
        if let ctl = currentSubviewController, ctl.responds(to: #selector(search(_:))) {
            ctl.perform(#selector(search(_:)), with: sender)
        }
    }
    
    @IBAction func changeGridSize(_ sender: Any?) {
        if let ctl = currentSubviewController, ctl.responds(to: #selector(changeGridSize(_:))) {
            ctl.perform(#selector(changeGridSize(_:)), with: sender)
        }
    }
    
    override func magnify(with event: NSEvent) {
        if !toolbar!.gridSizeSlider.isEnabled {
            return
        }
        
        guard let ctl = currentSubviewController, ctl.responds(to: #selector(changeGridSize(_:))) else {
            return
        }
        
        let zoomChange = Float(event.magnification)
        let zoomValue = toolbar!.gridSizeSlider.floatValue

        toolbar!.gridSizeSlider.floatValue = zoomValue + zoomChange
        changeGridSize(toolbar!.gridSizeSlider)
    }
    
    // MARK: - File Menu Item Actions
    @IBAction func newCollection(_ sender: Any?) {
        if let ctl = currentSubviewController, ctl.responds(to: #selector(addCollectionAction(_:))) {
            ctl.perform(#selector(addCollectionAction(_:)), with: sender)
        }
    }
    
    @IBAction func newSmartCollection(_ sender: Any?) {
        
    }
    
    @IBAction func newCollectionFolder(_ sender: Any?) {
        
    }
    
    @IBAction func editSmartCollection(_ sender: Any?) {
        NSLog("Edit smart collection: ")
    }
    
    // MARK: - Edit Menu
    
    @IBAction func find(_ sender: Any?) {
        view.window?.makeFirstResponder(toolbar?.searchField)
    }
    
    // MARK: - Categories
    
    @IBAction func switchCategory(_ sender: Any?) {
        guard let category = Category(rawValue: toolbar!.categorySelector.selectedSegment) else {
            return
        }
        
        if category == selectedCatagory {
            return
        }
        
        showSubviewController(category: category)
        
        selectedCatagory = category
        
        UserDefaults.standard.set(category.rawValue, forKey: DefaultKeys.lastCategory.rawValue)
    }
    
    func subviewController(category: Category) -> (NSViewController & OELibrarySubviewController) {
        switch category {
        case .games:
            return libraryGamesViewController
            
        #if !USE_NEW
        case .saveStates:
            return saveStatesViewController

        case .screenshots:
            return screenshotsViewController
        #else
        case .saveStates:
            return saveStatesViewController2

        case .screenshots:
            return screenshotsViewController2
        #endif
            
        case .homebrew:
            return homebrewViewController
        }
    }
    
    func showSubviewController(category: Category) {
        let crossfadeDuration = TimeInterval(0.35)
        let newViewController = subviewController(category: category)
        
        newViewController.view.frame = view.bounds
        newViewController.view.autoresizingMask = [.width, .height]
        
        if let current = currentSubviewController {
            NSAnimationContext.beginGrouping()
            NSAnimationContext.current.duration = crossfadeDuration
            transition(from: current, to: newViewController, options: .crossfade, completionHandler: nil)
            NSAnimationContext.endGrouping()
        } else {
            view.addSubview(newViewController.view)
        }

        currentSubviewController = newViewController
        view.window?.makeFirstResponder(newViewController.view)
    }
    
    // MARK: -
    
    @objc(validateMenuItem:)
    func validate(menuItem: NSMenuItem) -> Bool {
        guard let sel = menuItem.action else { return false }
        
        switch sel {
        case #selector(startSelectedGame(_:)):
            if let ctl = currentSubviewController as? OELibraryGamesViewController, ctl.selectedGames.count > 0 {
                return true
            }
            
            if let ctl = currentSubviewController as? OEMediaViewController, ctl.representedObject() as? OEDBSavedGamesMedia != nil && ctl.selectedSaveStates.count > 0 {
                return true
            }
            
            return false
            
        case #selector(find(_:)):
            return toolbar?.searchField.isEnabled ?? false
            
        default:
            break
        }
        
        if let mode = ViewMode(rawValue: toolbar!.viewSelector.selectedSegment) {
            var state: NSControl.StateValue
            switch sel {
            case #selector(OECollectionViewController.switchToGridView(_:)):
                state = mode == .grid ? NSControl.StateValue.on : NSControl.StateValue.off
            case #selector(OECollectionViewController.switchToListView(_:)):
                state = mode == .list ? NSControl.StateValue.on : NSControl.StateValue.off
            default:
                return true
            }
            
            menuItem.state = state
        }
        
        return toolbar!.viewSelector.isEnabled
    }
    
    
    @IBAction func addToLibrary(_ sender: Any?) {
        guard let win = view.window else { return }
        
        let openPanel = NSOpenPanel()
        openPanel.allowsMultipleSelection = true
        openPanel.canChooseFiles = true
        openPanel.canCreateDirectories = false
        openPanel.canChooseDirectories = true
        
        openPanel.beginSheetModal(for: win) { result in
            if result == .OK {
                self.database.importer.importItems(atURLs: openPanel.urls)
            }
        }
    }

    @IBAction func startSelectedGame(_ sender: Any?) {
        var gamesToStart = [OEDBGame]()
        
        if let game = sender as? OEDBGame {
            gamesToStart.append(game)
        } else {
            guard
                let ctl = currentSubviewController,
                let games = ctl.selectedGames!
                else { return }
            
            gamesToStart.append(contentsOf: games)
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
        #if !USE_NEW
        guard
            let media = currentSubviewController as? OEMediaViewController,
            let games = media.selectedSaveStates,
            games.count == 1
            else { return }
        #else
        guard
            let ss = currentSubviewController as? SaveStateViewController
            else { return }
        let games = ss.selectedSaveStates
        if games.count != 1 { return }
        #endif
        
        start(saveState: games.first!)
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
    
    @objc var currentSubviewController: (NSViewController & OELibrarySubviewController)?
    
    @IBOutlet var toolbar: LibraryToolbar?
    
    var libraryGamesViewController: OELibraryGamesViewController!
    var saveStatesViewController: OEMediaViewController!
    var saveStatesViewController2: SaveStateViewController!
    var screenshotsViewController: OEMediaViewController!
    var screenshotsViewController2: ScreenshotViewController!
    var homebrewViewController: OEHomebrewViewController!
    
}

@objc(OELibraryControllerDelegate)
protocol LibraryControllerDelegate {
    @objc optional func libraryController(_ controller: OELibraryController, didSelectGame: OEDBGame)
    @objc optional func libraryController(_ controller: OELibraryController, didSelectRom: OEDBRom)
    @objc optional func libraryController(_ controller: OELibraryController, didSelectSaveState: OEDBSaveState)
}
