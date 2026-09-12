"""Library controls shared by the grid and list; no emulator implementation here."""
import json
from datetime import datetime
from pathlib import Path

from PySide6.QtCore import Qt, QMimeData, Signal, QRect, QSize
from PySide6.QtGui import QDrag, QColor, QPen, QPixmap
from PySide6.QtWidgets import (QAbstractItemView, QCheckBox, QComboBox, QDialog,
    QDialogButtonBox, QFormLayout, QLabel, QLineEdit, QListWidget, QSpinBox,
    QTableWidget, QTableWidgetItem, QVBoxLayout, QStyledItemDelegate, QStyle,
    QScrollArea, QWidget, QPlainTextEdit, QPushButton, QMessageBox)

from .library import SYSTEMS

GAME_MIME = "application/x-openemu-library-games"


def date_text(timestamp):
    return datetime.fromtimestamp(timestamp).strftime("%Y-%m-%d %H:%M") if timestamp else "Never"


class GameDragMixin:
    def startDrag(self, supported_actions):
        mime = QMimeData()
        mime.setData(GAME_MIME, json.dumps(self.game_ids()).encode())
        drag = QDrag(self)
        drag.setMimeData(mime)
        drag.exec(Qt.CopyAction)

    def dragEnterEvent(self, event):
        if event.mimeData().hasUrls():
            event.acceptProposedAction()

    def dragMoveEvent(self, event):
        if event.mimeData().hasUrls():
            event.acceptProposedAction()

    def dropEvent(self, event):
        if event.mimeData().hasUrls():
            paths = [url.toLocalFile() for url in event.mimeData().urls() if url.isLocalFile()]
            item = self.itemAt(event.position().toPoint())
            game_id = item.data(Qt.UserRole) if item else None
            self.files_dropped.emit(paths, game_id)
            event.acceptProposedAction()


class GameGrid(GameDragMixin, QListWidget):
    files_dropped = Signal(list, object)

    def game_ids(self):
        return [item.data(Qt.UserRole) for item in self.selectedItems()]


