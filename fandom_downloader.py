#!/usr/bin/env python3
"""
fandom_downloader.py

Downloads every page on a Fandom wiki as clean plain text and saves the result
as a gzip-compressed JSON Lines file (.jsonl.gz) for a small footprint.

Based on the approach from the GOLEM-lab/fandom-wiki library
(https://github.com/GOLEM-lab/fandom-wiki):
  - Fetches the WikiText source via each page's `?action=edit` URL
  - Parses it from the HTML with BeautifulSoup (same technique as fandom_extract.py)
  - Strips WikiText markup to produce clean plain text

Additionally adds:
  - Automatic enumeration of ALL pages via the MediaWiki API
  - WikiText → plain-text conversion
  - gzip compression for a compact output file

Dependencies (install once):
    pip install requests beautifulsoup4

Usage:
    python3 fandom_downloader.py <wiki-subdomain> [output-file]

Examples:
    python3 fandom_downloader.py minecraft
    python3 fandom_downloader.py starwars  starwars.jsonl.gz
    python3 fandom_downloader.py harrypotter hp_wiki.jsonl.gz

Reading the output later:
    python3 fandom_downloader.py --read minecraft_wiki.jsonl.gz
    python3 fandom_downloader.py --read minecraft_wiki.jsonl.gz "creeper"
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
# Page enumeration via the MediaWiki API
# ---------------------------------------------------------------------------

def iter_all_titles(wiki: str, namespace: int = 0):
    """Yield every page title in the given namespace via the allpages API."""
    session = requests.Session()
    session.headers["User-Agent"] = "FandomWikiDownloader/2.0 (github.com/GOLEM-lab/fandom-wiki inspired)"

    params = {
        "action": "query",
        "list": "allpages",
        "apnamespace": str(namespace),
        "aplimit": "500",
        "format": "json",
        "formatversion": "2",
    }
    base = f"https://{wiki}.fandom.com/api.php"

    while True:
        resp = _get_with_retry(session, base, params)
        data = resp.json()
        for page in data["query"]["allpages"]:
            yield page["title"]
        cont = data.get("continue", {})
        if not cont:
            break
        params.update(cont)


def _get_with_retry(session, url, params, retries=5):
    for attempt in range(retries):
        try:
            r = session.get(url, params=params, timeout=30)
            r.raise_for_status()
            return r
        except requests.RequestException as exc:
            if attempt == retries - 1:
                raise
            wait = 2 ** attempt
            print(f"  [retry {attempt+1} in {wait}s: {exc}]", file=sys.stderr)
            time.sleep(wait)


# ---------------------------------------------------------------------------
# WikiText extraction from the edit-page HTML
# Adapted from: src/fandom_extraction/fandom_extract.py (GOLEM-lab/fandom-wiki)
# ---------------------------------------------------------------------------

def fetch_wikitext(wiki: str, title: str, session: requests.Session) -> str | None:
    """
    Fetch the WikiText source for a page using the ?action=edit URL.

    This is the same technique used by the GOLEM-lab library's
    download_fandom_data.sh + fandom_extract.py:
      curl "https://wiki.fandom.com/wiki/PageTitle?action=edit" | fandom_extract.py
    """
    url = f"https://{wiki}.fandom.com/wiki/{requests.utils.quote(title, safe='')}?action=edit"
    try:
        resp = _get_with_retry(session, url, params={})
    except requests.RequestException:
        return None

    soup = BeautifulSoup(resp.text, "html.parser")

    # Detect page layout (logic from fandom_extract.py: detect_pagetype_from_html)
    text_box = soup.find(id="wpTextbox1")
    if text_box and "dummyTextbox" not in text_box.get("class", []):
        # Legacy layout: WikiText is directly in the textarea as a string
        return text_box.string or ""
    else:
        # Modern layout: WikiText is in a contenteditable div (role="textbox")
        textbox = soup.find(role="textbox")
        if textbox is None:
            return None
        paragraphs = textbox.find_all("p")
        return "\n".join(p.get_text() for p in paragraphs)


# ---------------------------------------------------------------------------
# WikiText → plain text
# Strips markup so the saved text is compact and human-readable.
# Uses patterns similar to those in wikitext_extract.py (GOLEM-lab/fandom-wiki)
# ---------------------------------------------------------------------------

# Compiled patterns (order matters)
_PATTERNS = [
    (re.compile(r"<!--.*?-->", re.DOTALL), ""),                    # HTML comments
    (re.compile(r"<ref[^>]*/\s*>", re.IGNORECASE), ""),           # self-closing <ref />
    (re.compile(r"<ref[^>]*>.*?</ref>", re.DOTALL | re.IGNORECASE), ""),  # <ref>...</ref>
    (re.compile(r"<[^>]+>"), ""),                                  # remaining HTML tags
    (re.compile(r"\[\[(?:File|Image):[^\]]+\]\]", re.IGNORECASE), ""),  # images/files
    (re.compile(r"\[\[[^\]|]+\|([^\]]+)\]\]"), r"\1"),             # [[link|display]] → display
    (re.compile(r"\[\[([^\]]+)\]\]"), r"\1"),                      # [[link]] → link
    (re.compile(r"'{2,3}"), ""),                                   # bold/italic markers
    (re.compile(r"^={1,6}\s*(.*?)\s*={1,6}", re.MULTILINE), r"\1"),  # == headings ==
    (re.compile(r"^\s*[*#:;]+\s?", re.MULTILINE), ""),            # list/indent markers
    (re.compile(r"\{\|.*?\|\}", re.DOTALL), ""),                   # wiki tables
    (re.compile(r"\{\{[^{}]*\}\}"), ""),                           # {{templates}} (single-pass)
    (re.compile(r"\{\{[^{}]*\}\}"), ""),                           # second pass (nested)
    (re.compile(r"\{\{[^{}]*\}\}"), ""),                           # third pass (deeper nesting)
    (re.compile(r"\{[^{}]*\}"), ""),                               # leftover { }
    (re.compile(r"https?://\S+"), ""),                             # bare URLs
    (re.compile(r"[ \t]+"), " "),                                  # collapse spaces
    (re.compile(r"\n{3,}"), "\n\n"),                               # collapse blank lines
]


def wikitext_to_plaintext(wikitext: str) -> str:
    text = wikitext
    for pattern, replacement in _PATTERNS:
        text = pattern.sub(replacement, text)
    return text.strip()


# ---------------------------------------------------------------------------
# Main download logic
# ---------------------------------------------------------------------------

def download_wiki(wiki: str, output_path: str):
    print(f"Enumerating pages on {wiki}.fandom.com ...")
    titles = list(iter_all_titles(wiki))
    total = len(titles)
    print(f"Found {total} pages. Downloading WikiText and converting to plain text ...")

    session = requests.Session()
    session.headers["User-Agent"] = "FandomWikiDownloader/2.0 (github.com/GOLEM-lab/fandom-wiki inspired)"

    written = 0
    skipped = 0

    with gzip.open(output_path, "wt", encoding="utf-8") as out:
        for i, title in enumerate(titles, 1):
            wikitext = fetch_wikitext(wiki, title, session)
            if not wikitext or not wikitext.strip():
                skipped += 1
            else:
                plain = wikitext_to_plaintext(wikitext)
                if plain:
                    record = {"title": title, "text": plain}
                    out.write(json.dumps(record, ensure_ascii=False) + "\n")
                    written += 1
                else:
                    skipped += 1

            print(f"  {i}/{total}  written={written}  skipped={skipped}", end="\r", flush=True)

            # Polite rate-limit: ~1 request / 0.2 s
            time.sleep(0.2)

    print(f"\nDone. {written} pages saved to: {output_path}")
    if skipped:
        print(f"Skipped {skipped} empty/unparseable pages.")

    size = os.path.getsize(output_path)
    print(f"File size: {size / 1024:.1f} KB" if size < 1024 ** 2 else f"File size: {size / 1024 ** 2:.2f} MB")


# ---------------------------------------------------------------------------
# Reader helper: search/browse the saved dump
# ---------------------------------------------------------------------------

def read_dump(path: str, search: str = None):
    """Print pages from the dump, optionally filtering by a search term."""
    count = 0
    with gzip.open(path, "rt", encoding="utf-8") as f:
        for line in f:
            record = json.loads(line)
            title = record["title"]
            text = record["text"]
            if search is None or search.lower() in title.lower() or search.lower() in text.lower():
                print(f"\n{'='*60}")
                print(f"  {title}")
                print('='*60)
                # Show first 600 chars to keep output readable
                print(text[:600] + (" [...]" if len(text) > 600 else ""))
                count += 1
    print(f"\n{count} page(s) matched.")


# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------

if __name__ == "__main__":
    args = sys.argv[1:]

    # Read mode: --read <file> [search-term]
    if args and args[0] == "--read":
        if len(args) < 2:
            sys.exit("Usage: python3 fandom_downloader.py --read <file.jsonl.gz> [search-term]")
        read_dump(args[1], args[2] if len(args) >= 3 else None)
        sys.exit(0)

    if not args:
        print(__doc__)
        sys.exit(1)

    wiki_name = args[0].lower().strip()
    out_file = args[1] if len(args) >= 2 else f"{wiki_name}_wiki.jsonl.gz"

    download_wiki(wiki_name, out_file)
