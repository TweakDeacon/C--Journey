#!/usr/bin/env python3
"""
Web Scraper Pipeline
====================
Stage 1  URL Discovery      – sitemap.xml + recursive link crawl
Stage 2  Network Inspection – intercept XHR/fetch calls via Playwright
Stage 3  Raw HTML Parsing   – BeautifulSoup content extraction
Stage 4  Playwright Render  – full JS headless-Chromium rendering
Stage 5  Markdown + ZIP     – per-page .md files bundled in a .zip
                              (--docx flag also produces a combined .docx)
"""

import argparse
import json
import os
import re
import sys
import time
import zipfile
from collections import deque
from pathlib import Path
from urllib.parse import urljoin, urlparse, urlunparse
from xml.etree import ElementTree

import requests
from bs4 import BeautifulSoup, NavigableString, Tag
from docx import Document
from docx.shared import Pt, RGBColor
from markdownify import markdownify as md_convert
from playwright.sync_api import sync_playwright


# ── Shared helpers ────────────────────────────────────────────────────────────

NOISE_TAGS = {"script", "style", "noscript", "svg", "meta", "link",
              "nav", "footer", "aside", "head"}


def _find_chromium() -> str | None:
    """Auto-detect the newest Playwright Chromium binary in the local cache."""
    cache = Path.home() / ".cache" / "ms-playwright"
    for pattern in ("chromium*/chrome-linux/chrome", "chromium*/chrome"):
        matches = sorted(cache.glob(pattern))
        if matches:
            return str(matches[-1])
    return None


def _chromium_launch_kwargs() -> dict:
    """Return kwargs for pw.chromium.launch(), honouring env-var override."""
    exe = os.environ.get("PLAYWRIGHT_CHROMIUM_EXECUTABLE_PATH") or _find_chromium()
    kw: dict = {"headless": True}
    if exe:
        kw["executable_path"] = exe
    return kw


def _clean(text: str) -> str:
    return re.sub(r"\s+", " ", text).strip()


def _same_origin(base: str, url: str) -> bool:
    b, u = urlparse(base), urlparse(url)
    return b.scheme == u.scheme and b.netloc == u.netloc


def _normalise(url: str) -> str:
    """Strip fragment and trailing slash for deduplication."""
    p = urlparse(url)
    path = p.path.rstrip("/") or "/"
    return urlunparse((p.scheme, p.netloc, path, p.params, p.query, ""))


def _page_slug(url: str) -> str:
    """Convert a URL to a safe filename stem."""
    p = urlparse(url)
    path = p.path.strip("/").replace("/", "__") or "index"
    slug = re.sub(r"[^\w\-]", "_", path)
    return (slug or "index")[:80]


# ── Stage 1 – URL Discovery ───────────────────────────────────────────────────

