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

final class ControlsButtonSetupView: NSView {
    
    static let controlsButtonHighlightRollsOver = "ButtonHighlightRollsOver"
    
    weak var target: AnyObject?
    var action: Selector?
    
    /// An object which support KVC/KVO for all OEGenericControlNamesKey of the current system
    @objc var bindingsProvider: AnyObject?
    
    private var sections: [Section]!
    private var orderedKeys: [String]!
    private var keyToButtonMap: [String: ControlsKeyButton]!
    
    private var _selectedKey: String?
    var selectedKey: String? {
        get {
            return _selectedKey
        }
        set {
            guard let keyToButtonMap = keyToButtonMap else { return }
            var value = newValue
            let button = keyToButtonMap[value ?? ""]
            if button == nil {
                value = nil
            }
            
            if _selectedKey != value {
                let previous = keyToButtonMap[_selectedKey ?? ""]
                
                _selectedKey = value
                
                previous?.state = .off
                button?.state = .on
                
                if value != nil && !NSWorkspace.shared.isVoiceOverEnabled {
                    // Scroll field into view
                    let clipView = enclosingScrollView?.contentView
                    var convertedButtonFrame = convert(button?.frame ?? .zero, to: clipView)
                    
                    convertedButtonFrame.origin.x = 0
                    convertedButtonFrame.origin.y -= (clipView?.frame.size.height ?? 0) / 2
                    convertedButtonFrame = backingAlignedRect(convertedButtonFrame, options: .alignAllEdgesNearest)
                    clipView?.animator().setBoundsOrigin(convertedButtonFrame.origin)
                }
            }
        }
    }
    
    /// - Parameter controlList: Content of the system’s Info.plist’s OEControlListKey object
    func setup(withControlList controlList: [AnyHashable]) {
        
        subviews.forEach { $0.removeFromSuperview() }
        
        let parser = ControlsSetupViewParser(target: self)
        parser.parseControlList(controlList)
        
        sections = parser.sections
        keyToButtonMap = parser.keyToButtonMap
        orderedKeys = parser.orderedKeys
        for (key, button) in keyToButtonMap {
            button.bind(
                NSBindingName("title"),
                to: self,
                withKeyPath: "bindingsProvider.\(key)",
                options: [.nullPlaceholder: ""]
            )
        }
        
        setupSubviews()
        layoutSubviews()
    }
    
    @objc fileprivate func selectInputControl(_ sender: ControlsKeyButton) {
        // Only one object should be there
        guard let key = keyToButtonMap.compactMap({ $1 == sender ? $0 : nil }).last
        else { return }
        
        selectedKey = key
        
        if let action = action {
            NSApp.sendAction(action, to: target, from: self)
        }
    }
    
    // MARK: - Drawing and Layout
    
    private var width: CGFloat = 227
    private var topGap: CGFloat = 16
    private var bottomGap: CGFloat = 16
    private var leftGap: CGFloat = 16
    private var itemHeight: CGFloat = 24
    private var verticalItemSpacing: CGFloat = 9 // item bottom to top
    private var labelButtonSpacing: CGFloat = 5
    private var buttonWidth: CGFloat = 130
    private var minimumFrameHeight: CGFloat = 259
    
    private var sectionTitleHeight: CGFloat = 25
    
    func setupSubviews() {
        // remove all subviews if any
        subviews.forEach { $0.removeFromSuperview() }
        
        for section in sections {
            
            addSubview(section.header)
            
            let groups = section.groups
            for group in groups {
                
                var i = 0
                while i < group.count {
                    if i == -1 { i += 2 }
                    
                    let item = group[i]
                    addSubview(item)
                    
                    // handle headline cell
                    if item is ControlsKeyHeadline {
                        i -= 1
                        continue
                    }
                    
                    // handle separator
                    if item is ControlsKeySeparatorView {
                        i -= 1
                        continue
                    }
                    
                    // handle buttons + label
                    let label = group[i + 1]
                    addSubview(label)
                    i += 2
                }
            }
        }
        
        // Sort so that section headers are on top
        sortSubviews({ view1, view2, _ in
            
            let view1IsHeader = view1 is ControlsSectionTitleView
            let view2IsHeader = view2 is ControlsSectionTitleView
            
            if view1IsHeader != view2IsHeader {
                return view1IsHeader ? .orderedDescending : .orderedAscending
            }
            return .orderedSame
        }, context: nil)
    }
    
