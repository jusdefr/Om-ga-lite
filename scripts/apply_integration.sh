#!/bin/sh
# Apply Omega Lite to a checkout of Omega.
#
#   scripts/apply_integration.sh /path/to/Omega
#
# The module directory is copied as is; the patch only touches the tracked
# settings files, which keeps the integration reviewable and reversible with
# `git checkout .` inside the Omega checkout.

set -eu

OMEGA_DIR="${1:-}"
if [ -z "$OMEGA_DIR" ]; then
    echo "usage: $0 /path/to/Omega" >&2
    exit 2
fi
REPO_DIR="$(cd "$(dirname "$0")/.." && pwd)"
PATCH="$REPO_DIR/integration/omega_lite_settings.patch"

if [ ! -d "$OMEGA_DIR/apps/settings" ]; then
    echo "error: $OMEGA_DIR does not look like an Omega checkout" >&2
    exit 2
fi

mkdir -p "$OMEGA_DIR/apps/settings/custom_colors"
cp -a "$REPO_DIR/apps/settings/custom_colors/." "$OMEGA_DIR/apps/settings/custom_colors/"
# The theme must be in the local theme folder for THEME_NAME=omega_lite.
mkdir -p "$OMEGA_DIR/themes/themes/local"
cp "$REPO_DIR/themes/omega_lite.json" "$OMEGA_DIR/themes/themes/local/omega_lite.json"
# The integration patch points the palette rule at this generator.
mkdir -p "$OMEGA_DIR/scripts"
cp "$REPO_DIR/scripts/generate_runtime_palette.py" "$OMEGA_DIR/scripts/generate_runtime_palette.py"
git -C "$OMEGA_DIR" apply --check "$PATCH"
git -C "$OMEGA_DIR" apply "$PATCH"
python3 "$REPO_DIR/scripts/add_i18n_messages.py" "$OMEGA_DIR"
echo "Omega Lite applied to $OMEGA_DIR"
echo "Build with: make -j4 THEME_REPO=local THEME_NAME=omega_lite"
