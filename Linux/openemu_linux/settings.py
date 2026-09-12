"""User settings and the Libretro core library."""
from pathlib import Path

from PySide6.QtCore import Qt, Signal
from PySide6.QtGui import QIcon
from PySide6.QtMultimedia import QMediaDevices
from PySide6.QtWidgets import (QDialog, QVBoxLayout, QHBoxLayout, QTabWidget, QWidget, QFormLayout,
    QLabel, QComboBox, QCheckBox, QSlider, QLineEdit, QPushButton, QFileDialog, QTableWidget,
    QTableWidgetItem, QHeaderView, QAbstractItemView, QSpinBox)

from .core import CORES
from .core_manager import CATALOG, CoreManager, CoreWorker
from .library import SYSTEMS


class SettingsDialog(QDialog):
    changed = Signal()

    def __init__(self, library, parent=None):
        super().__init__(parent)
        self.library = library
        self.manager = CoreManager(library.root)
        self.worker = None
        self.close_pending = False
        self.refreshing = False
        self.setWindowTitle("Settings — OpenEmu Linux")
        self.resize(840, 620)
        self.setMinimumSize(740, 550)
        self.setStyleSheet("""
            QDialog,QWidget {background:#24252b;color:#ebeaf0;font-size:13px}
            QTabWidget::pane {border:1px solid #464751;border-radius:5px}
            QTabBar::tab {background:#303139;padding:10px 20px}
            QTabBar::tab:selected {background:#514361}
            QComboBox,QLineEdit {background:#303139;border:1px solid #555662;padding:7px;border-radius:5px}
            QComboBox QAbstractItemView {background:#303139;selection-background-color:#514361}
            QPushButton {background:#383943;border:1px solid #555662;padding:9px 13px;border-radius:6px}
            QPushButton:hover {background:#494452}
            QPushButton:disabled {color:#777;background:#303139}
            QTableWidget {background:#202127;gridline-color:#383943;border:0;selection-background-color:#514361}
            QHeaderView::section {background:#303139;color:#ddd;padding:8px;border:0}
            QCheckBox {spacing:9px;padding:5px}
            QSlider::groove:horizontal {background:#444550;height:5px}
            QSlider::handle:horizontal {background:#b6a0e4;width:16px;margin:-5px 0;border-radius:7px}
        """)
        layout = QVBoxLayout(self)
        layout.setContentsMargins(22, 22, 22, 18)
        heading = QLabel("Settings")
        heading.setStyleSheet("font-size:25px;font-weight:650")
        layout.addWidget(heading)
        note = QLabel("Game settings and core choices apply the next time you open a game.")
        note.setWordWrap(True)
        note.setStyleSheet("color:#aaa8b5;padding-bottom:10px")
        layout.addWidget(note)
        self.tabs = QTabWidget()
        layout.addWidget(self.tabs, 1)
        self.general_tab()
        self.gameplay_tab()
        self.library_tab()
        self.cores_tab()
        self.downloads_tab()
        self.status = QLabel("Settings are saved automatically.")
        self.status.setWordWrap(True)
        self.status.setMinimumHeight(42)
        layout.addWidget(self.status)
        buttons = QHBoxLayout()
        buttons.addStretch()
        close = QPushButton("Done")
        close.clicked.connect(self.accept)
        buttons.addWidget(close)
        layout.addLayout(buttons)
        self.refresh_cores()

    def set_setting(self, key, value):
        self.library.set_setting(key, value)
        self.changed.emit()

    def checkbox(self, form, title, key, default):
        box = QCheckBox(title)
        box.setChecked(self.library.setting(key, default) == "1")
        box.toggled.connect(lambda value: self.set_setting(key, int(value)))
        form.addRow(box)
        return box

    def general_tab(self):
        page = QWidget()
        form = QFormLayout(page)
        form.setContentsMargins(20, 22, 20, 20)
        form.setSpacing(14)
        self.volume = QSlider(Qt.Horizontal)
        self.volume.setRange(0, 100)
        self.volume.setValue(int(self.library.setting("volume", "80")))
        self.volume.valueChanged.connect(lambda value: self.set_setting("volume", value))
        form.addRow("Game volume", self.volume)
        self.focus_pause = self.checkbox(form, "Pause games when switching to another window", "pause_unfocused", "1")
        self.integer = self.checkbox(form, "Use integer scaling for sharp pixels", "integer_scale", "0")
        self.auto_art = self.checkbox(form, "Automatically download box art", "artwork_auto", "1")
        self.backup_art = self.checkbox(form, "Use backup artwork source when needed", "artwork_backup", "1")
        folder = QWidget()
        row = QHBoxLayout(folder)
        row.setContentsMargins(0, 0, 0, 0)
        self.bios_path = QLineEdit(self.library.setting("bios_directory", str(self.library.root / "system")))
        self.bios_path.setReadOnly(True)
        row.addWidget(self.bios_path, 1)
        browse = QPushButton("Choose…")
        browse.clicked.connect(self.choose_bios)
        row.addWidget(browse)
        form.addRow("BIOS / system folder", folder)
        explanation = QLabel("Use this folder for files required by a core. BIOS files are not downloaded by the app.")
        explanation.setWordWrap(True)
        explanation.setStyleSheet("color:#aaa8b5")
        form.addRow(explanation)
        self.tabs.addTab(page, "General")

    def choose_bios(self):
        path = QFileDialog.getExistingDirectory(self, "Choose BIOS / system folder", self.bios_path.text())
        if path:
            self.bios_path.setText(path)
            self.set_setting("bios_directory", path)

    def gameplay_tab(self):
        page = QWidget()
        form = QFormLayout(page)
        form.setContentsMargins(20, 22, 20, 20)
        form.setSpacing(16)
        self.fullscreen_default = self.checkbox(form, "Open games in fullscreen", "fullscreen_default", "0")
        self.hide_cursor = self.checkbox(form, "Hide the pointer when gameplay controls disappear", "hide_game_cursor", "1")
        self.fast_speed = QSpinBox()
        self.fast_speed.setRange(2, 10)
        self.fast_speed.setSuffix("×")
        try:
            self.fast_speed.setValue(int(self.library.setting("fast_forward_speed", "3")))
        except ValueError:
            self.fast_speed.setValue(3)
        self.fast_speed.valueChanged.connect(lambda value: self.set_setting("fast_forward_speed", value))
        form.addRow("Fast-forward speed", self.fast_speed)
        self.aspect = QComboBox()
        self.aspect.addItem("System default", "system")
        self.aspect.addItem("Square pixels", "square")
        self.aspect.setCurrentIndex(max(0, self.aspect.findData(self.library.setting("aspect_mode", "system"))))
        self.aspect.currentIndexChanged.connect(lambda _: self.set_setting("aspect_mode", self.aspect.currentData()))
        form.addRow("Display aspect ratio", self.aspect)
        self.audio_output = QComboBox()
        self.audio_output.currentIndexChanged.connect(self.select_audio_output)
        form.addRow("Audio output", self.audio_output)
        self.audio_devices = QMediaDevices(self)
        self.audio_devices.audioOutputsChanged.connect(self.refresh_audio_outputs)
        self.refresh_audio_outputs()
        self.latency = QSpinBox()
        self.latency.setRange(20, 250)
        self.latency.setSuffix(" ms")
        try:
            self.latency.setValue(int(self.library.setting("audio_latency", "80")))
        except ValueError:
            self.latency.setValue(80)
        self.latency.valueChanged.connect(lambda value: self.set_setting("audio_latency", value))
        form.addRow("Audio buffer target", self.latency)
        note = QLabel("Move the mouse over a game to show its controls. Hold Tab to fast-forward; audio is muted while fast-forwarding. Lower audio buffers can reduce delay but may cause crackling on some devices. Integer scaling uses square pixels.")
        note.setWordWrap(True)
        note.setStyleSheet("color:#aaa8b5")
        form.addRow(note)
        self.tabs.addTab(page, "Gameplay")

    def refresh_audio_outputs(self):
        self.audio_output.blockSignals(True)
        selected = self.library.setting("audio_device", "")
        self.audio_output.clear()
        self.audio_output.addItem("System default", "")
        for device in QMediaDevices.audioOutputs():
            self.audio_output.addItem(device.description(), bytes(device.id()).hex())
        index = self.audio_output.findData(selected)
        if index < 0:
            self.audio_output.addItem("Disconnected device (uses system default)", selected)
            index = self.audio_output.count() - 1
        self.audio_output.setCurrentIndex(index)
        self.audio_output.blockSignals(False)

    def select_audio_output(self):
        if self.audio_output.currentIndex() >= 0:
            self.set_setting("audio_device", self.audio_output.currentData())

    def library_tab(self):
        page = QWidget()
        form = QFormLayout(page)
        form.setContentsMargins(20, 22, 20, 20)
        form.setSpacing(18)
        location = QLineEdit(str(self.library.root))
        location.setReadOnly(True)
        location.setCursorPosition(0)
        form.addRow("Library location", location)
        self.copy_games = self.checkbox(form, "Copy games into library when importing", "copy_games", "1")
        self.auto_metadata = self.checkbox(form, "Automatically look up game information", "metadata_auto", "1")
        note = QLabel("When enabled, copies are organized by system and keep their original filenames.\n\nWhen disabled, games stay in their current folders. Keep those files and drives available to play. ZIP imports always extract a managed copy.\n\nThis setting applies to new imports. To copy an existing external game, right-click it and choose Consolidate files into library. Originals are never moved or deleted.")
        note.setWordWrap(True)
        form.addRow(note)
        self.tabs.addTab(page, "Library")

    def cores_tab(self):
        page = QWidget()
        layout = QVBoxLayout(page)
        layout.setContentsMargins(20, 22, 20, 20)
        form = QFormLayout()
        form.setSpacing(18)
        self.choices = {}
        for key in ("gb", "gbc", "gba", "snes"):
            system = SYSTEMS[key]
            label = QWidget()
            row = QHBoxLayout(label)
            row.setContentsMargins(0, 0, 12, 0)
            icon = QLabel()
            icon.setPixmap(QIcon(str(system.icon)).pixmap(24, 24))
            row.addWidget(icon)
            row.addWidget(QLabel(system.name))
            combo = QComboBox()
            combo.currentIndexChanged.connect(lambda _, s=key: self.select_core(s))
            self.choices[key] = combo
            form.addRow(label, combo)
        layout.addLayout(form)
        description = QLabel("OpenEmu cores are bundled with this build. Download or import Libretro cores in the Core downloads tab, then select them here.\n\nBattery saves and save states are kept separately for each core. Save states also belong to the exact core build that created them.")
        description.setWordWrap(True)
        description.setStyleSheet("color:#aaa8b5;padding-top:20px")
        layout.addWidget(description)
        layout.addStretch()
        self.tabs.addTab(page, "Core selection")

    def select_core(self, system):
        if not self.refreshing:
            self.set_setting("core." + system, self.choices[system].currentData())

    def downloads_tab(self):
        page = QWidget()
        layout = QVBoxLayout(page)
        layout.setContentsMargins(16, 18, 16, 16)
        label = QLabel("Official Linux cores used by RetroArch. They run directly in OpenEmu Linux.")
        label.setWordWrap(True)
        layout.addWidget(label)
        self.table = QTableWidget(0, 3)
        self.table.setHorizontalHeaderLabels(["Core", "Systems", "Installed build"])
        self.table.horizontalHeader().setSectionResizeMode(QHeaderView.ResizeToContents)
        self.table.horizontalHeader().setSectionResizeMode(2, QHeaderView.Stretch)
        self.table.verticalHeader().hide()
        self.table.setSelectionBehavior(QAbstractItemView.SelectRows)
        self.table.setSelectionMode(QAbstractItemView.SingleSelection)
        self.table.setEditTriggers(QAbstractItemView.NoEditTriggers)
        self.table.itemSelectionChanged.connect(self.update_buttons)
        layout.addWidget(self.table, 1)
        controls = QHBoxLayout()
        self.download_button = QPushButton("Download / update selected")
        self.download_button.clicked.connect(self.download_selected)
        controls.addWidget(self.download_button)
        self.update_button = QPushButton("Update all installed")
        self.update_button.clicked.connect(lambda: self.start_job(update_all=True))
        controls.addWidget(self.update_button)
        self.import_button = QPushButton("Import core file…")
        self.import_button.clicked.connect(self.import_core)
        controls.addWidget(self.import_button)
        layout.addLayout(controls)
        note = QLabel("GB, GBC, GBA and SNES · software video · Linux .so files\nUpdates are checked when requested. Running games keep their current build.")
        note.setStyleSheet("color:#aaa8b5;font-size:11px")
        note.setWordWrap(True)
        layout.addWidget(note)
        self.tabs.addTab(page, "Core downloads")

    def refresh_cores(self):
        self.refreshing = True
        records = self.manager.installed()
        for system, combo in self.choices.items():
            selected = self.library.setting("core." + system, "builtin")
            combo.clear()
            name = {"gambatte": "Gambatte", "mgba": "mGBA", "snes9x": "Snes9x"}[CORES[system]]
            combo.addItem(f"{name} — OpenEmu", "builtin")
            for core_id, record in records.items():
                if system in record["systems"]:
                    combo.addItem(f"{record['name']} — Libretro ({record['version']})", core_id)
            index = combo.findData(selected)
            if index < 0:
                combo.addItem("Missing core — choose another", selected)
                index = combo.count() - 1
            combo.setCurrentIndex(index)
        selected_id = self.table.item(self.table.currentRow(), 0).data(Qt.UserRole) if self.table.currentRow() >= 0 else None
        all_cores = dict(CATALOG)
        all_cores.update({key: record for key, record in records.items() if key not in all_cores})
        self.table.setRowCount(len(all_cores))
        for row, (core_id, info) in enumerate(all_cores.items()):
            record = records.get(core_id)
            installed = f"{record['version']} · {record['build'] or 'Imported'}" if record else "Not installed"
            for column, value in enumerate((info["name"], " / ".join(s.upper() for s in info["systems"]), installed)):
                item = QTableWidgetItem(value)
                item.setData(Qt.UserRole, core_id)
                item.setToolTip(value)
                self.table.setItem(row, column, item)
            if core_id == selected_id:
                self.table.selectRow(row)
        if self.table.currentRow() < 0:
            self.table.selectRow(0)
        self.refreshing = False
        self.update_buttons()

    def update_buttons(self):
        row = self.table.currentRow()
        item = self.table.item(row, 0) if row >= 0 else None
        self.download_button.setEnabled(not self.worker and bool(item) and item.data(Qt.UserRole) in CATALOG)
        self.import_button.setEnabled(not self.worker)
        self.update_button.setEnabled(not self.worker and any(r["source"] == "buildbot" for r in self.manager.installed().values()))

    def download_selected(self):
        row = self.table.currentRow()
        if row >= 0:
            self.start_job(core_id=self.table.item(row, 0).data(Qt.UserRole))

    def import_core(self):
        path, _ = QFileDialog.getOpenFileName(self, "Import a Linux Libretro core", "", "Linux cores (*.so)")
        if path:
            self.start_job(local_file=path)

    def start_job(self, **kwargs):
        if self.worker:
            return
        self.worker = CoreWorker(self.library.root, **kwargs)
        self.worker.progress.connect(self.status.setText)
        self.worker.result.connect(self.job_result)
        self.worker.finished.connect(self.job_finished)
        self.status.setText("Contacting the core server…")
        self.update_buttons()
        self.worker.start()

    def job_result(self, message, success):
        self.status.setText(message)
        self.status.setStyleSheet("color:#b3d69b" if success else "color:#f0b79d")

    def job_finished(self):
        self.worker.deleteLater()
        self.worker = None
        self.refresh_cores()
        self.changed.emit()
        if self.close_pending:
            super().done(QDialog.Accepted)

    def done(self, result):
        if self.worker:
            self.close_pending = True
            self.worker.requestInterruption()
            self.status.setText("Stopping the download…")
            return
        super().done(result)
