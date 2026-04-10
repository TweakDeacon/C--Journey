#!/usr/bin/env python3
"""
fandom_downloader.py

Downloads every page on a Fandom wiki and saves them as a JSON file.

Based on the GOLEM-lab/fandom-wiki library approach
(https://github.com/GOLEM-lab/fandom-wiki), with two download modes:

  --fast  (default) Batch API mode: fetches 20 pages per request using the
          MediaWiki TextExtracts API. Best for large wikis with thousands of
          pages. Gives clean plain text directly.

  --slow  Per-page mode: fetches each page's ?action=edit URL and parses the
          WikiText out of the HTML with BeautifulSoup — exactly the technique
          used by fandom_extract.py in the GOLEM-lab library. Then strips the
          WikiText markup to plain text. Slower but extracts the raw source.

Output formats:
  .json       Plain JSON array  [ {"title": "...", "text": "..."}, ... ]
  .jsonl      JSON Lines        one JSON object per line
  .jsonl.gz   Gzip-compressed JSON Lines  (smallest file size)

Dependencies:
    pip install requests beautifulsoup4

Usage:
    python3 fandom_downloader.py <wiki-subdomain> [output-file] [--fast|--slow]

Examples:
    python3 fandom_downloader.py creepypasta
    python3 fandom_downloader.py creepypasta creepypasta.json
    python3 fandom_downloader.py minecraft  minecraft.jsonl.gz  --slow
    python3 fandom_downloader.py starwars   starwars.json       --fast

Reading / searching the output:
    python3 fandom_downloader.py --read creepypasta_wiki.json
    python3 fandom_downloader.py --read creepypasta_wiki.json "slender man"
"""

import sys
import re
import json
import gzip
import time
import os

try:
    import requests
    from bs4 import BeautifulSoup
except ImportError:
    sys.exit(
        "Missing dependencies. Install them with:\n"
        "    pip install requests beautifulsoup4"
    )


# ---------------------------------------------------------------------------
# Shared HTTP helper
# ---------------------------------------------------------------------------

def _make_session() -> requests.Session:
    s = requests.Session()
    s.headers["User-Agent"] = (
        "FandomWikiDownloader/2.0 "
        "(github.com/GOLEM-lab/fandom-wiki inspired; plain-text archiver)"
    )
    return s


def _get(session: requests.Session, url: str, params: dict = None, retries: int = 5):
    for attempt in range(retries):
        try:
            r = session.get(url, params=params or {}, timeout=30)
            r.raise_for_status()
            return r
        except requests.RequestException as exc:
            if attempt == retries - 1:
                raise
            wait = 2 ** attempt
            print(f"  [retry {attempt+1} in {wait}s: {exc}]", file=sys.stderr)
            time.sleep(wait)


# ---------------------------------------------------------------------------
# Page enumeration — MediaWiki allpages API
# ---------------------------------------------------------------------------

def iter_all_titles(wiki: str, namespace: int = 0):
    """Yield every page title in the main namespace."""
    session = _make_session()
    base = f"https://{wiki}.fandom.com/api.php"
    params = {
        "action": "query",
        "list": "allpages",
        "apnamespace": str(namespace),
        "aplimit": "500",
        "format": "json",
        "formatversion": "2",
    }
    while True:
        data = _get(session, base, params).json()
        for page in data["query"]["allpages"]:
            yield page["title"]
        cont = data.get("continue", {})
        if not cont:
            break
        params.update(cont)


# ---------------------------------------------------------------------------
# FAST MODE — TextExtracts API (20 pages per request)
# ---------------------------------------------------------------------------

def fetch_extracts_batch(wiki: str, titles: list[str], session: requests.Session) -> list[dict]:
    """Fetch plain text for up to 20 pages at once via the TextExtracts API."""
    params = {
        "action": "query",
        "prop": "extracts",
        "exlimit": str(len(titles)),
        "explaintext": "1",
        "exsectionformat": "plain",
        "titles": "|".join(titles),
        "redirects": "1",
        "format": "json",
        "formatversion": "2",
    }
    data = _get(session, f"https://{wiki}.fandom.com/api.php", params).json()
    results = []
    for page in data.get("query", {}).get("pages", []):
        title = page.get("title", "")
        text = (page.get("extract") or "").strip()
        if title and text:
            results.append({"title": title, "text": text})
    return results


