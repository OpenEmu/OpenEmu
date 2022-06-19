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

@objc(OEBlankSlateView)
class BlankSlateView: NSView {
    
    enum Mode {
        case homebrewLoading
        case homebrewError(Error?)
    }
    
    private enum Layout {
        static let containerSize = NSSize(width: 427, height: 418)
        
        static let boxImageToTop: CGFloat = 52 // distance between box image top and container top
        static let boxTextToTop: CGFloat = 202 // distance between box text top and container top
        static let headlineToTop: CGFloat = 327 // distance between headline top and container top
        static let bottomTextToTop: CGFloat = 357 // distance between bottom text top and container top
        static let bottomTextHeight: CGFloat = 67 // height of bottom text
    }
    
    @objc // OECollectionViewController
    weak var delegate: BlankSlateViewDelegate?
    
    @objc // OECollectionViewController
    var representedObject: Any? {
        didSet {
            guard representedObject != nil else { return }
            
            clearContainerView()
            setUpViewForRepresentedObject()
        }
    }
    
    private let containerView = NSView()
    private let dragIndicationLayer = CALayer()
    private lazy var lastDragOperation: NSDragOperation = []
    
    private lazy var isLoading = false {
        didSet {
            if isLoading {
                boxImageView.removeFromSuperview()
                containerView.addSubview(spinnerView)
                NSLayoutConstraint.activate([
                    spinnerView.topAnchor.constraint(equalTo: containerView.topAnchor, constant: Layout.boxImageToTop),
                    spinnerView.centerXAnchor.constraint(equalTo: containerView.centerXAnchor),
                ])
            } else {
                spinnerView.removeFromSuperview()
            }
        }
    }
    
    private var boxImage: NSImage? {
        get {
            return boxImageView.image
        }
        set {
            boxImageView.image = newValue
            containerView.addSubview(boxImageView)
            
            NSLayoutConstraint.activate([
                boxImageView.topAnchor.constraint(equalTo: containerView.topAnchor, constant: Layout.boxImageToTop),
                boxImageView.centerXAnchor.constraint(equalTo: containerView.centerXAnchor),
            ])
        }
    }
    
    private var boxText: String {
        get {
            return boxLabel.stringValue
        }
        set {
            boxLabel.stringValue = newValue
            containerView.addSubview(boxLabel)
            
            NSLayoutConstraint.activate([
                boxLabel.topAnchor.constraint(equalTo: containerView.topAnchor, constant: Layout.boxTextToTop),
                boxLabel.centerXAnchor.constraint(equalTo: containerView.centerXAnchor),
            ])
        }
    }
    
    private var headline: String {
        get {
            return headlineLabel.stringValue
        }
        set {
            headlineLabel.stringValue = newValue
            containerView.addSubview(headlineLabel)
            
            NSLayoutConstraint.activate([
                headlineLabel.leadingAnchor.constraint(equalTo: containerView.leadingAnchor),
                headlineLabel.topAnchor.constraint(equalTo: containerView.topAnchor, constant: Layout.headlineToTop),
            ])
        }
    }
    
    private var informationalText: String {
        get {
            return informationalTextView.string
        }
        set {
            informationalTextView.string = newValue
            containerView.addSubview(informationalTextView)
            
            NSLayoutConstraint.activate([
                informationalTextView.leadingAnchor.constraint(equalTo: containerView.leadingAnchor, constant: -4),
                informationalTextView.trailingAnchor.constraint(equalTo: containerView.trailingAnchor),
                informationalTextView.topAnchor.constraint(equalTo: containerView.topAnchor, constant: Layout.bottomTextToTop),
                informationalTextView.heightAnchor.constraint(equalToConstant: Layout.bottomTextHeight),
            ])
        }
    }
    
    // MARK: - Views
    
    private lazy var spinnerView: BlankSlateSpinnerView = {
        let spinnerView = BlankSlateSpinnerView()
        spinnerView.translatesAutoresizingMaskIntoConstraints = false
        
        return spinnerView
    }()
    
