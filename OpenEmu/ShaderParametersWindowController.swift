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
import OpenEmuKit

@objc(OEShaderParametersWindowController)
final class ShaderParametersWindowController: NSWindowController {
    @objc weak var controller: OEGameViewController?
    @objc var shader: OEShadersModel.OEShaderModel?
    @IBOutlet var outlineView: NSOutlineView?
    
    @objc public init(gameViewController: OEGameViewController) {
        controller = gameViewController
        super.init(window: nil)
    }
    
    required init?(coder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }

    override var windowNibName: NSNib.Name? {
        "ShaderParameters"
    }
    
    override func windowDidLoad() {
        super.windowDidLoad()
        
        window?.title = NSLocalizedString("Shader Parameters", comment: "Shader parameters window title")
        
        guard let outlineView = outlineView else { preconditionFailure("outlineView is required") }
        
        outlineView.delegate = self
        outlineView.dataSource = self
        
        outlineView.headerView = nil
        outlineView.gridStyleMask = []
        outlineView.allowsColumnReordering = false
        outlineView.allowsColumnResizing = false
        outlineView.allowsColumnSelection = false
        outlineView.allowsEmptySelection = true
        outlineView.allowsMultipleSelection = false
        outlineView.allowsTypeSelect = false
        
        outlineView.register(NSNib(nibNamed: "SliderCell", bundle: nil), forIdentifier: .sliderType)
        outlineView.register(NSNib(nibNamed: "GroupCell", bundle: nil), forIdentifier: .groupType)
        outlineView.register(NSNib(nibNamed: "CheckboxCell", bundle: nil), forIdentifier: .checkBoxType)
    }
    
    private var _groups: [OEShaderParamGroupValue]?
    private var _paramsKVO: [NSKeyValueObservation]?
    
    @objc var groups: [OEShaderParamGroupValue]? {
        set {
            willChangeValue(for: \.groups)
            
            if let groups = newValue {
                _groups = groups.filter { !$0.hidden }
            }
            
            didChangeValue(for: \.groups)
            
            params = _groups?.flatMap { $0.parameters }
            
            outlineView?.reloadData()
        }
        
        get { _groups }
    }
    
    @objc dynamic var params: [OEShaderParamValue]? {
        didSet {
            _paramsKVO = params?.map {
                $0.observe(\.value) { [weak self] (param, change) in
                    guard let controller = self?.controller else { return }
                    
                    controller.document.gameViewController(controller,
                                                           setShaderParameterValue: CGFloat(param.value.doubleValue),
                                                           at: UInt(param.index),
                                                           atGroupIndex: UInt(param.groupIndex))

                    guard
                        let shader = self?.shader,
                        let params = self?.params
                    else { return }

                    shader.write(parameters: params, identifier: controller.document.systemIdentifier)
                }
            }
        }
    }
    
    @IBAction func resetAll(_ sender: Any?) {
        params?.forEach { $0.value = $0.initial }
    }
}

extension ShaderParametersWindowController: NSOutlineViewDelegate {
    func outlineView(_ outlineView: NSOutlineView, viewFor tableColumn: NSTableColumn?, item: Any) -> NSView? {
        if let param = item as? OEShaderParamValue {
            guard let cellView = outlineView.makeView(withIdentifier: param.cellType, owner: self) else { return nil }
            switch param.cellType {
            case .checkBoxType:
                let checkbox = cellView.subviews.first! as! NSButton
                checkbox.title = param.desc
                checkbox.bind(.value, to: param, withKeyPath: "value", options: [NSBindingOption.continuouslyUpdatesValue: NSNumber(booleanLiteral: true)])
                
            case .sliderType:
                let lbl = cellView.subviews.first! as! NSTextField
                lbl.stringValue = param.desc
                
                let slid = cellView.viewWithTag(100) as! NSSlider
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
                
                let num = cellView.viewWithTag(101) as! NSTextField
                
                let step = cellView.viewWithTag(102) as! NSStepper
                step.minValue = slid.minValue
                step.maxValue = slid.maxValue
                step.increment = param.step.doubleValue
                
                let options = [NSBindingOption.continuouslyUpdatesValue: NSNumber(booleanLiteral: true)]
                
                slid.bind(.value, to: param, withKeyPath: "value", options: options)
                num.bind(.value, to: param, withKeyPath: "value", options: options)
                step.bind(.value, to: param, withKeyPath: "value", options: options)
                
            default:
                break
            }
            
            return cellView
        }
        
        if let group = item as? OEShaderParamGroupValue {
            guard let cellView = outlineView.makeView(withIdentifier: .groupType, owner: self) as? NSTableCellView else { return nil }
            cellView.textField?.stringValue = group.desc
            return cellView
        }
        
        return nil
    }
}

extension ShaderParametersWindowController: NSOutlineViewDataSource {
    func outlineView(_ outlineView: NSOutlineView, numberOfChildrenOfItem item: Any?) -> Int {
        guard let groups = _groups else { return 0 }

        if groups.count == 1 {
            return params?.count ?? 0
        }

        if let group = item as? OEShaderParamGroupValue {
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

        if let group = item as? OEShaderParamGroupValue {
            return group.parameters[index]
        }
        
        return groups[index]
    }
    
    func outlineView(_ outlineView: NSOutlineView, isItemExpandable item: Any) -> Bool {
        guard let groups = _groups else { preconditionFailure("expected groups") }

        if groups.count == 1 {
            return false
        }
        
        return item is OEShaderParamGroupValue
    }
}

extension NSUserInterfaceItemIdentifier {
    static let checkBoxType = NSUserInterfaceItemIdentifier("CheckBox")
    static let sliderType   = NSUserInterfaceItemIdentifier("Slider")
    static let groupType    = NSUserInterfaceItemIdentifier("Group")
}

@objc extension OEShaderParamValue {
    var cellType: NSUserInterfaceItemIdentifier {
        minimum.doubleValue == 0.0 && maximum.doubleValue == 1.0 && step.doubleValue == 1.0
            ? .checkBoxType
            : .sliderType
    }
}