class URLDiscovery:
    """
    Builds the list of URLs to scrape via two strategies (tried in order):
      a) sitemap.xml / sitemap index parsing
      b) BFS link crawl from the root URL up to `depth` hops
    """

    def __init__(
        self,
        root: str,
        depth: int = 1,
        max_pages: int = 50,
        session: requests.Session | None = None,
    ):
        self.root = root
        self.depth = depth
        self.max_pages = max_pages
        self.session = session or requests.Session()
        self.session.headers["User-Agent"] = (
            "Mozilla/5.0 (compatible; WebScraperBot/1.0)"
        )

    # ── internal ──────────────────────────────────────────────────────────────

    def _get(self, url: str) -> str | None:
        try:
            r = self.session.get(url, timeout=10)
            r.raise_for_status()
            return r.text
        except Exception:
            return None

    def _sitemap_urls(self) -> list[str]:
        base = f"{urlparse(self.root).scheme}://{urlparse(self.root).netloc}"
        urls: list[str] = []
        ns = {"sm": "http://www.sitemaps.org/schemas/sitemap/0.9"}

        for candidate in (f"{base}/sitemap.xml", f"{base}/sitemap_index.xml"):
            xml = self._get(candidate)
            if not xml:
                continue
            try:
                root_el = ElementTree.fromstring(xml)
            except ElementTree.ParseError:
                continue

            for loc in root_el.findall(".//sm:loc", ns):
                text = (loc.text or "").strip()
                if text.endswith(".xml"):
                    sub = self._get(text)
                    if sub:
                        try:
                            for subloc in ElementTree.fromstring(sub).findall(".//sm:loc", ns):
                                u = (subloc.text or "").strip()
                                if u and _same_origin(self.root, u):
                                    urls.append(u)
                        except ElementTree.ParseError:
                            pass
                elif text and _same_origin(self.root, text):
                    urls.append(text)

        return urls

    def _crawl(self) -> list[str]:
        visited: set[str] = {_normalise(self.root)}
        queue: deque[tuple[str, int]] = deque([(self.root, 0)])
        found: list[str] = [self.root]

        while queue and len(found) < self.max_pages:
            url, level = queue.popleft()
            if level >= self.depth:
                continue
            html = self._get(url)
            if not html:
                continue
            for a in BeautifulSoup(html, "html.parser").find_all("a", href=True):
                href = urljoin(url, a["href"])
                norm = _normalise(href)
                p = urlparse(href)
                if (
                    norm not in visited
                    and _same_origin(self.root, href)
                    and p.scheme in ("http", "https")
                    and not p.path.endswith((".pdf", ".png", ".jpg", ".zip", ".gz"))
                ):
                    visited.add(norm)
                    found.append(href)
                    queue.append((href, level + 1))
                    if len(found) >= self.max_pages:
                        break

        return found

    # ── public ────────────────────────────────────────────────────────────────

    def discover(self) -> list[str]:
        print(f"[Stage 1] URL discovery — root: {self.root}")
        urls = self._sitemap_urls()
        if urls:
            print(f"          sitemap: {len(urls)} URL(s) found")
            # ensure root is included
            norms = {_normalise(u) for u in urls}
            if _normalise(self.root) not in norms:
                urls.insert(0, self.root)
            return urls[: self.max_pages]

        print(f"          no sitemap — BFS crawl (depth={self.depth})")
        urls = self._crawl()
        print(f"          crawl: {len(urls)} URL(s) found")
        return urls


# ── Stage 2 – Network / API Inspection ───────────────────────────────────────

class NetworkInspector:
    """
    Uses Playwright request interception to capture every XHR / fetch
    response that fires while a page loads, alongside the fully rendered HTML.
    """

    CAPTURE_TYPES = {"xhr", "fetch"}

    def __init__(self, timeout: int = 30_000):
        self.timeout = timeout

    def inspect(self, url: str) -> dict:
        """
        Returns:
          {
            "html":      str,          # rendered page HTML
            "api_calls": [             # captured network calls
              {"url": str, "method": str, "status": int, "body": str}, …
            ]
          }
        """
        api_calls: list[dict] = []

        with sync_playwright() as pw:
            browser = pw.chromium.launch(**_chromium_launch_kwargs())
            page = browser.new_page()

            def _on_response(response):
                if response.request.resource_type in self.CAPTURE_TYPES:
                    try:
                        body = response.text()
                    except Exception:
                        body = ""
                    api_calls.append({
                        "url":    response.url,
                        "method": response.request.method,
                        "status": response.status,
                        "body":   body[:4_096],   # cap at 4 KB per response
                    })

            page.on("response", _on_response)
            page.goto(url, wait_until="networkidle", timeout=self.timeout)
            html = page.content()
            browser.close()

        return {"html": html, "api_calls": api_calls}


# ── Stage 3 – Raw HTML Parsing ────────────────────────────────────────────────

def parse_main_content(html: str) -> Tag:
    """
    Strip noise tags and return the best 'main content' BeautifulSoup element.
    """
    soup = BeautifulSoup(html, "html.parser")
    for tag in soup.find_all(NOISE_TAGS):
        tag.decompose()

    main = (
        soup.find("main")
        or soup.find("article")
        or soup.find(id=re.compile(r"content|main|body", re.I))
        or soup.find(class_=re.compile(r"content|main|body|post|doc", re.I))
        or soup.body
        or soup
    )
    return main  # type: ignore[return-value]


# ── Stage 4 – Playwright Rendering ───────────────────────────────────────────
# Rendering is handled inside NetworkInspector.inspect(); when --no-network is
# given, a lightweight render-only path is used (see _render_only below).

