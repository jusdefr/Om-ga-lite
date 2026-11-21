# Architecture

## Vue d'ensemble

Omega Lite ajoute une page « Personnaliser les couleurs » aux Paramètres
d'Omega. Le module vit dans `apps/settings/custom_colors/` et se branche sur
trois mécanismes existants d'Omega/Epsilon :

| Besoin | Mécanisme réutilisé |
|---|---|
| Couleurs du thème | `themes/themes_manager.py` → `Palette::` (constantes) |
| Persistance | `Ion::Storage` (enregistrement, pas de système de fichiers) |
| Navigation | `ViewController`, `StackViewController`, `SelectableTableView` |
| Rendu | `KDContext`, `KDFont`, `KDColor` (kandinsky) |

## Deux palettes, volontairement

Omega fige les couleurs à la compilation : `themes_manager.py` transforme les
clés du JSON en `constexpr static KDColor` dans `escher/palette.h`. Une valeur
`constexpr` ne peut pas changer à l'exécution.

Omega Lite introduit donc une **palette runtime** pour les 16 rôles
personnalisables, et garde la palette `Palette::` comme source des valeurs par
défaut :

```
themes/omega_lite.json
        │
        ├── scripts/generate_palette_defaults.py ──► palette_defaults_generated.h
        │                                             (16 RgbColor 8 bits)
        └── themes/themes_manager.py (Omega) ───────► escher/palette.h
                                                      (constantes de rendu)
```

Les défauts sont lus depuis le JSON en **8 bits**, pas depuis `Palette::` :
`KDColor` stocke du RGB565, donc relire les constantes afficherait `#081020`
au lieu de `#0B1220`. L'utilisateur voit exactement la valeur du thème, et
l'écran applique sa quantification au moment du dessin.

**Limite assumée** : les pages Omega Lite (liste, éditeur, aperçu) dessinent avec
la palette runtime. Les autres écrans d'Omega continuent d'utiliser `Palette::`
(617 usages dans 163 fichiers). Les repeindre demande de remplacer ces lectures
par la palette runtime — un travail mécanique mais massif, qui n'est pas simulé
ici. Le projet ne prétend donc pas thémer toute la calculatrice.

## Chaîne de personnalisation

```
CustomColorsController (liste des 16 rôles + 3 actions)
        │ ouvre
        ▼
ColorEditorController ──► ColorEditorView ──► drawPreview()
        │                        │
        │                        └── lit ColorScheme à chaque dessin
        ▼
ColorSettings (transaction) ──► ColorScheme (16 RgbColor)
        │                              ▲
        │ sauvegarde                   │ défauts
        ▼                              │
ColorOverrides ──► ColorStore ──► palette_defaults_generated.h
                       │
                       ├── MemoryColorStore (tests hôtes)
                       └── IonColorStore  (calculatrice)
```

- **`ColorScheme`** : 16 couleurs courantes + 16 défauts. `overrideMask()`
  indique les rôles modifiés.
- **`ColorSettings`** : transaction. `beginEdit()` prend un instantané,
  `revertEdit()` le restaure, `save()` n'écrit que les différences.
  `load()` applique les surcharges ; un enregistrement absent ou invalide laisse
  les défauts en place.
- **`ColorOverrides`** : ensemble des rôles modifiés, sérialisé en un petit
  blob validé (magic, version, checksum).
- **`ColorStore`** : interface ; `MemoryColorStore` pour les tests,
  `IonColorStore` pour la machine.

## Format de l'enregistrement

Enregistrement Ion `omegalite.clr`, contenu little-endian :

| Offset | Taille | Contenu |
|---|---|---|
| 0 | 4 | magie `OLC1` |
| 4 | 1 | version du format (1) |
| 5 | 1 | nombre d'entrées N (0–16) |
| 6 | 2 | réservé, zéro |
| 8 | 4×N | entrées triées : rôle, R, G, B |
| 8+4N | 4 | checksum FNV-1a 32 des octets précédents |

Taille : 12 octets sans surcharge, 76 au maximum, pour un stockage Ion de
32 Kio. Ne stocker que les différences permet de changer la palette par défaut
dans une version future sans invalider les réglages enregistrés.

`Ion::Storage` ne permet pas de réécrire un enregistrement avec une taille
différente ; `IonColorStore::save()` détruit puis recrée l'enregistrement dans
ce cas, et vérifie le résultat. En cas d'échec, aucun enregistrement partiel
n'est laissé : le chargement suivant repart des défauts.

## Interface

- **Liste** (`CustomColorsController`) : `SelectableTableView` + cellules
  maison (`ColorRoleCell` : pastille, libellé, HEX ; `ActionCell` : actions).
  Hauteurs de lignes différentes pour les actions ; défilement géré par la vue.
- **Éditeur** (`ColorEditorController` + `ColorEditorView`) : vue unique,
  aperçu en haut, lignes HEX/R/V/B en dessous. Aucun champ texte : la saisie se
  fait au clavier (`←/→` pour choisir un chiffre hexadécimal, `+/-` pour le
  changer ; `←/→` ±1 et `+/-` ±16 sur les canaux). C'est utilisable sans
  clavier alphabétique et sans risque de saisie invalide.
- **Aperçu** (`preview_renderer.cpp`) : barre d'outils, menu avec sélection,
  textes, contrôle, boutons, graphique avec grille, axes et courbe. Il lit la
  `ColorScheme` à chaque dessin : modifier une couleur puis redessiner suffit.
  `scripts/preview_palette.py` en est le pendant hors firmware (PNG 320×240 et
  contrôle de contraste WCAG), ce qui permet de vérifier un thème sans machine.

## Intégration dans Paramètres

Trois points seulement :

1. une entrée de menu (`I18n::Message::CustomColors`) ajoutée **en fin** de
   `s_modelMenu`, pour ne pas décaler les index de la cellule pop-up et de la
   cellule À propos ;
2. un membre `CustomColorsController` dans `MainController` ;
3. une branche dans `MainController::handleEvent` qui empile la page.

Le module s'enregistre dans le build par `include apps/settings/custom_colors/Makefile`
et `app_settings_src += $(app_settings_custom_colors_src)`.

Le chargement au démarrage est un appel unique dans `ion_main`, après
`Poincare::Init()` : `OmegaLite::Colors::loadAtStartup()`.

Détail du patch : [integration/README.md](../integration/README.md).

## Tests

- **Hôte** (`tests/host`) : le cœur portable ne dépend ni d'escher ni d'Ion, il
  se compile et s'exécute avec g++. 42 fonctions couvrent HEX/RGB, RGB565,
  rôles, sérialisation, corruption, transactions, sauvegarde et restauration.
- **Python** (`tests/python`) : schéma du thème, cohérence entre la table C++
  des rôles et celle des outils, validation, aperçu (PNG décodé, chaque rôle
  présent), contraste, et génération des défauts.
- **Syntaxe** (`scripts/check_omega_syntax.sh`) : compile les 13 sources du
  module contre les en-têtes réels d'Omega avec `palette.h` généré par Omega.
  Cela détecte les erreurs d'API sans chaîne ARM ; ce n'est pas une image
  firmware.
