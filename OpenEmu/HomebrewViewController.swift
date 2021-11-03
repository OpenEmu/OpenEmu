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

private extension NSUserInterfaceItemIdentifier {
    static let headerView  = NSUserInterfaceItemIdentifier("HeaderView")
    static let featureView = NSUserInterfaceItemIdentifier("FeatureView")
    static let gameView    = NSUserInterfaceItemIdentifier("GameView")
    static let dummyView   = NSUserInterfaceItemIdentifier("DummyView")
}

final class HomebrewViewController: NSViewController {
    
    private let homebrewGamesURL = URL(string: "https://raw.githubusercontent.com/OpenEmu/OpenEmu-Update/master/games.xml")!
    
    @IBOutlet var tableView: NSTableView!
    private var blankSlate: HomebrewBlankSlateView?
    var database: OELibraryDatabase?
    private var currentDownload: Download?
    private var games: [HomebrewGame]!
    private var headerIndices: [Int]!
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        view.autoresizingMask = [.width, .height]
        
        tableView.allowsColumnReordering = false
        tableView.allowsColumnResizing = false
        tableView.allowsEmptySelection = true
        tableView.allowsMultipleSelection = false
        tableView.allowsTypeSelect = false
        tableView.delegate = self
        tableView.dataSource = self
        tableView.sizeLastColumnToFit()
        tableView.columnAutoresizingStyle = .uniformColumnAutoresizingStyle
        tableView.tableColumns.last?.resizingMask = .autoresizingMask
    }
    
    override func viewDidAppear() {
        super.viewDidAppear()
        
        validateToolbarItems()
        
        // Fetch games if we haven't already, this allows reloading if an error occured, by switching to a different library view and then back to homebrew
        if games == nil || games.count == 0 {
            updateGames()
        }
    }
    
    private var toolbar: LibraryToolbar? {
        view.window?.toolbar as? LibraryToolbar
    }
    
    func validateToolbarItems() {
        guard let toolbar = toolbar else { return }
        
        toolbar.viewModeSelector.isEnabled = false
        toolbar.viewModeSelector.selectedSegment = -1
        
        toolbar.gridSizeSlider.isEnabled = false
        toolbar.decreaseGridSizeButton.isEnabled = false
        toolbar.increaseGridSizeButton.isEnabled = false
        
        toolbar.searchField.isEnabled = false
        toolbar.searchField.searchMenuTemplate = nil
        toolbar.searchField.stringValue = ""
        
        toolbar.addButton.isEnabled = false
        
        if #available(macOS 11.0, *) {
            for item in toolbar.items {
                if item.itemIdentifier == .oeSearch {
                    item.isEnabled = false
                }
            }
        }
    }
    
    // MARK: - Data Handling
    
    private func updateGames() {
        // Indicate that we're fetching some remote resources
        displayUpdate()
        
        // Cancel last download
        currentDownload?.cancel()
        
        let download = Download(url: homebrewGamesURL)
        weak var blockDL = download
        download.completionHandler = { [self] destination, error in
            if currentDownload == blockDL {
                currentDownload = nil
            }
            if let nsErr = error as? CocoaError, nsErr.code == .userCancelled {
                return
            }
            if error == nil, let destination = destination {
                do {
                    try parseFile(at: destination)
                    displayResults()
                } catch {
                    displayError(error)
                }
            }
            else {
                displayError(error)
            }
        }
        currentDownload = download
        download.start()
    }
    
    private func parseFile(at url: URL) throws {
        
        let document: XMLDocument
        do {
            document = try XMLDocument(contentsOf: url, options: [])
        }
        catch {
            return DLog("\(String(describing: error))")
        }
        
        var allHeaderIndices = [Int]()
        if let allGames = try? document.nodes(forXPath: "//system | //game") {
            games = allGames.enumerated().compactMap { (index, node) in
                // Keep track of system node indices to use for headers
                if let _ = try? node.nodes(forXPath: "@id").last {
                    allHeaderIndices.append(index)
                }
                return HomebrewGame(node: node)
            }
        }
        
        headerIndices = allHeaderIndices
    }
    
    // MARK: - View Management
    
    private func displayUpdate() {
        let blankSlate = HomebrewBlankSlateView(frame: view.bounds)
        blankSlate.representedObject = NSLocalizedString("Fetching Games…", comment: "Homebrew Blank Slate View Updating Info")
        displayBlankSlate(blankSlate)
    }
    
    private func displayError(_ error: Error?) {
        guard view.window != nil else {
            // User switched to another category. No need to display the error
            // because changing back to Homebrew brings up the spinner again.
            return
        }
        
        let blankSlate = HomebrewBlankSlateView(frame: view.bounds)
        blankSlate.representedObject = error
        displayBlankSlate(blankSlate)
    }
    
    private func displayResults() {
        displayBlankSlate(nil)
    }
    
    private func displayBlankSlate(_ newBlankSlate: HomebrewBlankSlateView?) {
        guard blankSlate != newBlankSlate else { return }
        
        // Determine if we are about to replace the current first responder or one of its superviews
        let firstResponder = view.window?.firstResponder
        let makeFirstResponder = (firstResponder as? NSView)?.isDescendant(of: view) ?? false
        
        blankSlate?.removeFromSuperview()
        
        blankSlate = newBlankSlate
        
        if let blankSlate = blankSlate, let window = view.window {
            blankSlate.frame = NSRect(origin: .zero, size: window.contentLayoutRect.size)
            blankSlate.autoresizingMask = [.height, .width]
            view.addSubview(blankSlate)
            
            tableView.isHidden = true
        }
        else {
            tableView.reloadData()
            tableView.isHidden = false
        }
        
        // Restore first responder if necessary
        if makeFirstResponder {
            if blankSlate != nil {
                view.window?.makeFirstResponder(blankSlate)
            } else {
                view.window?.makeFirstResponder(tableView)
            }
        }
    }
    
    // MARK: - Actions
    
    @IBAction func goToDeveloperWebsite(_ sender: NSButton) {
        
        let row = rowOfView(sender)
        
        guard row >= 0, row < games.count else { return }
        
        let game = games[row]
        
        if let url = URL(string: game.website) {
            NSWorkspace.shared.open(url)
        }
    }
    
    @IBAction func launchGame(_ sender: NSButton) {
        
        let row = rowOfView(sender)
        
        guard row >= 0, row < games.count else { return }
        
        let homebrewGame = games[row]
        let url = URL(string: homebrewGame.fileURLString)
        let fileIndex = homebrewGame.fileIndex
        let systemIdentifier = homebrewGame.systemIdentifier
        let md5 = homebrewGame.md5
        let name = homebrewGame.name
        
        let context = database!.mainThreadContext
        var rom: OEDBRom?
        do {
            rom = try OEDBRom(md5HashString: md5, in: context)
        } catch {
            let romDescription = OEDBRom.entityDescription(in: context)
            let gameDescription = OEDBGame.entityDescription(in: context)
            
            rom = OEDBRom(entity: romDescription, insertInto: context)
            rom!.sourceURL = url
            rom!.archiveFileIndex = NSNumber(value: fileIndex)
            rom!.md5 = md5.lowercased()
            
            let game = OEDBGame(entity: gameDescription, insertInto: context)
            game.roms = Set<OEDBRom>([rom!])
            game.name = name
            game.system = OEDBSystem(forPluginIdentifier: systemIdentifier, in: context)
            if let imageURL = homebrewGame.images?.first {
                game.setBoxImage(url: imageURL)
            }
        }
        
        let game = rom?.game
        game?.save()
        
        NSApp.sendAction(#selector(MainWindowController.startGame(_:)), to: nil, from: game)
    }
    
    private func rowOfView(_ view: NSView) -> Int {
        
        let viewRect = view.frame
        let viewRectOnView = tableView.convert(viewRect, from: view.superview)
        
        let row = tableView.row(at: NSPoint(x: viewRectOnView.midX, y: viewRectOnView.midY))
        
        if row == 1 {
            let container = view.superview?.superview
            if let superview = view.superview {
                return container?.subviews.firstIndex(where: { $0 === superview }) ?? -1
            }
            return -1
        }
        
        return row
    }
}