def _render_only(url: str, timeout: int) -> str:
    """Render a page with Playwright but without response interception."""
    with sync_playwright() as pw:
        browser = pw.chromium.launch(**_chromium_launch_kwargs())
        page = browser.new_page()
        page.goto(url, wait_until="networkidle", timeout=timeout)
        html = page.content()
        browser.close()
    return html


# ── Stage 5 – Markdown Conversion + ZIP Packaging ────────────────────────────

def _to_markdown(element: Tag) -> str:
    return md_convert(
        str(element),
        heading_style="ATX",
        bullets="-",
        strip=["script", "style", "nav", "footer", "aside"],
    )


def build_zip(
    pages: list[dict],
    output_path: str,
    include_api_json: bool = True,
) -> None:
    """
    Write one Markdown file per page into a ZIP archive, plus an INDEX.md
    and (optionally) JSON files for captured API responses.

    Each page dict:
      { "url": str, "title": str, "element": Tag, "api_calls": list[dict] }
    """
    with zipfile.ZipFile(output_path, "w", zipfile.ZIP_DEFLATED) as zf:
        index_lines = ["# Scraped Site Index\n"]

        for page in pages:
            slug     = _page_slug(page["url"])
            md_name  = f"{slug}.md"
            title    = page.get("title") or page["url"]
            content  = f"# {title}\n\n> Source: <{page['url']}>\n\n"
            content += _to_markdown(page["element"])

            zf.writestr(f"pages/{md_name}", content)
            index_lines.append(f"- [{title}](pages/{md_name})")

            if include_api_json and page.get("api_calls"):
                zf.writestr(
                    f"api/{slug}_api.json",
                    json.dumps(page["api_calls"], indent=2, ensure_ascii=False),
                )

        zf.writestr("INDEX.md", "\n".join(index_lines))

    print(f"[Stage 5] ZIP  → {output_path}  ({len(pages)} page(s))")


# ── Optional DOCX output ──────────────────────────────────────────────────────

_HEADING_LEVELS = {f"h{i}": i for i in range(1, 7)}
_CODE_FONT      = "Courier New"


def _blocks_from_element(element: Tag) -> list[dict]:
    blocks: list[dict] = []
    seen:   set[str]   = set()

    def walk(node):
        if isinstance(node, NavigableString):
            return
        if not isinstance(node, Tag):
            return
        tag = (node.name or "").lower()
        if tag in NOISE_TAGS:
            return
        if tag in _HEADING_LEVELS:
            text = _clean(node.get_text())
            if text and text not in seen:
                seen.add(text)
                blocks.append({"type": tag, "text": text})
            return
        if tag == "pre":
            blocks.append({"type": "code", "text": node.get_text()})
            return
        if tag == "li":
            text = _clean(node.get_text())
            if text and text not in seen:
                seen.add(text)
                blocks.append({"type": "li", "text": text})
            return
        if tag == "p":
            text = _clean(node.get_text())
            if text and text not in seen:
                seen.add(text)
                blocks.append({"type": "p", "text": text})
            return
        for child in node.children:
            walk(child)

    walk(element)
    return blocks


def build_docx(pages: list[dict], output_path: str) -> None:
    doc = Document()
    for i, page in enumerate(pages):
        if i > 0:
            doc.add_page_break()
        title = page.get("title") or page["url"]
        doc.add_heading(title, level=0)
        url_para = doc.add_paragraph(page["url"])
        url_para.runs[0].font.color.rgb = RGBColor(0x44, 0x72, 0xC4)

        for block in _blocks_from_element(page["element"]):
            btype, text = block["type"], block["text"]
            if btype in _HEADING_LEVELS:
                doc.add_heading(text, level=_HEADING_LEVELS[btype])
            elif btype == "p":
                doc.add_paragraph(text)
            elif btype == "li":
                doc.add_paragraph(text, style="List Bullet")
            elif btype == "code":
                p   = doc.add_paragraph()
                run = p.add_run(text)
                run.font.name = _CODE_FONT
                run.font.size = Pt(9)
                p.paragraph_format.left_indent = Pt(18)

    doc.save(output_path)
    print(f"[Stage 5] DOCX → {output_path}  ({len(pages)} page(s))")


