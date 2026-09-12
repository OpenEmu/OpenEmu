"""Transient gameplay controls; all emulator actions stay in the player API."""
from PySide6.QtCore import Qt, QEvent, QTimer
from PySide6.QtGui import QPainter, QColor, QIcon
from PySide6.QtWidgets import QFrame, QHBoxLayout, QSlider, QToolButton, QStyle, QApplication


def hud_icon(widget, standard):
    if standard in (QStyle.SP_DialogSaveButton, QStyle.SP_DialogOpenButton):
        return widget.style().standardIcon(standard)
    pixmap = widget.style().standardIcon(standard).pixmap(20, 20)
    painter = QPainter(pixmap)
    painter.setCompositionMode(QPainter.CompositionMode_SourceIn)
    painter.fillRect(pixmap.rect(), QColor("#f2edf8"))
    painter.end()
    return QIcon(pixmap)


class GameplayHUD(QFrame):
    def __init__(self, screen, *, actions, options, volume, set_volume, paused, hide_cursor=True):
        super().__init__(screen)
        self.screen, self.paused, self.hide_cursor = screen, paused, hide_cursor
        self.setObjectName("gameplayHUD")
        self.setCursor(Qt.ArrowCursor)
        self.setStyleSheet("""
            QFrame#gameplayHUD {background:#282932;border:1px solid #65616e;border-radius:12px;}
            QToolButton {color:#f2edf8;background:transparent;padding:8px;border:0;border-radius:6px;}
            QToolButton:hover {background:#50465e;}
            QSlider::groove:horizontal {height:5px;background:#55515e;}
            QSlider::handle:horizontal {background:#c0a5e9;width:13px;margin:-4px 0;border-radius:6px;}
        """)
        layout = QHBoxLayout(self)
        layout.setContentsMargins(10, 6, 10, 6)
        layout.setSpacing(4)
        self.buttons = {}
        for name, action, icon in actions:
            button = QToolButton(self)
            button.setDefaultAction(action)
            action.setIcon(hud_icon(self, icon))
            button.setFocusPolicy(Qt.NoFocus)
            button.setAccessibleName(action.text())
            action.changed.connect(lambda button=button, action=action: button.setAccessibleName(action.text()))
            layout.addWidget(button)
            self.buttons[name] = button
        self.volume = QSlider(Qt.Horizontal, self)
        self.volume.setRange(0, 100)
        self.volume.setValue(volume)
        self.volume.setFixedWidth(85)
        self.volume.setFocusPolicy(Qt.NoFocus)
        self.volume.setAccessibleName("Game volume")
        self.volume.setToolTip("Game volume")
        self.volume.valueChanged.connect(set_volume)
        layout.addWidget(self.volume)
        self.options = QToolButton(self)
        self.options.setText("Options")
        self.options.setIcon(self.style().standardIcon(QStyle.SP_FileDialogDetailedView))
        self.options.setMenu(options)
        self.options.setPopupMode(QToolButton.InstantPopup)
        self.options.setFocusPolicy(Qt.NoFocus)
        self.options.setAccessibleName("Gameplay options")
        layout.addWidget(self.options)
        self.timer = QTimer(self)
        self.timer.setSingleShot(True)
        self.timer.setInterval(2500)
        self.timer.timeout.connect(self.fade_away)
        screen.setMouseTracking(True)
        screen.installEventFilter(self)
        for widget in (self, *self.buttons.values(), self.volume, self.options):
            widget.setMouseTracking(True)
            widget.installEventFilter(self)
        self.position()
        self.reveal()

    def position(self):
        compact = self.screen.width() < 760
        for button in (*self.buttons.values(), self.options):
            button.setToolButtonStyle(Qt.ToolButtonIconOnly if compact else Qt.ToolButtonTextBesideIcon)
        self.volume.setVisible(self.screen.width() >= 540)
        self.adjustSize()
        self.move(max(8, (self.screen.width() - self.width()) // 2), max(8, self.screen.height() - self.height() - 20))

    def reveal(self):
        self.screen.unsetCursor()
        self.position()
        self.show()
        self.raise_()
        self.timer.start()

    def fade_away(self):
        if QApplication.mouseButtons() != Qt.NoButton or self.paused() or QApplication.activePopupWidget():
            self.timer.start()
            return
        self.hide()
        if self.hide_cursor:
            self.screen.setCursor(Qt.BlankCursor)

    def eventFilter(self, watched, event):
        # Hiding a child under a stationary pointer generates Enter on the
        # screen. That synthetic transition must not immediately reopen us.
        if event.type() == QEvent.MouseMove or (event.type() == QEvent.Enter and self.isVisible()):
            self.reveal()
        elif event.type() == QEvent.Resize and watched is self.screen:
            self.position()
        elif event.type() == QEvent.Leave and watched is self.screen:
            self.screen.unsetCursor()
        return super().eventFilter(watched, event)

    def stop(self):
        self.timer.stop()
        self.screen.unsetCursor()