// MARK: - NSTableView Delegate

extension HomebrewViewController: NSTableViewDelegate {
    
    func tableView(_ tableView: NSTableView, viewFor tableColumn: NSTableColumn?, row: Int) -> NSView? {
        
        if row == 0 || headerIndices.contains(row) {
            return tableView.makeView(withIdentifier: .headerView, owner: self) as? NSTableCellView
        }
        else if row == 2 {
            return tableView.makeView(withIdentifier: .dummyView, owner: self) as? NSTableCellView
        }
        else if row == 1 {
            guard
                let view = tableView.makeView(withIdentifier: .featureView, owner: self) as? NSTableCellView,
                let games = self.tableView(tableView, objectValueFor: tableColumn, row: row) as? [HomebrewGame]
            else { return nil }
            
            let subview = view.subviews.last?.subviews.last
            for (index, game) in games.enumerated() {
                let container = subview?.subviews[index]
                
                if let artworkView = container?.subviews[0] as? HomebrewCoverView {
                    artworkView.urls = game.images
                    artworkView.target = self
                    artworkView.doubleAction = #selector(launchGame(_:))
                }
                
                if let label = container?.subviews[1] as? NSTextField {
                    label.stringValue = game.name
                }
            }
            
            return view
        }
        else {
            guard
                let view = tableView.makeView(withIdentifier: .gameView, owner: self) as? NSTableCellView,
                let game = self.tableView(tableView, objectValueFor: tableColumn, row: row) as? HomebrewGame
            else { return nil }
            
            let subviews = view.subviews
            
            if let title = subviews[0] as? NSTextField {
                title.stringValue = game.name
            }
            
            if let system = subviews[1] as? NSButton {
                system.title = game.systemShortName
            }
            
            if let year = subviews[2] as? NSButton {
                let formatter = DateFormatter()
                formatter.dateFormat = "Y"
                year.title = formatter.string(from: game.released)
                year.isHidden = game.released.timeIntervalSince1970 == 0
            }
            
            if let description = subviews[3] as? NSTextField {
                description.stringValue = game.gameDescription
            }
            
            if let developer = subviews[5] as? TextButton {
                developer.title = game.developer
                developer.target = self
                developer.action = #selector(goToDeveloperWebsite(_:))
                developer.objectValue = game.website
                developer.textColor = NSColor(from: "#929DA5")
                developer.textColorHover = NSColor(from: "#9c2a32")
            }
            
            if let imagesView = subviews[4] as? HomebrewCoverView {
                imagesView.urls = game.images
                imagesView.target = self
                imagesView.doubleAction = #selector(launchGame(_:))
            }
            
            return view
        }
    }
    