# ── Pipeline orchestrator ─────────────────────────────────────────────────────

def run_pipeline(
    root_url:    str,
    depth:       int,
    max_pages:   int,
    timeout:     int,
    zip_output:  str,
    docx_output: str | None,
    no_network:  bool,
    no_api_json: bool,
) -> None:

    # ── Stage 1: URL Discovery ────────────────────────────────────────────────
    urls = URLDiscovery(root_url, depth=depth, max_pages=max_pages).discover()

    # ── Stages 2–4: per-page render + inspect + parse ─────────────────────────
    inspector = NetworkInspector(timeout=timeout)
    pages: list[dict] = []

    for idx, url in enumerate(urls, 1):
        label = f"[Stage 2-4] ({idx}/{len(urls)})"
        print(f"{label} {url}")
        try:
            if no_network:
                # Stage 4 only — lightweight render without interception
                html      = _render_only(url, timeout)
                api_calls = []
            else:
                # Stage 2 + 4 combined
                result    = inspector.inspect(url)
                html      = result["html"]
                api_calls = result["api_calls"]
                if api_calls:
                    print(f"            ↳ {len(api_calls)} API call(s) captured")

            # Stage 3: raw HTML parse → main content element
            element   = parse_main_content(html)
            soup      = BeautifulSoup(html, "html.parser")
            title_tag = soup.find("title")
            title     = _clean(title_tag.get_text()) if title_tag else url

            pages.append({
                "url":       url,
                "title":     title,
                "element":   element,
                "api_calls": api_calls,
            })

        except Exception as exc:
            print(f"            ↳ ERROR: {exc}", file=sys.stderr)

        time.sleep(0.5)  # polite crawl delay

    if not pages:
        print("No pages scraped successfully.", file=sys.stderr)
        sys.exit(1)

    # ── Stage 5: output ───────────────────────────────────────────────────────
    print(f"\n[Stage 5] Building output for {len(pages)} page(s)…")
    build_zip(pages, zip_output, include_api_json=not no_api_json)
    if docx_output:
        build_docx(pages, docx_output)


# ── CLI ───────────────────────────────────────────────────────────────────────

def _default_stem(url: str) -> str:
    host = urlparse(url).netloc.replace("www.", "").replace(".", "_")
    return re.sub(r"[^\w\-]", "", host)[:40] or "output"


def main() -> None:
    parser = argparse.ArgumentParser(
        description=(
            "5-stage web scraper pipeline:\n"
            "  1. URL Discovery       sitemap.xml + recursive link crawl\n"
            "  2. Network Inspection  intercept XHR/fetch via Playwright\n"
            "  3. Raw HTML Parsing    BeautifulSoup content extraction\n"
            "  4. Playwright Render   headless Chromium JS rendering\n"
            "  5. Markdown + ZIP      per-page .md files in a .zip archive\n"
            "                         (use --docx to also produce a .docx)"
        ),
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument("url",
        help="Root URL to scrape")
    parser.add_argument("-o", "--output",
        help="Output ZIP path (default: <host>.zip)")
    parser.add_argument("--docx", metavar="FILE",
        help="Also write a combined DOCX to this path")
    parser.add_argument("--depth", type=int, default=1,
        help="Link-crawl depth for URL discovery — 0 = root page only (default: 1)")
    parser.add_argument("--max-pages", type=int, default=50,
        help="Maximum pages to scrape (default: 50)")
    parser.add_argument("--timeout", type=int, default=30_000,
        help="Page load timeout in ms (default: 30000)")
    parser.add_argument("--no-network", action="store_true",
        help="Skip Stage 2 network interception (faster, no API JSON)")
    parser.add_argument("--no-api-json", action="store_true",
        help="Omit captured API JSON files from the ZIP")

    args    = parser.parse_args()
    stem    = _default_stem(args.url)
    zip_out = args.output or f"{stem}.zip"

    run_pipeline(
        root_url    = args.url,
        depth       = args.depth,
        max_pages   = args.max_pages,
        timeout     = args.timeout,
        zip_output  = zip_out,
        docx_output = args.docx,
        no_network  = args.no_network,
        no_api_json = args.no_api_json,
    )


if __name__ == "__main__":
    main()
