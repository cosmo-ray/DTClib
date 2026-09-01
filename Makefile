VENV := .venv
PYTHON := $(VENV)/bin/python
PIP := $(VENV)/bin/pip

JSON_C_CFLAGS := $(shell pkg-config --cflags json-c)
JSON_C_LDFLAGS := $(shell pkg-config --libs json-c)

.PHONY: all venv pip-dev tests-py tests-c json-cli tests clean

all: json-cli

venv:
	@test -d $(VENV) || python3 -m venv $(VENV)

json-cli: json-cli.c dtc-json-c.h dtclib.h
	$(CC) json-cli.c -std=gnu23 $(JSON_C_CFLAGS) -I./ -o json-cli $(JSON_C_LDFLAGS)

pip-dev: venv
	$(PIP) install -e '.[dev]' --force-reinstall

tests-py: pip-dev
	$(PYTHON) -m pytest

tests-c: json-cli
	@for f in tests/html/*.html; do \
		if [ -f "$$f" ]; then \
			echo "Testing $$f"; \
			./json-cli -f "$$f" > /dev/null || exit 1; \
		fi; \
	done

tests: tests-py tests-c

clean:
	rm -f json-cli
	rm -rf build
	rm -rf *.egg-info
	find . -name '*.so' -delete
	find . -type d -name __pycache__ -exec rm -rf {} +
