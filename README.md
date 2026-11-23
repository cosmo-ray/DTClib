# DTClib

[![tests](https://github.com/cosmo-ray/DTClib/actions/workflows/test.yaml/badge.svg)](https://github.com/cosmo-ray/DTClib/actions/workflows/test.yaml)
[![pypi](https://img.shields.io/pypi/v/dtclib.svg)](https://pypi.org/project/dtclib/)
[![license: MIT](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

**D**om **T**ree **C**ontainer — take your HTML and put it into a JSON-like tree,
or a Python `dict`/`list` tree, well, into any container.

DTClib is a header-only C parser for HTML-like documents. It is
backend-agnostic: the same parser can target different output representations
by defining a small set of backend macros.

Backends shipped in this repository:

- `dtc-python.h` — outputs Python `list`/`dict` objects via a C extension.
- `dtc-json-c.h` — outputs [`json-c`](https://github.com/json-c/json-c) objects.

## Install

```sh
pip install dtclib
```

Prebuilt wheels are available for Linux (glibc and musl, x86_64), macOS
(arm64) and Windows (amd64, built with MinGW-w64). On other platforms the
extension is compiled from the source distribution, so a C compiler is
required.

## Python API

```python
import dtclib

tree = dtclib.parse('<p class="x">hi <b>there</b></p>')
```

The result is a list of nodes, where each element is either:

- a `str` — a text node,
- a `dict` with the keys:
  - `name` (`str`) — the tag name (`!doctype` for a doctype),
  - `attributes` (`dict`, optional) — attribute name to value; a valueless
    attribute such as `<input disabled>` maps to `True`,
  - `content` (`list`, optional) — child nodes. Void elements
    (`br`, `img`, `input`, …) have no `content` key.

Example:

```python
>>> import dtclib
>>> dtclib.parse('<p class="x">hi <b>there</b></p>')
[{'name': 'p', 'attributes': {'class': 'x'}, 'content': ['hi ', {'name': 'b', 'content': ['there']}]}]
```

On a parse error, `dtclib.parse` raises `ValueError` with line/column
information:

```python
>>> dtclib.parse('<div><p></div>')
Traceback (most recent call last):
  ...
ValueError: parse error at line 0 col 9: trying to close the wrong tag got 'div' instead of 'p' :(
```

Comments (`<!-- ... -->`) are skipped. Unknown tag names are kept as text
instead of failing the parse.

## C CLI (json-c backend)

Requires `json-c` and a C compiler:

```sh
make json-cli
./json-cli -f tests/html/gol.html
```

## Use the parser in your own backend

`dtclib.h` is header-only. Define the `DTC_*` macros expected by the parser
(see `dtc-python.h` or `dtc-json-c.h` for reference implementations), then:

```c
#define DTCLIB_PREFIX mybackend
#include "dtclib.h"
```

## Example browser: cliowser

`examples/cliowser.py` is a tiny interactive text-mode browser that fetches a
page, parses it with `dtclib`, and uses
[`csonpath`](https://pypi.org/project/csonpath/) to inspect the tree:

```sh
pip install 'dtclib[browser]'
python examples/cliowser.py https://example.com
```

Commands inside the prompt:

- `p` — print page (URL, title, extracted text)
- `l` — list links found on the page
- `i` — show tag statistics (Python) and hrefs/srcs (csonpath)
- `r` — dump the raw parsed tree as JSON
- `m` — toggle between plain text and markdown rendering
- `g <n>` — navigate to link number `n`
- `b` — go back to the previous page
- `q` — quit

## Development

```sh
make pip-dev    # creates .venv and installs the extension in editable mode
make tests-py   # python tests
make tests-c    # C smoke tests (needs json-c)
make tests      # both
make dist       # sdist + wheel in dist/
```

Releases are cut with
[bump-my-version](https://github.com/callowayproject/bump-my-version),
installed by `make pip-dev`:

```sh
bump-my-version bump patch   # or minor / major
git push --follow-tags
```

It bumps `version` in `pyproject.toml`, commits `Bump to X.Y.Z` and creates
the `vX.Y.Z` tag. Pushing the tag triggers `pypi.yaml`, which builds the
wheels and publishes with the `PYPI_TOKEN` repository secret. Preview a bump
with `bump-my-version bump patch --dry-run -vv`.

## License

MIT — see [LICENSE](LICENSE).
