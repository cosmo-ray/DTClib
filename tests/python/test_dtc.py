import pytest
import dtclib


def test_parse_simple_document():
    result = dtclib.parse("<html><body>hello</body></html>")
    assert isinstance(result, list)
    assert len(result) == 1

    html = result[0]
    assert html["name"] == "html"
    assert isinstance(html["content"], list)
    assert len(html["content"]) == 1

    body = html["content"][0]
    assert body["name"] == "body"
    assert body["content"] == ["hello"]


def test_parse_text_and_tags():
    result = dtclib.parse("<p>before <b>bold</b> after</p>")
    assert len(result) == 1
    p = result[0]
    assert p["name"] == "p"
    content = p["content"]
    assert content[0] == "before "
    assert content[1]["name"] == "b"
    assert content[1]["content"] == ["bold"]
    assert content[2] == "after"


def test_parse_attributes():
    result = dtclib.parse('<div class="foo" id="bar">content</div>')
    assert len(result) == 1
    div = result[0]
    assert div["name"] == "div"
    assert div["attributes"] == {"class": "foo", "id": "bar"}
    assert div["content"] == ["content"]


def test_parse_boolean_attribute():
    result = dtclib.parse('<input disabled>')
    assert len(result) == 1
    inp = result[0]
    assert inp["name"] == "input"
    assert inp["attributes"] == {"disabled": True}


def test_parse_void_element():
    result = dtclib.parse("<br>")
    assert len(result) == 1
    assert result[0]["name"] == "br"


def test_parse_empty():
    result = dtclib.parse("")
    assert result == [""]


def test_parse_only_text():
    result = dtclib.parse("just text")
    assert result == ["just text"]


def test_parse_error_wrong_closing_tag():
    with pytest.raises(ValueError):
        dtclib.parse("<div></span></div>")


def test_parse_error_unclosed_tag():
    with pytest.raises(ValueError):
        dtclib.parse("<div>")



