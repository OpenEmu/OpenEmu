"""Typed, instance-based bridge to OpenEmu's original compiled cores."""
import ctypes as C
import hashlib
import json
import os
from pathlib import Path
import tempfile

ROOT = Path(__file__).resolve().parents[1]
CORES = {"gb": "gambatte", "gbc": "gambatte", "gba": "mgba", "snes": "snes9x"}


class CoreError(RuntimeError):
    pass


class Core:
    def __init__(self, rom, system, save_dir, *, libretro_path=None, core_id=None, system_dir=None):
        self.rom = Path(rom).resolve()
        self.name = CORES[system]
        self.path = Path(os.environ.get("OPENEMU_CORE_DIR", ROOT / "build/cores")) / f"liboe_{self.name}.so"
        self.is_libretro = libretro_path is not None
        if self.is_libretro:
            self.path = Path(os.environ.get("OPENEMU_CORE_DIR", ROOT / "build/cores")) / "liboe_libretro.so"
            binary_digest = hashlib.sha256(Path(libretro_path).read_bytes()).hexdigest()
            self.name = f"libretro:{core_id}:{binary_digest}"
        self.handle = None
        if not self.path.is_file():
            raise CoreError(f"The {self.name} core has not been built. Run Linux/build.sh first.")
        self.lib = C.CDLL(str(self.path))
        signatures = {
            "create": (C.c_void_p, [C.c_char_p, C.c_char_p]),
            "destroy": (None, [C.c_void_p]), "reset": (None, [C.c_void_p]),
            "frame": (C.c_int, [C.c_void_p, C.c_uint]),
            "pixels": (C.c_void_p, [C.c_void_p]), "audio": (C.c_void_p, [C.c_void_p]),
            "width": (C.c_uint, [C.c_void_p]), "height": (C.c_uint, [C.c_void_p]),
            "fps": (C.c_double, [C.c_void_p]), "sample_rate": (C.c_uint, [C.c_void_p]),
            "save_state": (C.c_int, [C.c_void_p, C.c_char_p]),
            "load_state": (C.c_int, [C.c_void_p, C.c_char_p]),
            "flush": (None, [C.c_void_p]),
        }
        for name, (result, args) in signatures.items():
            fn = getattr(self.lib, "oe_" + name)
            fn.restype, fn.argtypes = result, args
        Path(save_dir).mkdir(parents=True, exist_ok=True)
        self.digest = hashlib.sha256(self.rom.read_bytes()).hexdigest()
        if self.is_libretro:
            self.lib.oe_libretro_create.restype = C.c_void_p
            self.lib.oe_libretro_create.argtypes = [C.c_char_p] * 4
            self.lib.oe_error.restype = C.c_char_p
            self.lib.oe_error.argtypes = []
            system_dir = Path(system_dir or Path(save_dir) / "system")
            system_dir.mkdir(parents=True, exist_ok=True)
            self.handle = self.lib.oe_libretro_create(os.fsencode(libretro_path), os.fsencode(self.rom),
                                                      os.fsencode(save_dir), os.fsencode(system_dir))
        else:
            self.handle = self.lib.oe_create(os.fsencode(self.rom), os.fsencode(save_dir))
        if not self.handle:
            raise CoreError(self.error_message(f"{self.name} could not load {self.rom.name}. Check that the ROM is valid."))
        self.width = self.lib.oe_width(self.handle)
        self.height = self.lib.oe_height(self.handle)
        self.fps = self.lib.oe_fps(self.handle)
        self.sample_rate = self.lib.oe_sample_rate(self.handle)

    def frame(self, keys=0):
        if not self.handle:
            raise CoreError("The game has already stopped.")
        n = self.lib.oe_frame(self.handle, keys)
        if n < 0 or n > 8192:
            raise CoreError(self.error_message("The emulator did not complete a frame."))
        self.width = self.lib.oe_width(self.handle)
        self.height = self.lib.oe_height(self.handle)
        self.fps = self.lib.oe_fps(self.handle)
        self.sample_rate = self.lib.oe_sample_rate(self.handle)
        if not (0 < self.width <= 1024 and 0 < self.height <= 1024):
            raise CoreError("The emulator returned invalid frame dimensions.")
        return (C.string_at(self.lib.oe_pixels(self.handle), self.width * self.height * 4),
                C.string_at(self.lib.oe_audio(self.handle), n * 4))

    def error_message(self, default):
        if self.is_libretro:
            return (self.lib.oe_error() or default.encode()).decode(errors="replace")
        return default

    def flush(self):
        self.lib.oe_flush(self.handle)

    def save_state(self, path):
        """Atomically publish an integrity-checked state, keyed to ROM and core."""
        path = Path(path)
        path.parent.mkdir(parents=True, exist_ok=True)
        with tempfile.TemporaryDirectory(dir=path.parent) as tmp:
            raw = Path(tmp) / "raw.state"
            if not self.lib.oe_save_state(self.handle, os.fsencode(raw)):
                raise CoreError("Could not save the game state.")
            data = raw.read_bytes()
            header = json.dumps({"format": 1, "core": self.name, "rom": self.digest,
                                 "sha256": hashlib.sha256(data).hexdigest()}).encode()
            output = Path(tmp) / "state"
            with output.open("wb") as f:
                f.write(b"OELINUX1\n" + header + b"\n" + data)
                f.flush()
                os.fsync(f.fileno())
            os.replace(output, path)

    def load_state(self, path):
        path = Path(path)
        if not path.is_file():
            raise CoreError("There is no saved state in this slot yet.")
        if path.stat().st_size > 32 * 1024 * 1024:
            raise CoreError("The saved state is too large.")
        try:
            magic, header, raw = path.read_bytes().split(b"\n", 2)
            meta = json.loads(header)
            valid = (magic == b"OELINUX1" and meta["format"] == 1 and meta["core"] == self.name
                     and meta["rom"] == self.digest and meta["sha256"] == hashlib.sha256(raw).hexdigest())
        except (ValueError, KeyError, TypeError):
            valid = False
        if not valid:
            raise CoreError("This state is damaged or belongs to a different game or core.")
        with tempfile.TemporaryDirectory() as tmp:
            state = Path(tmp) / "state"
            state.write_bytes(raw)
            if not self.lib.oe_load_state(self.handle, os.fsencode(state)):
                raise CoreError("The emulator could not restore this state.")

    def reset(self):
        self.lib.oe_reset(self.handle)

    def close(self):
        if self.handle:
            self.lib.oe_destroy(self.handle)
            self.handle = None

    def __enter__(self):
        return self

    def __exit__(self, *_):
        self.close()
