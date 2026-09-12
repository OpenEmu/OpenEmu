"""Background box art: OpenEmu's OpenVGDB catalog, with a Libretro backup.

ROM hashes are queried locally. Network requests fetch the public catalog,
thumbnail indexes and images; no ROM bytes are uploaded. See the upstream
GameInfoHelper.swift and OpenVGDB.swift for OpenEmu's catalog and query model.
"""
from contextlib import closing
import hashlib
import io
import json
import os
from pathlib import Path
import re
import sqlite3
import tempfile
import time
import unicodedata
from urllib.error import HTTPError
from urllib.parse import quote, urlsplit, urlunsplit
from urllib.request import Request, urlopen
import zipfile

from PySide6.QtCore import QBuffer, QByteArray, QIODevice, QLocale, QSize, Qt, QThread, Signal
from PySide6.QtGui import QImageReader

from . import __version__
from .library import Library, MAX_ROM, SYSTEMS

RELEASES_URL = "https://api.github.com/repos/OpenVGDB/OpenVGDB/releases?page=1&per_page=1"
REPOSITORIES = {
    "gb": "Nintendo_-_Game_Boy", "gbc": "Nintendo_-_Game_Boy_Color",
    "gba": "Nintendo_-_Game_Boy_Advance", "snes": "Nintendo_-_Super_Nintendo_Entertainment_System",
}
MIB = 1024 * 1024


class Cancelled(Exception):
    pass


def https_url(url):
    parts = urlsplit(url)
    if parts.scheme not in ("http", "https") or not parts.hostname or parts.username or parts.password:
        raise ValueError("Invalid artwork URL")
    return urlunsplit(("https", parts.netloc, parts.path, parts.query, ""))


def atomic_bytes(path, data):
    path.parent.mkdir(parents=True, exist_ok=True)
    fd, name = tempfile.mkstemp(prefix=".artwork-", dir=path.parent)
    try:
        with os.fdopen(fd, "wb") as out:
            out.write(data)
        os.replace(name, path)
    finally:
        Path(name).unlink(missing_ok=True)


class Downloads:
    def __init__(self, cancelled=lambda: False):
        self.cancelled = cancelled

    def check(self):
        if self.cancelled():
            raise Cancelled()

    def get(self, url, limit, progress=lambda *_: None):
        self.check()
        request = Request(https_url(url), headers={"User-Agent": f"OpenEmu-Linux/{__version__}",
                                                   "Accept": "*/*"})
        start = time.monotonic()
        try:
            response = urlopen(request, timeout=10)
        except HTTPError as error:
            error.close()
            raise
        with response:
            if urlsplit(response.url).scheme != "https":
                raise ValueError("Insecure artwork redirect")
            total = int(response.headers.get("Content-Length", 0))
            if total > limit:
                raise ValueError("Artwork download is too large")
            data = bytearray()
            while True:
                self.check()
                if time.monotonic() - start > 180:
                    raise TimeoutError("Artwork download timed out")
                chunk = response.read(min(64 * 1024, limit + 1 - len(data)))
                if not chunk:
                    break
                data.extend(chunk)
                if len(data) > limit:
                    raise ValueError("Artwork download is too large")
                progress(len(data), total)
            if total and len(data) != total:
                raise ValueError("Artwork download was interrupted")
        self.check()
        return bytes(data)


def validate_catalog(path):
    with closing(sqlite3.connect(path.as_uri() + "?mode=ro", uri=True)) as db:
        db.execute("PRAGMA trusted_schema=OFF")
        db.execute("""SELECT romHashMD5, romFileName, systemOEID, releaseTitleName,
            releaseCoverFront, regionName FROM ROMs JOIN SYSTEMS USING(systemID)
            JOIN RELEASES USING(romID) LEFT JOIN REGIONS ON regionLocalizedID=REGIONS.regionID LIMIT 1""")
        if db.execute("PRAGMA quick_check").fetchone()[0] != "ok":
            raise ValueError("Invalid OpenVGDB database")