    func tableView(_ tableView: NSTableView, shouldSelectRow row: Int) -> Bool {
        return false
    }
}

// MARK: - NSTableView DataSource

extension HomebrewViewController: NSTableViewDataSource {
    
    func numberOfRows(in tableView: NSTableView) -> Int {
        return games?.count ?? 0 // -3 for featured games which share a row, +1 for the shared row, + 2 for headers
    }
    
    func tableView(_ tableView: NSTableView, objectValueFor tableColumn: NSTableColumn?, row: Int) -> Any? {
        
        if row == 0 {
            return NSLocalizedString("Featured Games", comment: "")
        }
        if row == 2 {
            return nil
        }
        if row == 1 {
            return Array(games[0...2])
        }
        
        if headerIndices.contains(row) {
            // Use system lastLocalizedName for header
            let game = games[row]
            let identifier = game.systemGroup
            
            let context = OELibraryDatabase.default!.mainThreadContext
            
            let system = OEDBSystem(forPluginIdentifier: identifier, in: context)
            
            return system.lastLocalizedName
        }
        
        return games[row]
    }
}

// MARK: - HomebrewGame

private struct HomebrewGame {
    
    let name: String
    let developer: String
    let website: String
    let fileURLString: String
    let fileIndex: Int
    let gameDescription: String
    //let added: Date
    let released: Date
    let systemIdentifier: String
    let md5: String
    let systemGroup: String
    let images: [URL]?
    
    init(node: XMLNode) {
        
        func stringValue(_ xPath: String, node: XMLNode = node) -> String? {
            return try? node.nodes(forXPath: xPath).last?.stringValue?.trimmingCharacters(in: .whitespacesAndNewlines)
        }
        func integerValue(_ xPath: String) -> Int? {
            return Int(stringValue(xPath) ?? "")
        }
        func dateValue(_ xPath: String) -> Date {
            return Date(timeIntervalSince1970: TimeInterval(integerValue(xPath) ?? 0))
        }
        
        name = stringValue("@name") ?? ""
        developer = stringValue("@developer") ?? ""
        website = stringValue("@website") ?? ""
        fileURLString = stringValue("@file") ?? ""
        fileIndex = integerValue("@fileIndex") ?? 0
        gameDescription = stringValue("description") ?? ""
        //added = dateValue("@added")
        released = dateValue("@released")
        systemIdentifier = stringValue("@system") ?? ""
        md5 = stringValue("@md5") ?? ""
        systemGroup = stringValue("@id") ?? ""
        
        if let imageNodes = try? node.nodes(forXPath: "images/image") {
            images = imageNodes.compactMap {
                URL(string: stringValue("@src", node: $0) ?? "")
            }
        }
        else { images = nil }
    }
    
    var systemShortName: String {
        let identifier = systemIdentifier
        let context = OELibraryDatabase.default!.mainThreadContext
        
        let system = OEDBSystem(forPluginIdentifier: identifier, in: context)
        if let shortname = system.shortname {
            return shortname
        } else {
            return identifier.components(separatedBy: ".").last?.uppercased() ?? ""
        }
    }
}