class CoverDelegate(QStyledItemDelegate):
    """Keep covers, two title lines and ratings aligned at every grid size."""
    def sizeHint(self, option, index):
        return self.parent().gridSize()

    def paint(self, painter, option, index):
        painter.save()
        rect = option.rect.adjusted(5, 5, -5, -5)
        if option.state & QStyle.State_Selected:
            painter.setBrush(QColor("#393444"))
            painter.setPen(QPen(QColor("#a993d8"), 1))
            painter.drawRoundedRect(rect, 8, 8)
        elif option.state & QStyle.State_MouseOver:
            painter.setBrush(QColor("#2d2e36"))
            painter.setPen(Qt.NoPen)
            painter.drawRoundedRect(rect, 8, 8)
        size = self.parent().iconSize().width()
        icon = index.data(Qt.DecorationRole)
        art_rect = QRect(rect.center().x() - size // 2, rect.top() + 7, size, size)
        icon.paint(painter, art_rect, Qt.AlignCenter)
        title_rect = QRect(rect.left() + 6, art_rect.bottom() + 6, rect.width() - 12, 36)
        painter.setFont(option.font)
        painter.setPen(QColor("#ebeaf0"))
        painter.setClipRect(title_rect)
        painter.drawText(title_rect, Qt.AlignHCenter | Qt.TextWordWrap, index.data(Qt.UserRole + 1))
        painter.setClipping(False)
        rating = index.data(Qt.UserRole + 2)
        if rating:
            painter.setPen(QColor("#c6b2ee"))
            painter.drawText(QRect(rect.left(), title_rect.bottom(), rect.width(), 19), Qt.AlignCenter,
                             "★" * rating + "☆" * (5 - rating))
        painter.restore()


class GameTable(GameDragMixin, QTableWidget):
    files_dropped = Signal(list, object)

    def game_ids(self):
        return [self.item(index.row(), 0).data(Qt.UserRole) for index in self.selectionModel().selectedRows()]


class SortItem(QTableWidgetItem):
    def __init__(self, text, game_id, sort_value=None):
        super().__init__(text)
        self.setData(Qt.UserRole, game_id)
        self.setData(Qt.UserRole + 1, text.casefold() if sort_value is None else sort_value)

    def __lt__(self, other):
        return self.data(Qt.UserRole + 1) < other.data(Qt.UserRole + 1)


class LibrarySidebar(QListWidget):
    games_dropped = Signal(int, list)

    def dragEnterEvent(self, event):
        if event.mimeData().hasFormat(GAME_MIME):
            event.acceptProposedAction()

    def dragMoveEvent(self, event):
        item = self.itemAt(event.position().toPoint())
        if item and item.data(Qt.UserRole + 1) == "regular" and event.mimeData().hasFormat(GAME_MIME):
            event.acceptProposedAction()
        else:
            event.ignore()

    def dropEvent(self, event):
        item = self.itemAt(event.position().toPoint())
        if not item or item.data(Qt.UserRole + 1) != "regular":
            event.ignore()
            return
        try:
            ids = json.loads(bytes(event.mimeData().data(GAME_MIME)))
            if not isinstance(ids, list) or not all(isinstance(key, str) for key in ids):
                raise ValueError("Invalid games")
            self.games_dropped.emit(int(item.data(Qt.UserRole).split(":")[1]), ids)
            event.acceptProposedAction()
        except (ValueError, TypeError):
            event.ignore()


class SmartCollectionDialog(QDialog):
    def __init__(self, parent=None, record=None):
        super().__init__(parent)
        self.setWindowTitle("Edit smart collection" if record else "New smart collection")
        self.setMinimumWidth(440)
        layout = QVBoxLayout(self)
        layout.addWidget(QLabel("Games appear automatically when they match all selected rules."))
        form = QFormLayout()
        self.name = QLineEdit(record["name"] if record else "")
        form.addRow("Name", self.name)
        rules = json.loads(record["rules"]) if record else {}
        self.system = QComboBox()
        self.system.addItem("Any system", None)
        for key, system in SYSTEMS.items():
            self.system.addItem(system.name, key)
        self.system.setCurrentIndex(max(0, self.system.findData(rules.get("system"))))
        form.addRow("System", self.system)
        self.rating = QComboBox()
        self.rating.addItem("Any rating", None)
        for value in range(1, 6):
            self.rating.addItem("★" * value + " or higher", value)
        self.rating.setCurrentIndex(max(0, self.rating.findData(rules.get("minimum_rating"))))
        form.addRow("Rating", self.rating)
        self.favorite = QCheckBox("Favorites only")
        self.favorite.setChecked(rules.get("favorite", False))
        form.addRow(self.favorite)
        self.never = QCheckBox("Never played")
        self.never.setChecked(rules.get("never_played", False))
        form.addRow(self.never)
        self.days = {}
        for key, label in [("added_days", "Added in last"), ("played_days", "Played in last")]:
            spin = QSpinBox()
            spin.setRange(0, 36500)
            spin.setSpecialValueText("Any time")
            spin.setSuffix(" days")
            spin.setValue(rules.get(key, 0))
            form.addRow(label, spin)
            self.days[key] = spin
        layout.addLayout(form)
        self.error = QLabel()
        self.error.setWordWrap(True)
        layout.addWidget(self.error)
        buttons = QDialogButtonBox(QDialogButtonBox.Save | QDialogButtonBox.Cancel)
        buttons.accepted.connect(self.validate)
        buttons.rejected.connect(self.reject)
        layout.addWidget(buttons)

    def rules(self):
        result = {}
        if self.system.currentData(): result["system"] = self.system.currentData()
        if self.rating.currentData(): result["minimum_rating"] = self.rating.currentData()
        if self.favorite.isChecked(): result["favorite"] = True
        if self.never.isChecked(): result["never_played"] = True
        for key, spin in self.days.items():
            if spin.value(): result[key] = spin.value()
        return result

    def validate(self):
        if not self.name.text().strip() or not self.rules():
            self.error.setText("Enter a name and select at least one rule.")
        elif self.never.isChecked() and self.days["played_days"].value():
            self.error.setText("Never played cannot be combined with a recent play date.")
        else:
            self.accept()


class GameInfoDialog(QDialog):
    def __init__(self, library, game_id, parent=None):
        super().__init__(parent)
        self.library, self.game_id = library, game_id
        game = library.get(game_id)
        self.setWindowTitle("Game information")
        self.resize(620, 780)
        layout = QVBoxLayout(self)
        scroll = QScrollArea()
        scroll.setWidgetResizable(True)
        body = QWidget()
        body_layout = QVBoxLayout(body)
        scroll.setWidget(body)
        layout.addWidget(scroll, 1)
        if game["cover"]:
            cover = QPixmap(str(library.root / game["cover"]))
            if not cover.isNull():
                preview = QLabel()
                preview.setAlignment(Qt.AlignCenter)
                preview.setPixmap(cover.scaled(110, 110, Qt.KeepAspectRatio, Qt.SmoothTransformation))
                body_layout.addWidget(preview)
        form = QFormLayout()
        self.title = QLineEdit(game["title"])
        self.original_title = game["title"]
        self.original_rating = game["rating"]
        form.addRow("Title", self.title)
        form.addRow("System", QLabel(SYSTEMS[game["system"]].name))
        self.rating = QComboBox()
        self.rating.addItems(["Unrated"] + ["★" * i for i in range(1, 6)])
        self.rating.setCurrentIndex(game["rating"])
        form.addRow("Rating", self.rating)
        metadata = library.metadata(game_id)
        self.original_metadata = metadata
        self.fields = {}
        for key, label in [("region", "Region"), ("developer", "Developer"), ("publisher", "Publisher"),
                           ("release_date", "Release date"), ("genre", "Genre"), ("players", "Players"), ("notes", "Notes")]:
            field = QLineEdit(metadata.get(key, ""))
            self.fields[key] = field
            form.addRow(label, field)
        self.description = QPlainTextEdit(metadata.get("description", ""))
        self.description.setMaximumHeight(115)
        form.addRow("Description", self.description)
        lookup = library.metadata_lookup(game_id)
        if lookup:
            source = QLabel(f"{lookup['provider']} · Last checked {date_text(lookup['checked'])}")
            source.setTextFormat(Qt.PlainText)
            source.setWordWrap(True)
            form.addRow("Information source", source)
            if lookup["source_url"]:
                reference = QLineEdit(lookup["source_url"])
                reference.setReadOnly(True)
                form.addRow("Reference", reference)
        for label, value in [("ROM filename", Path(game["rom_path"]).name),
                             ("ROM path", str(library.root / game["rom_path"])), ("SHA-256", game["id"]),
                             ("Imported", date_text(game["added"])), ("Last played", date_text(game["last_played"])),
                             ("Play count", str(game["play_count"]))]:
            field = QLineEdit(value)
            field.setReadOnly(True)
            field.setCursorPosition(0)
            form.addRow(label, field)
        for label, key in [("MD5", "md5"), ("SHA-1", "sha1"), ("CRC32", "crc32")]:
            value = library.hashes(game_id).get(key)
            if value:
                field = QLineEdit(value)
                field.setReadOnly(True)
                form.addRow(label, field)
        body_layout.addLayout(form)
        automatic = QPushButton("Use downloaded information…")
        automatic.setEnabled(bool(lookup and json.loads(lookup["payload"] or "{}")))
        automatic.clicked.connect(self.use_downloaded)
        layout.addWidget(automatic)
        buttons = QDialogButtonBox(QDialogButtonBox.Save | QDialogButtonBox.Cancel)
        buttons.accepted.connect(self.save)
        buttons.rejected.connect(self.reject)
        layout.addWidget(buttons)

    def save(self):
        if not self.title.text().strip():
            self.title.setFocus()
            return
        # Only publish edited controls. An automatic result arriving while this
        # dialog is open must not be replaced with stale, untouched text.
        if self.title.text() != self.original_title:
            self.library.rename(self.game_id, self.title.text())
        if self.rating.currentIndex() != self.original_rating:
            self.library.rate([self.game_id], self.rating.currentIndex())
        changes = {key: field.text() for key, field in self.fields.items()
                   if field.text() != self.original_metadata.get(key, "")}
        if self.description.toPlainText() != self.original_metadata.get("description", ""):
            changes["description"] = self.description.toPlainText()
        if changes:
            self.library.update_metadata(self.game_id, changes)
        self.accept()

    def use_downloaded(self):
        if QMessageBox.question(self, "Use downloaded information?",
                "Replace your edits to fields that have downloaded information? Your notes and fields without downloaded values will be kept.",
                QMessageBox.Yes | QMessageBox.No, QMessageBox.No) == QMessageBox.Yes:
            self.library.use_automatic_metadata(self.game_id)
            self.accept()
