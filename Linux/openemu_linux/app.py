"""Qt Linux implementation of the OpenEmu game-library window."""
import argparse
import json
import os
from pathlib import Path
import sys

from PySide6.QtCore import Qt, QSize, QProcess, QThread, Signal, QTimer, QRect, QUrl, QItemSelectionModel
from PySide6.QtGui import QAction, QColor, QDesktopServices, QFont, QFontMetrics, QIcon, QPainter, QPixmap, QLinearGradient
from PySide6.QtWidgets import (QApplication, QMainWindow, QWidget, QHBoxLayout, QVBoxLayout,
    QLabel, QPushButton, QListWidget, QListWidgetItem, QLineEdit, QFileDialog, QMessageBox,
    QInputDialog, QMenu, QAbstractItemView, QStackedWidget, QSlider, QButtonGroup, QHeaderView, QProgressBar)

from . import __version__
from .artwork import ArtworkWorker
from .core import ROOT, CORES
from .library import Library, SYSTEMS, EXTENSIONS
from .importing import Importer, ImportIssuesDialog
from .metadata import MetadataWorker
from .library_widgets import (GameGrid, GameTable, CoverDelegate, LibrarySidebar, SortItem, SmartCollectionDialog,
                              GameInfoDialog, date_text)

STYLE = """
QMainWindow, QWidget { background:#24252b; color:#ebeaf0; font-family:'Noto Sans'; font-size:13px; }
QWidget#sidebar {background:#1b1c21; border-right:1px solid #35363e;}
QLabel#brand {font-size:24px; font-weight:700; background:transparent;}
QLabel#eyebrow {color:#9495a2; font-size:10px; letter-spacing:2px; background:transparent;}
QLabel#heading {font-size:29px; font-weight:650;}
QLabel#subtle {color:#a4a5b2; background:transparent;}
QListWidget {border:0; outline:0; background:transparent;}
QListWidget#nav::item {height:28px; border-radius:6px; padding-left:12px; margin:2px 0;}
QListWidget#nav::item:selected {background:#393545; color:#dfd0ff;}
QListWidget#nav::item:hover {background:#2c2d35;}
QTableWidget {background:#24252b; alternate-background-color:#292a31; border:0; gridline-color:#383943; selection-background-color:#494052;}
QHeaderView::section {background:#303139; color:#c7c5d1; padding:8px; border:0; border-right:1px solid #40414a;}
QPushButton {background:#383943; border:1px solid #50515c; padding:9px 17px; border-radius:7px;}
QPushButton:hover {background:#474854;}
QPushButton:checked {background:#554866; border-color:#a993d8;}
QPushButton#primary {background:#b6a0e4; color:#201b2b; border:0; font-weight:700;}
QPushButton#primary:hover {background:#c6b2ee;}
QPushButton:disabled {color:#70717d; background:#303139; border-color:#393a43;}
QPushButton#primary:disabled {color:#70717d; background:#303139; border-color:#393a43;}
QLineEdit {border:1px solid #464751; border-radius:7px; background:#1d1e24; padding:10px;}
QMenuBar,QMenu {background:#202127;} QMenu::item:selected {background:#494052;}
QStatusBar {background:#202127; color:#a4a5b2; padding:5px;}
QToolTip {color:#eee; background:#303139; border:1px solid #777;}
QScrollBar:vertical {background:#24252b; width:9px; margin:0;}
QScrollBar:horizontal {background:#24252b; height:9px; margin:0;}
QScrollBar::handle {background:#555560; border-radius:4px; min-height:24px; min-width:24px;}
QScrollBar::add-line, QScrollBar::sub-line {width:0; height:0;}
QScrollBar::add-page, QScrollBar::sub-page {background:none;}
"""


