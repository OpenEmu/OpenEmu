"""A separate process owns each emulator, like OpenEmu's helper application."""
import argparse
from datetime import datetime
from pathlib import Path
import sys
import time

from PySide6.QtCore import Qt, QTimer, QLockFile
from PySide6.QtGui import QAction, QActionGroup, QImage, QPainter, QColor, QKeySequence
from PySide6.QtMultimedia import QAudioFormat, QAudioSink, QMediaDevices
from PySide6.QtWidgets import QApplication, QMainWindow, QMessageBox, QWidget, QComboBox, QStyle

from .core import Core, CoreError
from .core_manager import CoreManager
from .gamepad import Gamepad
from .library import Library
from .gameplay import GameplayHUD, hud_icon

KEYS = {Qt.Key_X: 1, Qt.Key_Z: 2, Qt.Key_Backspace: 4, Qt.Key_Return: 8,
        Qt.Key_Enter: 8, Qt.Key_Right: 16, Qt.Key_Left: 32, Qt.Key_Up: 64,
        Qt.Key_Down: 128, Qt.Key_W: 256, Qt.Key_Q: 512, Qt.Key_S: 1024, Qt.Key_A: 2048}


class Screen(QWidget):
    def __init__(self):
        super().__init__()
        self.frame = QImage()
        self.integer_scale = False
        self.display_aspect = None
        self.setMinimumSize(320, 288)
        self.setFocusPolicy(Qt.NoFocus)

    def paintEvent(self, _):
        p = QPainter(self)
        p.fillRect(self.rect(), QColor("#0c0d10"))
        if not self.frame.isNull():
            scale = min(self.width() / self.frame.width(), self.height() / self.frame.height())
            if self.integer_scale and scale >= 1:
                scale = int(scale)
            w, h = round(self.frame.width() * scale), round(self.frame.height() * scale)
            if self.display_aspect and not self.integer_scale:
                w = min(self.width(), round(self.height() * self.display_aspect))
                h = round(w / self.display_aspect)
            p.drawImage((self.width() - w) // 2, (self.height() - h) // 2, self.frame.scaled(w, h, Qt.IgnoreAspectRatio, Qt.FastTransformation))


class Player(QMainWindow):
    def __init__(self, library, game_id, muted=False, frame_limit=0, screenshot=None):
        super().__init__()
        self.closed = False
        self.library = library
        self.game = library.get(game_id)
        if not self.game:
            raise CoreError("The game is no longer in the library.")
        self.game_id = game_id
        self.lock = QLockFile(str(library.root / "saves" / (game_id + ".lock")))
        self.lock.setStaleLockTime(0)
        if not self.lock.tryLock(0):
            raise CoreError("This game is already running. Close its other window first.")
        self.state_directory = library.root / "states" / game_id
        save_directory = library.root / "saves" / game_id
        try:
            rom_path = library.validate_game(game_id)
            manager = CoreManager(library.root)
            selected = manager.selection(library, self.game["system"])
            options = {}
            if selected:
                self.state_directory /= "libretro/" + selected["id"]
                self.state_directory /= selected["sha256"]
                save_directory /= "libretro/" + selected["id"]
                options = {"libretro_path": manager.path(selected), "core_id": selected["id"],
                           "system_dir": library.setting("bios_directory", str(library.root / "system"))}
            library.prepare_save_filenames(self.game, save_directory)
            self.core = Core(rom_path, self.game["system"], save_directory, **options)
        except Exception:
            self.lock.unlock()
            raise
        self.pad = Gamepad()
        self.keys = 0
        self.paused = False
        self.focus_paused = False
        self.muted = muted or library.setting("muted", "0") == "1"
        self.volume = self.preference_int("volume", 80, 0, 100) / 100
        self.pause_unfocused = library.setting("pause_unfocused", "1") == "1"
        self.fast = False
        self.fast_speed = self.preference_int("fast_forward_speed", 3, 2, 10)
        self.frames = 0
        self.frame_limit, self.capture_path = frame_limit, screenshot
        self.audio = self.audio_device = None
        self.audio_pending = bytearray()
        self.screen = Screen()
        self.screen.integer_scale = library.setting("integer_scale", "0") == "1"
        if self.game["system"] == "snes" and library.setting("aspect_mode", "system") == "system":
            self.screen.display_aspect = 4 / 3
        self.setCentralWidget(self.screen)
        self.setWindowTitle(f"{self.game['title']} — {selected['name'] + ' (Libretro)' if selected else 'OpenEmu Linux'}")
        self.resize(850, 630)
        self.setStyleSheet("""
            QMainWindow,QToolBar,QStatusBar,QMenuBar,QMenu {background:#202127;color:#eee}
            QToolBar {spacing:12px;padding:8px}
            QToolButton {color:#eee;padding:6px;border:1px solid transparent;border-radius:4px}
            QToolButton:hover,QMenuBar::item:selected,QMenu::item:selected {background:#41424d}
            QComboBox {padding:5px;background:#383943;color:#eee;border:1px solid #626370}
        """)
        self.setFocusPolicy(Qt.StrongFocus)
        bar = self.addToolBar("Game")
        bar.setMovable(False)
        self.game_toolbar = bar
        self.pause_action = self.action(bar, "Pause", "Ctrl+P", self.toggle_pause)
        self.save_action = self.action(bar, "Save state", "F5", self.save)
        self.load_action = self.action(bar, "Load state", "F8", self.load)
        self.slot = QComboBox()
        self.slot.addItems([f"Slot {i}" for i in range(1, 10)])
        self.slot.setFocusPolicy(Qt.NoFocus)
        bar.addWidget(self.slot)
        menu = self.menuBar().addMenu("Game")
        menu.addActions([self.pause_action, self.save_action, self.load_action])
        slots = menu.addMenu("Save slot")
        slot_group = QActionGroup(self)
        self.slot_actions = []
        for index in range(9):
            action = slots.addAction(f"Slot {index + 1}")
            action.setCheckable(True)
            action.setChecked(index == 0)
            slot_group.addAction(action)
            action.triggered.connect(lambda checked=False, index=index: self.slot.setCurrentIndex(index))
            self.slot_actions.append(action)
        self.slot.currentIndexChanged.connect(lambda index: self.slot_actions[index].setChecked(True))
        self.action(menu, "Reset", "Ctrl+R", self.reset)
        self.action(menu, "Restore automatic save", "Ctrl+F8", self.load_auto)
        self.action(menu, "Screenshot", "F12", self.screenshot)
        self.fullscreen_action = self.action(menu, "Full screen", "F11", self.fullscreen)
        self.mute_action = self.action(menu, "Mute", "Ctrl+M", self.toggle_mute)
        self.mute_action.setCheckable(True)
        self.mute_action.setChecked(self.muted)
        self.action(menu, "Volume up", "Ctrl+Up", lambda: self.set_volume(round(self.volume * 100) + 5))
        self.action(menu, "Volume down", "Ctrl+Down", lambda: self.set_volume(round(self.volume * 100) - 5))
        integer = self.action(menu, "Integer scaling", "Ctrl+I", self.integer_scale)
        integer.setCheckable(True)
        integer.setChecked(self.screen.integer_scale)
        self.action(menu, "Controls", "F1", self.controls)
        self.stop_action = self.action(menu, "Stop", "Ctrl+W", self.close)
        self.hud = GameplayHUD(self.screen, actions=[
            ("stop", self.stop_action, QStyle.SP_MediaStop),
            ("pause", self.pause_action, QStyle.SP_MediaPause),
            ("save", self.save_action, QStyle.SP_DialogSaveButton),
            ("load", self.load_action, QStyle.SP_DialogOpenButton),
            ("mute", self.mute_action, QStyle.SP_MediaVolume),
            ("fullscreen", self.fullscreen_action, QStyle.SP_TitleBarMaxButton)],
            options=menu, volume=round(self.volume * 100), set_volume=self.set_volume,
            paused=lambda: self.paused or self.focus_paused,
            hide_cursor=library.setting("hide_game_cursor", "1") == "1")
        self.mute_action.setIcon(hud_icon(self, QStyle.SP_MediaVolumeMuted if self.muted else QStyle.SP_MediaVolume))
        bar.hide()
        self.statusBar().showMessage("Arrows: move   X/Z: A/B   S/A: X/Y   Enter: Start   Q/W: L/R   F1: all controls")
        self.setup_audio()
        self.audio_devices = QMediaDevices(self)
        self.audio_devices.audioOutputsChanged.connect(self.audio_output_changed)
        self.library.played(game_id)
        self.next_frame = time.perf_counter()
        self.timer = QTimer(self)
        self.timer.setTimerType(Qt.PreciseTimer)
        self.timer.timeout.connect(self.tick)
        self.timer.start(1)
        self.autosave = QTimer(self)
        self.autosave.timeout.connect(self.save_auto)
        self.autosave.start(30000)
        if library.setting("fullscreen_default", "0") == "1" and not frame_limit:
            QTimer.singleShot(0, self.initial_fullscreen)

    def preference_int(self, key, default, minimum, maximum):
        try:
            return max(minimum, min(maximum, int(self.library.setting(key, str(default)))))
        except ValueError:
            return default

    def initial_fullscreen(self):
        if not self.closed and self.isVisible() and not self.isFullScreen():
            self.fullscreen()

    def action(self, parent, title, key, fn):
        action = QAction(title, self)
        action.setShortcut(QKeySequence(key))
        action.triggered.connect(fn)
        parent.addAction(action)
        # Keep shortcuts active when the fullscreen menu and toolbar are hidden.
        self.addAction(action)
        return action

    def setup_audio(self):
        device = QMediaDevices.defaultAudioOutput()
        wanted = self.library.setting("audio_device", "")
        if wanted:
            device = next((item for item in QMediaDevices.audioOutputs() if bytes(item.id()).hex() == wanted), device)
        fmt = QAudioFormat()
        fmt.setSampleRate(self.core.sample_rate)
        fmt.setChannelCount(2)
        fmt.setSampleFormat(QAudioFormat.Int16)
        if device.isNull() or not device.isFormatSupported(fmt):
            self.statusBar().showMessage("Audio output unavailable. The game can still run.")
            return
        self.audio = QAudioSink(device, fmt, self)
        self.audio.setBufferSize(self.core.sample_rate * 4 * self.preference_int("audio_latency", 80, 20, 250) // 1000)
        self.audio_device = self.audio.start()
        self.audio.setVolume(0 if self.muted else self.volume)
        if self.audio_device is None:
            self.statusBar().showMessage("Audio output could not start. Choose another output in Settings.")
        self.sync_audio_pause()

    def sync_audio_pause(self):
        if self.audio:
            if self.paused or self.focus_paused or self.fast:
                self.audio.suspend()
            else:
                self.audio.resume()

    def audio_output_changed(self):
        if self.closed:
            return
        if self.audio:
            self.audio.stop()
            self.audio.deleteLater()
        self.audio = self.audio_device = None
        self.audio_pending.clear()
        self.setup_audio()

    def tick(self):
        if self.paused or self.focus_paused:
            self.next_frame = time.perf_counter()
            return
        now = time.perf_counter()
        if now < self.next_frame:
            return
        period = 1 / (self.core.fps * (self.fast_speed if self.fast else 1))
        if now - self.next_frame > 0.15:
            self.next_frame = now
        self.next_frame += period
        try:
            previous_rate = self.core.sample_rate
            pixels, audio = self.core.frame(self.keys | self.pad.poll())
            if self.core.sample_rate != previous_rate:
                if self.audio:
                    self.audio.stop()
                    self.audio.deleteLater()
                self.audio = self.audio_device = None
                self.audio_pending.clear()
                self.setup_audio()
            self.screen.frame = QImage(pixels, self.core.width, self.core.height,
                                       self.core.width * 4, QImage.Format_RGB32).copy()
            self.screen.update()
            if self.audio_device and not self.fast:
                self.audio_pending.extend(audio)
                # Bound latency after underruns, device changes or a slow desktop.
                max_bytes = self.core.sample_rate * 4 // 8
                if len(self.audio_pending) > max_bytes:
                    del self.audio_pending[:-max_bytes]
                count = min(len(self.audio_pending), self.audio.bytesFree()) & ~3
                if count:
                    written = self.audio_device.write(bytes(self.audio_pending[:count]))
                    if written > 0:
                        del self.audio_pending[:written]
            self.frames += 1
            if self.frame_limit and self.frames >= self.frame_limit:
                if self.capture_path:
                    self.grab().save(str(self.capture_path))
                self.close()
        except Exception as e:
            self.timer.stop()
            QMessageBox.critical(self, "Emulation stopped", str(e))
            self.close()

    def state_path(self, name=None):
        return self.state_directory / (name or f"slot-{self.slot.currentIndex()+1}.oesavestate")

    def save_auto(self):
        try:
            self.core.flush()
            self.core.save_state(self.state_path("auto.oesavestate"))
        except (OSError, CoreError) as e:
            self.statusBar().showMessage(f"Automatic save failed: {e}")

    def save(self):
        try:
            self.core.save_state(self.state_path())
            self.statusBar().showMessage(f"Saved to slot {self.slot.currentIndex()+1}", 5000)
        except (OSError, CoreError) as e:
            self.report(e)

    def load(self):
        try:
            self.core.load_state(self.state_path())
            self.flush_audio()
            self.statusBar().showMessage(f"Restored slot {self.slot.currentIndex()+1}", 5000)
        except (OSError, CoreError) as e:
            self.report(e)

    def report(self, error):
        was_paused = self.paused
        self.paused = True
        self.sync_audio_pause()
        QMessageBox.warning(self, "OpenEmu Linux", str(error))
        self.paused = was_paused
        self.sync_audio_pause()
        self.next_frame = time.perf_counter()

    def load_auto(self):
        try:
            self.core.load_state(self.state_path("auto.oesavestate"))
            self.flush_audio()
            self.statusBar().showMessage("Restored the last automatic save", 5000)
        except (OSError, CoreError) as e:
            self.report(e)

    def reset(self):
        self.core.reset()
        self.flush_audio()

    def screenshot(self):
        path = self.library.root / "screenshots" / f"{self.game_id[:12]}-{datetime.now():%Y%m%d-%H%M%S}.png"
        if self.screen.frame.save(str(path)):
            self.statusBar().showMessage(f"Screenshot saved: {path}", 7000)

    def toggle_pause(self):
        self.paused = not self.paused
        self.pause_action.setText("Resume" if self.paused else "Pause")
        self.pause_action.setIcon(hud_icon(self, QStyle.SP_MediaPlay if self.paused else QStyle.SP_MediaPause))
        self.clear_input()
        self.sync_audio_pause()
        self.hud.reveal()
        self.next_frame = time.perf_counter()

    def toggle_mute(self):
        self.muted = not self.muted
        self.library.set_setting("muted", int(self.muted))
        self.mute_action.setChecked(self.muted)
        self.mute_action.setIcon(hud_icon(self, QStyle.SP_MediaVolumeMuted if self.muted else QStyle.SP_MediaVolume))
        if self.audio:
            self.audio.setVolume(0 if self.muted else self.volume)

    def set_volume(self, value):
        value = max(0, min(100, int(value)))
        self.volume = value / 100
        self.library.set_setting("volume", value)
        if self.audio:
            self.audio.setVolume(0 if self.muted else self.volume)
        self.hud.volume.blockSignals(True)
        self.hud.volume.setValue(value)
        self.hud.volume.blockSignals(False)

    def set_fast(self, enabled):
        if self.fast == enabled:
            return
        self.fast = enabled
        self.flush_audio()
        self.next_frame = time.perf_counter()

    def flush_audio(self):
        self.audio_pending.clear()
        if self.audio:
            self.audio.reset()
            self.audio_device = self.audio.start()
            self.audio.setVolume(0 if self.muted else self.volume)
        self.sync_audio_pause()

    def integer_scale(self, value):
        self.screen.integer_scale = value
        self.screen.update()

    def fullscreen(self):
        leaving = self.isFullScreen()
        self.showNormal() if leaving else self.showFullScreen()
        self.menuBar().setVisible(leaving)
        self.statusBar().setVisible(leaving)
        self.fullscreen_action.setText("Full screen" if leaving else "Exit full screen")
        self.hud.reveal()

    def controls(self):
        self.report("Move: arrow keys\nA: X    B: Z\nSNES X: S    SNES Y: A\nStart: Enter    Select: Backspace\nL: Q    R: W\nPause: Ctrl+P\nSave: F5    Load: F8\nRestore automatic save: Ctrl+F8\nFull screen: F11    Screenshot: F12\nFast forward: hold Tab\n\nController: d-pad / left stick, east=A, south=B, north=X, west=Y, Start, Back and shoulders.\nGames pause when their window loses focus.")

    def clear_input(self):
        self.keys = 0
        self.set_fast(False)
        self.audio_pending.clear()

    def changeEvent(self, event):
        from PySide6.QtCore import QEvent
        if event.type() == QEvent.ActivationChange and hasattr(self, "keys") and not self.closed:
            self.focus_paused = self.pause_unfocused and not self.isActiveWindow() and not self.frame_limit
            self.clear_input()
            self.sync_audio_pause()
        super().changeEvent(event)

    def keyPressEvent(self, event):
        if event.isAutoRepeat(): return
        if event.key() in KEYS:
            self.keys |= KEYS[event.key()]
            event.accept()
        elif event.key() == Qt.Key_Tab:
            self.set_fast(True)
        elif event.key() == Qt.Key_Escape and self.isFullScreen():
            self.fullscreen()
        else:
            super().keyPressEvent(event)

    def keyReleaseEvent(self, event):
        if event.isAutoRepeat(): return
        if event.key() in KEYS:
            self.keys &= ~KEYS[event.key()]
        elif event.key() == Qt.Key_Tab:
            self.set_fast(False)

    def event(self, event):
        from PySide6.QtCore import QEvent
        if event.type() in (QEvent.KeyPress, QEvent.KeyRelease) and event.key() == Qt.Key_Tab:
            (self.keyPressEvent if event.type() == QEvent.KeyPress else self.keyReleaseEvent)(event)
            return True
        return super().event(event)

    def closeEvent(self, event):
        if self.closed:
            event.accept()
            return
        self.closed = True
        self.timer.stop()
        self.autosave.stop()
        self.hud.stop()
        self.save_auto()
        self.core.close()
        self.pad.close()
        if self.audio:
            self.audio.stop()
        self.lock.unlock()
        event.accept()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--data-dir", required=True)
    parser.add_argument("--game", required=True)
    parser.add_argument("--mute", action="store_true")
    parser.add_argument("--frames", type=int, default=0)
    parser.add_argument("--screenshot")
    args = parser.parse_args()
    app = QApplication(sys.argv[:1])
    app.setApplicationName("OpenEmu Linux")
    library = Library(args.data_dir)
    try:
        player = Player(library, args.game, args.mute, args.frames, args.screenshot)
    except Exception as e:
        print(str(e), file=sys.stderr)
        QMessageBox.critical(None, "Could not start game", str(e))
        return 1
    player.show()
    print("OPENEMU_GAME_READY", flush=True)
    result = app.exec()
    library.close()
    return result


if __name__ == "__main__":
    sys.exit(main())
