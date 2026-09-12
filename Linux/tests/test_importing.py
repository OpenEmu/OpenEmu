"""Managed/external ROMs, resumable import issues and real background UI flows."""
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
from PySide6.QtTest import QTest
from PySide6.QtWidgets import QApplication
from openemu_linux.library import Library, ImportProblem
from openemu_linux.importing import Importer
from openemu_linux.settings import SettingsDialog
from openemu_linux.app import Window
from openemu_linux.player import Player
from snes_rom import snes


class ImportingTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.app = QApplication.instance() or QApplication([])

    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = Path(self.tmp.name)
        self.library = Library(self.root / "library")
        self.library.set_setting("artwork_auto", "0")
        self.rom = self.root / "Keep_Name (USA).SFC"
        self.data = snes()
        self.rom.write_bytes(self.data)
        self.window = None

    def tearDown(self):
        if self.window:
            self.window.close()
        else:
            self.library.close()
        self.tmp.cleanup()

    def test_external_import_consolidation_keeps_bytes_name_and_identity(self):
        self.library.set_setting("copy_games", "0")
        key = self.library.import_file(self.rom)[0]
        row = self.library.get(key)
        self.assertEqual(row["rom_path"], str(self.rom))
        self.assertEqual(list((self.library.root / "roms").iterdir()), [])
        self.library.rate([key], 5)
        self.assertTrue(self.library.consolidate(key))
        row = self.library.get(key)
        self.assertEqual(Path(row["rom_path"]).parts, ("roms", "snes", key, self.rom.name))
        self.assertEqual((self.library.root / row["rom_path"]).read_bytes(), self.data)
        self.assertEqual(self.rom.read_bytes(), self.data)
        self.assertEqual(row["rating"], 5)
        self.assertFalse(self.library.consolidate(key))

    def test_zip_extracts_managed_copies_even_in_external_mode_and_partial_counts(self):
        self.library.set_setting("copy_games", "0")
        archive = self.root / "roms.zip"
        with zipfile.ZipFile(archive, "w") as z:
            z.writestr("folder/Keep_Name.SFC", self.data)
            z.writestr("Bad.sfc", b"truncated")
        worker = Importer(self.library.root, [archive])
        results = []
        worker.result.connect(lambda count, errors: results.append((count, errors)))
        worker.run()
        self.assertEqual(worker.new_games, 1)
        self.assertEqual(results[0][0], 1)
        self.assertTrue(results[0][1])
        row = self.library.games()[0]
        self.assertFalse(Path(row["rom_path"]).is_absolute())
        self.assertEqual(Path(row["rom_path"]).name, "Keep_Name.SFC")
        self.assertEqual(len(self.library.import_issues()), 1)

    def test_relink_requires_same_hash_and_preserves_saves_across_name_changes(self):
        self.library.set_setting("copy_games", "0")
        key = self.library.import_file(self.rom)[0]
        self.library.rename(key, "Custom library title")
        saves = self.library.root / "saves" / key
        saves.mkdir()
        (saves / (self.rom.stem + ".sav")).write_bytes(b"battery")
        renamed = self.root / "New_Name.sfc"
        self.rom.rename(renamed)
        with self.assertRaisesRegex(ValueError, "missing"):
            self.library.validate_game(key)
        self.library.relink(key, renamed)
        self.assertEqual(self.library.get(key)["title"], "Custom library title")
        self.assertEqual(self.library.validate_game(key), renamed)
        # A second relink before playing must not lose the initial save prefix.
        renamed_again = self.root / "Final_Name.sfc"
        renamed.rename(renamed_again)
        self.library.relink(key, renamed_again)
        self.library.prepare_save_filenames(self.library.get(key), saves)
        self.assertEqual((saves / "Final_Name.sav").read_bytes(), b"battery")
        wrong = self.root / "Other.sfc"
        wrong.write_bytes(snes(pal=True))
        with self.assertRaisesRegex(ValueError, "different ROM"):
            self.library.relink(key, wrong)
        self.assertEqual(self.library.get(key)["rom_path"], str(renamed_again))
        renamed_again.write_bytes(snes(pal=True))
        with self.assertRaisesRegex(ValueError, "changed"):
            self.library.validate_game(key)

    def test_folder_scan_ignores_sidecars_and_symlink_cycles_but_reports_unknowns(self):
        folder = self.root / "collection"
        folder.mkdir()
        (folder / "Game.sfc").write_bytes(self.data)
        (folder / "readme.txt").write_text("Not a ROM")
        (folder / "cover.png").write_bytes(b"not imported")
        (folder / "Mystery.bin").write_bytes(snes(pal=True))
        (folder / "cycle").symlink_to(folder, target_is_directory=True)
        worker = Importer(self.library.root, [folder, folder / "Game.sfc"])
        progress = []
        worker.progress.connect(lambda done, total, message: progress.append((done, total)))
        worker.run()
        self.assertEqual(worker.new_games, 1)
        self.assertEqual(len(self.library.games()), 1)
        self.assertEqual(progress[-1], (2, 2))
        issue = self.library.import_issues()[0]
        self.assertEqual(issue["code"], "unknown_system")
        self.assertEqual(Path(issue["path"]).name, "Mystery.bin")
        worker = Importer(self.library.root, [folder / "Mystery.bin"], system_override="snes")
        worker.run()
        self.assertEqual(len(self.library.games()), 2)
        self.assertEqual(self.library.import_issues(), [])

    def test_specific_error_messages_persist_and_duplicate_count_is_separate(self):
        empty = self.root / "Empty.sfc"
        empty.touch()
        corrupt = self.root / "Broken.zip"
        corrupt.write_bytes(b"broken archive")
        rar = self.root / "Archive.rar"
        rar.write_bytes(b"archive")
        missing = self.root / "Missing.sfc"
        worker = Importer(self.library.root, [self.rom, empty, corrupt, rar, missing])
        worker.run()
        self.assertEqual(worker.new_games, 1)
        self.assertEqual({r["code"] for r in self.library.import_issues()},
                         {"empty_file", "invalid_archive", "unsupported_archive", "missing_file"})
        worker = Importer(self.library.root, [self.rom])
        worker.run()
        self.assertEqual((worker.new_games, worker.duplicates), (0, 1))
        reopened = Library(self.library.root)
        self.assertEqual(len(reopened.import_issues()), 4)
        reopened.close()
        denied = self.root / "Denied.sfc"
        with patch.object(Library, "import_file", side_effect=PermissionError("Denied")):
            Importer(self.library.root, [denied]).run()
        self.assertEqual(self.library.import_issues()[-1]["code"], "permission")

    def wait_worker(self):
        deadline = time.monotonic() + 4
        while time.monotonic() < deadline:
            QTest.qWait(10)
            if self.window.worker and not self.window.worker.isRunning() and self.window.import_button.isEnabled():
                return
        self.fail("Background import did not finish")

    def test_issue_resolver_retry_and_settings_external_import(self):
        settings = SettingsDialog(self.library)
        settings.copy_games.setChecked(False)
        settings.close()
        mystery = self.root / "Mystery.bin"
        mystery.write_bytes(self.data)
        self.window = Window(self.library, auto_artwork=False)
        self.window.show()
        self.window.import_paths([mystery])
        self.wait_worker()
        dialog = self.window.issue_dialog
        self.assertTrue(dialog.isVisible())
        self.assertEqual(len(self.library.import_issues()), 1)
        dialog.system.setCurrentIndex(dialog.system.findData("snes"))
        dialog.retry_selected()
        self.wait_worker()
        self.assertEqual(self.library.import_issues(), [])
        self.assertFalse(self.window.issues_button.isVisible())
        game = self.library.games()[0]
        self.assertEqual(game["rom_path"], str(mystery))
        with patch.object(Player, "setup_audio"):
            player = Player(self.library, game["id"], muted=True)
        player.timer.stop()
        try:
            for _ in range(120):
                video, audio = player.core.frame(0)
            self.assertTrue(video)
        finally:
            player.close()

    def test_cancel_finishes_current_file_and_keeps_committed_games(self):
        second = self.root / "Second.sfc"
        second.write_bytes(snes(pal=True))
        worker = Importer(self.library.root, [self.rom, second])
        cancelled = False
        original = Library.import_file
        def import_one(library, path, **kwargs):
            nonlocal cancelled
            ids = original(library, path, **kwargs)
            cancelled = True
            return ids
        # The cooperative stop check is the same one used by QThread cancellation.
        with patch.object(worker, "isInterruptionRequested", side_effect=lambda: cancelled), \
             patch.object(Library, "import_file", import_one):
            worker.run()
        self.assertTrue(worker.cancelled)
        self.assertEqual(worker.new_games, 1)
        self.assertEqual(len(self.library.games()), 1)
        self.assertEqual(self.rom.read_bytes(), self.data)


if __name__ == "__main__":
    unittest.main()