def cover_pixmap(game, library):
    if game["cover"]:
        p = QPixmap(str(library.root / game["cover"]))
        if not p.isNull():
            cover = p.scaled(256, 256, Qt.KeepAspectRatio, Qt.SmoothTransformation)
            frame = QPixmap(256, 256)
            frame.fill(Qt.transparent)
            painter = QPainter(frame)
            painter.drawPixmap((256 - cover.width()) // 2, 256 - cover.height(), cover)
            painter.end()
            return frame
    p = QPixmap(256, 256)
    p.fill(QColor("#353640"))
    painter = QPainter(p)
    painter.scale(256 / 176, 256 / 176)
    painter.setRenderHint(QPainter.Antialiasing)
    accent = QColor(SYSTEMS[game["system"]].color)
    gradient = QLinearGradient(0, 0, 176, 176)
    gradient.setColorAt(0, accent.darker(145))
    gradient.setColorAt(1, accent.darker(360))
    painter.fillRect(p.rect(), gradient)
    painter.setPen(QColor(255, 255, 255, 28))
    for x in range(-176, 352, 20):
        painter.drawLine(x, 176, x + 176, 0)
    painter.setPen(accent.lighter(145))
    painter.setFont(QFont("Noto Sans", 9, QFont.Bold))
    painter.drawText(QRect(16, 14, 144, 26), Qt.AlignLeft, game["system"].upper())
    font = QFont("Noto Sans", 17, QFont.Bold)
    text_rect = QRect(16, 48, 144, 99)
    title = game["title"][:55]
    while font.pointSize() > 10 and QFontMetrics(font).boundingRect(text_rect, Qt.TextWordWrap, title).height() > text_rect.height():
        font.setPointSize(font.pointSize() - 1)
    painter.setFont(font)
    painter.setPen(QColor("#fff"))
    painter.drawText(text_rect, Qt.AlignLeft | Qt.AlignVCenter | Qt.TextWordWrap, title)
    painter.setPen(accent)
    painter.drawLine(16, 159, 55, 159)
    painter.end()
    return p


class Window(QMainWindow):
    def __init__(self, library, *, auto_artwork=True, auto_metadata=None):
        super().__init__()
        self.library = library
        self.worker = None
        self.issue_dialog = None
        self.art_worker = None
        self.art_pending = []
        self.metadata_worker = None
        self.metadata_pending = []
        self.metadata_enabled = auto_artwork if auto_metadata is None else auto_metadata
        self.closing = False
        self.processes = {}
        self.rows = {}
        self.view_mode = self.library.setting("library.view", "grid")
        if self.view_mode not in ("grid", "list"):
            self.view_mode = "grid"
        self.setWindowTitle("OpenEmu Linux")
        self.resize(1140, 750)
        self.setMinimumSize(850, 560)
        self.setAcceptDrops(True)
        self.setStyleSheet(STYLE)
        outer = QWidget()
        self.setCentralWidget(outer)
        layout = QHBoxLayout(outer)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setSpacing(0)
        sidebar = QWidget()
        sidebar.setObjectName("sidebar")
        sidebar.setFixedWidth(260)
        side = QVBoxLayout(sidebar)
        side.setContentsMargins(20, 28, 18, 18)
        brand = QLabel("OpenEmu")
        brand.setObjectName("brand")
        side.addWidget(brand)
        sub = QLabel("L I N U X  ·  P R E V I E W")
        sub.setObjectName("eyebrow")
        side.addWidget(sub)
        side.addSpacing(34)
        label = QLabel("LIBRARY")
        label.setObjectName("eyebrow")
        side.addWidget(label)
        self.nav = LibrarySidebar()
        self.nav.setObjectName("nav")
        self.nav.setIconSize(QSize(24, 24))
        self.rebuild_sidebar(self.library.setting("library.section", "all"))
        self.nav.currentRowChanged.connect(self.refresh)
        self.nav.setAcceptDrops(True)
        self.nav.games_dropped.connect(self.add_to_collection)
        self.nav.setContextMenuPolicy(Qt.CustomContextMenu)
        self.nav.customContextMenuRequested.connect(self.collection_menu)
        side.addWidget(self.nav)
        collection_button = QPushButton("+  New collection")
        collection_options = QMenu(collection_button)
        collection_options.addAction("New collection…", self.new_collection)
        collection_options.addAction("New smart collection…", self.new_smart_collection)
        collection_button.setMenu(collection_options)
        side.addWidget(collection_button)
        demo = QPushButton("Try the test ROMs")
        demo.clicked.connect(self.demo)
        side.addWidget(demo)
        info = QLabel("Community Linux port\nOriginal OpenEmu cores")
        info.setObjectName("subtle")
        info.setStyleSheet("font-size:11px;line-height:1.5")
        side.addWidget(info)
        layout.addWidget(sidebar)
        content = QWidget()
        main = QVBoxLayout(content)
        main.setContentsMargins(32, 30, 30, 20)
        top = QHBoxLayout()
        titles = QVBoxLayout()
        self.heading = QLabel("All games")
        self.heading.setObjectName("heading")
        titles.addWidget(self.heading)
        self.count = QLabel()
        self.count.setObjectName("subtle")
        titles.addWidget(self.count)
        top.addLayout(titles)
        top.addStretch()
        self.import_button = QPushButton("+  Import games")
        self.import_button.setObjectName("primary")
        self.import_button.clicked.connect(self.choose_files)
        top.addWidget(self.import_button)
        main.addLayout(top)
        main.addSpacing(14)
        self.search = QLineEdit()
        self.search.setPlaceholderText("Search games, systems, filenames and information")
        self.search.setClearButtonEnabled(True)
        self.search.textChanged.connect(self.refresh)
        main.addWidget(self.search)
        controls = QHBoxLayout()
        self.grid_button = QPushButton("Grid")
        self.list_button = QPushButton("List")
        group = QButtonGroup(self)
        for button, mode in [(self.grid_button, "grid"), (self.list_button, "list")]:
            button.setCheckable(True)
            button.setChecked(self.view_mode == mode)
            group.addButton(button)
            button.clicked.connect(lambda checked=False, mode=mode: self.change_view(mode))
            controls.addWidget(button)
        controls.addStretch()
        controls.addWidget(QLabel("Cover size"))
        self.cover_size = QSlider(Qt.Horizontal)
        self.cover_size.setRange(96, 256)
        try:
            self.cover_size.setValue(int(self.library.setting("library.cover_size", "176")))
        except ValueError:
            self.cover_size.setValue(176)
        self.cover_size.setFixedWidth(130)
        self.cover_size.setAccessibleName("Cover size")
        controls.addWidget(self.cover_size)
        main.addLayout(controls)
        self.pages = QStackedWidget()
        self.games = GameGrid()
        self.games.setObjectName("games")
        self.games.setViewMode(QListWidget.IconMode)
        self.games.setResizeMode(QListWidget.Adjust)
        self.games.setMovement(QListWidget.Static)
        self.games.setIconSize(QSize(176, 176))
        self.games.setGridSize(QSize(204, 239))
        self.games.setWordWrap(True)
        self.games.setItemDelegate(CoverDelegate(self.games))
        self.games.setSelectionMode(QAbstractItemView.ExtendedSelection)
        self.games.setVerticalScrollMode(QAbstractItemView.ScrollPerPixel)
        self.games.setDragDropMode(QAbstractItemView.DragDrop)
        self.games.files_dropped.connect(self.files_dropped)
        self.games.itemDoubleClicked.connect(lambda _: self.play())
        self.games.itemSelectionChanged.connect(self.selected)
        self.games.setContextMenuPolicy(Qt.CustomContextMenu)
        self.games.customContextMenuRequested.connect(self.context_menu)
        self.pages.addWidget(self.games)
        empty = QWidget()
        empty_layout = QVBoxLayout(empty)
        empty_layout.setAlignment(Qt.AlignCenter)
        self.empty_title = QLabel("Your games, together.")
        self.empty_title.setAlignment(Qt.AlignCenter)
        self.empty_title.setStyleSheet("font-size:27px;font-weight:600;")
        empty_layout.addWidget(self.empty_title)
        self.empty_text = QLabel()
        self.empty_text.setAlignment(Qt.AlignCenter)
        self.empty_text.setWordWrap(True)
        self.empty_text.setObjectName("subtle")
        empty_layout.addWidget(self.empty_text)
        self.pages.addWidget(empty)
        self.table = GameTable(0, 8)
        self.table.setHorizontalHeaderLabels(["Game title", "System", "Rating", "Last played", "Date added", "Play count", "Core", "File status"])
        self.table.verticalHeader().hide()
        self.table.setAlternatingRowColors(True)
        self.table.setShowGrid(False)
        self.table.setSelectionBehavior(QAbstractItemView.SelectRows)
        self.table.setSelectionMode(QAbstractItemView.ExtendedSelection)
        self.table.setEditTriggers(QAbstractItemView.NoEditTriggers)
        self.table.setVerticalScrollMode(QAbstractItemView.ScrollPerPixel)
        self.table.setDragDropMode(QAbstractItemView.DragDrop)
        self.table.files_dropped.connect(self.files_dropped)
        self.table.setSortingEnabled(True)
        self.table.sortItems(0, Qt.AscendingOrder)
        self.table.horizontalHeader().setSectionResizeMode(QHeaderView.Interactive)
        self.table.horizontalHeader().setStretchLastSection(True)
        try:
            widths = json.loads(self.library.setting("library.columns", "[230,155,110,145,145,95,160,100]"))
            for i, width in enumerate(widths[:8]):
                self.table.setColumnWidth(i, max(60, min(1000, int(width))))
            sort = json.loads(self.library.setting("library.sort", "[0,0]"))
            if len(sort) == 2 and 0 <= sort[0] < 8 and sort[1] in (0, 1):
                self.table.sortItems(sort[0], Qt.SortOrder(sort[1]))
        except (ValueError, TypeError):
            pass
        self.table.itemSelectionChanged.connect(self.selected)
        self.table.itemDoubleClicked.connect(lambda _: self.play())
        self.table.setContextMenuPolicy(Qt.CustomContextMenu)
        self.table.customContextMenuRequested.connect(lambda position: self.context_menu(position, self.table))
        self.pages.addWidget(self.table)
        self.cover_size.valueChanged.connect(self.resize_covers)
        self.resize_covers(self.cover_size.value())
        main.addWidget(self.pages, 1)
        bottom = QHBoxLayout()
        self.selection = QLabel("Select a game to play")
        self.selection.setObjectName("subtle")
        bottom.addWidget(self.selection, 1)
        self.play_button = QPushButton("▶  Play")
        self.play_button.setObjectName("primary")
        self.play_button.clicked.connect(self.play)
        bottom.addWidget(self.play_button)
        main.addLayout(bottom)
        layout.addWidget(content, 1)
        self.setup_menu()
        self.import_progress = QProgressBar()
        self.import_progress.setMaximumWidth(170)
        self.import_progress.hide()
        self.statusBar().addPermanentWidget(self.import_progress)
        self.cancel_import = QPushButton("Cancel import")
        self.cancel_import.clicked.connect(lambda: self.worker.requestInterruption() if self.worker else None)
        self.cancel_import.hide()
        self.statusBar().addPermanentWidget(self.cancel_import)
        self.issues_button = QPushButton()
        self.issues_button.clicked.connect(self.show_import_issues)
        self.statusBar().addPermanentWidget(self.issues_button)
        self.update_issues()
        for widget in (self.games, self.table):
            launch = QAction("Play", widget)
            launch.setShortcut("Return")
            launch.setShortcutContext(Qt.WidgetWithChildrenShortcut)
            launch.triggered.connect(self.play)
            widget.addAction(launch)
        self.refresh()
        self.art_timer = QTimer(self)
        self.art_timer.setInterval(5 * 60 * 1000)
        self.art_timer.timeout.connect(self.start_background)
        if auto_artwork or self.metadata_enabled:
            self.art_timer.start()
            QTimer.singleShot(250, self.start_background)
        if self.library.needs_filename_restore():
            QTimer.singleShot(0, lambda: self.import_paths([], restore_names=True))

    def setup_menu(self):
        file = self.menuBar().addMenu("Library")
        actions = [("Import games…", "Ctrl+O", self.choose_files),
                   ("Import folder…", "Ctrl+Shift+O", self.choose_folder),
                   ("Settings…", "Ctrl+,", self.open_settings),
                   ("Open library folder", "", lambda: QDesktopServices.openUrl(QUrl.fromLocalFile(str(self.library.root)))),
                   ("Quit", "Ctrl+Q", self.close)]
        for title, key, fn in actions:
            a = QAction(title, self)
            if key: a.setShortcut(key)
            a.triggered.connect(fn)
            file.addAction(a)
        file.addSeparator()
        file.addAction("New collection…", self.new_collection)
        file.addAction("New smart collection…", self.new_smart_collection)
        file.addAction("Import issues…", self.show_import_issues)
        artwork = self.menuBar().addMenu("Artwork")
        self.download_art_action = artwork.addAction("Download missing box art")
        self.download_art_action.triggered.connect(lambda: self.start_artwork(force=True))
        self.cancel_art_action = artwork.addAction("Cancel cover downloads")
        self.cancel_art_action.setEnabled(False)
        self.cancel_art_action.triggered.connect(self.cancel_artwork)
        artwork.addSeparator()
        self.auto_art_action = artwork.addAction("Automatically download box art")
        self.auto_art_action.setCheckable(True)
        self.auto_art_action.setChecked(self.library.setting("artwork_auto", "1") == "1")
        self.auto_art_action.toggled.connect(self.toggle_auto_artwork)
        self.backup_art_action = artwork.addAction("Use backup source when needed")
        self.backup_art_action.setCheckable(True)
        self.backup_art_action.setChecked(self.library.setting("artwork_backup", "1") == "1")
        self.backup_art_action.toggled.connect(lambda enabled: self.library.set_setting("artwork_backup", int(enabled)))
        information = self.menuBar().addMenu("Game information")
        self.lookup_action = information.addAction("Look up game information")
        self.lookup_action.triggered.connect(lambda: self.start_metadata(force=True))
        self.cancel_metadata_action = information.addAction("Cancel information lookup")
        self.cancel_metadata_action.setEnabled(False)
        self.cancel_metadata_action.triggered.connect(self.cancel_metadata)
        self.auto_metadata_action = information.addAction("Automatically look up game information")
        self.auto_metadata_action.setCheckable(True)
        self.auto_metadata_action.setChecked(self.library.setting("metadata_auto", "1") == "1")
        self.auto_metadata_action.toggled.connect(self.toggle_auto_metadata)
        help_menu = self.menuBar().addMenu("Help")
        a = help_menu.addAction("About this Linux port")
        a.triggered.connect(lambda: QMessageBox.information(self, "OpenEmu Linux", f"OpenEmu Linux {__version__} — community preview\n\nA native Linux platform layer built in the OpenEmu source tree. Uses OpenEmu's pinned Gambatte, mGBA and Snes9x cores.\n\nPlayable systems: Game Boy, Game Boy Color, Game Boy Advance, Super Nintendo.\n\nSystem icons: OpenEmu. Box art: OpenVGDB, with Libretro thumbnails as a backup.\n\nThe remaining consoles and macOS library migration are not ported yet.\n\nSee Linux/README.md and Linux/THIRD_PARTY_NOTICES.md for details and credits."))

    def open_settings(self):
        from .settings import SettingsDialog
        dialog = SettingsDialog(self.library, self)
        def sync():
            self.auto_art_action.setChecked(self.library.setting("artwork_auto", "1") == "1")
            self.backup_art_action.setChecked(self.library.setting("artwork_backup", "1") == "1")
            self.auto_metadata_action.setChecked(self.library.setting("metadata_auto", "1") == "1")
        dialog.changed.connect(sync)
        dialog.exec()
        self.refresh()


    def toggle_auto_artwork(self, enabled):
        self.library.set_setting("artwork_auto", int(enabled))
        if enabled:
            self.start_artwork()
        else:
            self.cancel_artwork()

    def cancel_artwork(self):
        self.art_pending.clear()
        if self.art_worker:
            self.art_worker.requestInterruption()
        self.cancel_art_action.setEnabled(False)

    def cancel_metadata(self):
        self.metadata_pending.clear()
        if self.metadata_worker:
            self.metadata_worker.requestInterruption()
        self.cancel_metadata_action.setEnabled(False)

    def toggle_auto_metadata(self, enabled):
        self.library.set_setting("metadata_auto", int(enabled))
        if enabled:
            self.start_metadata()
        else:
            self.cancel_metadata()

    def start_background(self):
        self.start_metadata()
        self.start_artwork()

    def start_metadata(self, *, force=False, game_ids=None):
        if self.closing or (not force and (not self.metadata_enabled or self.library.setting("metadata_auto", "1") != "1")):
            return
        if self.metadata_worker or self.art_worker:
            if force or not self.metadata_pending:
                self.metadata_pending.append({"force": force, "game_ids": game_ids})
            self.cancel_metadata_action.setEnabled(True)
            return
        if not self.library.metadata_candidates(force, game_ids):
            return
        self.metadata_worker = MetadataWorker(self.library.root, force=force, game_ids=game_ids)
        self.metadata_worker.progress.connect(self.statusBar().showMessage)
        self.metadata_worker.result.connect(self.metadata_done)
        self.metadata_worker.finished.connect(self.metadata_finished)
        self.lookup_action.setEnabled(False)
        self.cancel_metadata_action.setEnabled(True)
        self.metadata_worker.start()

    def metadata_done(self, summary):
        if self.closing:
            return
        self.refresh()
        if summary["cancelled"]:
            message = "Game information lookup cancelled."
        elif summary["error"]:
            message = "Game information is temporarily unavailable. Retry from the Game information menu."
        else:
            message = f"Identified {summary['matched']} game(s) · {summary['not_found']} without a match"
            if summary["failed"]:
                message += f" · {summary['failed']} unavailable; retry from Game information"
        self.statusBar().showMessage(message, 12000)

    def metadata_finished(self):
        self.metadata_worker.deleteLater()
        self.metadata_worker = None
        self.lookup_action.setEnabled(True)
        self.cancel_metadata_action.setEnabled(bool(self.metadata_pending))
        if self.closing:
            self.close()
        else:
            self.drain_lookups()

    def drain_lookups(self):
        # One catalog consumer at a time avoids duplicate initial downloads.
        # Keep explicit per-game requests separate so replacement never spreads
        # to unrelated covers when an automatic lookup is also queued.
        while not self.art_worker and not self.metadata_worker:
            if self.metadata_pending:
                self.start_metadata(**self.metadata_pending.pop(0))
            elif self.art_pending:
                self.start_artwork(**self.art_pending.pop(0))
            else:
                break
        self.cancel_art_action.setEnabled(bool(self.art_worker or self.art_pending))
        self.cancel_metadata_action.setEnabled(bool(self.metadata_worker or self.metadata_pending))

    def start_artwork(self, *, force=False, game_ids=None, replace=False):
        if self.closing or (not force and self.library.setting("artwork_auto", "1") != "1"):
            return
        if self.art_worker or self.metadata_worker:
            if force or not self.art_pending:
                self.art_pending.append({"force": force, "game_ids": game_ids, "replace": replace})
            self.cancel_art_action.setEnabled(True)
            return
        if not self.library.artwork_candidates(force, game_ids, replace=replace):
            if force:
                self.statusBar().showMessage("There are no missing covers to download.", 5000)
            return
        self.art_worker = ArtworkWorker(self.library.root, force=force, game_ids=game_ids, replace=replace)
        self.art_worker.progress.connect(self.statusBar().showMessage)
        self.art_worker.changed.connect(self.cover_downloaded)
        self.art_worker.result.connect(self.artwork_done)
        self.art_worker.finished.connect(self.artwork_finished)
        self.download_art_action.setEnabled(False)
        self.cancel_art_action.setEnabled(True)
        self.art_worker.start()

    def cover_downloaded(self, game_id):
        row = self.library.get(game_id)
        if not row:
            return
        if game_id in self.rows:
            self.rows[game_id] = row
        for i in range(self.games.count()):
            item = self.games.item(i)
            if item.data(Qt.UserRole) == game_id:
                item.setIcon(QIcon(cover_pixmap(row, self.library)))
                break

    def artwork_done(self, summary):
        if self.closing:
            return
        self.refresh()
        if summary["error"]:
            message = "Box art is temporarily unavailable. Use Artwork → Download missing box art to retry."
        elif summary["cancelled"]:
            message = "Box art downloads paused."
        else:
            parts = [f"{summary['downloaded']} cover{'s' if summary['downloaded'] != 1 else ''} downloaded"]
            if summary["not_found"]:
                parts.append(f"{summary['not_found']} without a matching cover")
            if summary["failed"]:
                parts.append(f"{summary['failed']} temporarily unavailable; retry from Artwork")
            message = " · ".join(parts)
        self.statusBar().showMessage(message, 15000)

    def artwork_finished(self):
        self.art_worker.deleteLater()
        self.art_worker = None
        self.download_art_action.setEnabled(True)
        self.cancel_art_action.setEnabled(bool(self.art_pending))
        if self.closing:
            self.close()
        else:
            self.drain_lookups()

    def rebuild_sidebar(self, selected=None):
        selected = selected or (self.nav.currentItem().data(Qt.UserRole) if self.nav.currentItem() else "all")
        self.nav.blockSignals(True)
        self.nav.clear()
        entries = [("All games", "all"), ("Recently played", "recent"), ("Favorites", "favorites"),
                   ("Recently added", "added"), ("Never played", "never")]
        entries += [(system.name, key) for key, system in SYSTEMS.items()]
        for title, key in entries:
            item = QListWidgetItem(title)
            item.setData(Qt.UserRole, key)
            if key in SYSTEMS:
                item.setIcon(QIcon(str(SYSTEMS[key].icon)))
            self.nav.addItem(item)
        if self.library.collections():
            label = QListWidgetItem("COLLECTIONS")
            label.setFlags(Qt.NoItemFlags)
            label.setForeground(QColor("#9495a2"))
            self.nav.addItem(label)
        for record in self.library.collections():
            item = QListWidgetItem(("◇  " if record["rules"] is not None else "▤  ") + record["name"])
            item.setData(Qt.UserRole, f"collection:{record['id']}")
            item.setData(Qt.UserRole + 1, "smart" if record["rules"] is not None else "regular")
            item.setToolTip("Matches rules automatically" if record["rules"] is not None else "Drop games here to add them")
            self.nav.addItem(item)
        self.nav.setCurrentRow(0)
        for i in range(self.nav.count()):
            if self.nav.item(i).data(Qt.UserRole) == selected:
                self.nav.setCurrentRow(i)
                break
        self.nav.blockSignals(False)

    def current_collection(self):
        key = self.nav.currentItem().data(Qt.UserRole) if self.nav.currentItem() else "all"
        return int(key.split(":")[1]) if key and key.startswith("collection:") else None

    def new_collection(self, checked=False, game_ids=None):
        name, ok = QInputDialog.getText(self, "New collection", "Collection name")
        if ok and name.strip():
            collection_id = self.library.save_collection(name)
            if game_ids:
                self.library.add_to_collection(collection_id, game_ids)
            self.rebuild_sidebar(f"collection:{collection_id}")
            self.refresh()

    def new_smart_collection(self, checked=False, record=None):
        dialog = SmartCollectionDialog(self, record)
        if dialog.exec():
            collection_id = self.library.save_collection(dialog.name.text(), dialog.rules(), record["id"] if record else None)
            self.rebuild_sidebar(f"collection:{collection_id}")
            self.refresh()

    def add_to_collection(self, collection_id, game_ids):
        self.library.add_to_collection(collection_id, game_ids)
        self.refresh()
        self.statusBar().showMessage(f"Added {len(game_ids)} game(s) to {self.library.collection(collection_id)['name']}.", 5000)

    def collection_menu(self, position):
        item = self.nav.itemAt(position)
        if not item or not (item.data(Qt.UserRole) or "").startswith("collection:"):
            return
        record = self.library.collection(int(item.data(Qt.UserRole).split(":")[1]))
        menu = QMenu(self)
        rename = menu.addAction("Rename collection…")
        edit = menu.addAction("Edit rules…") if record["rules"] is not None else None
        delete = menu.addAction("Delete collection (keep games)")
        action = menu.exec(self.nav.mapToGlobal(position))
        if action is None:
            return
        if action == rename:
            name, ok = QInputDialog.getText(self, "Rename collection", "Name", text=record["name"])
            if ok and name.strip():
                rules = json.loads(record["rules"]) if record["rules"] is not None else None
                self.library.save_collection(name, rules, record["id"])
        elif action == edit:
            self.new_smart_collection(record=record)
        elif action == delete:
            self.library.delete_collection(record["id"])
        self.rebuild_sidebar()
        self.refresh()

    def active_view(self):
        return self.table if self.view_mode == "list" else self.games

    def selected_ids(self):
        return self.active_view().game_ids()

    def restore_selection(self, game_ids):
        ids = set(game_ids)
        self.games.blockSignals(True)
        self.table.blockSignals(True)
        self.games.clearSelection()
        self.table.clearSelection()
        first = True
        for i in range(self.games.count()):
            item = self.games.item(i)
            if item.data(Qt.UserRole) in ids:
                item.setSelected(True)
                if first:
                    self.games.setCurrentItem(item, QItemSelectionModel.NoUpdate)
                    first = False
        first = True
        for i in range(self.table.rowCount()):
            if self.table.item(i, 0).data(Qt.UserRole) in ids:
                self.table.selectionModel().select(self.table.model().index(i, 0),
                    QItemSelectionModel.Select | QItemSelectionModel.Rows)
                if first:
                    self.table.setCurrentCell(i, 0, QItemSelectionModel.NoUpdate)
                    first = False
        self.games.blockSignals(False)
        self.table.blockSignals(False)
        self.selected()

    def change_view(self, mode):
        ids = self.selected_ids()
        self.view_mode = mode
        self.library.set_setting("library.view", mode)
        self.grid_button.setChecked(mode == "grid")
        self.list_button.setChecked(mode == "list")
        self.cover_size.setEnabled(mode == "grid")
        self.restore_selection(ids)
        self.pages.setCurrentIndex((2 if mode == "list" else 0) if self.rows else 1)

    def resize_covers(self, size):
        self.games.setIconSize(QSize(size, size))
        self.games.setGridSize(QSize(size + 28, size + 86))
        self.library.set_setting("library.cover_size", size)

    def refresh(self, *_):
        previous = self.selected_ids()
        scroll_grid = self.games.verticalScrollBar().value()
        scroll_list = self.table.verticalScrollBar().value()
        key = self.nav.currentItem().data(Qt.UserRole)
        self.library.set_setting("library.section", key)
        self.heading.setText(self.nav.currentItem().text())
        rows = self.library.games(system=key if key in SYSTEMS else None, search=self.search.text(),
                                  favorites=key == "favorites", recent=key == "recent", added=key == "added",
                                  never=key == "never", collection=self.current_collection())
        self.rows = {r["id"]: r for r in rows}
        self.games.blockSignals(True)
        self.table.blockSignals(True)
        self.games.clear()
        self.table.setSortingEnabled(False)
        self.table.setRowCount(len(rows))
        from .core_manager import CoreManager
        manager = CoreManager(self.library.root)
        try:
            installed = manager.installed()
        except (ValueError, OSError, RuntimeError):
            installed = {}
        for index, row in enumerate(rows):
            name = ("★ " if row["favorite"] else "") + row["title"]
            if row["rating"]:
                name += "\n" + "★" * row["rating"] + "☆" * (5 - row["rating"])
            item = QListWidgetItem(QIcon(cover_pixmap(row, self.library)), name)
            item.setData(Qt.UserRole, row["id"])
            item.setData(Qt.UserRole + 1, ("★ " if row["favorite"] else "") + row["title"])
            item.setData(Qt.UserRole + 2, row["rating"])
            item.setToolTip(f"{row['title']}\n{SYSTEMS[row['system']].name}\nFile: {Path(row['rom_path']).name}\nDouble-click to play")
            info = self.library.artwork_info(row["id"])
            if info and info["status"] == "downloaded" and row["cover"]:
                source = "Libretro (backup)" if "libretro-thumbnails/" in (info["source_url"] or "") else "OpenVGDB"
                item.setToolTip(item.toolTip() + f"\nBox art: {source}")
            elif info and not row["cover"]:
                item.setToolTip(item.toolTip() + ("\nNo matching box art found" if info["status"] == "not_found"
                                                else "\nBox art unavailable; use Download missing cover to retry"))
            self.games.addItem(item)
            core_id = self.library.setting("core." + row["system"], "builtin")
            core_name = "Bundled" if core_id == "builtin" else installed.get(core_id, {}).get("name", core_id)
            values = [(row["title"], None), (SYSTEMS[row["system"]].name, None),
                      ("★" * row["rating"] or "—", row["rating"]),
                      (date_text(row["last_played"]), row["last_played"] or 0),
                      (date_text(row["added"]), row["added"]), (str(row["play_count"]), row["play_count"]),
                      (core_name, None), ("Available" if (self.library.root / row["rom_path"]).is_file() else "Missing", None)]
            for column, (text, order) in enumerate(values):
                cell = SortItem(text, row["id"], order)
                cell.setToolTip(item.toolTip())
                self.table.setItem(index, column, cell)
        self.table.setSortingEnabled(True)
        self.games.blockSignals(False)
        self.table.blockSignals(False)
        self.restore_selection(previous)
        self.games.verticalScrollBar().setValue(scroll_grid)
        self.table.verticalScrollBar().setValue(scroll_list)
        self.count.setText(f"{len(rows)} game{'s' if len(rows) != 1 else ''}")
        self.cover_size.setEnabled(self.view_mode == "grid")
        self.pages.setCurrentIndex((2 if self.view_mode == "list" else 0) if rows else 1)
        if self.search.text() or key != "all":
            self.empty_title.setText("No games here yet")
            self.empty_text.setText("Try another view, change your search, or import a game.")
        else:
            self.empty_title.setText("Your games, together.")
            self.empty_text.setText("Drop your Game Boy, Game Boy Color, Game Boy Advance or SNES ROMs here.\n\nOr use Import games to start your library.\nZIP archives are supported.")
        self.selected()

    def selected(self):
        ids = self.selected_ids()
        self.play_button.setEnabled(len(ids) == 1)
        self.selection.setText(self.rows[ids[0]]["title"] if len(ids) == 1 else
                               f"{len(ids)} games selected" if ids else "Select a game to play")

    def choose_files(self):
        formats = " ".join("*" + ext for ext in (*EXTENSIONS, ".zip"))
        paths, _ = QFileDialog.getOpenFileNames(self, "Import games", "", f"Game ROMs ({formats});;All files (*)")
        if paths: self.import_paths(paths)

    def choose_folder(self):
        path = QFileDialog.getExistingDirectory(self, "Import a ROM folder")
        if path: self.import_paths([path])

    def import_paths(self, paths, *, restore_names=False, system_override=None, consolidate_ids=None):
        if self.closing:
            return
        if self.worker and self.worker.isRunning():
            self.statusBar().showMessage("An import is already running. Please wait.")
            return
        self.import_button.setEnabled(False)
        self.statusBar().showMessage("Restoring original ROM filenames…" if restore_names else "Importing games…")
        self.worker = Importer(self.library.root, paths, restore_names=restore_names,
                               system_override=system_override, consolidate_ids=consolidate_ids)
        self.worker.progress.connect(self.import_progress_changed)
        self.worker.result.connect(self.import_done)
        if self.issue_dialog:
            self.issue_dialog.set_busy(True)
        if not restore_names:
            self.import_progress.setRange(0, 0)
            self.import_progress.show()
            self.cancel_import.show()
        self.worker.start()

    def import_progress_changed(self, current, total, message):
        self.import_progress.setRange(0, total)
        self.import_progress.setValue(current)
        self.statusBar().showMessage(message)

    def update_issues(self):
        count = len(self.library.import_issues())
        self.issues_button.setText(f"{count} import issue{'s' if count != 1 else ''}")
        self.issues_button.setVisible(count > 0)
        if self.issue_dialog:
            self.issue_dialog.refresh()

    def show_import_issues(self):
        if self.issue_dialog is None:
            self.issue_dialog = ImportIssuesDialog(self.library, self)
            self.issue_dialog.retry.connect(lambda path, system: self.import_paths([path], system_override=system))
            self.issue_dialog.finished.connect(self.update_issues)
        self.issue_dialog.refresh()
        self.issue_dialog.show()
        self.issue_dialog.raise_()
        self.issue_dialog.activateWindow()

    def import_done(self, count, errors):
        self.import_button.setEnabled(True)
        self.import_progress.hide()
        self.cancel_import.hide()
        if self.issue_dialog:
            self.issue_dialog.set_busy(False)
        self.refresh()
        if self.worker.error:
            QMessageBox.warning(self, "Import could not finish", self.worker.error)
            return
        if self.worker.restore_names:
            message = f"Restored {count} original ROM filename{'s' if count != 1 else ''}."
            if errors:
                message += f" Reimport {errors[0]} original ROM(s) or ZIP(s) to recover their filenames."
            self.statusBar().showMessage(message, 15000)
            self.start_background()
            return
        if self.worker.consolidate_ids is not None:
            message = f"Copied {count} game(s) into the library. Original files were kept."
            if self.worker.cancelled:
                message = "Copying cancelled. " + message
            self.statusBar().showMessage(message, 9000)
            if errors:
                QMessageBox.warning(self, "Some games could not be copied", "\n".join(errors[:12]))
        else:
            message = f"{self.worker.new_games} new game(s) · {self.worker.duplicates} duplicate(s) kept once"
            if self.worker.cancelled:
                message = "Import cancelled. " + message
            self.statusBar().showMessage(message, 9000)
            self.update_issues()
            if errors:
                self.show_import_issues()
        self.start_background()

    def demo(self):
        self.import_paths([ROOT / "demos"])

    def play(self):
        ids = self.selected_ids()
        if len(ids) != 1: return
        game_id = ids[0]
        if game_id in self.processes:
            self.statusBar().showMessage("This game is already running in its game window.", 6000)
            return
        process = QProcess(self)
        process.setProgram(sys.executable)
        process.setArguments(["-m", "openemu_linux.player", "--data-dir", str(self.library.root), "--game", game_id])
        process.setWorkingDirectory(str(ROOT))
        process.setProcessChannelMode(QProcess.MergedChannels)
        process.log = bytearray()
        process.readyReadStandardOutput.connect(lambda: self.process_output(process))
        process.finished.connect(lambda code, status: self.game_closed(game_id, code))
        process.errorOccurred.connect(lambda error: self.statusBar().showMessage(f"Game process error: {process.errorString()}"))
        self.processes[game_id] = process
        process.start()
        self.statusBar().showMessage(f"Opening {self.rows[game_id]['title']}…", 5000)

    def process_output(self, process):
        process.log.extend(bytes(process.readAllStandardOutput()))
        del process.log[:-16000]
        if b"OPENEMU_GAME_READY" in process.log:
            self.statusBar().showMessage("Game opened in its own window.", 5000)

    def game_closed(self, game_id, code):
        process = self.processes.pop(game_id)
        self.process_output(process)
        if code:
            QMessageBox.warning(self, "The game stopped", process.log.decode(errors="replace")[-2000:] or "The emulator process exited unexpectedly.")
        process.deleteLater()
        self.refresh()

    def context_menu(self, position, view=None):
        view = view or self.games
        item = view.itemAt(position)
        if not item: return
        if not item.isSelected():
            self.restore_selection([item.data(Qt.UserRole)])
        menu = self.game_menu(self.selected_ids())
        menu.exec(view.viewport().mapToGlobal(position))
        self.refresh()

    def game_menu(self, game_ids):
        game_id = game_ids[0]
        row = self.rows[game_id]
        single = len(game_ids) == 1
        menu = QMenu(self)
        if single:
            menu.addAction("Play", self.play)
        all_favorites = all(self.rows[key]["favorite"] for key in game_ids)
        def favorite():
            for key in game_ids:
                if bool(self.rows[key]["favorite"]) == all_favorites:
                    self.library.favorite(key)
        menu.addAction("Remove from favorites" if all_favorites else "Add to favorites", favorite)
        ratings = menu.addMenu("Rating")
        for value in range(6):
            action = ratings.addAction("★" * value if value else "Unrated")
            action.setCheckable(True)
            action.setChecked(all(self.rows[key]["rating"] == value for key in game_ids))
            action.triggered.connect(lambda checked=False, value=value: self.library.rate(game_ids, value))
        collections = menu.addMenu("Add to collection")
        regular = [record for record in self.library.collections() if record["rules"] is None]
        collections.setEnabled(bool(regular))
        for record in regular:
            action = collections.addAction(record["name"])
            action.triggered.connect(lambda checked=False, collection_id=record["id"]: self.add_to_collection(collection_id, game_ids))
        menu.addAction("Create collection from selection…", lambda: self.new_collection(game_ids=game_ids))
        collection_id = self.current_collection()
        if collection_id and self.library.collection(collection_id)["rules"] is None:
            menu.addAction("Remove from collection", lambda: self.library.remove_from_collection(collection_id, game_ids))
        menu.addSeparator()
        if single:
            menu.addAction("Rename game…", lambda: self.rename_game(game_id))
            menu.addAction("Add cover art from file…", lambda: self.choose_cover(game_id))
        download_cover = menu.addAction("Download missing cover art", lambda: self.start_artwork(force=True, game_ids=set(game_ids)))
        download_cover.setEnabled(not self.art_worker and any(not self.rows[key]["cover"] or
            not (self.library.root / self.rows[key]["cover"]).is_file() for key in game_ids))
        if any(self.rows[key]["cover"] for key in game_ids):
            menu.addAction("Download replacement cover art…", lambda: self.download_replacement(game_ids))
        menu.addAction("Look up game information", lambda: self.start_metadata(force=True, game_ids=set(game_ids)))
        if single:
            from .core_manager import CoreManager
            from .core import CoreError
            state_directory = self.library.root / "states" / game_id
            try:
                selected_core = CoreManager(self.library.root).selection(self.library, row["system"])
                if selected_core:
                    state_directory /= "libretro/" + selected_core["id"] + "/" + selected_core["sha256"]
                if (state_directory / "auto.oesavestate").is_file():
                    menu.addAction("Play (Ctrl+F8 restores automatic save)", self.play)
            except (CoreError, ValueError, OSError):
                pass
            menu.addAction("Open ROM folder", lambda: QDesktopServices.openUrl(QUrl.fromLocalFile(str((self.library.root / row["rom_path"]).parent))))
        menu.addAction("Copy ROM path" if single else "Copy ROM paths", lambda: QApplication.clipboard().setText(
            "\n".join(str(self.library.root / self.rows[key]["rom_path"]) for key in game_ids)))
        external = [key for key in game_ids if Path(self.rows[key]["rom_path"]).is_absolute()]
        if external:
            menu.addAction("Consolidate files into library", lambda: self.import_paths([], consolidate_ids=external))
        if single:
            menu.addAction("Locate missing ROM…", lambda: self.locate_rom(game_id))
        menu.addSeparator()
        def remove():
            for key in game_ids:
                self.library.remove(key)
        menu.addAction("Remove from library (keep files)", remove)
        if single:
            menu.addAction("Game information…", lambda: GameInfoDialog(self.library, game_id, self).exec())
        return menu

    def download_replacement(self, game_ids):
        answer = QMessageBox.question(self, "Replace cover art?",
            "Download new covers for the selected games? Their current covers will remain visible unless a replacement downloads successfully.",
            QMessageBox.Yes | QMessageBox.No, QMessageBox.No)
        if answer == QMessageBox.Yes:
            self.start_artwork(force=True, game_ids=set(game_ids), replace=True)

    def locate_rom(self, game_id):
        if game_id in self.processes:
            self.statusBar().showMessage("Close this game's window before changing its ROM location.", 7000)
            return
        path, _ = QFileDialog.getOpenFileName(self, "Locate the original ROM file")
        if path:
            try:
                self.library.relink(game_id, path)
                self.refresh()
            except (OSError, ValueError) as error:
                QMessageBox.warning(self, "Could not reconnect ROM", str(error))

    def rename_game(self, game_id):
        row = self.library.get(game_id)
        name, ok = QInputDialog.getText(self, "Rename game", "Title", text=row["title"])
        if ok:
            self.library.rename(game_id, name)

    def choose_cover(self, game_id):
        path, _ = QFileDialog.getOpenFileName(self, "Choose cover", "", "Images (*.png *.jpg *.jpeg *.webp)")
        if path:
            self.replace_cover(game_id, path)

    def replace_cover(self, game_id, path):
        image = QPixmap(path)
        if image.isNull():
            self.statusBar().showMessage("This image could not be opened. Choose a PNG, JPEG or WebP file.", 8000)
            return
        target = Path("covers") / (game_id + ".png")
        if image.scaled(700, 700, Qt.KeepAspectRatio, Qt.SmoothTransformation).save(str(self.library.root / target)):
            self.library.set_manual_cover(game_id, target)
            self.refresh()

    def files_dropped(self, paths, game_id):
        if game_id and len(paths) == 1 and Path(paths[0]).suffix.lower() in (".png", ".jpg", ".jpeg", ".webp"):
            self.replace_cover(game_id, paths[0])
        else:
            self.import_paths(paths)

    def dragEnterEvent(self, event):
        if event.mimeData().hasUrls(): event.acceptProposedAction()

    def dropEvent(self, event):
        self.import_paths([u.toLocalFile() for u in event.mimeData().urls() if u.isLocalFile()])
        event.acceptProposedAction()

    def closeEvent(self, event):
        if (self.worker and self.worker.isRunning()) or self.processes:
            self.statusBar().showMessage("Close the game windows and let imports finish before closing the library.")
            event.ignore()
            return
        self.closing = True
        self.art_timer.stop()
        if self.art_worker or self.metadata_worker:
            self.cancel_artwork()
            self.cancel_metadata()
            self.statusBar().showMessage("Stopping background lookups…")
            event.ignore()
            return
        header = self.table.horizontalHeader()
        self.library.set_setting("library.columns", json.dumps([self.table.columnWidth(i) for i in range(8)]))
        self.library.set_setting("library.sort", json.dumps([header.sortIndicatorSection(), header.sortIndicatorOrder().value]))
        if self.issue_dialog:
            self.issue_dialog.close()
        self.library.close()
        event.accept()


def main():
    parser = argparse.ArgumentParser(description="OpenEmu Linux community port")
    parser.add_argument("roms", nargs="*")
    parser.add_argument("--data-dir")
    parser.add_argument("--screenshot", help=argparse.SUPPRESS)
    args = parser.parse_args()
    app = QApplication(sys.argv[:1])
    app.setApplicationName("OpenEmu Linux")
    app.setDesktopFileName("openemu-linux")
    window = Window(Library(args.data_dir), auto_artwork=not args.screenshot)
    window.show()
    if args.roms: window.import_paths(args.roms)
    if args.screenshot:
        def capture():
            window.grab().save(args.screenshot)
            window.close()
        QTimer.singleShot(1000, capture)
    return app.exec()
