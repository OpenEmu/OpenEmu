"""Provider independence, identity checks, edit protection and lookup lifecycle."""
from contextlib import closing
import hashlib
import json
import os
from pathlib import Path
import sqlite3
import tempfile
import threading
import time
import unittest
from unittest.mock import patch

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")
from PySide6.QtCore import QBuffer, QIODevice
from PySide6.QtGui import QImage, QColor
from PySide6.QtTest import QTest
from PySide6.QtWidgets import QApplication
from openemu_linux import artwork as art
from openemu_linux import metadata as meta
from openemu_linux.app import Window
from openemu_linux.library import Library
from openemu_linux.library_widgets import GameInfoDialog
from test_artwork import catalog_file
from snes_rom import snes


class MetadataTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.app = QApplication.instance() or QApplication([])

    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = Path(self.tmp.name)
        self.lib = Library(self.root / "library")
        self.lib.set_setting("artwork_auto", "0")
        self.path = self.root / "Original_Name.SFC"
        self.data = snes()
        self.path.write_bytes(self.data)
        self.game_id = self.lib.import_file(self.path)[0]
        self.catalog = self.root / "catalog.sqlite"
        catalog_file(self.catalog, self.data)
        with closing(sqlite3.connect(self.catalog)) as db:
            for column in ("releaseDeveloper", "releasePublisher", "releaseDate", "releaseGenre", "releaseDescription", "releaseReferenceURL"):
                db.execute(f"ALTER TABLE RELEASES ADD COLUMN {column} TEXT")
            db.execute("""UPDATE RELEASES SET releaseDeveloper='Test Studio',releasePublisher='Test Publisher',
                releaseDate='1993',releaseGenre='Action,Platformer',releaseDescription='<p>A colorful adventure.</p><script>ignore me</script><p>Jump &amp; explore.</p>',
                releaseReferenceURL='https://example.org/game'""")
            db.commit()
        self.window = None

    def tearDown(self):
        if self.window:
            self.window.close()
            deadline = time.monotonic() + 3
            while (self.window.art_worker or self.window.metadata_worker) and time.monotonic() < deadline:
                QTest.qWait(10)
        else:
            self.lib.close()
        self.tmp.cleanup()

    def run_worker(self, **kwargs):
        worker = meta.MetadataWorker(self.lib.root, **kwargs)
        results = []
        worker.result.connect(results.append)
        with patch.object(art.Catalog, "ensure", return_value=self.catalog), patch.object(art.Downloads, "get") as network:
            worker.run()
            network.assert_not_called()
        return results[-1]

    def test_exact_match_enriches_search_hashes_and_preserves_rom_filename(self):
        original = self.lib.get(self.game_id)["rom_path"]
        self.assertEqual(self.run_worker()["matched"], 1)
        row = self.lib.get(self.game_id)
        self.assertEqual(row["title"], "Test Game")
        self.assertEqual(row["rom_path"], original)
        self.assertEqual(self.path.read_bytes(), self.data)
        details = self.lib.metadata(self.game_id)
        self.assertEqual(details["developer"], "Test Studio")
        self.assertEqual(details["players"], "")
        self.assertIn("Jump & explore.", details["description"])
        self.assertNotIn("ignore me", details["description"])
        self.assertEqual(self.lib.games(search="colorful adventure")[0]["id"], self.game_id)
        self.assertEqual(self.lib.hashes(self.game_id)["md5"], hashlib.md5(self.data).hexdigest())
        self.assertEqual(self.lib.metadata_lookup(self.game_id)["provider"], "OpenVGDB")
        self.assertEqual(self.lib.metadata_candidates(), [])
        self.assertEqual(len(self.lib.metadata_candidates(force=True)), 1)
        self.assertEqual(self.run_worker()["matched"], 0)

    def test_manual_fields_and_cleared_values_win_a_background_result(self):
        self.lib.rename(self.game_id, "My custom title")
        self.lib.update_metadata(self.game_id, {"developer": "", "notes": "Keep my notes", "players": "2"})
        self.assertEqual(self.run_worker()["matched"], 1)
        self.assertEqual(self.lib.get(self.game_id)["title"], "My custom title")
        details = self.lib.metadata(self.game_id)
        self.assertEqual((details["developer"], details["notes"], details["players"]), ("", "Keep my notes", "2"))
        self.assertEqual(details["publisher"], "Test Publisher")
        self.assertTrue(self.lib.use_automatic_metadata(self.game_id))
        self.assertEqual(self.lib.get(self.game_id)["title"], "Test Game")
        self.assertEqual(self.lib.metadata(self.game_id)["developer"], "Test Studio")
        self.assertEqual(self.lib.metadata(self.game_id)["notes"], "Keep my notes")

    def test_upgrade_recognizes_existing_manual_information_and_titles(self):
        self.lib.rename(self.game_id, "Older custom title")
        self.lib.update_metadata(self.game_id, {"developer": "Older custom developer"})
        # Recreate the v0.6 schema after putting user edits in its existing fields.
        with self.lib.db:
            for table in ("metadata_overrides", "metadata_lookups", "rom_hashes"):
                self.lib.db.execute(f"DROP TABLE {table}")
            self.lib.db.execute("ALTER TABLE metadata DROP COLUMN description")
            self.lib.db.execute("DELETE FROM schema_migrations WHERE version=3")
        self.lib.close()
        self.lib = Library(self.root / "library")
        self.assertEqual(self.run_worker()["matched"], 1)
        self.assertEqual(self.lib.get(self.game_id)["title"], "Older custom title")
        self.assertEqual(self.lib.metadata(self.game_id)["developer"], "Older custom developer")
        self.assertEqual(self.lib.metadata(self.game_id)["publisher"], "Test Publisher")

    def test_open_editor_does_not_publish_untouched_stale_fields(self):
        dialog = GameInfoDialog(self.lib, self.game_id)
        self.run_worker()
        dialog.fields["notes"].setText("My note")
        dialog.save()
        self.assertEqual(self.lib.get(self.game_id)["title"], "Test Game")
        self.assertEqual(self.lib.metadata(self.game_id)["developer"], "Test Studio")
        self.assertEqual(self.lib.metadata(self.game_id)["notes"], "My note")

    def test_provider_can_be_replaced_and_deleted_games_are_not_restored(self):
        lib = self.lib
        class OtherProvider:
            identifier = "Another catalog"
            def __init__(self, root, downloads): pass
            def prepare(self, progress): pass
            def identify(self, data, system, region):
                return meta.MetadataMatch({"title": "Other title", "players": "4"}, "https://example.org/other")
            def close(self): pass
        self.assertEqual(self.run_worker(provider_factory=OtherProvider)["matched"], 1)
        self.assertEqual(self.lib.metadata(self.game_id)["players"], "4")
        self.assertEqual(self.lib.metadata_lookup(self.game_id)["provider"], "Another catalog")
        class RemovedProvider(OtherProvider):
            def identify(self, data, system, region):
                lib.remove(self_game_id)
                return super().identify(data, system, region)
        self_game_id = self.game_id
        self.assertEqual(self.run_worker(force=True, provider_factory=RemovedProvider)["matched"], 0)
        self.assertIsNone(self.lib.get(self.game_id))
        self.assertIsNone(self.lib.metadata_lookup(self.game_id))

    def test_failed_and_unmatched_lookups_back_off_and_cached_match_survives(self):
        self.run_worker()
        with patch.object(art.Catalog, "ensure", side_effect=OSError("Offline")):
            worker = meta.MetadataWorker(self.lib.root, force=True)
            worker.run()
        self.assertEqual(self.lib.metadata_candidates(), [])
        self.assertEqual(self.lib.metadata_lookup(self.game_id)["status"], "failed")
        self.lib.rename(self.game_id, "Temporary edit")
        self.assertTrue(self.lib.use_automatic_metadata(self.game_id))
        self.assertEqual(self.lib.get(self.game_id)["title"], "Test Game")
        other = self.root / "Unknown.sfc"
        other.write_bytes(snes(pal=True))
        unknown = self.lib.import_file(other)[0]
        self.assertEqual(self.run_worker(game_ids={unknown})["not_found"], 1)
        self.assertEqual(self.lib.get(unknown)["title"], "Unknown")
        self.assertGreater(self.lib.metadata_lookup(unknown)["retry_after"], time.time() + 86400)

    def test_headered_match_and_external_changed_file_rejection(self):
        headered = self.root / "Headered.smc"
        headered.write_bytes(bytes(512) + self.data)
        key = self.lib.import_file(headered)[0]
        self.assertEqual(self.run_worker(game_ids={key})["matched"], 1)
        self.assertEqual(self.lib.hashes(key)["md5"], hashlib.md5(headered.read_bytes()).hexdigest())
        self.lib.remove(self.game_id)
        self.lib.set_setting("copy_games", "0")
        self.lib.import_file(self.path)
        self.path.write_bytes(snes(pal=True))
        self.assertEqual(self.run_worker(game_ids={self.game_id})["failed"], 1)
        self.assertEqual(self.lib.get(self.game_id)["title"], "Original_Name")

    def wait_lookups(self):
        deadline = time.monotonic() + 4
        while time.monotonic() < deadline:
            QTest.qWait(10)
            if not self.window.art_worker and not self.window.metadata_worker and not self.window.metadata_pending and not self.window.art_pending:
                return
        self.fail("Lookups did not finish")

    def test_startup_import_and_cancel_ui(self):
        self.window = Window(self.lib, auto_artwork=False, auto_metadata=True)
        self.window.show()
        with patch.object(art.Catalog, "ensure", return_value=self.catalog):
            QTest.qWait(300)
            self.wait_lookups()
        self.assertEqual(self.lib.get(self.game_id)["title"], "Test Game")
        self.window.auto_metadata_action.setChecked(False)
        self.assertEqual(self.lib.setting("metadata_auto"), "0")
        started = threading.Event()
        def waiting(catalog, progress):
            started.set()
            while not catalog.downloads.cancelled():
                time.sleep(0.005)
            catalog.downloads.check()
        with patch.object(art.Catalog, "ensure", waiting):
            self.window.start_metadata(force=True)
            deadline = time.monotonic() + 3
            while not started.is_set() and time.monotonic() < deadline:
                QTest.qWait(10)
            self.assertTrue(started.is_set())
            self.window.cancel_metadata_action.trigger()
            self.wait_lookups()
        self.assertEqual(self.lib.get(self.game_id)["title"], "Test Game")
        self.assertFalse(self.window.cancel_metadata_action.isEnabled())

    def test_cover_replacement_retains_old_cover_on_failure_and_manual_race(self):
        image = QImage(40, 60, QImage.Format_RGB32)
        image.fill(QColor("#88aaff"))
        buffer = QBuffer()
        buffer.open(QIODevice.WriteOnly)
        image.save(buffer, "PNG")
        png = bytes(buffer.data())
        original = self.lib.root / "covers/manual.png"
        original.write_bytes(png)
        self.lib.set_manual_cover(self.game_id, "covers/manual.png")
        self.lib.set_setting("artwork_backup", "0")
        with patch.object(art.Catalog, "ensure", return_value=self.catalog), \
             patch.object(art.Downloads, "get", side_effect=OSError("Offline")):
            art.ArtworkWorker(self.lib.root, force=True, game_ids={self.game_id}, replace=True).run()
        self.assertEqual(self.lib.get(self.game_id)["cover"], "covers/manual.png")
        with patch.object(art.Catalog, "ensure", return_value=self.catalog), \
             patch.object(art.Downloads, "get", return_value=png):
            art.ArtworkWorker(self.lib.root, force=True, game_ids={self.game_id}, replace=True).run()
        downloaded = self.lib.get(self.game_id)["cover"]
        downloaded_bytes = (self.lib.root / downloaded).read_bytes()
        self.assertNotEqual(downloaded, "covers/manual.png")
        self.assertEqual(original.read_bytes(), png)
        def racing(*args, **kwargs):
            self.lib.set_manual_cover(self.game_id, "covers/manual.png")
            return png
        with patch.object(art.Catalog, "ensure", return_value=self.catalog), patch.object(art.Downloads, "get", racing):
            art.ArtworkWorker(self.lib.root, force=True, game_ids={self.game_id}, replace=True).run()
        self.assertEqual(self.lib.get(self.game_id)["cover"], "covers/manual.png")
        self.assertEqual((self.lib.root / downloaded).read_bytes(), downloaded_bytes)

    def test_import_triggers_information_with_artwork_disabled(self):
        self.lib.remove(self.game_id)
        self.window = Window(self.lib, auto_artwork=False, auto_metadata=True)
        with patch.object(art.Catalog, "ensure", return_value=self.catalog):
            self.window.import_paths([self.path])
            deadline = time.monotonic() + 4
            while time.monotonic() < deadline:
                QTest.qWait(10)
                if self.lib.metadata_lookup(self.game_id) and not self.window.metadata_worker:
                    break
            self.assertEqual(self.lib.get(self.game_id)["title"], "Test Game")
            self.assertEqual(self.lib.metadata_lookup(self.game_id)["status"], "matched")

    def test_replacement_request_stays_scoped_when_queued_with_automatic_work(self):
        other_path = self.root / "Other.sfc"
        other_path.write_bytes(snes(pal=True))
        other_id = self.lib.import_file(other_path)[0]
        image = QImage(40, 60, QImage.Format_RGB32)
        image.fill(QColor("#88aaff"))
        image.save(str(self.lib.root / "covers/manual.png"))
        png = (self.lib.root / "covers/manual.png").read_bytes()
        for key in (self.game_id, other_id):
            self.lib.set_manual_cover(key, "covers/manual.png")
        self.lib.set_setting("artwork_auto", "1")
        self.window = Window(self.lib, auto_artwork=False)
        started, release = threading.Event(), threading.Event()
        catalog_path = self.catalog
        def ensure(catalog, progress):
            if not started.is_set():
                started.set()
                while not release.is_set():
                    catalog.downloads.check()
                    time.sleep(0.005)
            return catalog_path
        with patch.object(art.Catalog, "ensure", ensure), patch.object(art.Downloads, "get", return_value=png) as network:
            self.window.start_metadata(force=True, game_ids={self.game_id})
            deadline = time.monotonic() + 3
            while not started.is_set() and time.monotonic() < deadline:
                QTest.qWait(10)
            self.assertTrue(started.is_set())
            self.window.start_artwork(force=True, game_ids={self.game_id}, replace=True)
            self.window.start_artwork()
            release.set()
            self.wait_lookups()
            self.assertEqual(network.call_count, 1)
        self.assertNotEqual(self.lib.get(self.game_id)["cover"], "covers/manual.png")
        self.assertEqual(self.lib.get(other_id)["cover"], "covers/manual.png")


if __name__ == "__main__":
    unittest.main()
