# Agent Notes for DTClib

## Project Overview

DTClib is a header-only C library that parses HTML-like documents and produces
a JSON-like tree. It is backend-agnostic: the same parser can target different
output representations by defining a small set of backend macros.

Backends shipped in this repo:

- `dtc-json-c.h` — outputs `json-c` objects.
- `dtc-python.h` — outputs Python `list`/`dict` objects via a C extension.

## Building

### C CLI (json-c backend)

Requires `json-c` and a C compiler.

```sh
make json-cli
```

### Python extension (Python backend)

Requires Python 3.x and a C compiler. `make pip-dev` creates a local `.venv`
and installs the extension in editable mode.

```sh
make pip-dev      # pip install -e .[dev]
```

This builds and installs the `dtclib` extension in editable mode.

## Running Tests

### Python tests

```sh
make tests-py
# or, after pip-dev:
.venv/bin/python -m pytest
```

### C smoke tests

```sh
make tests-c
```

### All tests

```sh
make tests
```

## Key Files

- `dtclib.h` — core parser (header-only).
- `dtc-json-c.h` — json-c backend.
- `dtc-python.h` — Python backend macros.
- `dtc_python.c` — Python C extension module entry point.
- `json-cli.c` — example CLI using the json-c backend.
- `setup.py` / `pyproject.toml` — Python packaging.
- `tests/html/` — sample HTML files used by both C and Python tests.
- `tests/python/` — pytest tests for the Python backend.

## Python API

```python
import dtclib

result = dtclib.parse("<html><body>hello</body></html>")
# result is a list of nodes, where each element is either:
#   - a str (text node)
#   - a dict with keys: "name" (str), "attributes" (dict, optional),
#     "content" (list, optional)
```

On parse error, `dtclib.parse` raises `ValueError` with line/column
information.
