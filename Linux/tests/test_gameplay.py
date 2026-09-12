"""Gameplay HUD, hidden-menu shortcuts, saved settings and audio transitions."""
import os
from pathlib import Path
import tempfile
import unittest
from media_stub import isolate_audio
isolate_audio()
from unittest.mock import Mock, patch

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")
from PySide6.QtCore import Qt, QPoint, QByteArray
from PySide6.QtTest import QTest
from PySide6.QtWidgets import QApplication
from openemu_linux.library import Library
from openemu_linux.player import Player
from openemu_linux.settings import SettingsDialog
from snes_rom import snes


class GameplayTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.app = QApplication.instance() or QApplication([])

    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = Path(self.tmp.name)
        self.library = Library(self.root / "library")
        rom = self.root / "HUD diagnostic.sfc"
        rom.write_bytes(snes())
        self.game_id = self.library.import_file(rom)[0]
        self.player = None

    def tearDown(self):
        if self.player:
            self.player.close()
        self.library.close()
        self.tmp.cleanup()

    def make_player(self, frame_limit=10000):
        with patch.object(Player, "setup_audio"):
            self.player = Player(self.library, self.game_id, muted=True, frame_limit=frame_limit)
        self.player.timer.stop()
        self.player.show()
        self.player.activateWindow()
        QTest.qWait(20)
        self.player.focus_paused = False
        return self.player

    def test_hud_reveals_hides_keeps_pause_controls_and_fits_small_window(self):
        player = self.make_player()
        QTest.mouseMove(player.screen, QPoint(20, 20))
        self.assertTrue(player.hud.isVisible())
        QTest.mouseMove(player.hud, player.hud.rect().center())
        player.hud.fade_away()
        QTest.qWait(20)
        self.assertFalse(player.hud.isVisible())
        self.assertEqual(player.screen.cursor().shape(), Qt.BlankCursor)
        QTest.mouseMove(player.screen, QPoint(35, 35))
        self.assertTrue(player.hud.isVisible())
        self.assertNotEqual(player.screen.cursor().shape(), Qt.BlankCursor)
        QTest.mouseClick(player.hud.buttons["pause"], Qt.LeftButton)
        self.assertTrue(player.paused)
        self.assertEqual(player.hud.buttons["pause"].accessibleName(), "Resume")
        player.hud.fade_away()
        self.assertTrue(player.hud.isVisible())
        QTest.mouseClick(player.hud.buttons["pause"], Qt.LeftButton)
        self.assertFalse(player.paused)
        player.resize(420, 400)
        QTest.qWait(20)
        self.assertLessEqual(player.hud.geometry().right(), player.screen.width())
        self.assertGreaterEqual(player.hud.geometry().left(), 0)
        self.assertFalse(player.hud.volume.isVisible())

    def test_fullscreen_shortcuts_and_slots_stay_available_without_toolbar(self):
        player = self.make_player()
        for _ in range(120):
            player.next_frame = 0
            player.tick()
        self.assertFalse(player.game_toolbar.isVisible())
        player.slot_actions[3].trigger()
        self.assertEqual(player.slot.currentIndex(), 3)
        player.fullscreen()
        QTest.qWait(20)
        self.assertTrue(player.isFullScreen())
        self.assertFalse(player.menuBar().isVisible())
        self.assertFalse(player.statusBar().isVisible())
        QTest.keyClick(player, Qt.Key_F5)
        self.assertTrue(player.state_path().is_file())
        QTest.keyClick(player, Qt.Key_F8)
        self.assertIn("Restored slot 4", player.statusBar().currentMessage())
        QTest.keyClick(player, Qt.Key_Escape)
        self.assertFalse(player.isFullScreen())
        self.assertTrue(player.menuBar().isVisible())
        self.assertTrue(player.statusBar().isVisible())

    def test_volume_pause_and_fast_forward_reset_audio_and_persist(self):
        self.library.set_setting("fast_forward_speed", "6")
        player = self.make_player()
        audio = Mock()
        player.audio = audio
        player.audio_device = audio.start.return_value
        player.hud.volume.setValue(45)
        self.assertEqual(player.volume, 0.45)
        self.assertEqual(self.library.setting("volume"), "45")
        audio.setVolume.assert_called_with(0)
        player.toggle_mute()
        audio.setVolume.assert_called_with(0.45)
        self.assertEqual(self.library.setting("muted"), "0")
        QTest.keyPress(player, Qt.Key_Tab)
        self.assertTrue(player.fast)
        self.assertEqual(player.fast_speed, 6)
        audio.reset.assert_called()
        audio.suspend.assert_called()
        QTest.keyRelease(player, Qt.Key_Tab)
        self.assertFalse(player.fast)
        audio.resume.assert_called()
        player.toggle_pause()
        audio.suspend.assert_called()
        player.toggle_pause()
        audio.resume.assert_called()
        QTest.keyClick(player, Qt.Key_Up, Qt.ControlModifier)
        self.assertEqual(self.library.setting("volume"), "50")
        self.assertEqual(player.keys, 0)

    def test_gameplay_settings_and_default_fullscreen(self):
        settings = SettingsDialog(self.library)
        settings.fullscreen_default.setChecked(True)
        settings.hide_cursor.setChecked(False)
        settings.fast_speed.setValue(7)
        settings.latency.setValue(120)
        settings.aspect.setCurrentIndex(settings.aspect.findData("square"))
        settings.close()
        player = self.make_player(frame_limit=0)
        self.assertTrue(player.isFullScreen())
        self.assertFalse(player.hud.hide_cursor)
        self.assertEqual(player.fast_speed, 7)
        self.assertIsNone(player.screen.display_aspect)
        self.assertEqual(self.library.setting("audio_latency"), "120")

    def test_audio_output_selection_and_disconnect_fallback_keep_pause_state(self):
        class Device:
            def __init__(self, name): self.name = name
            def id(self): return QByteArray(self.name.encode())
            def isNull(self): return False
            def isFormatSupported(self, fmt): return True
        default, selected = Device("default"), Device("selected")
        self.library.set_setting("audio_device", bytes(selected.id()).hex())
        self.library.set_setting("audio_latency", "120")
        player = self.make_player()
        player.paused = True
        with patch("openemu_linux.player.QMediaDevices.defaultAudioOutput", return_value=default), \
             patch("openemu_linux.player.QMediaDevices.audioOutputs", return_value=[default, selected]) as devices, \
             patch("openemu_linux.player.QAudioSink") as sink:
            player.setup_audio()
            self.assertIs(sink.call_args.args[0], selected)
            sink.return_value.setBufferSize.assert_called_with(player.core.sample_rate * 4 * 120 // 1000)
            sink.return_value.suspend.assert_called()
            devices.return_value = [default]
            player.audio_output_changed()
            self.assertIs(sink.call_args.args[0], default)
            sink.return_value.suspend.assert_called()
            self.assertTrue(player.paused)


if __name__ == "__main__":
    unittest.main()
