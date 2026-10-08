import os
import sys

import pytest
import dtclib

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "..", "examples"))
import cliowser


SIMPLE_HTML = """<html>
<head><title>Page Title</title></head>
<body>
<h1>Main Title</h1>
<p>First paragraph.<br>Second line.</p>
<p>Another para with <b>bold</b> and <i>italic</i>.</p>
<ul><li>item one</li><li>item two</li></ul>
<script>alert("ignored");</script>
<a href="/relative">click me</a>
</body>
</html>"""


def parse(html):
    return dtclib.parse(html)


def test_render_plain_skips_invisible_tags():
    tree = parse(SIMPLE_HTML)
    text = cliowser.render_text(tree, mode=cliowser.MODE_PLAIN)
    assert "Page Title" not in text
    assert 'alert("ignored");' not in text
    assert "Main Title" in text


def test_render_plain_block_newlines():
    tree = parse("<body><p>one</p><p>two</p></body>")
    text = cliowser.render_text(tree, mode=cliowser.MODE_PLAIN)
    assert "one" in text
    assert "two" in text
    assert text.index("one") < text.index("two")


def test_render_plain_br_as_newline():
    tree = parse("<p>line one<br>line two</p>")
    text = cliowser.render_text(tree, mode=cliowser.MODE_PLAIN)
    assert "line one" in text
    assert "line two" in text
    assert text.index("line one") < text.index("line two")


def test_render_plain_lists():
    tree = parse("<ul><li>a</li><li>b</li></ul>")
    text = cliowser.render_text(tree, mode=cliowser.MODE_PLAIN)
    assert "a" in text
    assert "b" in text


def test_render_markdown_headings():
    tree = parse("<body><h1>T1</h1><h2>T2</h2></body>")
    text = cliowser.render_text(tree, mode=cliowser.MODE_MARKDOWN)
    assert "# T1" in text
    assert "## T2" in text


def test_render_markdown_bold_italic():
    tree = parse("<p><b>bold</b> <i>italic</i></p>")
    text = cliowser.render_text(tree, mode=cliowser.MODE_MARKDOWN)
    assert "**bold**" in text
    assert "*italic*" in text


def test_render_markdown_links():
    tree = parse('<a href="/path">link text</a>')
    text = cliowser.render_text(tree, mode=cliowser.MODE_MARKDOWN, base_url="https://example.com")
    assert "[link text](https://example.com/path)" in text


def test_render_markdown_lists():
    tree = parse("<ul><li>first</li><li>second</li></ul>")
    text = cliowser.render_text(tree, mode=cliowser.MODE_MARKDOWN)
    assert "- first" in text
    assert "- second" in text


def test_extract_title():
    tree = parse(SIMPLE_HTML)
    assert cliowser.extract_title(tree) == "Page Title"


def test_extract_links():
    tree = parse(SIMPLE_HTML)
    base_url = "https://example.com"
    links = cliowser.extract_links(tree, base_url)
    assert len(links) == 1
    assert links[0]["url"] == "https://example.com/relative"
    assert links[0]["text"] == "click me"


def test_tag_stats():
    tree = parse("<div><p>a</p><p>b</p></div>")
    stats = cliowser.tag_stats(tree)
    assert stats.get("div") == 1
    assert stats.get("p") == 2
