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

typealias OEAlertCompletionHandler = (OEAlert, NSApplication.ModalResponse) -> Void

@objc
@objcMembers
class OEAlert: NSObject {
    
    private let TopInset: CGFloat = 16
    private let BottomInset: CGFloat = 18
    private let LeadingInset: CGFloat = 20
    private let TrailingInset: CGFloat = 20
    private let MinimumWidth: CGFloat = 420
    private let MaximumWidth: CGFloat = 500
    
    private let ImageLeadingInset: CGFloat = 24
    private let ImageWidth: CGFloat = 64
    private let ImageHeight: CGFloat = 64
    
    private let HeadlineToMessageSpacing: CGFloat = 6
    private let ProgressBarSpacing: CGFloat = 12
    private let ButtonTopSpacing: CGFloat = 10
    private let OtherButtonSpacing: CGFloat = 42
    private let InterButtonSpacing: CGFloat = 14
    
    private let MinimumButtonWidth: CGFloat = 79
    
    
    private(set) var result = NSApplication.ModalResponse(0)
    private(set) var window: NSWindow!
    
    private var blocks = [(() -> Void)]()
    private var sheetMode = false
    private var needsRebuild = true
    
    
    // dialog buttons
    private var defaultButton: NSButton!
    private var alternateButton: NSButton!
    private var otherButton: NSButton!
    // touch bar buttons
    private var defaultTBButton: NSButton!
    private var alternateTBButton: NSButton!
    private var otherTBButton: NSButton!
    
    private var messageLabel: NSTextField!
    private var headlineLabel: NSTextField!
    
    private var inputLabel: NSTextField!
    private var otherInputLabel: NSTextField!
    private var inputField: NSTextField!
    private var otherInputField: NSTextField!
    
    private var progressbar: NSProgressIndicator!
    
    private var suppressionButton: NSButton!
    
    
    override init() {
        super.init()
        
        window = NSPanel(contentRect: .zero, styleMask: .titled, backing: .buffered, defer: true)
        window.isReleasedWhenClosed = false
        
        if #available(macOS 11.0, *) {
            window.titleVisibility = .hidden
            window.titlebarAppearsTransparent = true
            window.styleMask.insert(.fullSizeContentView)
            window.isMovableByWindowBackground = true
            
            let veView = NSVisualEffectView()
            veView.material = .popover
            window.contentView = veView
        }
        