    func layoutSubviews(animated: Bool = false) {
        
        let animator = animated ? animator() : self
        
        if animated {
            CATransaction.begin()
        }
        
        let rightGap: CGFloat = 14 + ((NSScroller.preferredScrollerStyle == .legacy) ? 14 : 0)
        
        // determine required height
        var frame = self.frame
        let viewHeight = self.viewHeight
        if frame.height != viewHeight {
            frame.size.height = viewHeight > minimumFrameHeight ? viewHeight : minimumFrameHeight
            animator.frame = frame
        }
        
        var y = frame.height
        for section in sections {
            
            let heading = section.header
            let groups = section.groups
            
            // layout section header
            let headerOrigin = NSPoint(x: 0, y: y - sectionTitleHeight)
            let headerFrame = NSRect(origin: headerOrigin, size: NSSize(width: width + 2, height: sectionTitleHeight))
            heading.frame = headerFrame
            
            y -= headerFrame.size.height + topGap
            
            for group in groups {
                var i = 0
                while i < group.count {
                    if i == -1 { i += 2 }
                    
                    let item = group[i]
                    
                    // handle headline cell
                    if item is ControlsKeyHeadline {
                        i -= 1
                        
                        let headlineFrame = NSRect(x: leftGap, y: y - itemHeight, width: width - leftGap - rightGap, height: itemHeight)
                        item.frame = headlineFrame.integral
                        y -= itemHeight + verticalItemSpacing
                        
                        continue
                    }
                    
                    // handle separator
                    if item is ControlsKeySeparatorView {
                        i -= 1
                        
                        let seperatorLineRect = NSRect(x: leftGap, y: y - itemHeight, width: width - leftGap - rightGap, height: itemHeight)
                        item.frame = seperatorLineRect.integral
                        y -= itemHeight + verticalItemSpacing
                        
                        continue
                    }
                    
                    // handle buttons + label
                    let buttonRect = NSRect(x: width - buttonWidth, y: y - itemHeight, width: buttonWidth - rightGap, height: itemHeight)
                    item.frame = buttonRect.integral
                    
                    let label = group[i + 1]
                    var labelRect = NSIntegralRect(NSRect(x: leftGap, y: buttonRect.origin.y + buttonRect.size.height / 2 + 1, width: width - leftGap - labelButtonSpacing - buttonWidth, height: 100000))
                    
                    var labelFitSize = (label as? NSTextField)?.cell?.cellSize(forBounds: labelRect) ?? .zero
                    if labelFitSize.height > 30 {
                        // If the label size returned is too tall, enlarge the
                        // label to attempt fitting in 2 lines anyway
                        labelRect.origin.x -= 5
                        labelRect.size.width += 5
                        labelFitSize = (label as? NSTextField)?.cell?.cellSize(forBounds: labelRect) ?? .zero
                    }
                    labelRect.origin.y -= labelFitSize.height / 2
                    labelRect.size.height = labelFitSize.height
                    
                    label.frame = labelRect
                    
                    y -= itemHeight + verticalItemSpacing
                    i += 2
                }
            }
            
            y += verticalItemSpacing
            y -= bottomGap
        }
        
        layoutSectionHeadings(nil)
        
        if animated {
            CATransaction.commit()
        }
    }
    
    private var viewHeight: CGFloat {
        var height: CGFloat = 0
        for section in sections {
            height += heightOfSection(section)
        }
        return height
    }
    
    private func heightOfSection(_ section: Section) -> CGFloat {
        var height = sectionTitleHeight
        
        height += topGap + bottomGap
        let numberOfRows = CGFloat(section.numberOfRows)
        height += (numberOfRows - 1) * verticalItemSpacing + numberOfRows * itemHeight
        
        return height
    }
    
    override func viewWillMove(toSuperview newSuperview: NSView?) {
        if let oldClipView = enclosingScrollView?.contentView {
            NotificationCenter.default.removeObserver(self, name: NSView.boundsDidChangeNotification, object: oldClipView)
        }
    }
    