class Catalog:
    def __init__(self, root, downloads):
        self.root = Path(root) / "metadata"
        self.root.mkdir(parents=True, exist_ok=True)
        self.path = self.root / "openvgdb.sqlite"
        self.downloads = downloads

    def ensure(self, progress=lambda _: None):
        state_file = self.root / "openvgdb.json"
        try:
            state = json.loads(state_file.read_text())
        except (OSError, ValueError):
            state = {}
        usable = False
        if self.path.is_file():
            try:
                validate_catalog(self.path)
                usable = True
            except (sqlite3.Error, ValueError):
                pass
        if usable and time.time() - state.get("checked", 0) < 86400:
            return self.path
        try:
            progress("Checking OpenEmu’s game catalog…")
            releases = json.loads(self.downloads.get(RELEASES_URL, 2 * MIB))
            release = releases[0]
            version = release["tag_name"]
            if not usable or version != state.get("version"):
                asset = next(a for a in release["assets"] if a["name"] == "openvgdb.zip")
                archive = self.downloads.get(asset["browser_download_url"], 64 * MIB,
                    lambda done, total: progress(f"Downloading game catalog… {done // MIB} / {total // MIB} MB"
                                                if total else "Downloading game catalog…"))
                with zipfile.ZipFile(io.BytesIO(archive)) as z:
                    info = z.getinfo("openvgdb.sqlite")
                    if info.file_size > 256 * MIB:
                        raise ValueError("OpenVGDB database is too large")
                    # Read only this named entry; never extract paths supplied by the ZIP.
                    with z.open(info) as f:
                        payload = f.read(256 * MIB + 1)
                    if len(payload) > 256 * MIB:
                        raise ValueError("OpenVGDB database is too large")
                self.downloads.check()
                with tempfile.TemporaryDirectory(prefix=".catalog-", dir=self.root) as tmp:
                    candidate = Path(tmp) / "openvgdb.sqlite"
                    candidate.write_bytes(payload)
                    validate_catalog(candidate)
                    self.downloads.check()
                    os.replace(candidate, self.path)
            atomic_bytes(state_file, json.dumps({"version": version, "checked": time.time()}).encode())
        except Cancelled:
            raise
        except Exception:
            if not usable:
                raise
            # An update failure must not disable a previously downloaded catalog.
            state["checked"] = time.time() - 86400 + 3600
            atomic_bytes(state_file, json.dumps(state).encode())
        return self.path


def preferred_region():
    territory = QLocale.system().territory()
    if territory == QLocale.Japan:
        return "Japan"
    if territory in (QLocale.UnitedStates, QLocale.Canada, QLocale.Mexico):
        return "USA"
    return "Europe"


def lookup(db, data, system, region="USA"):
    """Exact ROM identity, including SNES copier-header normalization."""
    if len(data) > MAX_ROM:
        raise ValueError("ROM is too large for artwork lookup")
    variants = [data]
    columns = {row[1] for row in db.execute("PRAGMA table_info(RELEASES)")}
    optional = {"releaseDeveloper": "developer", "releasePublisher": "publisher", "releaseDate": "release_date",
                "releaseGenre": "genre", "releaseDescription": "description", "releaseReferenceURL": "source_url"}
    extra = "".join(f", {column} AS {alias}" for column, alias in optional.items() if column in columns)
    if system == "snes" and len(data) % 32768 == 512:
        variants.insert(0, data[512:])
    for variant in variants:
        md5 = hashlib.md5(variant, usedforsecurity=False).hexdigest().upper()
        rows = db.execute("""SELECT DISTINCT releaseTitleName AS title, releaseCoverFront AS url,
            romFileName AS filename, regionName AS region""" + extra + """ FROM ROMs JOIN SYSTEMS USING(systemID)
            JOIN RELEASES USING(romID) LEFT JOIN REGIONS ON regionLocalizedID=REGIONS.regionID
            WHERE romHashMD5=? AND systemOEID=?""", (md5, SYSTEMS[system].identifier)).fetchall()
        if rows:
            return sorted([dict(row) for row in rows],
                          key=lambda row: (row["region"] != region, row["region"] != "USA", row["title"] or ""))
    return []


