#!/usr/bin/env python3
"""cliowser - a tiny text-mode web browser using DTClib and csonpath."""

import argparse
import json
import re
import sys
from urllib.parse import urljoin, urlparse

import requests

import dtclib
import csonpath


USER_AGENT = "cliowser/0.1"

SKIPPED_TAGS = {
    "head", "title", "meta", "link", "script", "style",
    "noscript", "template", "!doctype",
}

BLOCK_TAGS = {
    "address", "article", "aside", "blockquote", "body", "br", "div",
    "dd", "dl", "dt", "fieldset", "figcaption", "figure", "footer",
    "form", "h1", "h2", "h3", "h4", "h5", "h6", "header", "hr", "li",
    "main", "nav", "ol", "p", "pre", "section", "table", "tbody", "td",
    "tfoot", "th", "thead", "tr", "ul",
}

HEADING_TAGS = {"h1", "h2", "h3", "h4", "h5", "h6"}

LIST_TAGS = {"ul", "ol"}


MODE_PLAIN = "plain"
MODE_MARKDOWN = "markdown"


def fetch(url):
    """Fetch an URL and return decoded HTML text."""
    headers = {"User-Agent": USER_AGENT}
    resp = requests.get(url, headers=headers, timeout=30)
    resp.raise_for_status()
    return resp.text


def parse(html):
    """Parse HTML into a Python tree via DTClib."""
    return dtclib.parse(html)


def _clean_whitespace(text):
    """Collapse spaces, trim lines, remove empty lines in excess."""
    lines = text.splitlines()
    cleaned = []
    prev_empty = False
    for line in lines:
        line = line.strip()
        if not line:
            if not prev_empty:
                cleaned.append("")
            prev_empty = True
            continue
        cleaned.append(line)
        prev_empty = False
    return "\n".join(cleaned).strip()


def render_text(node, mode=MODE_PLAIN, base_url=None):
    """Render a DTClib node as readable text."""
    if mode == MODE_MARKDOWN:
        raw = _render_markdown(node, base_url)
    else:
        raw = _render_plain(node, base_url)
    return _clean_whitespace(raw)


def _render_plain(node, base_url):
    if isinstance(node, str):
        return node
    if isinstance(node, list):
        return _join_rendered([_render_plain(child, base_url) for child in node])
    if not isinstance(node, dict):
        return ""

    tag = node.get("name", "")
    if tag in SKIPPED_TAGS:
        return ""
    if tag == "br":
        return "\n"

    content = node.get("content")
    rendered = ""
    if isinstance(content, list):
        rendered = _join_rendered([_render_plain(child, base_url) for child in content])
    elif isinstance(content, str):
        rendered = content

    if tag in BLOCK_TAGS:
        return "\n" + rendered + "\n"
    return rendered


def _render_markdown(node, base_url):
    if isinstance(node, str):
        return node
    if isinstance(node, list):
        return _join_rendered([_render_markdown(child, base_url) for child in node])
    if not isinstance(node, dict):
        return ""

    tag = node.get("name", "")
    if tag in SKIPPED_TAGS:
        return ""
    if tag == "br":
        return "\n"

    content = node.get("content")
    inner = ""
    if isinstance(content, list):
        inner = _join_rendered([_render_markdown(child, base_url) for child in content])
    elif isinstance(content, str):
        inner = content

    if tag in HEADING_TAGS:
        level = int(tag[1])
        return "\n" + ("#" * level) + " " + inner.strip() + "\n"
    if tag in ("b", "strong"):
        return "**" + inner.strip() + "**"
    if tag in ("i", "em"):
        return "*" + inner.strip() + "*"
    if tag == "code":
        return "`" + inner.strip() + "`"
    if tag == "a":
        attrs = node.get("attributes", {})
        href = attrs.get("href")
        text = inner.strip() or href or ""
        if href:
            url = urljoin(base_url, href)
            return f"[{text}]({url})"
        return text
    if tag == "li":
        return "\n- " + inner.strip()
    if tag in BLOCK_TAGS:
        return "\n" + inner + "\n"
    return inner


def _join_rendered(parts):
    """Join rendered children, adding spaces between inline fragments."""
    result = []
    for part in parts:
        if not part:
            continue
        if part.startswith("\n") or part.endswith("\n"):
            result.append(part)
        else:
            if result and not result[-1].endswith("\n"):
                result.append(" ")
            result.append(part)
    return "".join(result)


def extract_links(node, base_url, links=None):
    """Recursively collect <a href=...> links as {'url', 'text'}."""
    if links is None:
        links = []
    if isinstance(node, list):
        for child in node:
            extract_links(child, base_url, links)
    elif isinstance(node, dict):
        if node.get("name") == "a":
            attrs = node.get("attributes", {})
            href = attrs.get("href")
            if href:
                links.append({
                    "url": urljoin(base_url, href),
                    "text": render_text(node, mode=MODE_PLAIN).strip() or href,
                })
        content = node.get("content")
        if isinstance(content, list):
            for child in content:
                extract_links(child, base_url, links)
    return links


