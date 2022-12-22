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
import OpenEmuShaders
import OpenEmuKit
import OrderedCollections

private extension Int {
    static let sliderTag: Self = 100
    static let fieldTag: Self = 101
    static let stepperTag: Self = 102
}

final class ShaderParametersViewController: NSViewController {
    let shaderControl: ShaderControl
    var shaderObserver: NSObjectProtocol?
    var presetsObserver: NSObjectProtocol?
    var currentShaderName: String?
    
    @IBOutlet var outlineView: NSOutlineView!
    @IBOutlet var shaderListPopUpButton: NSPopUpButton!
    private var noParametersLabel: NSTextField!
    
    // MARK: - Presets
    var avc: NSTitlebarAccessoryViewController! // ⚠️ is always 'nil' on 10.14
    @IBOutlet var accessoryView: NSView!
    @IBOutlet var presetList: NSPopUpButton!
    var presetToNameMap = [ShaderPreset: String]()
    var nameToPresetMap = OrderedDictionary<String, ShaderPreset>()
    
    enum State {
        case none, newPreset, renamePreset
    }
    
    var state = State.none
    
    public init(shaderControl: ShaderControl) {
        self.shaderControl = shaderControl
        super.init(nibName: "ShaderParametersViewController", bundle: nil)
    }
    
    required init?(coder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }
    
    func loadPreset() {
        let preset = shaderControl.preset
        
        if #available(macOS 10.15, *) {
            selectPresetInList(preset)
        }
        
        if currentShaderName == preset.shader.name {
            // The parameters are the same when changing between presets
            if let params = self.params {
                params.apply(parameters: preset.parameters)
                return
            }
        }
        
        currentShaderName = preset.shader.name
        
        // Assigning groups will also build params
        groups = preset.shader.readGroups()
        
        if let params = params, !params.isEmpty {
            avc?.isHidden = false
            noParametersLabel.isHidden = true
            // update with existing user preferences
            params.apply(parameters: preset.parameters)
            
            for item in shaderListPopUpButton.menu?.items ?? [] {
                if item.title == preset.shader.name {
                    shaderListPopUpButton.select(item)
                    break
                }
            }
        } else {
            avc?.isHidden = true
            noParametersLabel.isHidden = false
        }
    }
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        loadShaderMenu()
        
        noParametersLabel = NSTextField(labelWithString: NSLocalizedString("No parameters available.", comment: ""))
        view.addSubview(noParametersLabel)
        noParametersLabel.translatesAutoresizingMaskIntoConstraints = false
        
        NSLayoutConstraint.activate([
            noParametersLabel.centerXAnchor.constraint(equalTo: outlineView.centerXAnchor),
            noParametersLabel.centerYAnchor.constraint(equalTo: outlineView.centerYAnchor),
        ])
    }
    
    private func loadShaderMenu() {
        
        let shaderMenu = NSMenu()
        
        let systemShaders = OEShaderStore.shared.sortedSystemShaderNames
        systemShaders.forEach { shaderName in
            shaderMenu.addItem(withTitle: shaderName, action: #selector(GameViewController.selectShader(_:)), keyEquivalent: "")
        }
        
        let customShaders = OEShaderStore.shared.sortedCustomShaderNames
        if !customShaders.isEmpty {
            shaderMenu.addItem(.separator())
            
            customShaders.forEach { shaderName in
                shaderMenu.addItem(withTitle: shaderName, action: #selector(GameViewController.selectShader(_:)), keyEquivalent: "")
            }
        }
        
        shaderListPopUpButton.menu = shaderMenu
        
        let selectedShader = shaderControl.preset.shader.name
        
        if shaderListPopUpButton.item(withTitle: selectedShader) != nil {
            shaderListPopUpButton.selectItem(withTitle: selectedShader)
        } else {
            shaderListPopUpButton.selectItem(withTitle: OEShaderStore.shared.defaultShaderName)
        }
    }
    
    override func viewWillAppear() {
        super.viewWillAppear()
        
        shaderControl.helper?.setEffectsMode(.displayAlways)
        
        shaderObserver = shaderControl.observe(\.preset) { [weak self] (_, _) in
            guard let self = self else { return }
            self.undoManager?.removeAllActions(withTarget: self)
            self.loadPreset()
        }
        
        if #available(macOS 10.15, *) {
            presetsObserver = shaderControl.observe(\.presets) { [weak self] (_, _) in
                self?.loadPresetListForCurrentShader()
            }
        }
        
        if #available(macOS 10.15, *) {
            configurePresetsView()
        }
        loadPreset()
        configureToolbar()
    }
    
    override func viewWillDisappear() {
        shaderObserver  = nil
        presetsObserver = nil
        shaderControl.helper?.setEffectsMode(.reflectPaused)
    }
    
    private var _groups: [ShaderParamGroupValue]?
    private var _paramsKVO: [NSKeyValueObservation]?
    
    @objc var groups: [ShaderParamGroupValue]? {
        set {
            willChangeValue(for: \.groups)
            
            if let groups = newValue {
                _groups = groups.filter { !$0.hidden }
            }
            
            didChangeValue(for: \.groups)
            
            params = _groups?.flatMap { $0.parameters }
            
            outlineView.reloadData()
        }
        
        get { _groups }
    }
    
    @objc dynamic var params: [ShaderParamValue]? {
        didSet {
            _paramsKVO = params?.map {
                $0.observe(\.value) { [weak self] (param, _) in
                    guard let self = self else { return }
                    
                    self.shaderControl.setValue(CGFloat(param.value.doubleValue), forParameter: param.name)
                    
                    Self.cancelPreviousPerformRequests(withTarget: self, selector: #selector(Self.save(_:)), object: self)
                    self.perform(#selector(Self.save(_:)), with: self, afterDelay: 0.250)
                }
            }
        }
    }
    
    @objc func save(_ sender: Any?) {
        guard let params = self.params else { return }
        shaderControl.writeParameters(params)
    }
    
    @IBAction func resetAll(_ sender: Any?) {
        guard let params = params else {
            return
        }
        
        let changed = Dictionary(changedParams: params)
        undoManager?.registerUndo(withTarget: self) { vc in
            vc.params?.apply(parameters: changed)
        }
        
        params.forEach(ShaderParamValue.reset(_:))
        
        undoManager?.setActionName(NSLocalizedString("Reset parameters", comment: "undo action: Reset shader parameters"))
    }
    
    lazy var toolbar: NSToolbar = {
        let tb = NSToolbar(identifier: .shaderParametersToolbar)
        tb.delegate = self
        
        tb.allowsUserCustomization = false
        tb.autosavesConfiguration = false
        tb.displayMode = .default
        tb.sizeMode = .small
        
        return tb
    }()
    
    func configureToolbar() {
        guard let window = view.window else { return }
        window.toolbar = toolbar
        if #available(macOS 11.0, *) {
            window.toolbarStyle = .unifiedCompact
        }
        window.titleVisibility = .hidden
    }
}