def download_fast(wiki: str, titles: list[str], out) -> tuple[int, int]:
    """Fast batch mode: uses TextExtracts API, 20 pages per request."""
    session = _make_session()
    total = len(titles)
    written = skipped = 0
    batch_size = 20

    for i in range(0, total, batch_size):
        batch = titles[i: i + batch_size]
        try:
            records = fetch_extracts_batch(wiki, batch, session)
        except Exception as exc:
            print(f"\n  [batch {i}-{i+len(batch)} failed: {exc}]", file=sys.stderr)
            skipped += len(batch)
        else:
            for rec in records:
                _write_record(out, rec)
                written += 1
            skipped += len(batch) - len(records)

        done = min(i + batch_size, total)
        print(f"  {done}/{total}  written={written}  skipped={skipped}", end="\r", flush=True)
        time.sleep(0.05)  # ~20 batches/s max

    return written, skipped


# ---------------------------------------------------------------------------
# SLOW MODE — ?action=edit scraping (GOLEM-lab library technique)
# ---------------------------------------------------------------------------

def fetch_wikitext(wiki: str, title: str, session: requests.Session) -> str | None:
    """
    Fetch WikiText source via the ?action=edit URL.

    This is the exact technique used by the GOLEM-lab/fandom-wiki library
    (download_fandom_data.sh + fandom_extract.py):
      curl "https://wiki.fandom.com/wiki/Page?action=edit" | fandom_extract.py

    fandom_extract.py logic reproduced here:
      - "legacy" layout: WikiText is in <textarea id="wpTextbox1">
      - "modern" layout: WikiText is in a <div role="textbox"> as <p> tags
    """
    url = f"https://{wiki}.fandom.com/wiki/{requests.utils.quote(title, safe='')}?action=edit"
    try:
        resp = _get(session, url)
    except requests.RequestException:
        return None

    soup = BeautifulSoup(resp.text, "html.parser")

    # Legacy layout (fandom_extract._parse_wtsource_legacy)
    text_box = soup.find(id="wpTextbox1")
    if text_box and "dummyTextbox" not in text_box.get("class", []):
        return text_box.string or ""

    # Modern layout (fandom_extract._parse_wtsource_modern)
    textbox = soup.find(role="textbox")
    if textbox:
        return "\n".join(p.get_text() for p in textbox.find_all("p"))

    return None


# Strip WikiText markup to plain text
# Patterns inspired by wikitext_extract.py and wikitext_regex.py (GOLEM-lab)
_WT_PATTERNS = [
    (re.compile(r"<!--.*?-->", re.DOTALL), ""),
    (re.compile(r"<ref[^>]*/\s*>", re.IGNORECASE), ""),
    (re.compile(r"<ref[^>]*>.*?</ref>", re.DOTALL | re.IGNORECASE), ""),
    (re.compile(r"<[^>]+>"), ""),
    (re.compile(r"\[\[(?:File|Image):[^\]]+\]\]", re.IGNORECASE), ""),
    (re.compile(r"\[\[[^\]|]+\|([^\]]+)\]\]"), r"\1"),
    (re.compile(r"\[\[([^\]]+)\]\]"), r"\1"),
    (re.compile(r"'{2,3}"), ""),
    (re.compile(r"^={1,6}\s*(.*?)\s*={1,6}", re.MULTILINE), r"\1"),
    (re.compile(r"^\s*[*#:;]+\s?", re.MULTILINE), ""),
    (re.compile(r"\{\|.*?\|\}", re.DOTALL), ""),
    (re.compile(r"\{\{[^{}]*\}\}"), ""),
    (re.compile(r"\{\{[^{}]*\}\}"), ""),
    (re.compile(r"\{\{[^{}]*\}\}"), ""),
    (re.compile(r"https?://\S+"), ""),
    (re.compile(r"[ \t]+"), " "),
    (re.compile(r"\n{3,}"), "\n\n"),
]


def wikitext_to_plaintext(wt: str) -> str:
    for pat, sub in _WT_PATTERNS:
        wt = pat.sub(sub, wt)
    return wt.strip()


def download_slow(wiki: str, titles: list[str], out) -> tuple[int, int]:
    """Slow per-page mode: scrapes ?action=edit, parses WikiText with BeautifulSoup."""
    session = _make_session()
    total = len(titles)
    written = skipped = 0

    for i, title in enumerate(titles, 1):
        wt = fetch_wikitext(wiki, title, session)
        if wt:
            plain = wikitext_to_plaintext(wt)
            if plain:
                _write_record(out, {"title": title, "text": plain})
                written += 1
            else:
                skipped += 1
        else:
            skipped += 1

        print(f"  {i}/{total}  written={written}  skipped={skipped}", end="\r", flush=True)
        time.sleep(0.2)

    return written, skipped


