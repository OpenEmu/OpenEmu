"""Integration tests using the original 65816/SPC700 diagnostic cartridge."""
import hashlib
from pathlib import Path
import tempfile
import unittest
import zipfile

from openemu_linux.core import Core, CoreError
from openemu_linux.library import Library
from snes_rom import snes


class SnesIntegration(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = Path(self.tmp.name)
        self.rom = self.root / "test.sfc"
        self.rom.write_bytes(snes())

    def tearDown(self):
        self.tmp.cleanup()

    def core(self, path=None):
        return Core(path or self.rom, "snes", self.root / "saves")

    def run_frames(self, core, count=120, keys=0):
        for _ in range(count):
            video, audio = core.frame(keys)
        return video, audio

    def test_video_audio_and_all_twelve_buttons(self):
        with self.core() as c:
            baseline, audio = self.run_frames(c)
            self.assertEqual((c.width, c.height), (256, 224))
            self.assertEqual(len(baseline), 256 * 224 * 4)
            self.assertGreater(len(set(baseline)), 3)
            self.assertGreater(len(audio), 2500)
            self.assertLess(len(audio), 4000)
            self.assertGreater(len(set(audio)), 30)  # SPC tone is not a silent buffer.
            frames = set()
            for bit in range(12):
                with self.subTest(button=bit):
                    pressed, _ = self.run_frames(c, 5, 1 << bit)
                    self.assertNotEqual(pressed, baseline)
                    frames.add(hashlib.sha256(pressed).hexdigest())
                    released, _ = self.run_frames(c, 5)
                    self.assertEqual(released, baseline)
            self.assertEqual(len(frames), 12)  # No swapped/aliased button indicators.

    def test_state_restore_integrity_and_rom_identity(self):
        state = self.root / "slot.oesavestate"
        with self.core() as c:
            baseline, _ = self.run_frames(c)
            c.save_state(state)
            pressed, _ = self.run_frames(c, 5, 1)
            self.assertNotEqual(pressed, baseline)
            c.load_state(state)
            restored, _ = self.run_frames(c, 5)
            self.assertEqual(restored, baseline)
            original = state.read_bytes()
            state.write_bytes(original[:-1] + bytes([original[-1] ^ 1]))
            with self.assertRaises(CoreError): c.load_state(state)
            state.write_bytes(original)
        other = self.root / "other.sfc"
        other.write_bytes(snes(pal=True))
        with self.core(other) as c:
            with self.assertRaises(CoreError): c.load_state(state)

    def test_battery_persists_across_reopen_and_reset(self):
        with self.core() as c:
            self.run_frames(c)
            c.lib.oe_flush(c.handle)
            battery = self.root / "saves/test.sav"
            first = battery.read_bytes()[0]
            self.assertEqual(battery.stat().st_size, 8192)
        with self.core() as c:
            self.run_frames(c)
        second = battery.read_bytes()[0]
        self.assertEqual(second, (first + 1) & 255)
        with self.core() as c:
            self.run_frames(c)
            c.reset()
            self.run_frames(c)
        self.assertEqual(battery.read_bytes()[0], (second + 2) & 255)

    def test_pal_high_resolution_overscan_and_interlace(self):
        for mode, dimensions in [(0, (256, 224)), (4, (256, 239)), (12, (512, 239)), (13, (512, 478))]:
            with self.subTest(mode=mode):
                rom = self.root / f"pal-{mode}.sfc"
                rom.write_bytes(snes(pal=True, video_mode=mode))
                with self.core(rom) as c:
                    video, audio = self.run_frames(c)
                    self.assertEqual(c.fps, 50)
                    self.assertEqual((c.width, c.height), dimensions)
                    self.assertEqual(len(video), dimensions[0] * dimensions[1] * 4)
                    self.assertGreater(len(set(video)), 3)
                    self.assertGreater(len(audio), 3500)

    def test_headered_smc_and_zip_import(self):
        smc = self.root / "test.smc"
        smc.write_bytes(bytes(512) + self.rom.read_bytes())
        with self.core(smc) as c:
            video, _ = self.run_frames(c)
            self.assertGreater(len(set(video)), 3)
        zipped = self.root / "test.zip"
        with zipfile.ZipFile(zipped, "w") as z:
            z.writestr("../../never-extract-here.SFC", self.rom.read_bytes())
        library = Library(self.root / "library")
        try:
            game_id = library.import_file(zipped)[0]
            game = library.get(game_id)
            self.assertEqual(game["system"], "snes")
            self.assertEqual(len(library.games(system="snes")), 1)
            self.assertEqual(library.import_file(self.rom)[0], game_id)
            self.assertEqual((library.root / game["rom_path"]).read_bytes(), self.rom.read_bytes())
            self.assertFalse((self.root / "never-extract-here.SFC").exists())
        finally:
            library.close()

    def test_only_one_snes_instance_per_process(self):
        with self.core() as c:
            self.run_frames(c)
            with self.assertRaises(CoreError): self.core()
            video, _ = self.run_frames(c, 5)
            self.assertGreater(len(set(video)), 3)


if __name__ == "__main__":
    unittest.main()
