"""Install official Linux Libretro builds without modifying RetroArch's files."""
import argparse
import ctypes as C
from contextlib import contextmanager
import fcntl
import hashlib
import io
import json
import os
from pathlib import Path
import platform
import re
import subprocess
import sys
import tempfile
import time
import zipfile

from PySide6.QtCore import QThread, Signal
from .artwork import Downloads, Cancelled, atomic_bytes, MIB
from .core import CoreError, ROOT
from .library import EXTENSIONS

BUILDBOT = "https://buildbot.libretro.com/nightly/linux/x86_64/latest/"
CATALOG = {
    "gambatte": {"name": "Gambatte", "systems": ["gb", "gbc"]},
    "sameboy": {"name": "SameBoy", "systems": ["gb", "gbc"]},
    "mgba": {"name": "mGBA", "systems": ["gb", "gbc", "gba"]},
    "snes9x": {"name": "Snes9x", "systems": ["snes"]},
    "snes9x2010": {"name": "Snes9x 2010", "systems": ["snes"]},
    "bsnes": {"name": "bsnes", "systems": ["snes"]},
}


def probe(path):
    """Load metadata in a disposable child so a bad .so cannot crash Settings."""
    process = subprocess.run([sys.executable, "-m", "openemu_linux.core_manager", "--probe", str(path)],
                             cwd=ROOT, capture_output=True, timeout=20)
    for line in process.stdout.decode(errors="replace").splitlines():
        if process.returncode == 0 and line.startswith("OPENEMU_CORE_INFO "):
            return json.loads(line.removeprefix("OPENEMU_CORE_INFO "))
    raise CoreError("This core could not be loaded on this Linux system. " + process.stderr.decode(errors="replace")[-600:])


def validate_elf(data):
    machine = {"x86_64": 62, "amd64": 62, "aarch64": 183, "arm64": 183}.get(platform.machine().lower())
    if len(data) < 64 or data[:6] != b"\x7fELF\x02\x01" or int.from_bytes(data[18:20], "little") != machine:
        raise CoreError("Choose a 64-bit Linux core built for this computer’s processor.")


class CoreManager:
    def __init__(self, root):
        self.root = Path(root).resolve()
        self.directory = self.root / "cores"
        self.directory.mkdir(parents=True, exist_ok=True)
        self.manifest = self.directory / "installed.json"

    def installed(self):
        if not self.manifest.exists():
            return {}
        records = json.loads(self.manifest.read_text())
        if not isinstance(records, dict):
            raise CoreError("The installed core list is damaged.")
        return records

    def path(self, record):
        path = (self.root / record["path"]).resolve()
        if not path.is_relative_to(self.directory) or not path.is_file():
            raise CoreError("The selected core is missing. Download it again in Settings.")
        if hashlib.sha256(path.read_bytes()).hexdigest() != record["sha256"]:
            raise CoreError("The selected core file is damaged. Download it again in Settings.")
        return path

    @contextmanager
    def locked(self):
        with (self.directory / ".install.lock").open("a") as lock:
            fcntl.flock(lock, fcntl.LOCK_EX)
            yield

    def install_bytes(self, data, core_id, *, source="local", build=None, crc=None, systems=None):
        if len(data) > 128 * MIB:
            raise CoreError("The core file is too large.")
        if not re.fullmatch(r"[a-z0-9_-]+", core_id):
            raise CoreError("Invalid core identifier")
        validate_elf(data)
        digest = hashlib.sha256(data).hexdigest()
        with tempfile.TemporaryDirectory(prefix=".install-", dir=self.directory) as tmp:
            candidate = Path(tmp) / "core.so"
            candidate.write_bytes(data)
            info = probe(candidate)
            supported = sorted({EXTENSIONS["." + ext] for ext in info["extensions"].lower().split("|") if "." + ext in EXTENSIONS})
            if "gb" in supported and "gbc" not in supported:
                supported.append("gbc")
            if systems:
                supported = [s for s in systems if s in supported]
            if not supported:
                raise CoreError("This core does not advertise support for GB, GBC, GBA or SNES ROMs.")
            target = self.directory / core_id / (digest + ".so")
            record = {"id": core_id, "name": info["name"], "version": info["version"], "systems": supported,
                      "path": str(target.relative_to(self.root)), "sha256": digest, "source": source,
                      "build": build, "crc": crc, "installed": time.time()}
            with self.locked():
                target.parent.mkdir(parents=True, exist_ok=True)
                # Immutable builds let running games finish on their current version.
                if not target.exists() or hashlib.sha256(target.read_bytes()).hexdigest() != digest:
                    os.replace(candidate, target)
                records = self.installed()
                records[core_id] = record
                atomic_bytes(self.manifest, json.dumps(records, indent=2).encode())
        return record

    def import_file(self, path):
        path = Path(path)
        with path.open("rb") as file:
            data = file.read(128 * MIB + 1)
        digest = hashlib.sha256(data).hexdigest()
        return self.install_bytes(data, "local-" + digest[:16])

    def remote_index(self, downloads):
        if platform.machine().lower() not in ("x86_64", "amd64"):
            raise CoreError("Automatic downloads currently support Linux x86_64. You can import a native .so core on this processor.")
        text = downloads.get(BUILDBOT + ".index-extended", 4 * MIB).decode()
        entries = {}
        for line in text.splitlines():
            parts = line.split()
            if len(parts) == 3:
                build, crc, filename = parts
                if re.fullmatch(r"[0-9a-fA-F]{8}", crc):
                    entries[filename] = {"build": build, "crc": crc.lower()}
        return entries

    def download(self, core_id, downloads, progress=lambda _: None, index=None):
        if core_id not in CATALOG:
            raise CoreError("This core is not in the download catalog.")
        index = self.remote_index(downloads) if index is None else index
        filename = core_id + "_libretro.so"
        remote = index.get(filename + ".zip")
        if not remote:
            raise CoreError("This core is temporarily unavailable from the official build server.")
        current = self.installed().get(core_id)
        if current and current.get("crc") == remote["crc"]:
            try:
                if hashlib.sha256(self.path(current).read_bytes()).hexdigest() == current["sha256"]:
                    return current, False
            except CoreError:
                pass
        progress(f"Downloading {CATALOG[core_id]['name']}…")
        data = downloads.get(BUILDBOT + filename + ".zip", 64 * MIB)
        with zipfile.ZipFile(io.BytesIO(data)) as archive:
            entry = archive.getinfo(filename)
            if entry.file_size > 128 * MIB or f"{entry.CRC:08x}" != remote["crc"]:
                raise CoreError("The downloaded core does not match the server’s index. Try updating again.")
            with archive.open(entry) as file:
                binary = file.read(128 * MIB + 1)
        downloads.check()
        progress(f"Checking and installing {CATALOG[core_id]['name']}…")
        record = self.install_bytes(binary, core_id, source="buildbot", systems=CATALOG[core_id]["systems"], **remote)
        return record, True

    def selection(self, library, system):
        choice = library.setting("core." + system, "builtin")
        if choice == "builtin":
            return None
        record = self.installed().get(choice)
        if not record or system not in record["systems"]:
            raise CoreError("The selected core is unavailable. Select another core in Settings.")
        self.path(record)
        return record