    private lazy var boxBorderImageView: NSImageView = {
        let boxBorderImageView = NSImageView()
        boxBorderImageView.image = NSImage(named: "blank_slate_box")
        boxBorderImageView.unregisterDraggedTypes()
        boxBorderImageView.translatesAutoresizingMaskIntoConstraints = false
        
        return boxBorderImageView
    }()
    
    private lazy var boxImageView: NSImageView = {
        let boxImageView = NSImageView()
        boxImageView.imageScaling = .scaleNone
        boxImageView.imageAlignment = .alignTop
        boxImageView.unregisterDraggedTypes()
        boxImageView.translatesAutoresizingMaskIntoConstraints = false
        
        return boxImageView
    }()
    
    private lazy var boxLabel: NSTextField = {
        let boxLabel = NSTextField(labelWithString: "")
        boxLabel.font = .systemFont(ofSize: 24)
        boxLabel.textColor = NSColor(named: "blank_slate_box_text")
        boxLabel.alignment = .center
        boxLabel.translatesAutoresizingMaskIntoConstraints = false
        
        return boxLabel
    }()
    
    private lazy var headlineLabel: NSTextField = {
        let headlineLabel = NSTextField(labelWithString: "")
        headlineLabel.font = .systemFont(ofSize: 20, weight: .medium)
        headlineLabel.textColor = .secondaryLabelColor
        headlineLabel.translatesAutoresizingMaskIntoConstraints = false
        
        return headlineLabel
    }()
    
    private lazy var informationalTextView: NSTextView = {
        let textView = NSTextView()
        textView.drawsBackground = false
        textView.isEditable = false
        textView.isSelectable = false
        textView.font = .systemFont(ofSize: 12)
        textView.textColor = .labelColor
        textView.textContainerInset = .zero
        textView.translatesAutoresizingMaskIntoConstraints = false
        
        return textView
    }()
    
    // MARK: -
    
    required init?(coder: NSCoder) {
        super.init(coder: coder)
        commonInit()
    }
    
    override init(frame frameRect: NSRect) {
        super.init(frame: frameRect)
        commonInit()
    }
    
    private func commonInit() {
        // Set up drag indication layer
        let layer = CALayer()
        layer.delegate = self
        self.layer = layer
        
        if #available(macOS 11.0, *) {
            effectiveAppearance.performAsCurrentDrawingAppearance {
                dragIndicationLayer.borderColor = NSColor.controlAccentColor.cgColor
            }
        } else {
            NSAppearance.current = effectiveAppearance
            dragIndicationLayer.borderColor = NSColor.controlAccentColor.cgColor
        }
        dragIndicationLayer.borderWidth = 2
        dragIndicationLayer.cornerRadius = 8
        dragIndicationLayer.isHidden = true
        
        layer.addSublayer(dragIndicationLayer)
        
        // Set up container view
        addSubview(containerView)
        