extension ShaderParametersViewController: NSToolbarDelegate {
    func toolbarAllowedItemIdentifiers(_ toolbar: NSToolbar) -> [NSToolbarItem.Identifier] {
        [.title, .flexibleSpace, .shaderPicker]
    }
    
    func toolbarDefaultItemIdentifiers(_ toolbar: NSToolbar) -> [NSToolbarItem.Identifier] {
        [.title, .flexibleSpace, .shaderPicker]
    }
    
    func toolbar(_ toolbar: NSToolbar, itemForItemIdentifier itemIdentifier: NSToolbarItem.Identifier, willBeInsertedIntoToolbar flag: Bool) -> NSToolbarItem? {
        let isBordered = true
        
        switch itemIdentifier {
        case .title:
            let tbi = NSToolbarItem(itemIdentifier: .title)
            if #available(macOS 10.15, *) {
                tbi.isBordered = isBordered
            }
            let label = NSTextField(labelWithString: NSLocalizedString("Parameters", comment: "Window title: Shader parameters customisation window"))
            tbi.view = label
            return tbi
        case .shaderPicker:
            let tbi = NSToolbarItem(itemIdentifier: .shaderPicker)
            if #available(macOS 10.15, *) {
                tbi.isBordered = isBordered
            }
            tbi.visibilityPriority = .low
            tbi.view = shaderListPopUpButton
            let mi = NSMenuItem()
            mi.title = NSLocalizedString("Select Shader", comment: "Menu: Show a list of shaders the user may choose from")
            mi.submenu = shaderListPopUpButton.menu
            tbi.menuFormRepresentation = mi
            return tbi
        default:
            return nil
        }
    }
}

extension NSToolbar.Identifier {
    static let shaderParametersToolbar = Self.init("ShaderParametersToolbar")
}