class CoreWorker(QThread):
    progress = Signal(str)
    result = Signal(str, bool)

    def __init__(self, root, *, core_id=None, local_file=None, update_all=False):
        super().__init__()
        self.root, self.core_id, self.local_file, self.update_all = root, core_id, local_file, update_all

    def run(self):
        try:
            manager = CoreManager(self.root)
            downloads = Downloads(self.isInterruptionRequested)
            if self.local_file:
                self.progress.emit("Checking the selected core…")
                manager.import_file(self.local_file)
                message = "Core imported. You can now select it for a system."
            else:
                index = manager.remote_index(downloads)
                ids = [key for key, value in manager.installed().items() if value["source"] == "buildbot"] if self.update_all else [self.core_id]
                changed, failures = 0, []
                for core_id in ids:
                    downloads.check()
                    try:
                        _, updated = manager.download(core_id, downloads, self.progress.emit, index)
                        changed += int(updated)
                    except Cancelled:
                        raise
                    except Exception as error:
                        failures.append(f"{core_id}: {error}")
                message = f"{changed} core{'s' if changed != 1 else ''} installed or updated." if changed else "Installed cores are up to date."
                if failures:
                    self.result.emit(message + "\n" + "\n".join(failures), False)
                    return
            self.result.emit(message, True)
        except Cancelled:
            self.result.emit("Core download cancelled.", True)
        except Exception as error:
            self.result.emit(str(error), False)


def probe_child(path):
    class Info(C.Structure):
        _fields_ = [("name", C.c_char_p), ("version", C.c_char_p), ("extensions", C.c_char_p),
                    ("need_fullpath", C.c_bool), ("block_extract", C.c_bool)]
    core = C.CDLL(str(Path(path).resolve()))
    required = ["init", "deinit", "api_version", "get_system_info", "get_system_av_info", "set_environment",
                "set_video_refresh", "set_audio_sample", "set_audio_sample_batch", "set_input_poll", "set_input_state",
                "set_controller_port_device", "load_game", "unload_game", "run", "reset", "serialize_size",
                "serialize", "unserialize", "get_memory_data", "get_memory_size"]
    for name in required:
        getattr(core, "retro_" + name)
    core.retro_api_version.restype = C.c_uint
    if core.retro_api_version() != 1:
        raise CoreError("Unsupported Libretro API version")
    core.retro_get_system_info.argtypes = [C.POINTER(Info)]
    core.retro_get_system_info.restype = None
    info = Info()
    core.retro_get_system_info(C.byref(info))
    decode = lambda value: (value or b"").decode(errors="replace")
    print("OPENEMU_CORE_INFO " + json.dumps({"name": decode(info.name), "version": decode(info.version),
                                            "extensions": decode(info.extensions)}), flush=True)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--probe", required=True)
    args = parser.parse_args()
    probe_child(args.probe)
