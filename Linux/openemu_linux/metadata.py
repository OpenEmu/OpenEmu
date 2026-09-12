"""Replaceable metadata providers and background library enrichment.

Provider results contain ordinary library fields, not remote database schema.
ROM matching is local; no ROM data is uploaded.
"""
from dataclasses import dataclass
import hashlib
from html.parser import HTMLParser
import re
import sqlite3
from typing import Protocol
import zlib

from PySide6.QtCore import QThread, Signal

from .artwork import Catalog, Downloads, Cancelled, lookup, preferred_region
from .library import Library, MAX_ROM


@dataclass(frozen=True)
class MetadataMatch:
    fields: dict
    source_url: str = ""


class MetadataProvider(Protocol):
    identifier: str

    def prepare(self, progress): ...
    def identify(self, data: bytes, system: str, region: str) -> MetadataMatch | None: ...
    def close(self): ...


class PlainDescription(HTMLParser):
    def __init__(self):
        super().__init__(convert_charrefs=True)
        self.parts = []
        self.ignored = 0

    def handle_starttag(self, tag, attrs):
        if tag in ("script", "style"):
            self.ignored += 1
        if tag in ("p", "br", "div", "li"):
            self.parts.append("\n")

    def handle_endtag(self, tag):
        if tag in ("script", "style"):
            self.ignored = max(0, self.ignored - 1)
        if tag in ("p", "div", "li"):
            self.parts.append("\n")

    def handle_data(self, data):
        if not self.ignored:
            self.parts.append(data)


def plain_text(value):
    parser = PlainDescription()
    parser.feed(str(value or "")[:50000])
    return re.sub(r"\n{3,}", "\n\n", "".join(parser.parts)).strip()[:12000]


class OpenVGDBProvider:
    identifier = "OpenVGDB"

    def __init__(self, root, downloads):
        self.catalog = Catalog(root, downloads)
        self.db = None

    def prepare(self, progress):
        path = self.catalog.ensure(progress)
        self.db = sqlite3.connect(path.as_uri() + "?mode=ro", uri=True)
        self.db.row_factory = sqlite3.Row
        self.db.execute("PRAGMA trusted_schema=OFF")

    def identify(self, data, system, region):
        matches = lookup(self.db, data, system, region)
        if not matches:
            return None
        match = matches[0]
        fields = {key: str(match[key]).strip() for key in
                  ("title", "region", "developer", "publisher", "release_date", "genre") if match.get(key)}
        if match.get("description"):
            fields["description"] = plain_text(match["description"])
        if fields.get("genre"):
            fields["genre"] = ", ".join(part.strip() for part in fields["genre"].split(",") if part.strip())
        # OpenVGDB has no player-count field; never infer one from its prose.
        return MetadataMatch(fields, match.get("source_url") or "")

    def close(self):
        if self.db:
            self.db.close()
            self.db = None


class MetadataWorker(QThread):
    progress = Signal(str)
    changed = Signal(str)
    result = Signal(dict)

    def __init__(self, root, *, force=False, game_ids=None, provider_factory=OpenVGDBProvider):
        super().__init__()
        self.root, self.force, self.game_ids = root, force, game_ids
        self.provider_factory = provider_factory

    def run(self):
        library, provider = None, None
        summary = {"matched": 0, "not_found": 0, "failed": 0, "cancelled": False, "error": ""}
        try:
            library = Library(self.root)
            games = library.metadata_candidates(self.force, self.game_ids)
            if not games:
                return
            downloads = Downloads(self.isInterruptionRequested)
            provider = self.provider_factory(self.root, downloads)
            try:
                provider.prepare(self.progress.emit)
            except Cancelled:
                raise
            except Exception as error:
                # Retry backoff applies to provider failures too, so an offline
                # startup does not trigger a request on every library refresh.
                for game in games:
                    library.metadata_result(game["id"], provider.identifier, "failed", message=str(error))
                raise
            for number, game in enumerate(games, 1):
                downloads.check()
                self.progress.emit(f"Finding game information… {number} of {len(games)} · {game['title']}")
                try:
                    with (library.root / game["rom_path"]).open("rb") as rom:
                        data = rom.read(MAX_ROM + 1)
                    if hashlib.sha256(data).hexdigest() != game["id"]:
                        raise ValueError("The ROM changed since import. Locate the original ROM before identifying it.")
                    hashes = {"md5": hashlib.md5(data, usedforsecurity=False).hexdigest(),
                              "sha1": hashlib.sha1(data, usedforsecurity=False).hexdigest(),
                              "crc32": f"{zlib.crc32(data):08x}"}
                    match = provider.identify(data, game["system"], preferred_region())
                    downloads.check()
                    status = "matched" if match else "not_found"
                    if library.metadata_result(game["id"], provider.identifier, status,
                            payload=match.fields if match else None, source_url=match.source_url if match else None,
                            hashes=hashes):
                        summary[status] += 1
                        self.changed.emit(game["id"])
                except Cancelled:
                    raise
                except Exception as error:
                    library.metadata_result(game["id"], provider.identifier, "failed", message=str(error))
                    summary["failed"] += 1
        except Cancelled:
            summary["cancelled"] = True
        except Exception as error:
            summary["error"] = str(error)
        finally:
            if provider:
                try:
                    provider.close()
                except Exception as error:
                    summary["error"] = summary["error"] or str(error)
            if library:
                library.close()
            self.result.emit(summary)
