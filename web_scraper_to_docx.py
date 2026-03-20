#!/usr/bin/env python3
"""
Web Scraper to DOCX Converter
Renders JavaScript websites using Playwright and converts content to a readable .docx file.
"""

import argparse
import re
import sys
from pathlib import Path
from urllib.parse import urlparse

from bs4 import BeautifulSoup, NavigableString, Tag
from docx import Document
from docx.shared import Pt, RGBColor
from playwright.sync_api import sync_playwright


# ---------------------------------------------------------------------------
# Scraping
# ---------------------------------------------------------------------------

def scrape_js_site(url: str, wait_for: str | None = None, timeout: int = 30_000) -> str:
    """
    Use Playwright (headless Chromium) to fully render a JavaScript site
    and return the final HTML.

    Args:
        url:      Target URL.
        wait_for: Optional CSS selector to wait for before capturing HTML.
        timeout:  Milliseconds to wait for navigation / selector.

    Returns:
        Rendered HTML string.
    """
    with sync_playwright() as pw:
        browser = pw.chromium.launch(headless=True)
        page = browser.new_page()
        page.goto(url, wait_until="networkidle", timeout=timeout)

        if wait_for:
            page.wait_for_selector(wait_for, timeout=timeout)

        html = page.content()
        browser.close()

    return html


# ---------------------------------------------------------------------------
# HTML → structured content
# ---------------------------------------------------------------------------

BLOCK_TAGS = {
    "h1", "h2", "h3", "h4", "h5", "h6",
    "p", "li", "blockquote", "pre", "code",
    "td", "th", "caption", "figcaption",
    "article", "section", "header", "footer", "main",
}

SKIP_TAGS = {"script", "style", "noscript", "svg", "meta", "link", "head"}


def _clean_text(text: str) -> str:
    """Collapse whitespace and strip."""
    return re.sub(r"\s+", " ", text).strip()


def extract_content_blocks(html: str) -> list[dict]:
    """
    Parse HTML and return an ordered list of content blocks:
      {"type": "h1"|"h2"|"h3"|"h4"|"h5"|"h6"|"p"|"li"|"code"|"blockquote", "text": str}
    """
    soup = BeautifulSoup(html, "html.parser")

    # Remove noise tags
    for tag in soup.find_all(SKIP_TAGS):
        tag.decompose()

    # Try to isolate the main content area
    main = (
        soup.find("main")
        or soup.find("article")
        or soup.find(id=re.compile(r"content|main|body", re.I))
        or soup.find(class_=re.compile(r"content|main|body|post", re.I))
        or soup.body
        or soup
    )

    blocks: list[dict] = []
    seen: set[str] = set()

    def walk(node):
        if isinstance(node, NavigableString):
            return
        if not isinstance(node, Tag):
            return

        tag_name = node.name.lower() if node.name else ""

        if tag_name in SKIP_TAGS:
            return

        if tag_name in {"h1", "h2", "h3", "h4", "h5", "h6"}:
            text = _clean_text(node.get_text())
            if text and text not in seen:
                seen.add(text)
                blocks.append({"type": tag_name, "text": text})
            return  # don't descend into headings

        if tag_name == "pre":
            text = node.get_text()
            if text.strip():
                blocks.append({"type": "code", "text": text})
            return

        if tag_name == "code" and node.parent and node.parent.name != "pre":
            # inline code – treat as paragraph
            text = _clean_text(node.get_text())
            if text and text not in seen:
                seen.add(text)
                blocks.append({"type": "code_inline", "text": text})
            return

        if tag_name == "blockquote":
            text = _clean_text(node.get_text())
            if text and text not in seen:
                seen.add(text)
                blocks.append({"type": "blockquote", "text": text})
            return

        if tag_name == "li":
            text = _clean_text(node.get_text())
            if text and text not in seen:
                seen.add(text)
                blocks.append({"type": "li", "text": text})
            return

        if tag_name == "p":
            text = _clean_text(node.get_text())
            if text and text not in seen:
                seen.add(text)
                blocks.append({"type": "p", "text": text})
            return

        # Recurse into container elements
        for child in node.children:
            walk(child)

    walk(main)
    return blocks


