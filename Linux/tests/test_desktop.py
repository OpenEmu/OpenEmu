"""Qt keyboard/save integration and SDL controller mapping; no physical device needed."""
import ctypes as C
import os
from pathlib import Path
import tempfile
import unittest
from media_stub import isolate_audio
isolate_audio()
from unittest.mock import patch

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")
from PySide6.QtCore import Qt
from PySide6.QtTest import QTest
from PySide6.QtWidgets import QApplication
from openemu_linux.app import Window
from openemu_linux.gamepad import Gamepad
from openemu_linux.library import Library
from openemu_linux.player import Player
from snes_rom import snes


class DesktopIntegration(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.app = QApplication.instance() or QApplication([])

    def test_snes_library_keyboard_state_shortcuts(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            rom = root / "SNES diagnostic.sfc"
            rom.write_bytes(snes())
            library = Library(root / "library")
            game_id = library.import_file(rom)[0]
            window = Window(library, auto_artwork=False)
            snes_row = next(i for i in range(window.nav.count()) if window.nav.item(i).data(Qt.UserRole) == "snes")
            window.nav.setCurrentRow(snes_row)
            self.assertEqual(window.games.count(), 1)
            window.games.setCurrentRow(0)
            self.assertTrue(window.play_button.isEnabled())
            # Desktop audio is verified separately against the actual audio service.
            with patch.object(Player, "setup_audio"):
                p = Player(library, game_id, muted=True, frame_limit=10000)
            p.timer.stop()
            p.show()
            p.activateWindow()
            QTest.qWait(30)
            def advance(n):
                for _ in range(n):
                    p.next_frame = 0
                    p.tick()
                return p.screen.frame.copy()
            try:
                baseline = advance(120)
                for key in [Qt.Key_X, Qt.Key_Z, Qt.Key_S, Qt.Key_A, Qt.Key_Q, Qt.Key_W,
                            Qt.Key_Up, Qt.Key_Down, Qt.Key_Left, Qt.Key_Right, Qt.Key_Return, Qt.Key_Backspace]:
                    with self.subTest(key=key):
                        QTest.keyPress(p, key)
                        self.assertNotEqual(advance(5), baseline)
                        QTest.keyRelease(p, key)
                        self.assertEqual(advance(5), baseline)
                QTest.keyClick(p, Qt.Key_F5)
                self.assertTrue(p.state_path().is_file())
                QTest.keyPress(p, Qt.Key_X)
                self.assertNotEqual(advance(5), baseline)
                QTest.keyRelease(p, Qt.Key_X)
                QTest.keyClick(p, Qt.Key_F8)
                self.assertEqual(advance(5), baseline)
                self.assertEqual(p.screen.display_aspect, 4 / 3)
            finally:
                p.close()
                window.close()
            self.assertTrue((root / "library/states" / game_id / "auto.oesavestate").exists())

    def test_virtual_controller_buttons_and_disconnect(self):
        pad = Gamepad()
        if not pad.sdl:
            self.skipTest("SDL2 not installed")
        s = pad.sdl
        specs = {
            "SDL_JoystickAttachVirtual": (C.c_int, [C.c_int, C.c_int, C.c_int, C.c_int]),
            "SDL_JoystickOpen": (C.c_void_p, [C.c_int]),
            "SDL_JoystickSetVirtualButton": (C.c_int, [C.c_void_p, C.c_int, C.c_ubyte]),
            "SDL_JoystickDetachVirtual": (C.c_int, [C.c_int]),
            "SDL_JoystickClose": (None, [C.c_void_p]),
        }
        if not hasattr(s, "SDL_JoystickAttachVirtual"):
            pad.close()
            self.skipTest("SDL2 virtual joysticks require SDL 2.0.14 or newer")
        for name, (result, args) in specs.items():
            f = getattr(s, name)
            f.restype, f.argtypes = result, args
        index = s.SDL_JoystickAttachVirtual(1, 6, 15, 0)
        self.assertGreaterEqual(index, 0)
        pad.pad = s.SDL_GameControllerOpen(index)
        joy = s.SDL_JoystickOpen(index)
        self.assertTrue(pad.pad and joy)
        try:
            for button, expected in {1:1, 0:2, 3:1024, 2:2048, 9:512, 10:256,
                                      4:4, 6:8, 14:16, 13:32, 11:64, 12:128}.items():
                with self.subTest(button=button):
                    self.assertEqual(s.SDL_JoystickSetVirtualButton(joy, button, 1), 0)
                    self.assertEqual(pad.poll(), expected)
                    s.SDL_JoystickSetVirtualButton(joy, button, 0)
                    self.assertEqual(pad.poll(), 0)
            s.SDL_JoystickClose(joy)
            joy = None
            s.SDL_JoystickDetachVirtual(index)
            index = -1
            self.assertFalse(s.SDL_GameControllerGetAttached(pad.pad))
            if s.SDL_NumJoysticks() == 0:
                self.assertEqual(pad.poll(), 0)
                self.assertFalse(pad.pad)
        finally:
            if joy: s.SDL_JoystickClose(joy)
            if index >= 0: s.SDL_JoystickDetachVirtual(index)
            pad.close()


if __name__ == "__main__":
    unittest.main()
