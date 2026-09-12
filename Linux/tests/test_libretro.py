"""Integration with an official downloaded Snes9x core; opt in via environment."""
import os
from pathlib import Path
import tempfile
import subprocess
import sys
import unittest
from media_stub import isolate_audio
isolate_audio()
from unittest.mock import patch

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")
from PySide6.QtWidgets import QApplication
from openemu_linux.core import Core, CoreError
from openemu_linux.core_manager import CoreManager, CATALOG
from openemu_linux.library import Library
from openemu_linux.player import Player
from snes_rom import snes


@unittest.skipUnless(os.environ.get("OPENEMU_TEST_LIBRETRO_DIR"), "Set OPENEMU_TEST_LIBRETRO_DIR to a library with downloaded Snes9x")
class LibretroIntegration(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.app = QApplication.instance() or QApplication([])

    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = Path(self.tmp.name)
        self.rom = self.root / "test.sfc"
        self.rom.write_bytes(snes())
        manager = CoreManager(os.environ["OPENEMU_TEST_LIBRETRO_DIR"])
        self.record = manager.installed()["snes9x"]
        self.binary = manager.path(self.record)

    def tearDown(self):
        self.tmp.cleanup()

    def core(self):
        return Core(self.rom, "snes", self.root / "save", libretro_path=self.binary, core_id="snes9x")

    def frames(self, core, count=120, keys=0):
        for _ in range(count):
            video, audio = core.frame(keys)
        return video, audio

    def test_input_video_resampled_audio_state_and_battery(self):
        with self.core() as core:
            baseline, audio = self.frames(core)
            self.assertEqual((core.width, core.height, core.sample_rate), (256, 224, 48000))
            self.assertGreater(len(set(audio)), 30)
            self.assertTrue(3000 < len(audio) < 3500)
            for bit in range(12):
                pressed, _ = self.frames(core, 5, 1 << bit)
                self.assertNotEqual(pressed, baseline)
                released, _ = self.frames(core, 5)
                self.assertEqual(released, baseline)
            state = self.root / "state"
            core.save_state(state)
            self.frames(core, 5, 1)
            core.load_state(state)
            self.assertEqual(self.frames(core, 5)[0], baseline)
            core.flush()
            battery = self.root / "save/battery.srm"
            first = battery.read_bytes()[0]
        with self.core() as core:
            self.frames(core)
        self.assertEqual(battery.read_bytes()[0], (first + 1) & 255)
        with Core(self.rom, "snes", self.root / "native-save") as original:
            with self.assertRaises(CoreError):
                original.load_state(state)

    def test_pal_dynamic_resolution(self):
        self.rom.write_bytes(snes(pal=True, video_mode=13))
        with self.core() as core:
            self.frames(core)
            self.assertAlmostEqual(core.fps, 50, delta=0.02)
            # The Libretro Snes9x defaults crop overscan; interlace still doubles height.
            self.assertEqual((core.width, core.height), (512, 448))

    def test_player_selection_isolates_saves_and_applies_settings(self):
        library = Library(self.root / "library")
        manager = CoreManager(library.root)
        record = manager.install_bytes(self.binary.read_bytes(), "snes9x")
        game_id = library.import_file(self.rom)[0]
        library.set_setting("core.snes", "snes9x")
        library.set_setting("volume", "37")
        library.set_setting("integer_scale", "1")
        with patch.object(Player, "setup_audio"):
            player = Player(library, game_id, muted=True, frame_limit=10000)
        player.timer.stop()
        self.assertTrue(player.core.is_libretro)
        self.assertEqual(player.volume, 0.37)
        self.assertTrue(player.screen.integer_scale)
        expected = library.root / "states" / game_id / "libretro/snes9x" / record["sha256"]
        self.assertEqual(player.state_path("auto.oesavestate").parent, expected)
        self.frames(player.core)
        player.close()
        self.assertTrue((expected / "auto.oesavestate").is_file())
        self.assertFalse((library.root / "states" / game_id / "auto.oesavestate").exists())
        library.close()

    def test_each_installed_catalog_core_runs_its_offered_systems(self):
        manager = CoreManager(os.environ["OPENEMU_TEST_LIBRETRO_DIR"])
        demos = Path(__file__).resolve().parents[1] / "demos"
        # Each shared library gets a fresh helper process, just as in the app.
        script = '''
from openemu_linux.core import Core
from pathlib import Path
import sys, struct
rom, system, binary, core_id, save = sys.argv[1:]
with Core(rom, system, save, libretro_path=binary, core_id=core_id) as core:
    for _ in range(480): baseline, audio = core.frame()
    assert len(set(struct.unpack('<' + 'I' * (len(baseline)//4), baseline))) > 1
    assert core.sample_rate == 48000 and len(audio) > 0
    if system != 'gba': assert len(set(audio)) > 1
    for _ in range(30): pressed, _ = core.frame(1)
    assert pressed != baseline, 'A input was not delivered'
    for _ in range(30): released, _ = core.frame()
    assert released == baseline, 'Released input did not restore the display'
    state = Path(save) / 'test.state'
    core.save_state(state)
    core.load_state(state)
'''
        names = {"gb": "Game Boy.gb", "gbc": "Game Boy Color.gbc", "gba": "Game Boy Advance.gba", "snes": "SNES.sfc"}
        for core_id, record in manager.installed().items():
            if core_id not in CATALOG:
                continue
            for system in record["systems"]:
                with self.subTest(core=core_id, system=system):
                    result = subprocess.run([sys.executable, "-c", script,
                        str(demos / ("OpenEmu Linux - " + names[system])), system,
                        str(manager.path(record)), core_id, str(self.root / core_id / system)],
                        capture_output=True, timeout=20)
                    self.assertEqual(result.returncode, 0, (result.stdout + result.stderr).decode(errors="replace")[-2000:])


if __name__ == "__main__":
    unittest.main()