        containerView.translatesAutoresizingMaskIntoConstraints = false
        NSLayoutConstraint.activate([
            containerView.widthAnchor.constraint(equalToConstant: Layout.containerSize.width),
            containerView.heightAnchor.constraint(equalToConstant: Layout.containerSize.height),
            containerView.centerXAnchor.constraint(equalTo: centerXAnchor),
            containerView.centerYAnchor.constraint(equalTo: centerYAnchor),
        ])
    }
    
    override func viewDidChangeEffectiveAppearance() {
        if #available(macOS 11.0, *) {
            effectiveAppearance.performAsCurrentDrawingAppearance {
                dragIndicationLayer.borderColor = NSColor.controlAccentColor.cgColor
            }
        } else {
            NSAppearance.current = effectiveAppearance
            dragIndicationLayer.borderColor = NSColor.controlAccentColor.cgColor
        }
    }
    
    private func clearContainerView() {
        containerView.subviews.forEach { $0.removeFromSuperview() }
        
        containerView.addSubview(boxBorderImageView)
        
        NSLayoutConstraint.activate([
            boxBorderImageView.leadingAnchor.constraint(equalTo: containerView.leadingAnchor),
            boxBorderImageView.trailingAnchor.constraint(equalTo: containerView.trailingAnchor),
            boxBorderImageView.topAnchor.constraint(equalTo: containerView.topAnchor),
            boxBorderImageView.centerXAnchor.constraint(equalTo: containerView.centerXAnchor),
        ])
    }
    
    private func setUpViewForRepresentedObject() {
        
        if let system = representedObject as? OEDBSystem,
           let plugin = system.plugin {
            
            boxImage = NSImage(named: "blank_slate_arrow")
            boxText = NSLocalizedString("Drag & Drop Games Here", comment: "Blank Slate DnD Here")
            
            headline = plugin.systemName
            
            setUpView(systemPlugin: plugin)
        }
        else if let collection = representedObject as? GameCollectionViewItemProtocol {
            
            boxImage = NSImage(named: "blank_slate_arrow")
            boxText = NSLocalizedString("Drag & Drop Games Here", comment: "Blank Slate DnD Here")
            
            headline = NSLocalizedString("Collections", comment: "")
            // TODO: Use a different wording for smart collections, as games cannot be added manually.
            informationalText = .localizedStringWithFormat(NSLocalizedString("Create a personal game selection. To add to a collection, select a game from your console library and drag it to ’%@’ in the sidebar.", comment: ""), collection.collectionViewName)
        }
        else if representedObject is OEDBSaveStatesMedia {
            
            boxImage = NSImage(named: "blank_slate_arrow")
            boxText = NSLocalizedString("Drag & Drop Save States Here", comment: "Blank Slate DnD Save States Here")
            
            headline = NSLocalizedString("Save States", comment: "")
            informationalText = NSLocalizedString("With OpenEmu you can save your progress at any time using save states. We will even make auto saves for you, when you leave a game. Come back here to get an overview of all your saves.", comment: "")
        }
        else if representedObject is OEDBScreenshotsMedia {
            
            boxImage = NSImage(named: "blank_slate_camera")
            boxText = NSLocalizedString("No Screenshots Found", comment: "No screen shots found")
            
            headline = NSLocalizedString("Screenshots", comment: "")
            informationalText = NSLocalizedString("Create your personal collection of screenshots. To take a screenshot, you can use the keyboard shortcut ⌘ + T while playing a game.", comment: "")
        }
        else if let representedObject = representedObject as? Mode {
            
            switch representedObject {
            case .homebrewLoading:
                isLoading = true
                boxText = NSLocalizedString("Fetching Games…", comment: "Homebrew Blank Slate View Updating Info")
                
                headline = NSLocalizedString("Homebrew Games", comment: "")
                informationalText = NSLocalizedString("Check out some excellent homebrew games.", comment: "")
            case let .homebrewError(error):
                boxImage = NSImage(named: "blank_slate_warning")
                boxText = NSLocalizedString("No Internet Connection", comment: "Homebrew Blank Slate View Error Info")
                
                headline = NSLocalizedString("Homebrew Games", comment: "")
                informationalText = error?.localizedDescription ?? ""
            }
        }
        else {
            assertionFailure("Unknown represented object: \(String(describing: representedObject))")
        }
    }
    
    private func setUpView(systemPlugin plugin: OESystemPlugin) {
        let hStack = NSStackView()
        hStack.orientation = .horizontal
        hStack.alignment = .top
        
        containerView.addSubview(hStack)
        
        hStack.translatesAutoresizingMaskIntoConstraints = false
        NSLayoutConstraint.activate([
            hStack.leadingAnchor.constraint(equalTo: containerView.leadingAnchor, constant: -4),
            hStack.trailingAnchor.constraint(equalTo: containerView.trailingAnchor),
            hStack.topAnchor.constraint(equalTo: containerView.topAnchor, constant: Layout.bottomTextToTop),
            hStack.heightAnchor.constraint(equalToConstant: Layout.bottomTextHeight),
        ])
        
        let textView = ArrowCursorTextView()
        
        let text: String
        if plugin.supportsDiscsWithDescriptorFile {
            text = .localizedStringWithFormat(NSLocalizedString("%@ games will appear here. Check out %@ on how to add disc-based games.", comment: ""), plugin.systemName, NSLocalizedString("this guide", comment: ""))
        } else {
            text = .localizedStringWithFormat(NSLocalizedString("%@ games you add to OpenEmu will appear in this Console Library", comment: ""), plugin.systemName)
        }
        
        textView.drawsBackground = false
        textView.isEditable = false
        textView.selectionGranularity = .selectByCharacter
        textView.delegate = self
        textView.font = .systemFont(ofSize: 12)
        textView.textColor = .labelColor
        textView.textContainerInset = .zero
        
        let paraStyle = NSMutableParagraphStyle()
        paraStyle.lineSpacing = 2
        
        textView.defaultParagraphStyle = paraStyle
        textView.string = text
        
        var attributes = textView.typingAttributes
        attributes[.cursor] = NSCursor.arrow
        
        textView.typingAttributes = attributes
        textView.markedTextAttributes = attributes
        textView.selectedTextAttributes = attributes
        
        if plugin.supportsDiscsWithDescriptorFile {
            let linkRange = (textView.string as NSString).range(of: NSLocalizedString("this guide", comment: "this guide"))
            textView.textStorage?.addAttribute(.link, value: URL.userGuideCDBasedGames, range: linkRange)
        }
        
        var linkAttributes = attributes
        linkAttributes[.underlineStyle] = NSUnderlineStyle.single.rawValue
        linkAttributes[.cursor] = NSCursor.pointingHand
        textView.linkTextAttributes = linkAttributes
        
        textView.translatesAutoresizingMaskIntoConstraints = false
        textView.widthAnchor.constraint(equalToConstant: Layout.containerSize.width / 8 * 5).isActive = true
        
        hStack.addArrangedSubview(textView)
        
        let coreIconView = NSImageView()
        coreIconView.image = NSImage(named: "blank_slate_core_icon")
        coreIconView.unregisterDraggedTypes()
        
        hStack.addArrangedSubview(coreIconView)
        
        let vStack = NSStackView()
        vStack.orientation = .vertical
        vStack.alignment = .leading
        
        hStack.addArrangedSubview(vStack)
        
        let coreSuppliedByLabel = NSTextField(labelWithString: NSLocalizedString("Core Provided By…", comment: ""))
        coreSuppliedByLabel.font = .systemFont(ofSize: 12)
        coreSuppliedByLabel.textColor = .labelColor
        
        vStack.addArrangedSubview(coreSuppliedByLabel)
        vStack.setCustomSpacing(2, after: coreSuppliedByLabel)
        
        // Get core plugins that can handle the system
        let pluginsForSystem = OECorePlugin.allPlugins.filter { $0.systemIdentifiers.contains(plugin.systemIdentifier) }
        
        for core in pluginsForSystem {
            let projectURL = core.infoDictionary["OEProjectURL"] as? String
            let name = core.displayName
            
            // Create weblink button for current core
            
            let gotoButton = TextButton()
            gotoButton.alignment = .left
            gotoButton.target = self
            gotoButton.action = #selector(openProjectURL(_:))
            gotoButton.title = name
            gotoButton.toolTip = .localizedStringWithFormat(NSLocalizedString("Takes you to the %@ project website", comment: "Weblink tooltip"), name)
            gotoButton.cell?.representedObject = projectURL
            gotoButton.font = .boldSystemFont(ofSize: 12)
            gotoButton.textColor = .labelColor
            gotoButton.textColorHover = .labelColor.withSystemEffect(.rollover)
            gotoButton.showArrow = true
            
            gotoButton.translatesAutoresizingMaskIntoConstraints = false
            gotoButton.heightAnchor.constraint(equalToConstant: 12).isActive = true
            
            vStack.addArrangedSubview(gotoButton)
            vStack.setCustomSpacing(3, after: gotoButton)
        }
    }
    
    // MARK: - Actions
    
    @objc func openProjectURL(_ sender: NSButton) {
        if let urlString = sender.cell?.representedObject as? String,
           let url = URL(string: urlString) {
            NSWorkspace.shared.open(url)
        }
    }
}

