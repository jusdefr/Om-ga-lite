# Architecture

## Vue d'ensemble

Omega Lite ajoute une page « Personnaliser les couleurs » aux Paramètres
d'Omega. Le module vit dans `apps/settings/custom_colors/` et se branche sur
trois mécanismes existants d'Omega/Epsilon :

| Besoin | Mécanisme réutilisé |
|---|---|
| Couleurs du thème | `scripts/generate_runtime_palette.py` → `Palette::` (variables) |
| Persistance | `Ion::Storage` (enregistrement, pas de système de fichiers) |
| Navigation | `ViewController`, `StackViewController`, `SelectableTableView` |
| Rendu | `KDContext`, `KDFont`, `KDColor` (kandinsky) |

## Une palette runtime, pour toute la calculatrice

Omega fige les couleurs à la compilation : `themes/themes_manager.py` transforme
les clés du JSON en `constexpr static KDColor` dans `escher/palette.h`. Une
valeur `constexpr` ne peut pas changer à l'exécution.

Omega Lite remplace ce générateur par `scripts/generate_runtime_palette.py`, qui
produit deux fichiers :

- `escher/palette.h` : `Palette::` devient un ensemble de `static KDColor`
  mutables (les 102 couleurs du thème), et `ThemeDefaults` conserve les valeurs
  du thème en `constexpr`, plus `Palette::resetToThemeDefaults()` ;
- `escher/palette_definitions.inc`, inclus par `escher/src/palette.cpp` :
  définitions des membres, des tableaux (`DataColor`, `DataColorLight`,
  `AtomColor`) et de la fonction de remise à zéro.

```
themes/omega_lite.json
        │
        ├── scripts/generate_palette_defaults.py ──► palette_defaults_generated.h
        │                                             (16 RgbColor 8 bits)
        └── scripts/generate_runtime_palette.py ───► escher/palette.h + .inc
                                                      (102 couleurs mutables)
```

Les 16 rôles personnalisables sont projetés sur les 102 clés par
`apps/settings/custom_colors/palette_apply.cpp`. C'est ce pont qui fait qu'un
changement de couleur repeint **tous** les écrans Omega (menus, calculs,
graphiques, Python, réglages), et pas seulement la page de personnalisation.
La palette est appliquée à trois moments : au démarrage
(`loadAtStartup()`), après « Enregistrer » et après « Annuler ».

Les défauts sont lus depuis le JSON en **8 bits**, pas depuis `Palette::` :
`KDColor` stocke du RGB565, donc relire les constantes afficherait `#081020`
au lieu de `#0B1220`. L'utilisateur voit exactement la valeur du thème, et
l'écran applique sa quantification au moment du dessin.

### Ce que la migration impose

Omega compile en **C++11**. Rendre `Palette::` mutable casse les rares endroits
qui exigeaient une constante de compilation ; le patch les convertit :

| Fichier | Changement |
|---|---|
| `apps/probability/distribution_curve_view.*` | `constexpr k_backgroundColor` → `backgroundColor()` |
| `apps/solver/solutions_controller.*` | idem pour le fond des solutions |
| `escher/.../modal_view_empty_controller.*` | idem pour le fond vide |
| `apps/shared/banner_view.h` | `constexpr` retiré des couleurs de bandeau |
| `apps/code/python_text_area.cpp` | constantes de coloration → fonctions |
| `python/port/port.cpp` | `constexpr NamedColor pairs[]` → `const`, lu à l'appel |

Les tableaux de `ThemeDefaults` sont indexés par la boucle de remise à zéro :
en C++11 cet usage exige une définition hors classe, émise dans le `.inc`.

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
  se compile (en C++11, comme le firmware) et s'exécute avec g++. 44 fonctions
  couvrent HEX/RGB, RGB565, rôles, sérialisation, corruption, transactions,
  sauvegarde et restauration (dont l'annulation d'une restauration).
- **Python** (`tests/python`) : schéma du thème, cohérence entre la table C++
  des rôles et celle des outils, validation, aperçu (PNG décodé, chaque rôle
  présent), contraste, génération des défauts. Le générateur de palette runtime
  est testé en compilant et exécutant réellement sa sortie avec un `KDColor`
  factice (mutabilité, remise à zéro, `nextDataColor`).
- **Syntaxe** (`scripts/check_omega_syntax.sh`) : compile les 14 sources du
  module contre les en-têtes réels d'Omega, en C++11, avec le `palette.h`
  runtime. Cela détecte les erreurs d'API sans chaîne ARM ; ce n'est pas une
  image firmware.
- **Build complet** : le simulateur Linux a été compilé et lié avec Omega Lite
  (`EPSILON_APPS="calculation settings"`), voir
  [DEVELOPMENT.md](DEVELOPMENT.md).