# ---------------------------------------------------------------------------
# Output helpers — supports .json, .jsonl, .jsonl.gz
# ---------------------------------------------------------------------------

class _JsonArrayWriter:
    """Writes a JSON array incrementally to avoid loading everything into memory."""
    def __init__(self, f):
        self._f = f
        self._first = True
        self._f.write("[\n")

    def write(self, record: dict):
        if not self._first:
            self._f.write(",\n")
        self._f.write(json.dumps(record, ensure_ascii=False))
        self._first = False

    def close(self):
        self._f.write("\n]\n")


def _open_output(path: str):
    """Return (file_handle, writer_object, closer_fn)."""
    if path.endswith(".jsonl.gz"):
        fh = gzip.open(path, "wt", encoding="utf-8")
        return fh, fh, fh.close          # jsonl: write directly
    elif path.endswith(".json"):
        fh = open(path, "w", encoding="utf-8")
        writer = _JsonArrayWriter(fh)
        def close():
            writer.close()
            fh.close()
        return fh, writer, close
    else:  # .jsonl (plain)
        fh = open(path, "w", encoding="utf-8")
        return fh, fh, fh.close


def _write_record(out, record: dict):
    if isinstance(out, _JsonArrayWriter):
        out.write(record)
    else:
        out.write(json.dumps(record, ensure_ascii=False) + "\n")


# ---------------------------------------------------------------------------
# Main download orchestration
# ---------------------------------------------------------------------------

def download_wiki(wiki: str, output_path: str, fast: bool = True):
    mode = "fast (TextExtracts API)" if fast else "slow (?action=edit scraping)"
    print(f"Mode: {mode}")
    print(f"Enumerating pages on {wiki}.fandom.com ...")

    titles = list(iter_all_titles(wiki))
    total = len(titles)
    print(f"Found {total} pages. Downloading ...")

    _fh, out, close_fn = _open_output(output_path)
    try:
        if fast:
            written, skipped = download_fast(wiki, titles, out)
        else:
            written, skipped = download_slow(wiki, titles, out)
    finally:
        close_fn()

    print(f"\nDone. {written} pages saved to: {output_path}")
    if skipped:
        print(f"Skipped {skipped} empty or unparseable pages.")

    size = os.path.getsize(output_path)
    label = f"{size/1024**2:.2f} MB" if size >= 1024**2 else f"{size/1024:.1f} KB"
    print(f"File size: {label}")


# ---------------------------------------------------------------------------
# Reader — browse / search the saved dump
# ---------------------------------------------------------------------------

def read_dump(path: str, search: str = None):
    def _iter():
        if path.endswith(".json"):
            with open(path, encoding="utf-8") as f:
                for rec in json.load(f):
                    yield rec
        elif path.endswith(".jsonl.gz"):
            with gzip.open(path, "rt", encoding="utf-8") as f:
                for line in f:
                    yield json.loads(line)
        else:
            with open(path, encoding="utf-8") as f:
                for line in f:
                    yield json.loads(line)

    count = 0
    for rec in _iter():
        title, text = rec["title"], rec["text"]
        if search is None or search.lower() in title.lower() or search.lower() in text.lower():
            print(f"\n{'='*60}\n  {title}\n{'='*60}")
            print(text[:600] + (" [...]" if len(text) > 600 else ""))
            count += 1
    print(f"\n{count} page(s) matched.")


# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------

if __name__ == "__main__":
    args = sys.argv[1:]

    if args and args[0] == "--read":
        if len(args) < 2:
            sys.exit("Usage: --read <file> [search-term]")
        read_dump(args[1], args[2] if len(args) >= 3 else None)
        sys.exit(0)

    if not args or args[0].startswith("-"):
        print(__doc__)
        sys.exit(1)

    wiki_name = args[0].lower().strip()
    remaining = [a for a in args[1:] if not a.startswith("--")]
    flags     = [a for a in args[1:] if a.startswith("--")]

    fast = "--slow" not in flags  # fast is the default

    if remaining:
        out_file = remaining[0]
    else:
        out_file = f"{wiki_name}_wiki.jsonl.gz"

    download_wiki(wiki_name, out_file, fast=fast)