// MARK: - CALayerDelegate

extension BlankSlateView: CALayerDelegate {
    
    func layoutSublayers(of layer: CALayer) {
        CATransaction.begin()
        defer { CATransaction.commit() }
        CATransaction.setDisableActions(true)
        
        let bounds = bounds
        let contentLayoutRect = window!.contentLayoutRect
        
        for layer in layer.sublayers ?? [] {
            
            if layer == dragIndicationLayer {
                
                var dragIndicationFrame = bounds
                dragIndicationFrame.size.height = contentLayoutRect.height
                
                layer.frame = dragIndicationFrame.insetBy(dx: 1, dy: 1)
            } else if layer != subviews.last?.layer {
                layer.frame = bounds
            }
        }
    }
}

// MARK: - NSTextViewDelegate

extension BlankSlateView: NSTextViewDelegate {
    
    func textView(_ textView: NSTextView, clickedOnLink link: Any, at charIndex: Int) -> Bool {
        if let link = link as? URL {
            NSWorkspace.shared.open(link)
        }
        return true
    }
    
    func textView(_ textView: NSTextView, willChangeSelectionFromCharacterRange oldSelectedCharRange: NSRange, toCharacterRange newSelectedCharRange: NSRange) -> NSRange {
        return NSRange(location: 0, length: 0)
    }
}

