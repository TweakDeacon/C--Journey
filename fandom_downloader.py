#!/usr/bin/env python3
"""
fandom_downloader.py

Scrapes every page on a Fandom wiki and saves the text as a compact file.
Works by crawling Special:AllPages to discover every page, then visiting
each one and pulling the article text directly from the HTML — no API key
or authentication needed.

Dependencies:
    pip install requests beautifulsoup4

Usage:
    python3 fandom_downloader.py <wiki-subdomain> [output-file]

Examples:
    python3 fandom_downloader.py analog-horror-0
    python3 fandom_downloader.py creepypasta  creepypasta.jsonl.gz
    python3 fandom_downloader.py minecraft    minecraft.json

Output formats (detected from file extension):
    .jsonl.gz   Gzip-compressed JSON Lines — smallest size (default)
    .jsonl      Plain JSON Lines
    .json       Plain JSON array

Reading the output:
    python3 fandom_downloader.py --read analog-horror-0_wiki.jsonl.gz
    python3 fandom_downloader.py --read analog-horror-0_wiki.jsonl.gz "mandela"
"""

import sys
import re
import json
import gzip
import time
import os

try:
    from bs4 import BeautifulSoup
except ImportError:
    sys.exit("Missing dependencies. Run:  pip install cloudscraper beautifulsoup4")

try:
    import cloudscraper
except ImportError:
    sys.exit("Missing dependency. Run:  pip install cloudscraper")


# ---------------------------------------------------------------------------
# HTTP session — cloudscraper bypasses Cloudflare bot protection
# ---------------------------------------------------------------------------

def _make_session():
    return cloudscraper.create_scraper(browser={"browser": "chrome", "platform": "windows"})


def _fetch(session, url: str, retries: int = 4):
    for attempt in range(retries):
        try:
            r = session.get(url, timeout=30)
            r.raise_for_status()
            return r
        except Exception as exc:
            if attempt == retries - 1:
                return None
            time.sleep(2 ** attempt)
    return None


# ---------------------------------------------------------------------------
# Step 1 — discover every page URL via Special:AllPages
# ---------------------------------------------------------------------------

def get_all_page_urls(wiki: str, session: requests.Session) -> list[str]:
    """
    Scrape the wiki's Local_Sitemap page to get every article URL.
    """
    base = f"https://{wiki}.fandom.com"
    sitemap_url = f"{base}/wiki/Local_Sitemap"

    print(f"  Fetching page list from: {sitemap_url}")
    resp = _fetch(session, sitemap_url)
    if resp is None:
        print(f"  Could not reach {sitemap_url}")
        return []

    soup = BeautifulSoup(resp.text, "html.parser")

    # Skip namespaced pages (Special:, File:, User:, Talk:, etc.)
    _SKIP = re.compile(r"^/wiki/[^:]+:", re.IGNORECASE)

    urls = []
    for a in soup.find_all("a", href=True):
        href = a["href"]
        if href.startswith("/wiki/") and not _SKIP.match(href):
            urls.append(base + href)

    return urls


# ---------------------------------------------------------------------------
# Step 2 — scrape article text from a single page
# ---------------------------------------------------------------------------

# Fandom classes that are clutter, not content
_JUNK_CLASSES = re.compile(
    r"(navbox|toc|infobox|noprint|mw-editsection|"
    r"reference|reflist|thumb|gallery|wikia-menu|"
    r"page-header|global-navigation|fandom-sticky)",
    re.IGNORECASE,
)


def scrape_page(url: str, session: requests.Session) -> dict | None:
    resp = _fetch(session, url)
    if resp is None:
        return None

    soup = BeautifulSoup(resp.text, "html.parser")

    # Article title
    title_tag = (
        soup.find("h1", class_="page-header__title")
        or soup.find("h1", id="firstHeading")
        or soup.find("h1")
    )
    title = title_tag.get_text(strip=True) if title_tag else url.split("/wiki/")[-1].replace("_", " ")

    # Main article body
    content = soup.find("div", class_="mw-parser-output")
    if not content:
        return None

    # Remove junk elements in-place
    for tag in content.find_all(True):
        classes = " ".join(tag.get("class") or [])
        if _JUNK_CLASSES.search(classes):
            tag.decompose()

    # Also remove script / style tags
    for tag in content.find_all(["script", "style", "noscript"]):
        tag.decompose()

    text = content.get_text(separator="\n")
    # Collapse excessive blank lines and whitespace
    text = re.sub(r"[ \t]+", " ", text)
    text = re.sub(r"\n{3,}", "\n\n", text)
    text = text.strip()

    if not text:
        return None

    return {"title": title, "text": text}