def normalized_title(value):
    value = re.sub(r"\([^)]*\)|\[[^]]*\]", "", value)
    value = unicodedata.normalize("NFKD", value.casefold())
    value = "".join(c for c in value if not unicodedata.combining(c))
    value = re.sub(r"[^a-z0-9]+", " ", value).strip()
    words = value.split()
    if words and words[-1] in ("the", "a", "an"):
        words = words[-1:] + words[:-1]
    return " ".join(words)


class BackupArt:
    def __init__(self, root, downloads):
        self.root = Path(root) / "metadata"
        self.downloads = downloads
        self.indexes = {}

    def index(self, system):
        if system in self.indexes:
            return self.indexes[system]
        path = self.root / f"libretro-{system}.json"
        try:
            saved = json.loads(path.read_text())
            names = saved["names"]
            if time.time() - saved["checked"] < 7 * 86400:
                self.indexes[system] = names
                return names
        except (OSError, ValueError, KeyError):
            names = None
        try:
            url = f"https://api.github.com/repos/libretro-thumbnails/{REPOSITORIES[system]}/git/trees/master?recursive=1"
            tree = json.loads(self.downloads.get(url, 16 * MIB))
            if tree.get("truncated"):
                raise ValueError("The backup artwork index is incomplete")
            names = [i["path"].removeprefix("Named_Boxarts/") for i in tree["tree"]
                     if i["type"] == "blob" and i["path"].startswith("Named_Boxarts/") and i["path"].endswith(".png")]
            atomic_bytes(path, json.dumps({"names": names, "checked": time.time()}).encode())
        except Cancelled:
            raise
        except Exception:
            if names is None:
                raise
        self.indexes[system] = names
        return names

    def urls(self, game, matches, region):
        names = self.index(game["system"])
        # Prefer OpenVGDB's verified ROM filename, then its release title. For
        # unknown ROMs only an exact normalized filename/title match is used.
        titles = [Path(m["filename"]).stem for m in matches if m["filename"]]
        titles += [m["title"] for m in matches if m["title"]]
        if not matches:
            titles = [game["title"]]
        results = []
        for title in titles:
            sanitized = re.sub(r'[&*/:`<>?\\|\"]', "_", title)
            exact = [n for n in names if Path(n).stem.casefold() == sanitized.casefold()]
            candidates = exact or [n for n in names if normalized_title(Path(n).stem) == normalized_title(title)]
            candidates.sort(key=lambda n: (f"({region}" not in n, "(World)" not in n, "(USA" not in n, n))
            for name in candidates:
                url = f"https://raw.githubusercontent.com/libretro-thumbnails/{REPOSITORIES[game['system']]}/master/Named_Boxarts/{quote(name, safe='')}"
                if url not in results:
                    results.append(url)
        return results[:3]


def image_png(data):
    """Validate downloaded bytes and bound decoded image memory before saving."""
    buffer = QBuffer()
    buffer.setData(QByteArray(data))
    buffer.open(QIODevice.ReadOnly)
    reader = QImageReader(buffer)
    reader.setAutoTransform(True)
    size = reader.size()
    if not size.isValid() or size.width() * size.height() > 20_000_000:
        raise ValueError("Invalid or oversized cover image")
    if size.width() > 1000 or size.height() > 1000:
        reader.setScaledSize(size.scaled(QSize(1000, 1000), Qt.KeepAspectRatio))
    image = reader.read()
    if image.isNull():
        raise ValueError("The artwork server did not return an image")
    output = QBuffer()
    output.open(QIODevice.WriteOnly)
    if not image.save(output, "PNG"):
        raise ValueError("Could not save cover image")
    return bytes(output.data())