extension NSToolbarItem.Identifier {
    static let title = Self.init(rawValue: "TitleItem")
    static let shaderPicker = Self.init(rawValue: "ShaderPickerItem")
}

extension ShaderParametersViewController: NSOutlineViewDelegate {
    func outlineView(_ outlineView: NSOutlineView, viewFor tableColumn: NSTableColumn?, item: Any) -> NSView? {
        if let param = item as? ShaderParamValue {
            guard let cellView = outlineView.makeView(withIdentifier: param.cellType, owner: self) else { return nil }
            switch param.cellType {
            case .checkBoxType:
                let checkbox = cellView.subviews.first! as! NSButton
                checkbox.title = param.desc
                checkbox.bind(.value, to: param, withKeyPath: #keyPath(ShaderParamValue.value), options: [NSBindingOption.continuouslyUpdatesValue: NSNumber(booleanLiteral: true)])
                
            case .sliderType:
                let lbl = cellView.subviews.first! as! NSTextField
                lbl.stringValue = param.desc
                
                let slid = cellView.viewWithTag(.sliderTag) as! NSSlider
                slid.isContinuous = true
                slid.minValue = param.minimum.doubleValue
                slid.maxValue = param.maximum.doubleValue
                let ticks = Int(slid.maxValue - slid.minValue) + 1
                if param.step.doubleValue == 1.0 && ticks <= 11 {
                    slid.numberOfTickMarks = ticks
                    slid.allowsTickMarkValuesOnly = true
                } else {
                    slid.numberOfTickMarks = 0
                    slid.allowsTickMarkValuesOnly = false
                }
                
                let num = cellView.viewWithTag(.fieldTag) as! NSTextField
                if let nf = num.formatter as? NumberFormatter {
                    nf.minimum = param.minimum
                    nf.maximum = param.maximum
                }
                
                let step = cellView.viewWithTag(.stepperTag) as! NSStepper
                step.minValue = slid.minValue
                step.maxValue = slid.maxValue
                step.increment = param.step.doubleValue
                
                let options: [NSBindingOption: NSNumber] = [
                    .continuouslyUpdatesValue: true,
                    .allowsNullArgument: false,
                    .nullPlaceholder: param.initial,
                ]
                
                slid.bind(.value, to: param, withKeyPath: #keyPath(ShaderParamValue.value), options: options)
                num.bind(.value, to: param, withKeyPath: #keyPath(ShaderParamValue.value), options: options)
                step.bind(.value, to: param, withKeyPath: #keyPath(ShaderParamValue.value), options: options)
                
            default:
                break
            }
            
            return cellView
        }
        
        if let group = item as? ShaderParamGroupValue {
            guard let cellView = outlineView.makeView(withIdentifier: .groupType, owner: self) as? NSTableCellView else { return nil }
            cellView.textField?.stringValue = group.desc
            return cellView
        }
        
        return nil
    }
}

extension ShaderParametersViewController: NSControlTextEditingDelegate {
    func controlTextDidEndEditing(_ n: Notification) {
        if let tf = n.object as? NSTextField,
           tf.tag == .fieldTag,
           tf.stringValue.isEmpty,
           let param = tf.infoForBinding(.value)?[.observedObject] as? ShaderParamValue
        {
            param.value = param.initial
        }
    }
}

extension ShaderParametersViewController: NSOutlineViewDataSource {
    func outlineView(_ outlineView: NSOutlineView, numberOfChildrenOfItem item: Any?) -> Int {
        guard let groups = _groups else { return 0 }
        
        if groups.count == 1 {
            return params?.count ?? 0
        }
        
        if let group = item as? ShaderParamGroupValue {
            return group.parameters.count
        }
        
        return groups.count
    }
    
    func outlineView(_ outlineView: NSOutlineView, child index: Int, ofItem item: Any?) -> Any {
        guard
            let groups = _groups,
            let params = params
        else { preconditionFailure("expected groups and parameters") }
        
        if groups.count == 1 {
            return params[index]
        }
        
        if let group = item as? ShaderParamGroupValue {
            return group.parameters[index]
        }
        
        return groups[index]
    }
    
    func outlineView(_ outlineView: NSOutlineView, isItemExpandable item: Any) -> Bool {
        guard let groups = _groups else { preconditionFailure("expected groups") }
        
        if groups.count == 1 {
            return false
        }
        
        return item is ShaderParamGroupValue
    }
}

private extension NSUserInterfaceItemIdentifier {
    static let checkBoxType = NSUserInterfaceItemIdentifier("CheckBox")
    static let sliderType   = NSUserInterfaceItemIdentifier("Slider")
    static let groupType    = NSUserInterfaceItemIdentifier("Group")
}

private extension ShaderParamValue {
    var cellType: NSUserInterfaceItemIdentifier {
        minimum.doubleValue == 0.0 && maximum.doubleValue == 1.0 && step.doubleValue == 1.0
        ? .checkBoxType
        : .sliderType
    }
}

// MARK: Menu validation

extension ShaderParametersViewController: NSMenuItemValidation {
    func validateMenuItem(_ menuItem: NSMenuItem) -> Bool {
        switch menuItem.action {
        case #selector(paste(_:)):
            if let text   = paramsFromClipboard(),
               let preset = try? ShaderPresetTextReader.read(text: text)
            {
                return !preset.shader.isEmpty
            } else {
                return false
            }
            
        case #selector(renamePreset(_:)), #selector(delete(_:)):
            if #available(macOS 10.15, *) {
                return !shaderControl.preset.isDefault
            }
            return false
            
        default:
            break
        }
        
        return true
    }
}

// MARK: - Copy / Paste support

extension ShaderParametersViewController {
    
    func paramsFromClipboard() -> String? {
        let pb = NSPasteboard.general
        guard pb.availableType(from: [.string]) != nil
        else { return nil }
        
        guard let text = pb.string(forType: .string)
        else { return nil }
        
        return text.trimmingCharacters(in: .whitespacesAndNewlines)
    }
    
    func paramsToClipboard(_ text: String) {
        let pb = NSPasteboard.general
        pb.clearContents()
        pb.declareTypes([.string], owner: nil)
        pb.setString(text, forType: .string)
    }
    
    @IBAction func copy(_ sender: Any) {
        guard let params = params else { return }
        let name   = shaderControl.preset.shader.name
        let preset = ShaderPresetData(name: "Unnamed shader preset", shader: name, parameters: Dictionary(changedParams: params))
        guard
            let text = try? ShaderPresetTextWriter.write(preset: preset, options: [.shader])
        else { return }
        paramsToClipboard(text)
    }
    
    @IBAction func paste(_ sender: Any) {
        guard
            let text   = paramsFromClipboard(),
            let preset = try? ShaderPresetTextReader.read(text: text),
            !preset.shader.isEmpty
        else { return }
        
        if let params = params, preset.shader == shaderControl.preset.shader.name {
            // If the selected shader is the same as what the user pasted, update the values
            // to avoid rebuilding the entire view.
            let changed = Dictionary(changedParams: params)
            undoManager?.registerUndo(withTarget: self) { vc in
                vc.params?.apply(parameters: changed)
            }
            params.apply(parameters: preset.parameters)
            undoManager?.setActionName(NSLocalizedString("Paste parameters", comment: "undo: Paste parameter values"))
        } else {
            guard
                let params = try? ShaderPresetTextWriter.write(preset: preset, options: [])
            else { return }
            
            if let shader = OEShaderStore.shared.shader(withName: preset.shader) {
                OESystemShaderStore.shared.write(parameters: params, forShader: preset.shader, identifier: shaderControl.systemIdentifier)
                shaderControl.changeShader(shader)
            }
        }
    }
}

// MARK: - Preset Actions

extension ShaderParametersViewController {
    @IBAction func renamePreset(_ sender: Any?) {
        guard #available(macOS 10.15, *) else { return }
        
        let nameShader = NameShaderPreset()
        nameShader.title = NSLocalizedString("Rename Shader Preset", comment: "Window title: Used to rename an existing shader preset")
        nameShader.existingName = shaderControl.preset.name
        state = .renamePreset
        presentAsModalWindow(nameShader)
    }
    
    @IBAction override func delete(_ sender: Any?) {
        guard #available(macOS 10.15, *) else { return }
        
        let alert: OEAlert = .deleteShaderPreset(name: shaderControl.preset.name)
        guard alert.runModal() == .alertFirstButtonReturn else { return }
        
        let currentId = shaderControl.preset.id
        guard let newPreset = nameToPresetMap.values.first(where: { $0.id != currentId })
        else { return }
        
        Task {
            do {
                let oldPreset = self.shaderControl.preset
                try await self.shaderControl.changePreset(newPreset)
                try self.shaderControl.deletePreset(oldPreset)
            } catch {
                NSApp.presentError(error)
            }
        }
    }
}

// MARK: - Manage Presets

@available(macOS 10.15, *)
extension ShaderParametersViewController: NameShaderPresetDelegate {

    func configurePresetsView() {
        if avc == nil {
            avc = NSTitlebarAccessoryViewController()
            avc.view = accessoryView
            avc.layoutAttribute = .bottom
            view.window?.addTitlebarAccessoryViewController(avc)
        }
        loadPresetListForCurrentShader()
    }
    
    static var dateFormat: DateFormatter = {
        let formatter = DateFormatter()
        formatter.dateStyle = .short
        formatter.timeStyle = .medium
        formatter.timeZone  = .current
        return formatter
    }()
    
    func makePresetNameMapping() -> ([ShaderPreset: String], OrderedDictionary<String, ShaderPreset>) {
        // Sorted by name and then createdAt
        let presets = shaderControl.presets.sorted()
        
        var presetToNameMap = [ShaderPreset: String](minimumCapacity: presets.count)
        var nameToPresetMap = OrderedDictionary<String, ShaderPreset>(minimumCapacity: presets.count)
        
        let grouped = OrderedDictionary<String, [ShaderPreset]>(grouping: presets) { $0.name }
        for (k, v) in grouped {
            if v.count == 1 {
                // only one preset with this name, so use, as-is
                nameToPresetMap[k] = v[0]
                presetToNameMap[v[0]] = k
            } else {
                for p in v {
                    let name = "\(p.name)\"(\(Self.dateFormat.string(from: p.createdAt)))"
                    nameToPresetMap[name] = p
                    presetToNameMap[p] = name
                }
            }
        }
        return (presetToNameMap, nameToPresetMap)
    }
    
    func loadPresetListForCurrentShader() {
        (presetToNameMap, nameToPresetMap) = makePresetNameMapping()
        
        let menu = NSMenu()
        for (name, preset) in nameToPresetMap {
            let item = NSMenuItem()
            menu.addItem(item)
            
            let components = name.split(separator: "\"")
            if components.count == 2 {
                // color the date addendum differently to make it clear that it is not part of the name
                let title = "\(components[0]) \(components[1])"
                let attributedString = NSMutableAttributedString(string: title)
                
                let dateRange = (title as NSString).range(of: String(components[1]))
                attributedString.addAttribute(.foregroundColor, value: NSColor.secondaryLabelColor, range: dateRange)
                
                item.attributedTitle = attributedString
            }
            
            item.title = name
            item.action = #selector(selectPreset(_:))
            item.representedObject = preset
        }
        presetList.menu = menu
        selectPresetInList(shaderControl.preset)
    }
    
    func selectPresetInList(_ preset: ShaderPreset) {
        if let name = presetToNameMap[preset] {
            presetList.selectItem(withTitle: name)
        }
    }
    
    @IBAction func newPreset(_ sender: Any?) {
        let nameShader = NameShaderPreset()
        nameShader.title = NSLocalizedString("New Shader Preset", comment: "Window title: Used to specify the name of a new shader preset")
        state = .newPreset
        presentAsModalWindow(nameShader)
    }
    
    @IBAction func selectPreset(_ sender: Any?) {
        guard
            let item = sender as? NSMenuItem,
            let preset = item.representedObject as? ShaderPreset
        else { return }
        changePresetWithError(preset)
    }
    
    private func changePresetWithError(_ preset: ShaderPreset) {
        Task {
            do {
                try await shaderControl.changePreset(preset)
            } catch {
                NSApp.presentError(error)
            }
        }
    }
    
    // MARK: - NameShaderPresetDelegate
    
    func setPresetName(_ name: String) {
        switch state {
        case .newPreset:
            let old = shaderControl.preset
            let new = ShaderPreset(name: name, shader: old.shader, parameters: old.parameters)
            if let _ = try? shaderControl.savePreset(new) {
                changePresetWithError(new)
            }
        case .renamePreset:
            let preset = shaderControl.preset
            preset.name = name
            try? shaderControl.savePreset(preset)
            break
        case .none:
            return
        }
    }
    
    func cancelSetPresetName() {
        state = .none
    }
}

extension Array where Array.Element == ShaderParamValue {
    func apply(parameters: [String: Double]) {
        for param in self {
            if let val = parameters[param.name] {
                param.value = val as NSNumber
            } else {
                param.reset()
            }
        }
    }
}