# ---------------------------------------------------------------------------
# Output helpers
# ---------------------------------------------------------------------------

class _JsonArrayWriter:
    def __init__(self, f):
        self._f = f
        self._first = True
        self._f.write("[\n")

    def write(self, record):
        if not self._first:
            self._f.write(",\n")
        self._f.write(json.dumps(record, ensure_ascii=False))
        self._first = False

    def close(self):
        self._f.write("\n]\n")


def _open_output(path: str):
    if path.endswith(".jsonl.gz"):
        fh = gzip.open(path, "wt", encoding="utf-8")
        return fh, fh, fh.close
    elif path.endswith(".json"):
        fh = open(path, "w", encoding="utf-8")
        w = _JsonArrayWriter(fh)
        def close():
            w.close(); fh.close()
        return fh, w, close
    else:
        fh = open(path, "w", encoding="utf-8")
        return fh, fh, fh.close


def _write(out, record):
    if isinstance(out, _JsonArrayWriter):
        out.write(record)
    else:
        out.write(json.dumps(record, ensure_ascii=False) + "\n")


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def download_wiki(wiki: str, output_path: str):
    session = _make_session()

    print(f"Finding all pages on {wiki}.fandom.com ...")
    urls = get_all_page_urls(wiki, session)

    if not urls:
        print("\nNo pages found. The wiki may be private or the subdomain may be wrong.")
        print(f"Tried: https://{wiki}.fandom.com/wiki/Local_Sitemap")
        return

    # De-duplicate (Special:AllPages can list the same page twice near boundaries)
    urls = list(dict.fromkeys(urls))
    total = len(urls)
    print(f"Found {total} pages. Scraping text ...")

    _fh, out, close_fn = _open_output(output_path)
    written = skipped = 0

    try:
        for i, url in enumerate(urls, 1):
            record = scrape_page(url, session)
            if record:
                _write(out, record)
                written += 1
            else:
                skipped += 1

            print(f"  {i}/{total}  saved={written}  skipped={skipped}  {url.split('/wiki/')[-1][:40]}",
                  end="\r", flush=True)
            time.sleep(0.15)  # polite rate limit
    finally:
        close_fn()

    print(f"\nDone. {written} pages saved to: {output_path}")
    if skipped:
        print(f"Skipped {skipped} pages (no readable content).")
    size = os.path.getsize(output_path)
    print(f"File size: {size/1024**2:.2f} MB" if size >= 1024**2 else f"File size: {size/1024:.1f} KB")


# ---------------------------------------------------------------------------
# Reader
# ---------------------------------------------------------------------------

def read_dump(path: str, search: str = None):
    def _iter():
        if path.endswith(".jsonl.gz"):
            with gzip.open(path, "rt", encoding="utf-8") as f:
                for line in f:
                    yield json.loads(line)
        elif path.endswith(".json"):
            with open(path, encoding="utf-8") as f:
                for rec in json.load(f):
                    yield rec
        else:
            with open(path, encoding="utf-8") as f:
                for line in f:
                    yield json.loads(line)

    count = 0
    for rec in _iter():
        if search is None or search.lower() in rec["title"].lower() or search.lower() in rec["text"].lower():
            print(f"\n{'='*60}\n  {rec['title']}\n{'='*60}")
            print(rec["text"][:600] + (" [...]" if len(rec["text"]) > 600 else ""))
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

    if not args:
        print(__doc__)
        sys.exit(1)

    wiki_name = args[0].lower().strip()
    out_file = args[1] if len(args) >= 2 else f"{wiki_name}_wiki.jsonl.gz"
    download_wiki(wiki_name, out_file)