    override func viewDidMoveToSuperview() {
        let clipView = enclosingScrollView?.contentView
        clipView?.postsBoundsChangedNotifications = true
        NotificationCenter.default.addObserver(self, selector: #selector(layoutSectionHeadings(_:)), name: NSView.boundsDidChangeNotification, object: clipView)
        
        layoutSectionHeadings(nil)
    }
    
    @objc private func layoutSectionHeadings(_ sender: Any?) {
        guard let sections = sections else {
            return
        }
        
        var minY: CGFloat = 0
        var i = sections.count - 1
        while i >= 0 {
            let section = sections[i]
            let sectionHeader = section.header
            
            let sectionStart = headerPositionOfSection(at: i)
            let sectionHeight = heightOfSection(section)
            
            let sectionRect = NSRect(x: 0, y: sectionStart - sectionHeight, width: width, height: sectionHeight)
            let visibleSectionRect = visibleRect.intersection(sectionRect)
            
            if visibleSectionRect != .null {
                sectionHeader.setFrameOrigin(NSPoint(x: 0, y: max(minY, visibleSectionRect.maxY - sectionTitleHeight)))
                minY = visibleSectionRect.maxY
                
                if minY >= visibleRect.maxY - sectionTitleHeight - 1 {
                    sectionHeader.pinned = true
                } else {
                    sectionHeader.pinned = false
                }
            } else {
                sectionHeader.pinned = false
            }
            i -= 1
        }
    }
    
    private func headerPositionOfSection(at index: Int) -> CGFloat {
        var y = bounds.height
        for i in 0..<index {
            y -= heightOfSection(sections[i])
        }
        return y
    }
    
    // MARK: -
    
    func selectNextKeyButton() {
        guard orderedKeys.count > 1 else { return }
        
        let i: Int = orderedKeys.firstIndex(of: selectedKey!)!
        
        var newKey: String?
        
        if i + 1 >= orderedKeys.count {
            if UserDefaults.standard.bool(forKey: Self.controlsButtonHighlightRollsOver) {
                newKey = orderedKeys[0]
            }
        } else {
            newKey = orderedKeys[i + 1]
        }
        
        CATransaction.begin()
        selectedKey = newKey
        CATransaction.commit()
    }
    
    func selectNextKeyAfterKeys(_ keys: [String]) {
        let i = orderedKeys.lastIndex(where: { keys.contains($0) })!
        
        var newKey: String?
        
        if i + 1 >= orderedKeys.count {
            if UserDefaults.standard.bool(forKey: Self.controlsButtonHighlightRollsOver) {
                newKey = orderedKeys[0]
            }
        } else {
            newKey = orderedKeys[i + 1]
        }
        
        CATransaction.begin()
        selectedKey = newKey
        CATransaction.commit()
    }
    
    // unused
    private func sectionOfKey(_ key: String) -> Section? {
        let button = keyToButtonMap[key]
        for section in sections {
            for group in section.groups {
                for element in group {
                    if element == button {
                        return section
                    }
                }
            }
        }
        return nil
    }
    
    // MARK: -
    
    override func scrollWheel(with event: NSEvent) {
        superview?.scrollWheel(with: event)
    }
    
    override var acceptsFirstResponder: Bool {
        return true
    }
    
    override func keyUp(with event: NSEvent) {
    }
    
    override func keyDown(with event: NSEvent) {
    }
}

private struct Section {
    var numberOfRows: Int
    var groups: [[NSView]]
    var header: ControlsSectionTitleView
}

private class ControlsSetupViewParser {
    
    private var target: ControlsButtonSetupView
    private var elementGroups = [[NSView]]()
    private var currentGroup: [NSView]?
    private var numberOfRows = 0
    
    var sections = [Section]()
    var keyToButtonMap = [String: ControlsKeyButton]()
    var orderedKeys = [String]()
    
    init(target: ControlsButtonSetupView) {
        self.target = target
    }
    
    func parseControlList(_ controlList: [AnyHashable]) {
        assert(controlList.count % 2 == 0, "control list has to have an even number of items (headline and group pairs)")
        
        var i = 0
        while i < controlList.count {
            let sectionTitle = NSLocalizedString(controlList[i] as? String ?? "", tableName: "ControlLabels", comment: "Section Title")
            let sectionContents = controlList[i + 1] as? [[AnyHashable]] ?? []
            for group in sectionContents {
                addGroup()
                
                for row in group {
                    numberOfRows += 1
                    if let row = row as? String {
                        if row == "-" {
                            addRowSeperator()
                        } else {
                            addGroupLabel(NSLocalizedString(row, tableName: "ControlLabels", comment: "Group Label"))
                        }
                    }
                    else if let row = row as? [String: String] {
                        
                        let label = NSLocalizedString(row[OEControlListKeyLabelKey] ?? "", tableName: "ControlLabels", comment: "Button Label")
                        
                        addButton(name: row[OEControlListKeyNameKey] ?? "",
                                  label: label + ":")
                    }
                }
            }
            
            if let currentGroup = currentGroup {
                elementGroups.append(currentGroup)
            }
            
            sections.append(
                Section(
                    numberOfRows: numberOfRows,
                    groups: elementGroups,
                    header: createSectionHeading(name: sectionTitle)
                )
            )
            
            elementGroups = [[NSView]]()
            currentGroup = nil
            numberOfRows = 0
            
            i += 2
        }
    }
    
    private func addButton(name: String, label: String) {
        let button = ControlsKeyButton()
        button.target = target
        button.action = #selector(ControlsButtonSetupView.selectInputControl(_:))
        button.label = label
        
        orderedKeys.append(name)
        keyToButtonMap[name] = button
        
        currentGroup?.append(button)
        
        let labelField = ControlsKeyLabel()
        labelField.stringValue = label
        
        currentGroup?.append(labelField)
    }
    
    private func addGroupLabel(_ label: String) {
        let labelField = ControlsKeyHeadline()
        labelField.stringValue = label
        currentGroup?.append(labelField)
    }
    
    private func addRowSeperator() {
        let view = ControlsKeySeparatorView()
        currentGroup?.append(view)
    }
    
    private func addGroup() {
        if let currentGroup = currentGroup {
            elementGroups.append(currentGroup)
        }
        currentGroup = [NSView]()
    }
    
    private func createSectionHeading(name: String) -> ControlsSectionTitleView {
        let labelField = ControlsSectionTitleView()
        labelField.stringValue = name
        return labelField
    }
}