# ---------------------------------------------------------------------------
# Content → DOCX
# ---------------------------------------------------------------------------

HEADING_LEVELS = {"h1": 1, "h2": 2, "h3": 3, "h4": 4, "h5": 5, "h6": 6}

CODE_FONT = "Courier New"
CODE_BG   = RGBColor(0xF0, 0xF0, 0xF0)


def _add_code_paragraph(doc: Document, text: str) -> None:
    """Add a code block with monospace styling."""
    para = doc.add_paragraph()
    run = para.add_run(text)
    run.font.name = CODE_FONT
    run.font.size = Pt(9)
    para.paragraph_format.left_indent = Pt(18)


def _add_blockquote(doc: Document, text: str) -> None:
    """Add a blockquote styled paragraph."""
    para = doc.add_paragraph()
    run = para.add_run(f'"{text}"')
    run.font.italic = True
    run.font.color.rgb = RGBColor(0x55, 0x55, 0x55)
    para.paragraph_format.left_indent = Pt(24)


def blocks_to_docx(blocks: list[dict], output_path: str, title: str = "") -> None:
    """
    Convert structured content blocks into a .docx file.

    Args:
        blocks:      List of dicts from extract_content_blocks().
        output_path: File path for the generated .docx.
        title:       Optional document title inserted at the top.
    """
    doc = Document()

    if title:
        doc.add_heading(title, level=0)

    for block in blocks:
        btype = block["type"]
        text  = block["text"]

        if btype in HEADING_LEVELS:
            doc.add_heading(text, level=HEADING_LEVELS[btype])

        elif btype == "p":
            doc.add_paragraph(text)

        elif btype == "li":
            doc.add_paragraph(text, style="List Bullet")

        elif btype in ("code", "code_inline"):
            _add_code_paragraph(doc, text)

        elif btype == "blockquote":
            _add_blockquote(doc, text)

    doc.save(output_path)
    print(f"Saved: {output_path}")


# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------

def _default_output(url: str) -> str:
    parsed = urlparse(url)
    host = parsed.netloc.replace("www.", "").replace(".", "_")
    path = parsed.path.strip("/").replace("/", "_") or "index"
    name = f"{host}_{path}"
    name = re.sub(r"[^\w\-]", "", name)[:80]
    return f"{name}.docx"


def main():
    parser = argparse.ArgumentParser(
        description="Scrape a JavaScript website and save it as a .docx file."
    )
    parser.add_argument("url", help="URL to scrape")
    parser.add_argument(
        "-o", "--output",
        help="Output .docx file path (default: auto-generated from URL)",
    )
    parser.add_argument(
        "--wait-for",
        metavar="SELECTOR",
        help="CSS selector to wait for before capturing content (e.g. '#main')",
    )
    parser.add_argument(
        "--timeout",
        type=int,
        default=30_000,
        help="Page load timeout in milliseconds (default: 30000)",
    )
    parser.add_argument(
        "--title",
        help="Document title (default: page <title> tag or URL)",
    )

    args = parser.parse_args()

    output = args.output or _default_output(args.url)

    print(f"Scraping: {args.url}")
    html = scrape_js_site(args.url, wait_for=args.wait_for, timeout=args.timeout)

    # Determine title
    title = args.title
    if not title:
        soup = BeautifulSoup(html, "html.parser")
        tag = soup.find("title")
        title = _clean_text(tag.get_text()) if tag else args.url

    print("Extracting content…")
    blocks = extract_content_blocks(html)

    if not blocks:
        print("No content extracted. The page may require authentication or "
              "use an unsupported rendering technique.", file=sys.stderr)
        sys.exit(1)

    print(f"Building DOCX ({len(blocks)} blocks)…")
    blocks_to_docx(blocks, output, title=title)


if __name__ == "__main__":
    main()
