import Cocoa

final class PrefAppearanceController: NSViewController {
    private let useLiquidGlassKey = "OEUseLiquidGlass"
    private let materialKey = "OEVisualEffectMaterial"
    private let appearanceDidChangeNotification = Notification.Name("OEAppearanceDidChange")

    private var enableCheckbox: NSButton!
    private var materialPopup: NSPopUpButton!
    private var previewEffect: NSVisualEffectView!
    private var opacitySlider: NSSlider!
    private var opacityLabel: NSTextField!

    override func loadView() {
        view = NSView()
        view.translatesAutoresizingMaskIntoConstraints = false

        enableCheckbox = NSButton(checkboxWithTitle: NSLocalizedString("Enable Liquid Glass (translucent UI)", comment: ""), target: self, action: #selector(toggleEnabled(_:)))
        enableCheckbox.translatesAutoresizingMaskIntoConstraints = false

        materialPopup = NSPopUpButton(frame: .zero, pullsDown: false)
        materialPopup.translatesAutoresizingMaskIntoConstraints = false
        materialPopup.addItems(withTitles: [NSLocalizedString("Sidebar", comment: ""), NSLocalizedString("Content Background", comment: ""), NSLocalizedString("Titlebar", comment: "")])
        materialPopup.target = self
        materialPopup.action = #selector(materialChanged(_:))

        let materialLabel = NSTextField(labelWithString: NSLocalizedString("Material:", comment: ""))
        materialLabel.translatesAutoresizingMaskIntoConstraints = false

        opacitySlider = NSSlider(value: 1.0, minValue: 0.2, maxValue: 1.0, target: self, action: #selector(opacityChanged(_:)))
        opacitySlider.translatesAutoresizingMaskIntoConstraints = false
        opacityLabel = NSTextField(labelWithString: NSLocalizedString("Opacity:", comment: ""))
        opacityLabel.translatesAutoresizingMaskIntoConstraints = false

        previewEffect = NSVisualEffectView()
        previewEffect.translatesAutoresizingMaskIntoConstraints = false
        previewEffect.wantsLayer = true
        previewEffect.layer?.cornerRadius = 8
        previewEffect.layer?.borderWidth = 1
        previewEffect.layer?.borderColor = NSColor.separatorColor.cgColor
        previewEffect.blendingMode = .withinWindow
        previewEffect.state = .active

        view.addSubview(enableCheckbox)
        view.addSubview(materialLabel)
        view.addSubview(materialPopup)
        view.addSubview(opacityLabel)
        view.addSubview(opacitySlider)
        view.addSubview(previewEffect)

        NSLayoutConstraint.activate([
            enableCheckbox.topAnchor.constraint(equalTo: view.topAnchor, constant: 20),
            enableCheckbox.leadingAnchor.constraint(equalTo: view.leadingAnchor, constant: 20),

            materialLabel.topAnchor.constraint(equalTo: enableCheckbox.bottomAnchor, constant: 18),
            materialLabel.leadingAnchor.constraint(equalTo: enableCheckbox.leadingAnchor),

            materialPopup.centerYAnchor.constraint(equalTo: materialLabel.centerYAnchor),
            materialPopup.leadingAnchor.constraint(equalTo: materialLabel.trailingAnchor, constant: 8),

            opacityLabel.topAnchor.constraint(equalTo: materialLabel.bottomAnchor, constant: 14),
            opacityLabel.leadingAnchor.constraint(equalTo: enableCheckbox.leadingAnchor),

            opacitySlider.centerYAnchor.constraint(equalTo: opacityLabel.centerYAnchor),
            opacitySlider.leadingAnchor.constraint(equalTo: opacityLabel.trailingAnchor, constant: 8),
            opacitySlider.widthAnchor.constraint(equalToConstant: 160),

            previewEffect.topAnchor.constraint(equalTo: opacityLabel.bottomAnchor, constant: 18),
            previewEffect.leadingAnchor.constraint(equalTo: view.leadingAnchor, constant: 20),
            previewEffect.widthAnchor.constraint(equalToConstant: 240),
            previewEffect.heightAnchor.constraint(equalToConstant: 120),

            view.trailingAnchor.constraint(greaterThanOrEqualTo: previewEffect.trailingAnchor, constant: 20),
            view.bottomAnchor.constraint(greaterThanOrEqualTo: previewEffect.bottomAnchor, constant: 20)
        ])
    }

    override func viewDidLoad() {
        super.viewDidLoad()
        // ensure default opacity exists
        let defaults = UserDefaults.standard
        if defaults.object(forKey: "OEVisualEffectOpacity") == nil {
            defaults.set(1.0, forKey: "OEVisualEffectOpacity")
        }
        loadSettings()
    }

    private func loadSettings() {
        let defaults = UserDefaults.standard
        let enabled = defaults.object(forKey: useLiquidGlassKey) as? Bool ?? true
        enableCheckbox.state = enabled ? .on : .off

        let requestedMaterial = defaults.string(forKey: materialKey) ?? "sidebar"
        switch requestedMaterial {
        case "contentBackground":
            materialPopup.selectItem(at: 1)
        case "titlebar":
            materialPopup.selectItem(at: 2)
        default:
            materialPopup.selectItem(at: 0)
        }

        let alpha = defaults.double(forKey: "OEVisualEffectOpacity")
        opacitySlider.doubleValue = alpha > 0 ? alpha : 1.0

        updatePreview()
    }

    @objc private func toggleEnabled(_ sender: NSButton) {
        let enabled = sender.state == .on
        UserDefaults.standard.set(enabled, forKey: useLiquidGlassKey)
        NotificationCenter.default.post(name: appearanceDidChangeNotification, object: nil)
        updatePreview()
    }

    @objc private func materialChanged(_ sender: NSPopUpButton) {
        let index = sender.indexOfSelectedItem
        var value = "sidebar"
        if index == 1 { value = "contentBackground" }
        else if index == 2 { value = "titlebar" }
        UserDefaults.standard.set(value, forKey: materialKey)
        NotificationCenter.default.post(name: appearanceDidChangeNotification, object: nil)
        updatePreview()
    }

    @objc private func opacityChanged(_ sender: NSSlider) {
        let value = sender.doubleValue
        UserDefaults.standard.set(value, forKey: "OEVisualEffectOpacity")
        NotificationCenter.default.post(name: appearanceDidChangeNotification, object: nil)
        updatePreview()
    }

    private func updatePreview() {
        let defaults = UserDefaults.standard
        let enabled = defaults.object(forKey: useLiquidGlassKey) as? Bool ?? true
        let alpha = defaults.double(forKey: "OEVisualEffectOpacity")
        previewEffect.isHidden = !enabled || NSWorkspace.shared.accessibilityDisplayShouldReduceTransparency

        if previewEffect.isHidden { return }

        let requestedMaterial = defaults.string(forKey: materialKey) ?? "sidebar"
        if #available(macOS 10.14, *) {
            switch requestedMaterial {
            case "contentBackground": previewEffect.material = .contentBackground
            case "titlebar": previewEffect.material = .titlebar
            default: previewEffect.material = .sidebar
            }
        }
        previewEffect.alphaValue = alpha > 0 ? CGFloat(alpha) : 1.0
    }
}

// MARK: - PreferencePane

extension PrefAppearanceController: PreferencePane {
    var icon: NSImage? { NSImage(named: "preferences_tab_icon") }
    var panelTitle: String { "Appearance" }
    var viewSize: NSSize { NSSize(width: 480, height: 240) }
}