class ArtworkWorker(QThread):
    progress = Signal(str)
    changed = Signal(str)
    result = Signal(dict)

    def __init__(self, root, *, force=False, game_ids=None, replace=False):
        super().__init__()
        self.root, self.force, self.game_ids = root, force, game_ids
        self.replace = replace

    def run(self):
        library = None
        summary = {"downloaded": 0, "not_found": 0, "failed": 0, "cancelled": False, "error": ""}
        try:
            library = Library(self.root)
            games = library.artwork_candidates(self.force, self.game_ids, replace=self.replace)
            if not games:
                return
            downloads = Downloads(self.isInterruptionRequested)
            catalog = Catalog(self.root, downloads)
            catalog_error = None
            try:
                path = catalog.ensure(self.progress.emit)
            except Cancelled:
                raise
            except Exception as error:
                catalog_error = error
                path = None
            fallback = library.setting("artwork_backup", "1") == "1"
            if path is None and not fallback:
                raise catalog_error
            backup = BackupArt(self.root, downloads)
            blocked_hosts = set()
            with closing(sqlite3.connect(path.as_uri() + "?mode=ro", uri=True) if path else sqlite3.connect(":memory:")) as db:
                db.row_factory = sqlite3.Row
                db.execute("PRAGMA trusted_schema=OFF")
                for number, game in enumerate(games, 1):
                    downloads.check()
                    self.progress.emit(f"Finding box art… {number} of {len(games)} · {game['title']}")
                    try:
                        with (library.root / game["rom_path"]).open("rb") as rom:
                            data = rom.read(MAX_ROM + 1)
                        if hashlib.sha256(data).hexdigest() != game["id"]:
                            raise ValueError("The ROM changed since import. Reconnect the original ROM before downloading its cover.")
                        matches = lookup(db, data, game["system"], preferred_region()) if path else []
                        urls = list(dict.fromkeys(m["url"] for m in matches if m["url"]))[:3]
                        image, source, errors = None, None, []
                        for primary in (True, False):
                            if not primary:
                                if not fallback:
                                    break
                                urls = backup.urls(game, matches, preferred_region())
                            for url in urls:
                                downloads.check()
                                host = urlsplit(url).hostname
                                if host in blocked_hosts:
                                    errors.append("The artwork host is temporarily refusing downloads")
                                    continue
                                try:
                                    image = image_png(downloads.get(url, 12 * MIB))
                                    source = https_url(url)
                                    break
                                except Cancelled:
                                    raise
                                except Exception as error:
                                    if isinstance(error, HTTPError) and error.code in (403, 429):
                                        blocked_hosts.add(host)
                                    if isinstance(error, HTTPError):
                                        error.close()
                                    errors.append(str(error))
                            if image is not None:
                                break
                        downloads.check()
                        if image is not None:
                            # Immutable image paths keep a cancelled/racing
                            # replacement from overwriting a visible old cover.
                            target = Path("covers") / f"{game['id']}.{hashlib.sha256(image).hexdigest()[:16]}.download.png"
                            atomic_bytes(library.root / target, image)
                            if library.set_downloaded_cover(game["id"], target, game["cover"], game["cover_revision"]):
                                library.artwork_result(game["id"], "downloaded", url=source)
                                summary["downloaded"] += 1
                                self.changed.emit(game["id"])
                        else:
                            if catalog_error:
                                errors.append(str(catalog_error))
                            status = "failed" if errors else "not_found"
                            message = errors[-1] if errors else "No matching box art found. You can choose a cover image yourself."
                            library.artwork_result(game["id"], status, message=message)
                            summary[status] += 1
                    except Cancelled:
                        raise
                    except Exception as error:
                        library.artwork_result(game["id"], "failed", message=str(error))
                        summary["failed"] += 1
        except Cancelled:
            summary["cancelled"] = True
        except Exception as error:
            summary["error"] = str(error)
        finally:
            if library:
                library.close()
            self.result.emit(summary)
