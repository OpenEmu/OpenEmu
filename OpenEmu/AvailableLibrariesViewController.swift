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

extension NSUserInterfaceItemIdentifier {
    static let availableLibrariesCollectionViewItem: NSUserInterfaceItemIdentifier = "AvailableLibrariesCollectionViewItem"
}

@objc(OEAvailableLibrariesViewController)
class AvailableLibrariesViewController: NSViewController {
    
    var data: [Model] = []
    
    @IBOutlet weak var collectionView: NSCollectionView!
    
    private var observerToken: NSObjectProtocol?
    
    @objc var isEnableObservers: Bool = false {
        didSet {
            if isEnableObservers {
                OEPlugin.addObserver(self, forKeyPath: #keyPath(OEPlugin.allPlugins), options: [.old, .new], context: nil)
                observerToken = NotificationCenter.default
                    .addObserver(forName: .OEDBSystemAvailabilityDidChange, object: nil, queue: .main) { [weak self] _ in
                        guard let self = self else { return }
                        
                        self.loadData()
                }
            } else {
                OEPlugin.removeObserver(self, forKeyPath: #keyPath(OEPlugin.allPlugins))
                observerToken = nil
            }
        }
    }
    
    override func observeValue(forKeyPath keyPath: String?, of object: Any?, change: [NSKeyValueChangeKey : Any]?, context: UnsafeMutableRawPointer?) {
        if let keyPath = keyPath, keyPath == "allPlugins" {
            loadData()
        }
    }
    
    override var nibName: NSNib.Name? {
        "AvailableLibrariesViewController"
    }
    
    override func viewDidLoad() {
        super.viewDidLoad()
    }
    
    override func viewWillAppear() {
        loadData()
    }
    
    func loadData() {
        guard
            let context = OELibraryDatabase.default?.mainThreadContext,
            let systems = OEDBSystem.allSystems(in: context)
            else { return }
        let identifiers: [String]
        if let plugins = OECorePlugin.allPlugins as? [OECorePlugin] {
            identifiers = plugins.flatMap { $0.systemIdentifiers }
        } else {
            identifiers = []
        }
        
        var data = [Model]()
        
        for system in systems {
            let isMissingCore = !identifiers.contains(system.systemIdentifier ?? "")
            data.append(Model(system: system, isMissingCore: isMissingCore))
        }
        
        self.data = data
        collectionView.reloadData()
    }
    
    struct Model {
        let system: OEDBSystem
        let isMissingCore: Bool
        let warnings: String?
        
        init(system: OEDBSystem, isMissingCore: Bool) {
            self.system = system
            self.isMissingCore = isMissingCore
            
            if isMissingCore || system.plugin == nil {
                var w = [String]()
                if system.plugin == nil {
                    w.append(NSLocalizedString("The System plugin could not be found!", comment: ""))
                }
                
                if isMissingCore {
                    w.append(NSLocalizedString("This System has no corresponding core installed.", comment: ""))
                }
                warnings = w.joined(separator: "\n")
            } else {
                warnings = nil
            }
        }
        
        var isEnabled: Bool {
            system.enabled?.boolValue ?? false
        }
        
        var isMissingPlugin: Bool {
            system.plugin == nil
        }
    }
}

extension AvailableLibrariesViewController: NSCollectionViewDataSource {
    func collectionView(_ collectionView: NSCollectionView, numberOfItemsInSection section: Int) -> Int {
        data.count
    }
    
    func collectionView(_ collectionView: NSCollectionView, itemForRepresentedObjectAt indexPath: IndexPath) -> NSCollectionViewItem {
        let item = collectionView.makeItem(withIdentifier: .availableLibrariesCollectionViewItem, for: indexPath)
        
        if let item = item as? AvailableLibrariesCollectionViewItem {
            item.data = data[indexPath.item]
        }
        
        return item
    }
}

class AvailableLibrariesCollectionViewItem: NSCollectionViewItem {
    @IBOutlet weak var button: NSButton!
    
    var data: AvailableLibrariesViewController.Model? {
        didSet {
            guard let data = data else { return }
            button.isEnabled = !data.isMissingPlugin
            button.state = data.isEnabled ? .on : .off
            button.title = data.system.name
            imageView?.isHidden = data.warnings == nil
            imageView?.toolTip = data.warnings
        }
    }
    
    @IBAction func toggleSystem(_ sender: Any?) {
        guard
            let data = data,
            let si   = data.system.systemIdentifier,
            let context = OELibraryDatabase.default?.mainThreadContext
            else { return }
        
        let system = OEDBSystem(forPluginIdentifier: si, in: context)
        system.toggleEnabledAndPresentError()
    }
}
