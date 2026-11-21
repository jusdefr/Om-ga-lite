#!/usr/bin/env python3
"""Validate an Omega theme file before building Omega Lite.

Usage:
    python3 scripts/validate_theme.py themes/omega_lite.json
    python3 scripts/validate_theme.py --list-keys
    python3 scripts/validate_theme.py --check-roles themes/omega_lite.json

Exit codes: 0 valid, 1 invalid, 2 usage or I/O error.

The validator checks the structure upstream ``themes/themes_manager.py``
expects, the exact key set (a missing key breaks the build, an unknown key is
never used), and that every color is a bare six-digit hexadecimal value: Omega
theme files do not accept a leading '#'.
"""

from __future__ import annotations

import argparse
import difflib
import json
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

from palette_schema import ROLE_PALETTE_KEYS, expected_paths  # noqa: E402

HEX_COLOR_RE = re.compile(r"^[0-9a-fA-F]{6}$")
MAX_GROUP_DEPTH = 2


def flatten_colors(colors, errors, prefix=""):
    """Return {path: value} for a theme ``colors`` object.

    A group whose members include "" is itself a color: upstream writes
    ``"Toolbar": {"": "ffb734", "Text": "000000"}`` and the generated constant
    is ``Palette::Toolbar``."""
    flat = {}
    if not isinstance(colors, dict):
        errors.append('"colors" must be a JSON object')
        return flat
    for name, value in colors.items():
        if not isinstance(name, str):
            errors.append("color names must be strings")
            continue
        if name == "":
            # The group itself is a color: "Toolbar": {"": "ffb734", ...}
            if not prefix:
                errors.append('an empty color name is only allowed inside a group')
                continue
            path = prefix
        else:
            path = name if not prefix else f"{prefix}.{name}"
        depth = path.count(".") + 1
        if isinstance(value, dict):
            if depth >= MAX_GROUP_DEPTH:
                errors.append(f'"{path}": groups cannot be nested this deeply')
                continue
            flat.update(flatten_colors(value, errors, path))
        elif isinstance(value, str):
            flat[path] = value
        else:
            errors.append(f'"{path}": color must be a string, got {type(value).__name__}')
    return flat


def validate_theme_data(data, schema_paths=None):
    """Return the list of problems found in a parsed theme."""
    errors = []
    if not isinstance(data, dict):
        return ["the theme must be a JSON object"]

    for field in ("name", "icons"):
        value = data.get(field)
        if not isinstance(value, str) or not value.strip():
            errors.append(f'"{field}" must be a non-empty string')

    if "colors" not in data:
        errors.append('missing "colors" object')
        return errors

    flat = flatten_colors(data["colors"], errors)

    for path, value in flat.items():
        if not isinstance(value, str) or not HEX_COLOR_RE.match(value):
            shown = value if isinstance(value, str) else type(value).__name__
            errors.append(
                f'"{path}": "{shown}" is not a six digit hexadecimal color '
                "(write 2FD3C4, without '#')"
            )

    valid_paths = expected_paths() if schema_paths is None else schema_paths
    valid_set = set(valid_paths)
    for path in sorted(set(flat) - valid_set):
        suggestion = difflib.get_close_matches(path, valid_paths, n=1)
        hint = f' (did you mean "{suggestion[0]}"?)' if suggestion else ""
        errors.append(f'unknown color key "{path}"{hint}')
    for path in sorted(valid_set - set(flat)):
        errors.append(f'missing color key "{path}"')

    return errors


def load_theme(path):
    try:
        with open(path, "r", encoding="utf-8") as handle:
            return json.load(handle)
    except FileNotFoundError:
        raise SystemExit(f"error: {path}: file not found")
    except json.JSONDecodeError as exc:
        raise SystemExit(f"error: {path}: invalid JSON: {exc}")


def missing_role_keys(flat_colors):
    """Every role of the customization page must map onto a theme key."""
    return [key for key in ROLE_PALETTE_KEYS.values() if key not in flat_colors]


def main(argv=None):
    parser = argparse.ArgumentParser(description="Validate an Omega theme file.")
    parser.add_argument("files", nargs="*", help="theme JSON files to validate")
    parser.add_argument("--list-keys", action="store_true", help="print the expected keys and exit")
    parser.add_argument(
        "--check-roles",
        action="store_true",
        help="also check that every customizable color role exists in the theme",
    )
    args = parser.parse_args(argv)

    if args.list_keys:
        for path in expected_paths():
            print(path)
        return 0

    if not args.files:
        parser.error("no theme file given")

    exit_code = 0
    for path in args.files:
        data = load_theme(path)
        errors = validate_theme_data(data)
        if args.check_roles and not errors:
            ignored = []
            flat = flatten_colors(data["colors"], ignored)
            for missing in missing_role_keys(flat):
                errors.append(f'missing color "{missing}" used by a customizable role')
        if errors:
            exit_code = 1
            print(f"INVALID {path}")
            for error in errors:
                print(f"  - {error}")
        else:
            print(f"OK      {path}")
    return exit_code


if __name__ == "__main__":
    sys.exit(main())