def extract_title(tree):
    """Try to extract <title> text using csonpath, fallback to Python."""
    try:
        wrapped = {"document": tree}
        p = csonpath.CsonPath('$.document..content[?@.name = "title"].content[0]')
        title = p.find_first(wrapped)
        if isinstance(title, str):
            return title
    except Exception:
        pass

    def walk(node):
        if isinstance(node, list):
            for child in node:
                found = walk(child)
                if found:
                    return found
        if isinstance(node, dict):
            if node.get("name") == "title":
                content = node.get("content")
                if isinstance(content, list) and content:
                    if isinstance(content[0], str):
                        return content[0]
            for child in node.get("content", []):
                found = walk(child)
                if found:
                    return found
        return None

    return walk(tree) or ""


def tag_stats(tree):
    """Return a dict {tag_name: count}."""
    counts = {}
    def walk(node):
        if isinstance(node, list):
            for child in node:
                walk(child)
        elif isinstance(node, dict):
            name = node.get("name")
            if isinstance(name, str):
                counts[name] = counts.get(name, 0) + 1
            for child in node.get("content", []):
                walk(child)
    walk(tree)
    return counts


class Cliowser:
    def __init__(self, mode=MODE_PLAIN):
        self.history = []
        self.current_url = None
        self.current_page = None
        self.links = []
        self.mode = mode

    def load(self, url):
        html = fetch(url)
        self.current_page = parse(html)
        self.current_url = url
        if not self.history or self.history[-1] != url:
            self.history.append(url)
        self.links = extract_links(self.current_page, url)

    def print_page(self):
        title = extract_title(self.current_page)
        print()
        print(f"URL: {self.current_url}")
        if title:
            print(f"Title: {title}")
        print("-" * 60)
        text = render_text(self.current_page, mode=self.mode, base_url=self.current_url)
        if text:
            print(text)
        else:
            print("(no text)")
        print("-" * 60)

    def list_links(self):
        if not self.links:
            print("(no links)")
            return
        for i, link in enumerate(self.links):
            scheme = urlparse(link["url"]).scheme
            marker = " " if scheme in ("http", "https") else "?"
            print(f"{marker} {i}: {link['text'][:50]} -> {link['url']}")

    def go(self, idx):
        if not (0 <= idx < len(self.links)):
            print("invalid link number")
            return
        url = self.links[idx]["url"]
        scheme = urlparse(url).scheme
        if scheme not in ("http", "https"):
            print(f"refusing non-http(s) URL: {url}")
            return
        self.load(url)

    def back(self):
        if len(self.history) < 2:
            print("no previous page")
            return
        self.history.pop()
        self.load(self.history[-1])

    def toggle_mode(self):
        if self.mode == MODE_PLAIN:
            self.mode = MODE_MARKDOWN
        else:
            self.mode = MODE_PLAIN
        print(f"mode: {self.mode}")

    def info(self):
        """Show page statistics using Python traversal and csonpath."""
        try:
            counts = tag_stats(self.current_page)
            print(f"tags: {len(counts)} unique")
            for name, count in sorted(counts.items(), key=lambda x: -x[1]):
                print(f"  {name}: {count}")

            wrapped = {"document": self.current_page}
            hrefs = csonpath.CsonPath("$.document..href").find_all(wrapped) or []
            srcs = csonpath.CsonPath("$.document..src").find_all(wrapped) or []
            if hrefs:
                print(f"hrefs ({len(hrefs)}):")
                for h in hrefs[:10]:
                    print(f"  {h}")
            if srcs:
                print(f"srcs ({len(srcs)}):")
                for s in srcs[:10]:
                    print(f"  {s}")
        except Exception as e:
            print(f"info error: {e}")

    def raw_tree(self):
        print(json.dumps(self.current_page, indent=2, ensure_ascii=False))

    def run(self, start_url):
        self.load(start_url)

        while True:
            try:
                cmd = input("(p)rint, (l)inks, (i)nfo, (r)aw, (m)ode, (g)o <n>, (b)ack, (q)uit: ").strip()
            except EOFError:
                print()
                break
            if not cmd:
                continue

            parts = cmd.split()
            op = parts[0].lower()

            if op == "q":
                break
            elif op == "p":
                self.print_page()
            elif op == "l":
                self.list_links()
            elif op == "i":
                self.info()
            elif op == "r":
                self.raw_tree()
            elif op == "m":
                self.toggle_mode()
            elif op == "g":
                if len(parts) < 2:
                    print("usage: g <n>")
                    continue
                try:
                    self.go(int(parts[1]))
                except ValueError:
                    print("invalid number")
            elif op == "b":
                self.back()
            else:
                print("unknown command")


def main():
    parser = argparse.ArgumentParser(description="tiny text-mode web browser")
    parser.add_argument("url", nargs="?", help="starting URL")
    parser.add_argument("--markdown", action="store_true", help="start in markdown rendering mode")
    args = parser.parse_args()

    url = args.url
    if not url:
        url = input("URL: ").strip()
    if not url:
        parser.print_help()
        sys.exit(1)

    mode = MODE_MARKDOWN if args.markdown else MODE_PLAIN
    Cliowser(mode=mode).run(url)


if __name__ == "__main__":
    main()