        createControls()
        createTouchBarControls()
    }
    
    // MARK: -
    
    @discardableResult
    func runModal() -> NSApplication.ModalResponse {
        sheetMode = false
        
        let suppressedResp = checkIfSuppressed()
        if suppressedResp != .continue {
            return suppressedResp
        }
        
        layoutWindowIfNeeded()
        
        window.animationBehavior = .alertPanel
        
        window.makeKeyAndOrderFront(nil)
        
        let executeBlocks = {
            DispatchQueue(label: "org.openemu.OEAlert").sync {
                while !self.blocks.isEmpty {
                    if let block = self.blocks.first {
                        block()
                    }
                    self.blocks.removeFirst()
                }
            }
            self.layoutWindowIfNeeded()
        }
        
        let session = NSApp.beginModalSession(for: window)
        while NSApp.runModalSession(session) == .continue {
            executeBlocks()
            RunLoop.current.run(until: Date(timeIntervalSinceNow: 0.05))
        }
        executeBlocks()
        
        NSApp.endModalSession(session)
        
        window.close()
        
        return result
    }
    
    func performBlockInModalSession(_ block: @escaping () -> Void) {
        if sheetMode {
            block()
            return
        }
        
        DispatchQueue(label: "org.openemu.OEAlert").sync {
            blocks.append(block)
        }
    }
    
    func close(withResult res: NSApplication.ModalResponse) {
        result = res
        
        if sheetMode {
            window.sheetParent?.endSheet(window, returnCode: res)
            return
        }
        
        NSApp.stopModal(withCode: result)
        
        DispatchQueue.main.async {
            self.window.close()
            self.performCallback()
        }
    }
    
    @objc(beginSheetModalForWindow:completionHandler:)
    func beginSheetModal(for sheetWindow: NSWindow, completionHandler handler: @escaping (NSApplication.ModalResponse) -> Void) {
        sheetMode = true
        
        let suppressedResp = checkIfSuppressed()
        if suppressedResp != .continue {
            handler(suppressedResp)
            return
        }
        
        layoutWindowIfNeeded()
        window.animationBehavior = .alertPanel
        sheetWindow.beginSheet(window) { result in
            self.result = result
            handler(result)
            self.performCallback()
        }
    }
    
    private func checkIfSuppressed() -> NSApplication.ModalResponse {
        if let key = suppressionUDKey, UserDefaults.standard.value(forKey: key) != nil {
            let suppressionValue = UserDefaults.standard.integer(forKey: key)
            result = (suppressionValue == 1 || suppressOnDefaultReturnOnly ? .alertFirstButtonReturn : .alertSecondButtonReturn)
            performCallback()
            return result
        }
        
        return .continue
    }
    
    private func stopModal() {
        if sheetMode {
            window.sheetParent?.endSheet(window, returnCode: result)
        } else {
            NSApp.stopModal(withCode: result)
            performCallback()
        }
    }
    
    // MARK: - Callbacks
    
    weak var target: AnyObject?
    var callback: Selector?
    
    var callbackHandler: OEAlertCompletionHandler? {
        didSet {
            setAlternateButtonAction(#selector(buttonAction(_:)), andTarget: self)
            setDefaultButtonAction(#selector(buttonAction(_:)), andTarget: self)
            setOtherButtonAction(#selector(buttonAction(_:)), andTarget: self)
        }
    }
    
    private func performCallback() {
        if let target = target, let callback = callback, target.responds(to: callback) {
            target.performSelector(onMainThread: callback, with: self, waitUntilDone: false)
        }
        
        if let callbackHandler = callbackHandler {
            callbackHandler(self, result)
            self.callbackHandler = nil
        }
    }
    
    // MARK: - Progress Bar
    
    var showsProgressbar: Bool = false {
        didSet {
            needsRebuild = true
        }
    }
    
    var progress: Double {
        get {
            return progressbar.doubleValue
        }
        set {
            progressbar.stopAnimation(nil)
            if newValue < 0 || newValue > 1 {
                progressbar.isIndeterminate = true
                progressbar.startAnimation(nil)
            } else {
                progressbar.isIndeterminate = false
                progressbar.doubleValue = newValue
            }
        }
    }
    
    // MARK: - Buttons
    
    var defaultButtonTitle: String {
        get {
            return defaultButton.title
        }
        set {
            defaultButton.title = newValue
            defaultTBButton.title = newValue
            needsRebuild = true
        }
    }
    
    var alternateButtonTitle: String {
        get {
            return alternateButton.title
        }
        set {
            alternateButton.title = newValue
            alternateTBButton.title = newValue
            needsRebuild = true
        }
    }
    
    var otherButtonTitle: String {
        get {
            return otherButton.title
        }
        set {
            otherButton.title = newValue
            otherTBButton.title = newValue
            needsRebuild = true
        }
    }
    
    @objc func buttonAction(_ sender: NSButton) {
        if sender == defaultButton || sender == defaultTBButton {
            result = .alertFirstButtonReturn
        } else if sender == alternateButton || sender == alternateTBButton {
            result = .alertSecondButtonReturn
        } else if sender == otherButton || sender == otherTBButton {
            result = .alertThirdButtonReturn
        } else {
            result = .alertFirstButtonReturn
        }
        if result != .alertThirdButtonReturn && suppressionButton.state == .on && (result == .alertFirstButtonReturn || !suppressOnDefaultReturnOnly),
           let key = suppressionUDKey {
            let suppressionValue = result == .alertFirstButtonReturn ? 1 : 0
            UserDefaults.standard.set(suppressionValue, forKey: key)
        }
        
        stopModal()
    }
    
    func setDefaultButtonAction(_ sel: Selector?, andTarget target: AnyObject?) {
        defaultButton.target = target
        defaultButton.action = sel
        defaultButton.isEnabled = sel != nil
        defaultTBButton.target = target
        defaultTBButton.action = sel
        defaultTBButton.isEnabled = sel != nil
    }
    
    func setAlternateButtonAction(_ sel: Selector?, andTarget target: AnyObject?) {
        alternateButton.target = target
        alternateButton.action = sel
        alternateButton.isEnabled = sel != nil
        alternateTBButton.target = target
        alternateTBButton.action = sel
        alternateTBButton.isEnabled = sel != nil
    }
    
    func setOtherButtonAction(_ sel: Selector?, andTarget target: AnyObject?) {
        otherButton.target = target
        otherButton.action = sel
        otherButton.isEnabled = sel != nil
        otherTBButton.target = target
        otherTBButton.action = sel
        otherButton.isEnabled = sel != nil
    }
    
    func addButton(withTitle title: String) {
        if defaultButtonTitle.isEmpty {
            defaultButtonTitle = title
        } else if alternateButtonTitle.isEmpty {
            alternateButtonTitle = title
        } else if otherButtonTitle.isEmpty {
            otherButtonTitle = title
        }
    }
    
    // MARK: - Message Text
    
    var messageText: String {
        get {
            return headlineLabel.stringValue
        }
        set {
            headlineLabel.stringValue = newValue
            needsRebuild = true
        }
    }
    
    /// When set to `true`, informativeText is interpreted as HTML.
    /// - warning: In case informativeText is generated by incorporating HTML markup in a
    /// printf-style format string, remember to percent-escape the format arguments!
    var messageUsesHTML: Bool = false {
        didSet {
            needsRebuild = true
        }
    }
    
    var informativeText: String = "" {
        didSet {
            needsRebuild = true
        }
    }
    
    // MARK: - Suppression Button
    
    var suppressionUDKey: String?
    
    /// Default is `true` meaning follow up alerts will not be suppressed if user clicked cancel.
    var suppressOnDefaultReturnOnly = true
    
    var showsSuppressionButton: Bool = false {
        didSet {
            needsRebuild = true
        }
    }
    
    func showSuppressionButton(forUDKey key: String) {
        showsSuppressionButton = true
        suppressionUDKey = key
        
        let checked = UserDefaults.standard.value(forKey: key) != nil
        suppressionButton.state = checked ? .on : .off
    }
    
    /// Default is "Do not ask me again", can be changed (e.g. if alert is too small).
    var suppressionLabelText: String {
        get {
            return suppressionButton.title
        }
        set {
            suppressionButton.title = newValue
        }
    }
    
    @objc func suppressionButtonAction(_ sender: NSButton) {
        if let key = suppressionUDKey,
           sender.state == .off && suppressOnDefaultReturnOnly {
            UserDefaults.standard.removeObject(forKey: key)
        }
    }
    
    var suppressionButtonState: Bool {
        get {
            return suppressionButton.state == .on
        }
        set {
            suppressionButton.state = newValue ? .on : .off
        }
    }
    
    // MARK: - Input Field
    
    var showsInputField: Bool = false {
        didSet {
            needsRebuild = true
        }
    }
    
    var showsOtherInputField: Bool = false {
        didSet {
            needsRebuild = true
        }
    }
    
    var stringValue: String {
        get {
            return inputField.stringValue
        }
        set {
            inputField.stringValue = newValue
        }
    }
    
    var otherStringValue: String {
        get {
            return otherInputField.stringValue
        }
        set {
            otherInputField.stringValue = newValue
        }
    }
    
    var inputLabelText: String {
        get {
            return inputLabel.stringValue
        }
        set {
            inputLabel.stringValue = newValue
        }
    }
    
    var otherInputLabelText: String {
        get {
            return otherInputLabel.stringValue
        }
        set {
            otherInputLabel.stringValue = newValue
        }
    }
    
    var inputPlaceholderText: String? {
        get {
            inputField.placeholderString
        }
        set {
            inputField.placeholderString = newValue
        }
    }
    
    var otherInputPlaceholderText: String? {
        get {
            return otherInputField.placeholderString
        }
        set {
            otherInputField.placeholderString = newValue
        }
    }
    
    var inputLimit: UInt {
        get {
            if let formatter = inputField.formatter as? InputLimitFormatter {
                return formatter.limit
            } else {
                return 0
            }
        }
        set {
            if let formatter = inputField.formatter as? InputLimitFormatter {
                if newValue == 0 {
                    inputField.formatter = nil
                } else {
                    formatter.limit = newValue
                }
            } else {
                inputField.formatter = InputLimitFormatter(limit: newValue)
            }
        }
    }
    
    // MARK: - Private Methods
    
    private func createControls() {
        // Setup Button
        defaultButton = NSButton(title: "", target: self, action: nil)
        defaultButton.translatesAutoresizingMaskIntoConstraints = false
        defaultButton.action = #selector(buttonAction(_:))
        defaultButton.keyEquivalent = "\r"
        
        alternateButton = NSButton(title: "", target: self, action: nil)
        alternateButton.translatesAutoresizingMaskIntoConstraints = false
        alternateButton.action = #selector(buttonAction(_:))
        alternateButton.keyEquivalent = ""
        
        otherButton = NSButton(title: "", target: self, action: nil)
        otherButton.translatesAutoresizingMaskIntoConstraints = false
        otherButton.action = #selector(buttonAction(_:))
        otherButton.keyEquivalent = ""
        
        // Setup Headline Text View
        headlineLabel = NSTextField(wrappingLabelWithString: "")
        headlineLabel.translatesAutoresizingMaskIntoConstraints = false
        headlineLabel.isEditable = false
        headlineLabel.isSelectable = true
        headlineLabel.font = .boldSystemFont(ofSize: NSFont.systemFontSize)
        
        // Setup Message Text View
        messageLabel = NSTextField(wrappingLabelWithString: "")
        messageLabel.translatesAutoresizingMaskIntoConstraints = false
        messageLabel.isEditable = false
        messageLabel.isSelectable = true
        
        // Setup Input Field
        inputField = NSTextField(string: "")
        inputField.translatesAutoresizingMaskIntoConstraints = false
        inputField.usesSingleLineMode = true
        inputField.isEditable = true
        
        inputLabel = NSTextField(labelWithString: "")
        inputLabel.translatesAutoresizingMaskIntoConstraints = false
        inputLabel.isEditable = false
        inputLabel.isSelectable = false
        
        // Setup Other Input Field
        otherInputField = NSTextField(string: "")
        otherInputField.translatesAutoresizingMaskIntoConstraints = false
        otherInputField.usesSingleLineMode = true
        otherInputField.isEditable = true
        
        otherInputLabel = NSTextField(labelWithString: "")
        otherInputLabel.translatesAutoresizingMaskIntoConstraints = false
        otherInputLabel.isEditable = false
        otherInputLabel.isSelectable = false
        
        // Setup Progressbar
        progressbar = NSProgressIndicator()
        progressbar.translatesAutoresizingMaskIntoConstraints = false
        progressbar.minValue = 0
        progressbar.maxValue = 1
        progressbar.usesThreadedAnimation = true
        
        // Setup Suppression Button
        suppressionButton = NSButton(checkboxWithTitle: "", target: self, action: nil)
        suppressionButton.translatesAutoresizingMaskIntoConstraints = false
        suppressionButton.setButtonType(.switch)
        suppressionButton.title = NSLocalizedString("Do not ask me again", comment: "")
        suppressionButton.action = #selector(suppressionButtonAction(_:))
    }
    
    private func layoutWindowIfNeeded() {
        if needsRebuild {
            layoutWindow()
            createTouchBar()
        }
    }
    
    private func layoutWindow() {
        let contentView = window.contentView!
        contentView.subviews = []
        contentView.translatesAutoresizingMaskIntoConstraints = false
        
        var lastAnchor = contentView.topAnchor
        var effectiveLeadingAnchor = contentView.leadingAnchor
        var image: NSImageView?
        
        // create an NSAlert-style decoration image only if at least one of the messages is displayed.
        if !messageText.isEmpty || !informativeText.isEmpty {
            image = NSImageView(image: NSImage(named: NSImage.applicationIconName)!)
            image!.translatesAutoresizingMaskIntoConstraints = false
            image!.imageFrameStyle = .none
            image!.imageScaling = .scaleProportionallyUpOrDown
            contentView.addSubview(image!)
            NSLayoutConstraint.activate([
                image!.topAnchor.constraint(equalTo: lastAnchor, constant: TopInset),
                image!.leadingAnchor.constraint(equalTo: contentView.leadingAnchor, constant: ImageLeadingInset),
                image!.heightAnchor.constraint(equalToConstant: ImageWidth),
                image!.widthAnchor.constraint(equalToConstant: ImageHeight),
            ])
            effectiveLeadingAnchor = image!.trailingAnchor
        }
        
        if !messageText.isEmpty {
            lastAnchor = layoutHeadlineUnderAnchor(lastAnchor, leading: effectiveLeadingAnchor)
        }
        if !informativeText.isEmpty {
            lastAnchor = layoutMessageUnderAnchor(lastAnchor, leading: effectiveLeadingAnchor)
        }
        if showsProgressbar {
            lastAnchor = layoutProgressBarUnderAnchor(lastAnchor, leading: effectiveLeadingAnchor)
        }
        
        // add padding under the previous views in case they have less height than the decoration image
        if let image = image {
            let dummyView = NSView()
            dummyView.translatesAutoresizingMaskIntoConstraints = false
            contentView.addSubview(dummyView)
            NSLayoutConstraint.activate([
                dummyView.topAnchor.constraint(equalTo: lastAnchor),
                dummyView.bottomAnchor.constraint(greaterThanOrEqualTo: image.bottomAnchor, constant: 0),
                dummyView.widthAnchor.constraint(equalToConstant: 0),
                dummyView.leftAnchor.constraint(equalTo: contentView.leftAnchor),
            ])
            lastAnchor = dummyView.bottomAnchor
        }
        
        if showsInputField || showsOtherInputField {
            effectiveLeadingAnchor = contentView.leadingAnchor
            lastAnchor = layoutInputFieldsUnderAnchor(lastAnchor, leading: effectiveLeadingAnchor)
        }
        lastAnchor = layoutButtonsUnderAnchor(lastAnchor, leading: effectiveLeadingAnchor)
        
        let maxWidthConstraint = contentView.widthAnchor.constraint(lessThanOrEqualToConstant: MaximumWidth)
        maxWidthConstraint.priority = .defaultHigh-1
        NSLayoutConstraint.activate([
            contentView.bottomAnchor.constraint(equalTo: lastAnchor, constant: BottomInset),
            contentView.widthAnchor.constraint(greaterThanOrEqualToConstant: MinimumWidth),
            maxWidthConstraint,
        ])
        
        // Set preferredMaxLayoutWidth on the message and the headline text fields.
        // In this way, in case the text wraps, the width of the label will be
        // exactly equal to the bounding box of the text itself.
        if contentView.fittingSize.width <= MaximumWidth {
            let maxTextWidth = MaximumWidth - TrailingInset - LeadingInset - ImageWidth - ImageLeadingInset
            messageLabel.preferredMaxLayoutWidth = maxTextWidth
            headlineLabel.preferredMaxLayoutWidth = maxTextWidth
        } else {
            messageLabel.preferredMaxLayoutWidth = 0
            headlineLabel.preferredMaxLayoutWidth = 0
        }
        
        window.setContentSize(contentView.fittingSize)
        window.center()
        
        needsRebuild = false
    }
    
    private func layoutHeadlineUnderAnchor(_ lastAnchor: NSLayoutYAxisAnchor, leading effectiveLeadingAnchor: NSLayoutXAxisAnchor) -> NSLayoutYAxisAnchor {
        let contentView = window.contentView!
        
        headlineLabel.setContentHuggingPriority(NSLayoutConstraint.Priority(NSLayoutConstraint.Priority.defaultHigh.rawValue + 1), for: .vertical)
        contentView.addSubview(headlineLabel)
        NSLayoutConstraint.activate([
            headlineLabel.topAnchor.constraint(equalTo: lastAnchor, constant: TopInset),
            headlineLabel.leadingAnchor.constraint(equalTo: effectiveLeadingAnchor, constant: LeadingInset),
            contentView.trailingAnchor.constraint(greaterThanOrEqualTo: headlineLabel.trailingAnchor, constant: TrailingInset),
        ])
        
        return headlineLabel.bottomAnchor
    }
    
    private func layoutMessageUnderAnchor(_ lastAnchor: NSLayoutYAxisAnchor, leading effectiveLeadingAnchor: NSLayoutXAxisAnchor) -> NSLayoutYAxisAnchor {
        let contentView = window.contentView!
        let hasHeadline = !messageText.isEmpty
        
        var messageFont: NSFont
        if !hasHeadline {
            messageFont = .systemFont(ofSize: NSFont.systemFontSize)
        } else {
            messageFont = .systemFont(ofSize: NSFont.smallSystemFontSize)
        }
        
        if messageUsesHTML {
            let adjustedHTML = String(format: "<span style=\"font-family: '-apple-system'; font-size:%fpx\">%@</span>", messageFont.pointSize, informativeText)
            let htmlData = adjustedHTML.data(using: .utf8)
            var attrString: NSMutableAttributedString?
            if let htmlData = htmlData {
                attrString = NSMutableAttributedString(
                    html: htmlData,
                    options: [.characterEncoding: String.Encoding.utf8.rawValue as NSNumber],
                    documentAttributes: nil)
            }
            attrString?.addAttribute(.foregroundColor, value: NSColor.labelColor, range: NSRange(location: 0, length: attrString?.length ?? 0))
            if let attrString = attrString {
                messageLabel.attributedStringValue = attrString
            }
            // selectable labels revert to non-attributed values as soon as they are touched, losing all formatting
            messageLabel.isSelectable = false
        } else {
            messageLabel.stringValue = informativeText
            messageLabel.font = messageFont
            messageLabel.isSelectable = true
        }
        
        contentView.addSubview(messageLabel)
        NSLayoutConstraint.activate([
            messageLabel.topAnchor.constraint(equalTo: lastAnchor, constant: hasHeadline ? HeadlineToMessageSpacing : TopInset),
            messageLabel.leadingAnchor.constraint(equalTo: effectiveLeadingAnchor, constant: LeadingInset),
            contentView.trailingAnchor.constraint(greaterThanOrEqualTo: messageLabel.trailingAnchor, constant: TrailingInset),
        ])
        
        return messageLabel.bottomAnchor
    }
    
    private func layoutProgressBarUnderAnchor(_ lastAnchor: NSLayoutYAxisAnchor, leading effectiveLeadingAnchor: NSLayoutXAxisAnchor) -> NSLayoutYAxisAnchor {
        let contentView = window.contentView!
        
        contentView.addSubview(progressbar)
        NSLayoutConstraint.activate([
            progressbar.topAnchor.constraint(equalTo: lastAnchor, constant: ProgressBarSpacing),
            progressbar.leadingAnchor.constraint(equalTo: effectiveLeadingAnchor, constant: LeadingInset),
            contentView.trailingAnchor.constraint(equalTo: progressbar.trailingAnchor, constant: TrailingInset),
        ])
        
        return progressbar.bottomAnchor
    }
    
    private func layoutInputFieldsUnderAnchor(_ lastAnchor: NSLayoutYAxisAnchor, leading effectiveLeadingAnchor: NSLayoutXAxisAnchor) -> NSLayoutYAxisAnchor {
        let contentView = window.contentView!
        
        let inputGrid = NSGridView(numberOfColumns: 2, rows: 0)
        inputGrid.translatesAutoresizingMaskIntoConstraints = false
        inputGrid.rowAlignment = .lastBaseline
        inputGrid.column(at: 0).xPlacement = .trailing
        contentView.addSubview(inputGrid)
        NSLayoutConstraint.activate([
            inputGrid.topAnchor.constraint(equalTo: lastAnchor, constant: TopInset),
            inputGrid.leadingAnchor.constraint(equalTo: effectiveLeadingAnchor, constant: LeadingInset),
            contentView.trailingAnchor.constraint(equalTo: inputGrid.trailingAnchor, constant: TrailingInset),
        ])
        
        if showsOtherInputField {
            inputGrid.addRow(with: [otherInputLabel, otherInputField])
        }
        if showsInputField {
            inputGrid.addRow(with: [inputLabel, inputField])
        }
        
        return inputGrid.bottomAnchor
    }
    
    private func layoutButtonsUnderAnchor(_ lastAnchor: NSLayoutYAxisAnchor, leading effectiveLeadingAnchor: NSLayoutXAxisAnchor) -> NSLayoutYAxisAnchor {
        let contentView = window.contentView!
        
        let buttonStackView = NSStackView()
        buttonStackView.orientation = .horizontal
        buttonStackView.translatesAutoresizingMaskIntoConstraints = false
        buttonStackView.alignment = .lastBaseline
        buttonStackView.spacing = InterButtonSpacing
        contentView.addSubview(buttonStackView)
        NSLayoutConstraint.activate([
            buttonStackView.topAnchor.constraint(equalTo: lastAnchor, constant: ButtonTopSpacing),
            buttonStackView.leadingAnchor.constraint(equalTo: contentView.leadingAnchor, constant: LeadingInset),
            contentView.trailingAnchor.constraint(equalTo: buttonStackView.trailingAnchor, constant: TrailingInset),
        ])
        
        if !alternateButtonTitle.isEmpty {
            buttonStackView.addView(alternateButton, in: .trailing)
            contentView.addConstraint(alternateButton.widthAnchor.constraint(greaterThanOrEqualToConstant: MinimumButtonWidth))
        }
        if !defaultButtonTitle.isEmpty {
            buttonStackView.addView(defaultButton, in: .trailing)
            contentView.addConstraint(defaultButton.widthAnchor.constraint(greaterThanOrEqualToConstant: MinimumButtonWidth))
        }
        if showsSuppressionButton {
            buttonStackView.addView(suppressionButton, in: .leading)
        }
        if !otherButtonTitle.isEmpty {
            buttonStackView.addView(otherButton, in: .leading)
            buttonStackView.setCustomSpacing(OtherButtonSpacing, after: otherButton)
            NSLayoutConstraint.activate([
                otherButton.widthAnchor.constraint(greaterThanOrEqualToConstant: MinimumButtonWidth),
                otherButton.leadingAnchor.constraint(greaterThanOrEqualTo: effectiveLeadingAnchor, constant: LeadingInset),
            ])
        } else if showsSuppressionButton {
            buttonStackView.setCustomSpacing(OtherButtonSpacing, after: suppressionButton)
            
        }
        
        return buttonStackView.bottomAnchor
    }
    
    private func createTouchBarControls() {
        defaultTBButton = NSButton(title: "", target: self, action: #selector(buttonAction(_:)))
        defaultTBButton.keyEquivalent = "\r"
        
        alternateTBButton = NSButton(title: "", target: self, action: #selector(buttonAction(_:)))
        
        otherTBButton = NSButton(title: "", target: self, action: #selector(buttonAction(_:)))
    }
    
    private func createTouchBar() {
        let tb = NSTouchBar()
        let childTb = NSGroupTouchBarItem(alertStyleWithIdentifier: .group)
        tb.templateItems = [childTb]
        tb.defaultItemIdentifiers = [.group]
        tb.principalItemIdentifier = .group
        
        var allItems = Set<NSTouchBarItem>()
        var allItemIDs = [NSTouchBarItem.Identifier]()
        
        if !otherButtonTitle.isEmpty {
            let otherTbi = NSCustomTouchBarItem(identifier: .otherButton)
            otherTbi.view = otherTBButton
            allItems.insert(otherTbi)
            allItemIDs.append(.otherButton)
            allItemIDs.append(.fixedSpaceLarge)
        }
        if !alternateButtonTitle.isEmpty {
            let altTbi = NSCustomTouchBarItem(identifier: .alternateButton)
            altTbi.view = alternateTBButton
            allItems.insert(altTbi)
            allItemIDs.append(.alternateButton)
        }
        if !defaultButtonTitle.isEmpty {
            let defaultTbi = NSCustomTouchBarItem(identifier: .defaultButton)
            defaultTbi.view = defaultTBButton
            allItems.insert(defaultTbi)
            allItemIDs.append(.defaultButton)
        }
        
        childTb.groupTouchBar.templateItems = allItems
        childTb.groupTouchBar.defaultItemIdentifiers = allItemIDs
        window.touchBar = tb
    }
}

private extension NSTouchBarItem.Identifier {
    static let group           = NSTouchBarItem.Identifier("OEAlertGroup")
    static let defaultButton   = NSTouchBarItem.Identifier("OEAlertDefaultButton")
    static let alternateButton = NSTouchBarItem.Identifier("OEAlertAlternateButton")
    static let otherButton     = NSTouchBarItem.Identifier("OEAlertOtherButton")
}
