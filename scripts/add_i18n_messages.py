#!/usr/bin/env python3
"""Add the Omega Lite settings messages to an Omega checkout.

Usage:
    python3 scripts/add_i18n_messages.py /path/to/Omega

The settings app translations are kept out of the integration patch on purpose:
the upstream .i18n files mix line endings (some are CRLF) and git normalises
them, so a content patch around the last line is fragile. Appending one line per
language with the file's own line ending is deterministic and idempotent.
"""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

MESSAGES = {
    "en": "Customize colors",
    "fr": "Personnaliser les couleurs",
    "de": "Farben anpassen",
    "es": "Personalizar colores",
    "it": "Personalizza colori",
    "nl": "Kleuren aanpassen",
    "pt": "Personalizar cores",
    "hu": "Színek testreszabása",
}

KEY = "CustomColors"


def ensure_message(path, label):
    with open(path, "r", encoding="utf-8", newline="") as handle:
        text = handle.read()
    if any(line.split("=", 1)[0].strip() == KEY for line in text.splitlines()):
        return False
    newline = "\r\n" if "\r\n" in text else "\n"
    if not text.endswith(newline):
        text += newline
    with open(path, "w", encoding="utf-8", newline="") as handle:
        handle.write(text + f'{KEY} = "{label}"' + newline)
    return True


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("omega", help="path to an Omega checkout")
    args = parser.parse_args(argv)

    settings = Path(args.omega) / "apps" / "settings"
    if not settings.is_dir():
        raise SystemExit(f"error: {settings} not found, is this an Omega checkout?")

    changed = 0
    for language, label in MESSAGES.items():
        path = settings / f"base.{language}.i18n"
        if not path.is_file():
            raise SystemExit(f"error: missing translation file {path}")
        if ensure_message(path, label):
            changed += 1
    print(f"Translations: {changed} file(s) updated, {len(MESSAGES) - changed} already present")
    return 0


if __name__ == "__main__":
    sys.exit(main())
