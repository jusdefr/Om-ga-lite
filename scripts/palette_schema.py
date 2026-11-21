"""Schema shared by the theme tools.

The key set below is the one upstream Omega 2.0 ships in
``themes/themes/local/epsilon_dark.json``. A theme used to build Omega must
provide exactly these keys: ``themes/themes_manager.py`` turns every leaf into
a ``Palette::`` constant, and a missing key breaks the build with a compile
error, while an unexpected key silently adds an unused constant.

Keep this file in sync with the Omega revision you build against. The
``Role`` table mirrors ``apps/settings/custom_colors/color_roles.cpp`` and is
checked against the C++ source by the test suite.
"""

# Nested description of a valid theme "colors" object.
# A leaf whose name is "" means "this group itself is a color", as upstream
# does for Toolbar, Battery and Control.
THEME_COLOR_KEYS = {
    "PrimaryText": None,
    "SecondaryText": None,
    "AccentText": None,
    "ApproximateSignText": None,
    "ApproximateExpressionText": None,
    "Background": {"Hard": None, "Apps": None, "AppsSecondary": None},
    "Toolbar": {"": None, "Text": None},
    "ExpressionInput": {"Background": None, "Border": None},
    "Grid": {"PrimaryLine": None, "SecondaryLine": None},
    "Battery": {"": None, "InCharge": None, "Low": None},
    "ScrollBar": {"Foreground": None, "Background": None},
    "Control": {"": None, "Enabled": None, "Disabled": None},
    "Calculation": {
        "BackgroundOdd": None,
        "BackgroundEven": None,
        "EmptyBox": None,
        "EmptyBoxNeeded": None,
        "TrigoAndComplexForeground": None,
    },
    "Code": {
        "Background": None,
        "BackgroundSelected": None,
        "Text": None,
        "Comment": None,
        "Number": None,
        "Keyword": None,
        "Operator": None,
        "String": None,
        "GutterViewBackground": None,
    },
    "Probability": {"Curve": None, "CellBorder": None, "HistogramBar": None},
    "Statistics": {
        "Box": None,
        "BoxVerticalLine": None,
        "Selected": None,
        "NotSelected": None,
    },
    "Graph": {"Tangent": None},
    "SubMenu": {"Background": None, "Border": None, "Text": None},
    "Toolbox": {
        "HeaderBackground": None,
        "HeaderText": None,
        "HeaderBorder": None,
        "Background": None,
    },
    "List": {
        "CellBackground": None,
        "CellBackgroundSelected": None,
        "CellBorder": None,
    },
    "Button": {
        "Background": None,
        "BackgroundSelected": None,
        "BackgroundSelectedHighContrast": None,
        "Border": None,
        "RowBorder": None,
        "BorderOut": None,
        "Shadow": None,
        "Text": None,
    },
    "Tab": {
        "Background": None,
        "BackgroundSelected": None,
        "BackgroundActive": None,
        "BackgroundSelectedAndActive": None,
        "Text": None,
        "TextActive": None,
    },
    "SubTab": {"Background": None, "BackgroundSelected": None, "Text": None},
    "Banner": {
        "FirstBackground": None,
        "FirstBorder": None,
        "FirstText": None,
        "FirstVariantBackground": None,
        "FirstVariantBorder": None,
        "FirstVariantText": None,
        "SecondBackground": None,
        "SecondBorder": None,
        "SecondText": None,
    },
    "Home": {
        "Background": None,
        "CellBackground": None,
        "CellBackgroundActive": None,
        "CellText": None,
        "CellTextActive": None,
        "CellTextExternal": None,
        "CellTextExternalActive": None,
    },
    "Atom": {
        "Unknown": None,
        "Text": None,
        "AlkaliMetal": None,
        "AlkaliEarthMetal": None,
        "Lanthanide": None,
        "Actinide": None,
        "TransitionMetal": None,
        "PostTransitionMetal": None,
        "Metalloid": None,
        "Halogen": None,
        "ReactiveNonmetal": None,
        "NobleGas": None,
        "TableLines": None,
        "Highlight": None,
        "Background": None,
    },
}

# Role export name -> theme key, mirroring color_roles.cpp.
# The special value "Toolbar" maps to the group value "Toolbar" ("").
ROLE_PALETTE_KEYS = {
    "Background": "Background.Hard",
    "AppBackground": "Background.Apps",
    "SecondaryBackground": "Background.AppsSecondary",
    "PrimaryText": "PrimaryText",
    "SecondaryText": "SecondaryText",
    "AccentText": "AccentText",
    "Toolbar": "Toolbar",
    "MenuBackground": "SubMenu.Background",
    "Button": "Button.Background",
    "ButtonSelected": "Button.BackgroundSelected",
    "Selection": "List.CellBackgroundSelected",
    "Border": "List.CellBorder",
    "Control": "Control.Enabled",
    "GraphCurve": "Probability.Curve",
    "GraphGrid": "Grid.PrimaryLine",
    "GraphAxis": "Grid.SecondaryLine",
}

HEX_DIGITS = set("0123456789abcdefABCDEF")


def expected_paths(schema=None):
    """Flatten the schema into the list of valid theme color paths.

    "Toolbar" is the path of the group value (key ""), while "Toolbar.Text"
    is the path of its Text member.
    """
    schema = THEME_COLOR_KEYS if schema is None else schema
    paths = []
    for name, members in schema.items():
        if members is None:
            paths.append(name)
            continue
        for member in members:
            paths.append(name if member == "" else f"{name}.{member}")
    return paths