// MARK: - NSDraggingDestination

extension BlankSlateView {
    
    override func draggingEntered(_ sender: NSDraggingInfo) -> NSDragOperation {
        // The delegate has to be able to validate and accept drops, if it can't do then there is no need to drag anything around
        if let delegate = delegate,
           delegate.responds(to: #selector(BlankSlateViewDelegate.blankSlateView(_:validateDrop:))),
           delegate.responds(to: #selector(BlankSlateViewDelegate.blankSlateView(_:acceptDrop:))) {
            lastDragOperation = delegate.blankSlateView!(self, validateDrop: sender)
        }
        
        dragIndicationLayer.isHidden = lastDragOperation == []
        
        return lastDragOperation
    }
    
    override func draggingUpdated(_ sender: NSDraggingInfo) -> NSDragOperation {
        if let delegate = delegate,
           delegate.responds(to: #selector(BlankSlateViewDelegate.blankSlateView(_:draggingUpdated:))) {
            lastDragOperation = delegate.blankSlateView!(self, draggingUpdated: sender)
        }
        
        dragIndicationLayer.isHidden = lastDragOperation == []
        
        return lastDragOperation
    }
    
    override func draggingExited(_ sender: NSDraggingInfo?) {
        dragIndicationLayer.isHidden = true
    }
    
    override func performDragOperation(_ sender: NSDraggingInfo) -> Bool {
        dragIndicationLayer.isHidden = true
        return delegate?.responds(to: #selector(BlankSlateViewDelegate.blankSlateView(_:acceptDrop:))) ?? false && delegate?.blankSlateView?(self, acceptDrop: sender) ?? false
    }
}

// MARK: - BlankSlateViewDelegate

@objc(OEBlankSlateViewDelegate)
protocol BlankSlateViewDelegate: NSObjectProtocol {
    @objc optional func blankSlateView(_ blankSlateView: BlankSlateView, validateDrop sender: NSDraggingInfo) -> NSDragOperation
    @objc optional func blankSlateView(_ blankSlateView: BlankSlateView, draggingUpdated sender: NSDraggingInfo) -> NSDragOperation
    @objc optional func blankSlateView(_ blankSlateView: BlankSlateView, acceptDrop sender: NSDraggingInfo) -> Bool
}
