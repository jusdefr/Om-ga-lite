# Développement

## Prérequis

Pour le module et les outils (ce qui a été exécuté dans cet environnement) :

- `git`, `make`, `g++` (le code est compilé en **C++11**, comme Omega),
  `python3` ≥ 3.8.

Pour le simulateur Omega (exécuté ici) :

- `freetype2` et `libpng` (rasterizer de polices et `escher/image/inliner`) ;
- les en-têtes X11, Xext, Xrandr, Xi, Xcursor, Xinerama, Xss, xkbcommon et
  libjpeg (`SDL` et `platform_images`).

Pour le firmware ARM (non exécuté ici) : la chaîne `arm-none-eabi-gcc` et les
sous-modules d'Omega (`git clone --recursive`).

## 1. Récupérer les deux dépôts

```sh
git clone https://github.com/jusdefr/Om-ga-lite.git
git clone --recursive https://github.com/Omega-Numworks/Omega.git
```

## 2. Appliquer Omega Lite à Omega

```sh
cd Om-ga-lite
scripts/apply_integration.sh ../Omega
```

Le script :

1. copie `apps/settings/custom_colors/` dans Omega ;
2. copie `themes/omega_lite.json` dans `themes/themes/local/` ;
3. copie `scripts/generate_runtime_palette.py` (appelé par le `Makefile`
   d'escher patché) ;
4. vérifie (`git apply --check`) puis applique `integration/omega_lite_settings.patch`
   (18 fichiers) ;
5. ajoute la traduction `CustomColors` aux 8 langues via
   `scripts/add_i18n_messages.py`.

Il échoue explicitement si un fichier d'Omega a changé de structure : c'est
volontaire, un patch silencieusement appliqué au mauvais endroit serait pire.
Dans ce cas, relancer `python3 scripts/build_integration_patch.py ../Omega --write-patch`
après avoir adapté les ancres, puis relire le diff.

Retour arrière : `git -C ../Omega checkout -- apps` et supprimer
`../Omega/apps/settings/custom_colors`.

## 3. Compiler

Avec le thème Omega Lite (la palette de référence) :

```sh
make -C ../Omega -j4 THEME_REPO=local THEME_NAME=omega_lite
```

Le firmware par défaut d'Omega cible **N0110**. Pour le simulateur :

```sh
make -C ../Omega -j4 PLATFORM=simulator EPSILON_APPS="calculation settings" \
    THEME_REPO=local THEME_NAME=omega_lite
```

**Résultat obtenu** (Omega `b06f3a1`, 21 novembre 2026) : compilation et
édition de liens réussies, `output/release/simulator/linux/epsilon.bin`
produit, les 14 objets du module `custom_colors` compilés. La machine de test
n'avait ni droits root ni en-têtes de développement ; `freetype`, `libpng` et
les en-têtes X11/jpeg ont été construits ou extraits sous `/tmp`, puis exposés
par `CPATH`, `LIBRARY_PATH`, `PKG_CONFIG_PATH` et `LD_LIBRARY_PATH`. Aucun
fichier d'Omega n'a été modifié pour cela.

La compilation **ARM** n'a pas été faite (chaîne `arm-none-eabi` absente) :
aucune image `.dfu` n'existe dans ce dépôt.

## 4. Tests

```sh
make -C tests/host                 # C++ portable, aucune dépendance
python3 -m unittest discover -s tests/python -v
python3 scripts/validate_theme.py --check-roles themes/omega_lite.json
python3 scripts/preview_palette.py themes/omega_lite.json \
    --output build/preview/omega_lite.png --min-contrast 3.0
```

Vérification syntaxique contre les en-têtes d'Omega (sans chaîne ARM) :

```sh
scripts/check_omega_syntax.sh ../Omega
```

C'est cette commande qui a validé les 14 sources du module contre les en-têtes
réels (`escher`, `kandinsky`, `ion`, `poincare`), en C++11, avec le `palette.h`
runtime généré depuis le thème du projet.

## 5. Modifier le thème

1. Éditer `themes/omega_lite.json` (HEX **sans** `#`, six chiffres).
2. `python3 scripts/generate_palette_defaults.py` pour régénérer
   `apps/settings/custom_colors/palette_defaults_generated.h`.
3. `python3 scripts/validate_theme.py --check-roles themes/omega_lite.json`.
4. `python3 scripts/preview_palette.py ...` pour contrôler rendu et contraste.
5. `make -C tests/host` et les tests Python (le généré est comparé au thème).

Le jeu de clés attendu est celui d'Omega 2.0 (`scripts/palette_schema.py`). Une
clé manquante casse la compilation du firmware, une clé inconnue est inutilisée :
les deux sont des erreurs pour le validateur.

## 6. Ajouter un rôle personnalisable

1. Ajouter l'entrée dans `Role` (`color_roles.h`) et dans la table
   `color_roles.cpp` (identifiant stable, libellé, clé de palette).
2. Ajouter la même entrée dans `ROLE_PALETTE_KEYS` (`scripts/palette_schema.py`).
3. Régénérer les défauts (étape 5).
4. Les tests vérifient automatiquement que les deux tables C++/Python restent
   identiques et que chaque rôle est présent dans le thème et dessiné dans
   l'aperçu.

Le format d'enregistrement n'a pas de taille fixe : ajouter un rôle reste
compatible avec les réglages déjà sauvegardés (les rôles absents prennent la
valeur du thème).

## 7. Dépannage

| Symptôme | Cause probable |
|---|---|
| Le patch ne s'applique pas | Omega a changé : relire l'ancre, régénérer le patch |
| « Echec de l'enregistrement » | stockage Ion plein ou enregistrement impossible : les couleurs restent celles de l'aperçu, rien n'est écrit |
| Les couleurs reviennent au thème au redémarrage | enregistrement absent, tronqué ou checksum invalide : c'est le comportement voulu, les défauts sont sûrs |
| Le HEX affiché diffère légèrement de l'écran | l'écran est en RGB565 ; le HEX affiché est la valeur enregistrée, l'écran quantifie au dessin |
| Rien ne s'installe sur N0120 | verrou constructeur, voir [COMPATIBILITY.md](COMPATIBILITY.md) ; ce n'est pas un bug du projet |

## 8. Conventions

- **C++11** (norme d'Omega), style Epsilon/Omega : deux espaces, accolades
  ouvrantes en fin de ligne, préfixe `m_` pour les membres, gardes d'en-tête
  `OMEGA_LITE_..._H`. Une fonction `constexpr` ne peut contenir qu'un seul
  `return` ; un membre `static constexpr` utilisé par adresse (ou indexé) exige
  une définition hors classe.
- Le cœur portable (`color_rgb`, `color_roles`, `color_overrides`,
  `color_scheme`, `color_store`, `color_settings`) ne doit **jamais** inclure
  escher, kandinsky ou Ion : c'est ce qui garantit qu'il reste testable sur un
  ordinateur.
- Toute conversion vers `KDColor` passe par `palette_bridge.h`, un seul point de
  contact avec le rendu.
- Pas de nouvelle dépendance : les outils Python n'utilisent que la bibliothèque
  standard.
