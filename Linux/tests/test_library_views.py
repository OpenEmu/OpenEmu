"""Library migrations, dynamic membership and real Qt grid/list interactions."""
import json
import os
from pathlib import Path
import sqlite3
import tempfile
import time
import unittest
from unittest.mock import patch

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")
from PySide6.QtCore import Qt, QMimeData, QPointF, QUrl
from PySide6.QtGui import QDropEvent, QImage, QColor
from PySide6.QtTest import QTest
from PySide6.QtWidgets import QApplication
from openemu_linux.app import Window
from openemu_linux.library import Library
from openemu_linux.library_widgets import GAME_MIME, GameInfoDialog, SmartCollectionDialog
from roms import gameboy
from snes_rom import snes


class LibraryViews(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.app = QApplication.instance() or QApplication([])

    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = Path(self.tmp.name)
        self.lib = Library(self.root / "library")
        self.ids = []
        for name, data in [("Zulu_100% (USA).SFC", snes()), ("Alpha.gb", gameboy()), ("Color.gbc", gameboy(True))]:
            path = self.root / name
            path.write_bytes(data)
            self.ids.extend(self.lib.import_file(path))
        self.window = None

    def tearDown(self):
        if self.window:
            self.window.close()
        else:
            self.lib.close()
        self.tmp.cleanup()

    def window_setup(self):
        self.lib.set_setting("artwork_auto", "0")
        self.window = Window(self.lib, auto_artwork=False)
        self.window.show()
        QTest.qWait(20)
        return self.window

    def test_old_database_upgrade_preserves_game_data_and_is_repeatable(self):
        old = self.root / "old"
        old.mkdir()
        db = sqlite3.connect(old / "library.sqlite3")
        db.execute("""CREATE TABLE games (id TEXT PRIMARY KEY,title TEXT NOT NULL,system TEXT NOT NULL,
            rom_path TEXT NOT NULL,source TEXT NOT NULL,added REAL NOT NULL,last_played REAL,
            play_count INTEGER NOT NULL DEFAULT 0,favorite INTEGER NOT NULL DEFAULT 0,cover TEXT)""")
        db.execute("INSERT INTO games VALUES('rom','Custom title','snes','roms/rom.sfc','/original/Keep.SFC',1,2,12,1,'covers/rom.png')")
        db.commit()
        db.close()
        for _ in range(2):
            lib = Library(old)
            row = lib.get("rom")
            self.assertEqual((row["title"], row["play_count"], row["favorite"], row["cover"], row["rating"]),
                             ("Custom title", 12, 1, "covers/rom.png", 0))
            self.assertEqual([r[0] for r in lib.db.execute("SELECT version FROM schema_migrations ORDER BY version")], [1, 2, 3])
            lib.close()

    def test_collection_operations_never_change_roms_and_remove_membership(self):
        original = {self.lib.root / self.lib.get(key)["rom_path"]:
                    (self.lib.root / self.lib.get(key)["rom_path"]).read_bytes() for key in self.ids}
        collection = self.lib.save_collection("Mixed systems")
        self.lib.add_to_collection(collection, self.ids[:2] + self.ids[:1])
        self.assertEqual({row["id"] for row in self.lib.games(collection=collection)}, set(self.ids[:2]))
        self.lib.remove_from_collection(collection, self.ids[:1])
        self.assertIsNotNone(self.lib.get(self.ids[0]))
        self.lib.remove(self.ids[1])
        self.assertEqual(self.lib.games(collection=collection), [])
        self.assertEqual(self.lib.db.execute("SELECT COUNT(*) FROM collection_games").fetchone()[0], 0)
        self.lib.delete_collection(collection)
        self.assertEqual(len(self.lib.games()), 2)
        for path, data in original.items():
            self.assertEqual(path.read_bytes(), data)

    def test_smart_rules_follow_ratings_play_history_and_dates(self):
        collection = self.lib.save_collection("Best unplayed SNES", {"system": "snes", "minimum_rating": 4, "never_played": True})
        self.assertEqual(self.lib.games(collection=collection), [])
        self.lib.rate(self.ids[:2], 5)
        self.assertEqual([r["id"] for r in self.lib.games(collection=collection)], self.ids[:1])
        self.lib.played(self.ids[0])
        self.assertEqual(self.lib.games(collection=collection), [])
        self.lib.save_collection("Played lately", {"played_days": 7}, collection)
        self.assertEqual([r["id"] for r in self.lib.games(collection=collection)], self.ids[:1])
        self.lib.favorite(self.ids[0])
        self.lib.save_collection("Favorites", {"favorite": True, "added_days": 1}, collection)
        self.assertEqual([r["id"] for r in self.lib.games(collection=collection)], self.ids[:1])
        with self.lib.db:
            self.lib.db.execute("UPDATE games SET added=? WHERE id=?", (time.time() - 40 * 86400, self.ids[0]))
        self.assertEqual(self.lib.games(collection=collection), [])
        self.assertEqual(len(self.lib.games(added=True)), 2)
        with self.assertRaises(ValueError):
            self.lib.add_to_collection(collection, self.ids)
        for rules in ({"sql": "1=1"}, {"minimum_rating": "4 OR 1=1"}, {"played_days": -2}):
            with self.assertRaises(ValueError): self.lib.save_collection("Invalid", rules)
        for rating in (-1, 6, 1.5, True):
            with self.assertRaises(ValueError): self.lib.rate(self.ids, rating)

    def test_search_respects_collection_and_matches_filename_system_metadata(self):
        self.lib.rename(self.ids[0], "A different title")
        self.lib.update_metadata(self.ids[0], {"developer": "Tiny Studio", "genre": "Platformer"})
        collection = self.lib.save_collection("SNES")
        self.lib.add_to_collection(collection, self.ids[:1])
        for query in ("different", "Zulu_100%", "Super Nintendo", "tiny studio", "Platformer"):
            self.assertEqual([r["id"] for r in self.lib.games(search=query, collection=collection)], self.ids[:1])
        self.assertEqual(self.lib.games(search="Alpha", collection=collection), [])
        self.assertEqual(self.lib.games(search="ZuluX100%", collection=collection), [])
        self.assertEqual([r["id"] for r in self.lib.games(collection=collection)], self.ids[:1])

    def test_grid_list_selection_sorting_ratings_and_reopen_preferences(self):
        window = self.window_setup()
        window.restore_selection(self.ids[:2])
        self.assertFalse(window.play_button.isEnabled())
        window.change_view("list")
        self.assertEqual(set(window.selected_ids()), set(self.ids[:2]))
        menu = window.game_menu(window.selected_ids())
        ratings = next(a.menu() for a in menu.actions() if a.text() == "Rating")
        ratings.actions()[4].trigger()
        window.refresh()
        self.assertEqual([self.lib.get(key)["rating"] for key in self.ids], [4, 4, 0])
        window.change_view("grid")
        self.assertEqual(set(window.selected_ids()), set(self.ids[:2]))
        window.change_view("list")
        with self.lib.db:
            self.lib.db.execute("UPDATE games SET play_count=12 WHERE id=?", (self.ids[0],))
            self.lib.db.execute("UPDATE games SET play_count=2 WHERE id=?", (self.ids[1],))
        window.refresh()
        window.table.sortItems(5, Qt.DescendingOrder)
        self.assertEqual([window.table.item(i, 5).text() for i in range(3)], ["12", "2", "0"])
        window.table.setColumnWidth(0, 290)
        window.cover_size.setValue(220)
        window.nav.setCurrentRow(next(i for i in range(window.nav.count()) if window.nav.item(i).data(Qt.UserRole) == "snes"))
        window.close()
        self.lib = Library(self.root / "library")
        self.window = Window(self.lib, auto_artwork=False)
        self.assertEqual(self.window.view_mode, "list")
        self.assertEqual(self.window.nav.currentItem().data(Qt.UserRole), "snes")
        self.assertEqual(self.window.cover_size.value(), 220)
        self.assertEqual(self.window.table.columnWidth(0), 290)
        self.assertEqual(self.window.table.horizontalHeader().sortIndicatorSection(), 5)

    def test_sidebar_drop_search_and_context_actions_match_both_views(self):
        collection = self.lib.save_collection("Mixed")
        window = self.window_setup()
        row = next(i for i in range(window.nav.count()) if window.nav.item(i).data(Qt.UserRole) == f"collection:{collection}")
        window.nav.scrollToItem(window.nav.item(row))
        mime = QMimeData()
        mime.setData(GAME_MIME, json.dumps(self.ids).encode())
        event = QDropEvent(QPointF(window.nav.visualItemRect(window.nav.item(row)).center()), Qt.CopyAction, mime, Qt.LeftButton, Qt.NoModifier)
        window.nav.dropEvent(event)
        self.assertTrue(event.isAccepted())
        window.nav.setCurrentRow(row)
        self.assertEqual(len(window.rows), 3)
        window.search.setText("Game Boy")
        self.assertEqual(len(window.rows), 2)
        window.search.clear()
        self.assertEqual(len(window.rows), 3)
        window.restore_selection(self.ids[:1])
        grid_menu = [a.text() for a in window.game_menu(window.selected_ids()).actions()]
        window.change_view("list")
        table_menu = window.game_menu(window.selected_ids())
        self.assertEqual(grid_menu, [a.text() for a in table_menu.actions()])
        next(a for a in table_menu.actions() if a.text() == "Remove from collection").trigger()
        window.refresh()
        self.assertEqual(len(window.rows), 2)
        self.assertEqual(len(self.lib.games()), 3)

    def test_information_edit_and_cover_drop_leave_rom_untouched(self):
        window = self.window_setup()
        game_id = self.ids[0]
        original = self.lib.get(game_id)["rom_path"]
        dialog = GameInfoDialog(self.lib, game_id, window)
        dialog.title.setText("My custom title")
        dialog.rating.setCurrentIndex(5)
        dialog.fields["developer"].setText("My studio")
        dialog.save()
        self.assertEqual(self.lib.get(game_id)["rom_path"], original)
        self.assertEqual(self.lib.get(game_id)["rating"], 5)
        self.assertEqual(self.lib.metadata(game_id)["developer"], "My studio")
        self.assertEqual(self.lib.games(search="My studio")[0]["id"], game_id)
        path = self.root / "cover.png"
        image = QImage(40, 60, QImage.Format_RGB32)
        image.fill(QColor("#ff9966"))
        image.save(str(path))
        item = next(window.games.item(i) for i in range(window.games.count()) if window.games.item(i).data(Qt.UserRole) == game_id)
        mime = QMimeData()
        mime.setUrls([QUrl.fromLocalFile(str(path))])
        event = QDropEvent(QPointF(window.games.visualItemRect(item).center()), Qt.CopyAction, mime, Qt.LeftButton, Qt.NoModifier)
        window.games.dropEvent(event)
        self.assertTrue(self.lib.get(game_id)["cover"])
        self.assertTrue((self.lib.root / original).is_file())

    def test_smart_dialog_validation_and_keyboard_launch_from_list(self):
        window = self.window_setup()
        dialog = SmartCollectionDialog(window)
        dialog.name.setText("Recent favorites")
        dialog.validate()
        self.assertEqual(dialog.result(), 0)
        dialog.favorite.setChecked(True)
        dialog.validate()
        self.assertEqual(dialog.result(), 1)
        window.change_view("list")
        window.table.setCurrentCell(0, 0)
        window.table.setFocus()
        window.activateWindow()
        QTest.qWait(30)
        # Return must reach the same QProcess launch path as grid double-click.
        with patch("openemu_linux.app.QProcess") as process:
            QTest.keyClick(window.table, Qt.Key_Return)
            self.assertTrue(process.return_value.start.called)
            args = process.return_value.setArguments.call_args.args[0]
            self.assertIn(window.selected_ids()[0], args)
        window.processes.clear()


if __name__ == "__main__":
    unittest.main()
