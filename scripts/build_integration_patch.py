#!/usr/bin/env python3
"""Integrate Omega Lite into a checkout of Omega and regenerate the patch.

Usage:
    python3 scripts/build_integration_patch.py /path/to/Omega --write-patch

The script copies apps/settings/custom_colors into the checkout, edits the
tracked settings files (menu entry, controller member, Makefile sources,
translations, boot hook) and can write the resulting diff to
integration/omega_lite_settings.patch.

It fails loudly instead of guessing when an anchor is not found, which is what
happens when the Omega revision you use changed those files. Nothing is
overwritten blindly: every replacement must match exactly once.
"""

from __future__ import annotations

import argparse
import re
import shutil
import subprocess
import sys
from pathlib import Path

WORKSPACE = Path(__file__).resolve().parents[1]
MODULE = WORKSPACE / "apps" / "settings" / "custom_colors"
PATCH_PATH = WORKSPACE / "integration" / "omega_lite_settings.patch"

TRANSLATIONS = {
    "en": "Customize colors",
    "fr": "Personnaliser les couleurs",
    "de": "Farben anpassen",
    "es": "Personalizar colores",
    "it": "Personalizza colori",
    "nl": "Kleuren aanpassen",
    "pt": "Personalizar cores",
    "hu": "Színek testreszabása",
}


def edit(omega, path, replacements):
    file = omega / path
    text = file.read_text(encoding="utf-8")
    for old, new in replacements:
        count = text.count(old)
        if count != 1:
            raise SystemExit(f"error: {path}: expected one occurrence of {old!r}, found {count}")
        text = text.replace(old, new)
    file.write_text(text, encoding="utf-8")


def add_menu_entry(omega, name):
    path = omega / f"apps/settings/main_controller_prompt_{name}.cpp"
    text = path.read_text(encoding="utf-8")
    start = text.find("s_modelMenu[] =")
    if start < 0:
        raise SystemExit(f"error: {path}: cannot locate s_modelMenu")
    end = text.find("};\n", start)
    if end < 0:
        raise SystemExit(f"error: {path}: cannot find the end of s_modelMenu")
    body = text[start:end]
    if not body.rstrip().endswith(")"):
        raise SystemExit(f"error: {path}: unexpected s_modelMenu layout")
    new_body = body + ",\n    SettingsMessageTree(I18n::Message::CustomColors)"
    path.write_text(text[:start] + new_body + text[end:], encoding="utf-8")


def patch(omega, write_patch):
    if not (omega / "apps" / "settings").is_dir():
        raise SystemExit(f"error: {omega} does not look like an Omega checkout")

    target = omega / "apps" / "settings" / "custom_colors"
    if target.exists():
        shutil.rmtree(target)
    shutil.copytree(MODULE, target, ignore=shutil.ignore_patterns("*.o", "Makefile.bak"))

    edit(omega, "apps/settings/Makefile", [
        ("apps += Settings::App\n",
         "apps += Settings::App\n\ninclude apps/settings/custom_colors/Makefile\n"),
        ("app_settings_src += $(app_settings_test_src)\n",
         "app_settings_src += $(app_settings_test_src)\napp_settings_src += $(app_settings_custom_colors_src)\n"),
    ])

    edit(omega, "apps/settings/main_controller.h", [
        ('#include "sub_menu/preferences_controller.h"\n',
         '#include "sub_menu/preferences_controller.h"\n'
         '#include "custom_colors/custom_colors_controller.h"\n'
         '#include "custom_colors/custom_colors_runtime.h"\n'),
        ("  PreferencesController m_preferencesController;\n",
         "  PreferencesController m_preferencesController;\n"
         "  OmegaLite::Colors::CustomColorsController m_customColorsController;\n"),
    ])

    edit(omega, "apps/settings/main_controller.cpp", [
        ("  m_preferencesController(this)\n{",
         "  m_preferencesController(this),\n"
         "  m_customColorsController(this, &OmegaLite::Colors::settings())\n{"),
        ("  if (event == Ion::Events::OK || event == Ion::Events::EXE || event == Ion::Events::Right) {\n"
         "    GenericSubController * subController = nullptr;\n",
         "  if (event == Ion::Events::OK || event == Ion::Events::EXE || event == Ion::Events::Right) {\n"
         "    if (model()->childAtIndex(selectedRow())->label() == I18n::Message::CustomColors) {\n"
         "      /* Omega Lite: the color page is not a SettingsMessageTree based\n"
         "       * sub controller, it owns its view and its storage. */\n"
         "      stackController()->push(&m_customColorsController);\n"
         "      return true;\n"
         "    }\n"
         "    GenericSubController * subController = nullptr;\n"),
    ])

    for name in ("none", "beta", "update"):
        add_menu_entry(omega, name)

    # Translations are added with their own script: the upstream .i18n files mix
    # line endings, so they are deliberately kept out of the patch.
    subprocess.run([sys.executable, str(WORKSPACE / "scripts" / "add_i18n_messages.py"), str(omega)],
                   check=True)

    edit(omega, "apps/main.cpp", [
        ('#include <poincare/init.h>\n',
         '#include <poincare/init.h>\n#include "settings/custom_colors/custom_colors_runtime.h"\n'),
        ("  // Initialize Poincare::TreePool::sharedPool\n  Poincare::Init();\n",
         "  // Initialize Poincare::TreePool::sharedPool\n  Poincare::Init();\n\n"
         "  /* Omega Lite: apply the saved color palette before the first frame. */\n"
         "  OmegaLite::Colors::loadAtStartup();\n"),
    ])

    if write_patch:
        diff = subprocess.run(
            ["git", "-C", str(omega), "diff", "--", "apps", "themes",
             ":(exclude)apps/settings/base.*.i18n"],
            check=True, capture_output=True, text=True,
        ).stdout
        PATCH_PATH.parent.mkdir(parents=True, exist_ok=True)
        PATCH_PATH.write_text(diff, encoding="utf-8")
        print(f"Wrote {PATCH_PATH} ({len(diff.splitlines())} lines)")
    print("Omega checkout patched")


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("omega", help="path to an Omega checkout")
    parser.add_argument("--write-patch", action="store_true", help="write integration/omega_lite_settings.patch")
    args = parser.parse_args(argv)
    patch(Path(args.omega).resolve(), args.write_patch)
    return 0


if __name__ == "__main__":
    sys.exit(main())
