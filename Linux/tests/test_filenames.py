"""Preserved ROM names, collision isolation, and legacy save continuity."""
import os
from pathlib import Path
import tempfile
import time
import unittest
from media_stub import isolate_audio
isolate_audio()
from unittest.mock import patch
import zipfile

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")
from PySide6.QtWidgets import QApplication
from PySide6.QtTest import QTest
from openemu_linux.library import Library
from openemu_linux.app import Window
from openemu_linux.core import Core
from openemu_linux.player import Player
from snes_rom import snes


class FilenameTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.app = QApplication.instance() or QApplication([])

    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = Path(self.tmp.name)
        self.lib = Library(self.root / "library")
        self.rom = self.root / "Café_Game (USA) [Rev 1].SFC"
        self.data = snes()
        self.rom.write_bytes(self.data)

    def tearDown(self):
        self.lib.close()
        self.tmp.cleanup()

    def legacy(self, path=None):
        game_id = self.lib.import_file(path or self.rom)[0]
        row = self.lib.get(game_id)
        old = Path("roms") / (game_id + Path(row["rom_path"]).suffix.lower())
        (self.lib.root / old).write_bytes((self.lib.root / row["rom_path"]).read_bytes())
        (self.lib.root / row["rom_path"]).unlink()
        with self.lib.db:
            self.lib.db.execute("UPDATE games SET rom_path=?,original_filename=NULL WHERE id=?", (str(old), game_id))
        return game_id

    def test_exact_filename_and_symlink_name_are_preserved(self):
        game_id = self.lib.import_file(self.rom)[0]
        row = self.lib.get(game_id)
        self.assertEqual(row["original_filename"], self.rom.name)
        self.assertEqual(Path(row["rom_path"]).name, self.rom.name)
        self.assertEqual(row["title"], self.rom.stem)
        self.assertEqual((self.lib.root / row["rom_path"]).read_bytes(), self.data)
        self.assertEqual(self.rom.read_bytes(), self.data)
        target = self.root / "unnamed.data"
        target.write_bytes(snes(pal=True))
        alias = self.root / "Link_Name.SmC"
        alias.symlink_to(target)
        linked = self.lib.get(self.lib.import_file(alias)[0])
        self.assertEqual(Path(linked["rom_path"]).name, alias.name)

    def test_same_names_do_not_collide_and_duplicates_keep_first_name(self):
        first = self.lib.import_file(self.rom)[0]
        other = self.root / "another" / self.rom.name
        other.parent.mkdir()
        other.write_bytes(snes(pal=True))
        second = self.lib.import_file(other)[0]
        self.assertNotEqual(first, second)
        paths = [self.lib.root / self.lib.get(i)["rom_path"] for i in (first, second)]
        self.assertEqual(paths[0].name, paths[1].name)
        self.assertNotEqual(paths[0].parent, paths[1].parent)
        self.assertNotEqual(paths[0].read_bytes(), paths[1].read_bytes())
        duplicate = self.root / "Another filename.sfc"
        duplicate.write_bytes(self.data)
        self.assertEqual(self.lib.import_file(duplicate), [first])
        self.assertEqual(Path(self.lib.get(first)["rom_path"]).name, self.rom.name)
        self.assertEqual(len(self.lib.games()), 2)

    def test_zip_keeps_member_basename_without_extracting_directories(self):
        archive = self.root / "Collection.zip"
        with zipfile.ZipFile(archive, "w") as zip:
            zip.writestr("../../Outside_Name.SFC", self.data)
            zip.writestr("subfolder/Outside_Name.SFC", snes(pal=True))
        ids = self.lib.import_file(archive)
        self.assertEqual(len(ids), 2)
        for game_id in ids:
            path = self.lib.root / self.lib.get(game_id)["rom_path"]
            self.assertEqual(path.name, "Outside_Name.SFC")
            self.assertTrue(path.resolve().is_relative_to(self.lib.root / "roms"))
        self.assertFalse((self.root / "Outside_Name.SFC").exists())

    def test_legacy_zip_recovery_uses_hash_and_reimport_recovers_missing_zip(self):
        archive = self.root / "Collection.zip"
        with zipfile.ZipFile(archive, "w") as zip:
            zip.writestr("wrong.sfc", snes(pal=True))
            zip.writestr("folder/Actual_Name.SFC", self.data)
        ids = self.lib.import_file(archive)
        game_id = next(i for i in ids if self.lib.get(i)["title"] == "Actual_Name")
        row = self.lib.get(game_id)
        old = Path("roms") / (game_id + ".sfc")
        (self.lib.root / old).write_bytes(self.data)
        with self.lib.db:
            self.lib.db.execute("UPDATE games SET rom_path=?,original_filename=NULL WHERE id=?", (str(old), game_id))
        self.assertEqual(self.lib.restore_filenames(), (1, 0))
        self.assertEqual(Path(self.lib.get(game_id)["rom_path"]).name, "Actual_Name.SFC")
        with self.lib.db:
            self.lib.db.execute("UPDATE games SET rom_path=?,original_filename=NULL WHERE id=?", (str(old), game_id))
        archive.unlink()
        self.assertEqual(self.lib.restore_filenames(), (0, 1))
        self.assertEqual(self.lib.import_file(self.rom), [game_id])
        self.assertEqual(Path(self.lib.get(game_id)["rom_path"]).name, self.rom.name)

    def test_legacy_restore_preserves_battery_and_existing_game_identity(self):
        game_id = self.legacy()
        old = self.lib.root / self.lib.get(game_id)["rom_path"]
        saves = self.lib.root / "saves" / game_id
        with Core(old, "snes", saves) as core:
            for _ in range(120): core.frame()
        previous = saves / (game_id + ".sav")
        old_data = previous.read_bytes()
        self.lib.rename(game_id, "My custom title")
        self.lib.favorite(game_id)
        self.rom.unlink()  # Plain imports retain the name even if the source is gone.
        self.assertEqual(self.lib.restore_filenames(), (1, 0))
        row = self.lib.get(game_id)
        self.assertEqual(row["title"], "My custom title")
        self.assertEqual(row["favorite"], 1)
        self.assertTrue(old.exists())
        with patch.object(Player, "setup_audio"):
            player = Player(self.lib, game_id, muted=True, frame_limit=10000)
        player.timer.stop()
        for _ in range(120): player.core.frame()
        player.close()
        named_save = saves / (self.rom.stem + ".sav")
        self.assertEqual(named_save.read_bytes()[0], (old_data[0] + 1) & 255)
        self.assertEqual(previous.read_bytes(), old_data)
        latest = named_save.read_bytes()
        self.lib.prepare_save_filenames(row, saves)
        self.assertEqual(named_save.read_bytes(), latest)
        self.assertTrue((self.lib.root / "states" / game_id / "auto.oesavestate").exists())

    def test_window_restores_legacy_names_in_background(self):
        game_id = self.legacy()
        self.lib.set_setting("artwork_auto", "0")
        window = Window(self.lib, auto_artwork=False)
        deadline = time.monotonic() + 3
        while time.monotonic() < deadline:
            QTest.qWait(10)
            if (window.worker and not window.worker.isRunning() and window.import_button.isEnabled()
                    and not self.lib.needs_filename_restore()):
                break
        self.assertEqual(Path(self.lib.get(game_id)["rom_path"]).name, self.rom.name)
        self.assertIn("Restored 1 original ROM filename", window.statusBar().currentMessage())
        window.close()
        self.lib = Library(self.root / "library")


if __name__ == "__main__":
    unittest.main()
