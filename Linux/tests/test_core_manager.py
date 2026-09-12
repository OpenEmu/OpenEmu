"""Core installation, updates, persistence and Settings without live networking."""
import io
import json
import os
from pathlib import Path
import tempfile
import time
import unittest
from media_stub import isolate_audio
isolate_audio()
from unittest.mock import patch
import zipfile
import zlib

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")
from PySide6.QtTest import QTest
from PySide6.QtWidgets import QApplication
from openemu_linux.artwork import Downloads
from openemu_linux.core import CoreError
from openemu_linux.core_manager import CoreManager, BUILDBOT
from openemu_linux.library import Library
from openemu_linux.settings import SettingsDialog


def binary(version=1):
    data = bytearray(128)
    data[:6] = b"\x7fELF\x02\x01"
    data[18:20] = (62).to_bytes(2, "little")
    data[-1] = version
    return bytes(data)


class ManagerTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.app = QApplication.instance() or QApplication([])

    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.lib = Library(self.tmp.name)
        self.manager = CoreManager(self.lib.root)
        self.probe = patch("openemu_linux.core_manager.probe", return_value={
            "name": "Snes9x", "version": "test", "extensions": "smc|sfc"})
        self.probe.start()

    def tearDown(self):
        self.probe.stop()
        self.lib.close()
        self.tmp.cleanup()

    def archive(self, data):
        buffer = io.BytesIO()
        with zipfile.ZipFile(buffer, "w") as zip:
            zip.writestr("snes9x_libretro.so", data)
            zip.writestr("../../unexpected.so", data)
        return buffer.getvalue()

    def test_download_update_integrity_and_unchanged_build(self):
        downloads = Downloads()
        first = binary()
        index = {"snes9x_libretro.so.zip": {"build": "2026-09-10", "crc": f"{zlib.crc32(first):08x}"}}
        with patch.object(downloads, "get", return_value=self.archive(first)) as get:
            old, updated = self.manager.download("snes9x", downloads, index=index)
            self.assertTrue(updated)
            self.assertFalse((self.lib.root.parent / "unexpected.so").exists())
            _, updated = self.manager.download("snes9x", downloads, index=index)
            self.assertFalse(updated)
            self.assertEqual(get.call_count, 1)
        old_path = self.manager.path(old)
        second = binary(2)
        second_index = {"snes9x_libretro.so.zip": {"build": "2026-09-11", "crc": f"{zlib.crc32(second):08x}"}}
        with patch.object(downloads, "get", return_value=self.archive(first)):
            with self.assertRaises(CoreError):
                self.manager.download("snes9x", downloads, index=second_index)
        self.assertEqual(self.manager.installed()["snes9x"], old)
        with patch.object(downloads, "get", return_value=self.archive(second)):
            new, changed = self.manager.download("snes9x", downloads, index=second_index)
        self.assertTrue(changed)
        self.assertNotEqual(old_path, self.manager.path(new))
        self.assertEqual(old_path.read_bytes(), first)  # Running builds are retained.
        self.assertEqual(self.manager.path(new).read_bytes(), second)

    def test_redownload_repairs_corrupt_file(self):
        first = binary()
        record = self.manager.install_bytes(first, "snes9x", source="buildbot", crc=f"{zlib.crc32(first):08x}")
        path = self.manager.path(record)
        path.write_bytes(b"corrupt")
        with self.assertRaises(CoreError):
            self.manager.path(record)
        index = {"snes9x_libretro.so.zip": {"build": "2026-09-10", "crc": record["crc"]}}
        with patch.object(Downloads, "get", return_value=self.archive(first)):
            repaired, changed = self.manager.download("snes9x", Downloads(), index=index)
        self.assertTrue(changed)
        self.assertEqual(self.manager.path(repaired).read_bytes(), first)

    def test_import_bad_binary_and_failed_probe_preserve_install(self):
        record = self.manager.install_bytes(binary(), "snes9x")
        with self.assertRaises(CoreError):
            self.manager.install_bytes(b"not an ELF file", "snes9x")
        with patch("openemu_linux.core_manager.probe", side_effect=CoreError("Missing dependency")):
            with self.assertRaises(CoreError):
                self.manager.install_bytes(binary(2), "snes9x")
        self.assertEqual(self.manager.installed()["snes9x"], record)
        local = self.lib.root / "my_libretro.so"
        local.write_bytes(binary(3))
        imported = self.manager.import_file(local)
        self.assertEqual(imported["source"], "local")
        self.assertEqual(imported["systems"], ["snes"])
        self.assertEqual(local.read_bytes(), binary(3))

    def test_settings_selection_and_general_options_persist(self):
        self.manager.install_bytes(binary(), "snes9x")
        dialog = SettingsDialog(self.lib)
        dialog.choices["snes"].setCurrentIndex(dialog.choices["snes"].findData("snes9x"))
        dialog.volume.setValue(37)
        dialog.focus_pause.setChecked(False)
        dialog.integer.setChecked(True)
        dialog.accept()
        reopened = Library(self.lib.root)
        try:
            self.assertEqual(reopened.setting("volume"), "37")
            self.assertEqual(reopened.setting("pause_unfocused"), "0")
            self.assertEqual(reopened.setting("integer_scale"), "1")
            self.assertEqual(self.manager.selection(reopened, "snes")["id"], "snes9x")
            self.assertIsNone(self.manager.selection(reopened, "gb"))
            reopened.set_setting("core.snes", "missing")
            with self.assertRaises(CoreError):
                self.manager.selection(reopened, "snes")
        finally:
            reopened.close()

    def test_settings_download_button_installs_and_enables_selection(self):
        data = binary()
        def get(_downloads, url, *args):
            if url.endswith(".index-extended"):
                return f"2026-09-10 {zlib.crc32(data):08x} snes9x_libretro.so.zip\n".encode()
            self.assertEqual(url, BUILDBOT + "snes9x_libretro.so.zip")
            return self.archive(data)
        dialog = SettingsDialog(self.lib)
        with patch.object(Downloads, "get", get):
            dialog.start_job(core_id="snes9x")
            deadline = time.monotonic() + 3
            while dialog.worker and time.monotonic() < deadline:
                QTest.qWait(10)
            self.assertIsNone(dialog.worker)
        self.assertGreaterEqual(dialog.choices["snes"].findData("snes9x"), 0)
        self.assertTrue(dialog.update_button.isEnabled())
        self.assertIn("installed or updated", dialog.status.text())
        dialog.accept()


if __name__ == "__main__":
    unittest.main()
