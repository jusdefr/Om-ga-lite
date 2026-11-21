# Omega Lite

Personnalisation des couleurs d'Omega depuis l'application **Paramètres** de la
calculatrice, avec aperçu en direct et sauvegarde persistante.

---

## Cible : NumWorks N0120 — à lire avant tout

Le projet est conçu **pour la NumWorks N0120** (édition lycée 120, STM32H725),
la machine qui tourne actuellement sous Epsilon 26.4.0.

**Omega Lite n'est pas installable sur N0120, et ce dépôt ne fournit aucun
firmware prêt à flasher.** Ce n'est pas une limite du code : NumWorks verrouille
l'installation de firmwares tiers depuis Epsilon 16, et aucune méthode publique
n'existe pour N0115/N0120. Omega lui-même ne compile que pour N0100/N0110
(voir [docs/COMPATIBILITY.md](docs/COMPATIBILITY.md), sources à l'appui).

Ce que ce dépôt contient donc : le module complet, testé sur station de travail,
intégrable dans un Omega que tu compiles pour **N0110 ou le simulateur**. Aucune
étape de ce projet ne contourne la signature, le bootloader ou une protection
matérielle, et aucune ne le fera.

## État réel du projet

| Élément | État |
|---|---|
| Moteur de couleurs, 16 rôles, HEX + RGB | Implémenté |
| Aperçu en direct (vue calculatrice) | Implémenté, rendu vérifié hors firmware |
| Persistance via `Ion::Storage` | Implémenté |
| Sauvegarde / annulation / restauration | Implémenté |
| Thème `themes/omega_lite.json` | Validé par le moteur de thèmes d'Omega |
| Palette dynamique, tous les écrans Omega | Implémentée (16 rôles → 102 clés) |
| Tests hôtes C++ | **44 fonctions, 16 893 vérifications, 0 échec** |
| Tests Python (thème, outils, palette runtime) | **38 tests, 0 échec** |
| Vérification syntaxique contre les en-têtes Omega | **14/14 sources, C++11, 0 erreur** |
| Patch d'intégration (18 fichiers) | Régénéré et appliqué sur un clone propre |
| **Compilation complète du simulateur** | **Réussie** : `epsilon.bin` lié, apps `calculation` + `settings` |
| Compilation ARM / image `.dfu` | **Non réalisée ici** (chaîne ARM absente) |
| Installation sur N0120 | **Impossible aujourd'hui** (verrou constructeur) |

## Fonctionnalités

- **16 couleurs personnalisables** : fond général, fond des applications, fond
  secondaire, texte principal, secondaire, accentué, barre d'outils, menus,
  boutons, boutons sélectionnés, sélection, bordures, contrôles, courbes,
  grille et axes des graphiques.
- **Édition HEX et RGB** : `#RRGGBB` au nibble près, ou canaux 0–255.
- **Aperçu en direct** : barre d'outils, menu avec sélection, textes, contrôle,
  boutons et graphique, redessinés à chaque changement.
- **Annulation** : rien n'est écrit avant « Enregistrer ».
- **Persistance** : enregistrement `omegalite.clr` dans le stockage Ion, rechargé
  au démarrage (voir le format dans [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md)).
- **Restauration** : le bouton « Restaurer » remet la palette du thème dans
  l'aperçu (c'est un brouillon) ; « Annuler » réaffiche la palette enregistrée,
  « Enregistrer » écrit le résultat.
- **Palette globale** : les couleurs choisies s'appliquent à **tous** les écrans
  d'Omega, pas seulement à la page de personnalisation.
- **Robustesse** : enregistrement corrompu ou tronqué → retour aux couleurs du
  thème, jamais de plantage ni de valeur partielle.

## Démarrage rapide

```sh
# 1. Vérifier le thème (structure Omega + rôles personnalisables)
python3 scripts/validate_theme.py --check-roles themes/omega_lite.json

# 2. Aperçu du thème en PNG 320x240 + ratios de contraste
python3 scripts/preview_palette.py themes/omega_lite.json \
    --output build/preview/omega_lite.png

# 3. Tests du cœur portable (aucune dépendance)
make -C tests/host

# 4. Tests des outils thème
python3 -m unittest discover -s tests/python -v
```

Intégration dans un Omega :

```sh
git clone --recursive https://github.com/Omega-Numworks/Omega.git
scripts/apply_integration.sh /path/to/Omega
make -C /path/to/Omega -j4 THEME_REPO=local THEME_NAME=omega_lite
```

Détails, prérequis et compilation dans [docs/DEVELOPMENT.md](docs/DEVELOPMENT.md).

## Structure

```
apps/settings/custom_colors/   module C++ (cœur portable + vues escher + Ion)
  color_rgb.*                  conversion HEX/RGB, RGB565, validation
  color_roles.*                les 16 rôles et leur clé de thème Omega
  color_scheme.*               palette courante + valeurs par défaut
  color_overrides.*            format binaire des surcharges (magic, CRC)
  color_store.h                interface de stockage + implémentation mémoire
  color_settings.*             chargement, transaction, sauvegarde
  ion_color_store.*            enregistrement Ion « omegalite.clr »
  custom_colors_runtime.*      chargement au démarrage
  palette_apply.*              applique les 16 rôles à la palette Omega
  custom_colors_controller.*   page « Personnaliser les couleurs »
  color_editor_controller.*    édition d'un rôle
  color_editor_view.*          vue d'édition (aperçu intégré)
  preview_renderer.*           rendu de l'aperçu, partagé
  custom_colors_cells.*        cellules de liste
  palette_defaults_generated.h défauts 8 bits générés depuis le thème
integration/                   patch des fichiers Omega + note de licence
themes/omega_lite.json         thème au format Omega
scripts/                       validateur, aperçu, générateurs, intégration
tests/host/                    tests C++ sans firmware
tests/python/                  tests du thème et des outils
docs/                          architecture, compatibilité, développement
```

## Tests et intégration continue

`.github/workflows/validate-theme.yml` exécute à chaque push : validation du
thème, vérification que les défauts générés sont à jour, tests Python, rendu de
l'aperçu avec seuil de contraste, et compilation/exécution des tests C++ hôtes.

## Limites connues

1. **N0120 non flashable** — voir ci-dessus, c'est la limite principale.
2. **Pas de compilation ARM dans cet environnement** : la chaîne
   `arm-none-eabi` et les sous-modules d'Omega ne sont pas installés. Le code a
   été vérifié contre les en-têtes réels (`scripts/check_omega_syntax.sh`), pas
   lié en firmware.
3. **Compilation ARM non testée** : le simulateur Linux a été compilé et lié
   ici, mais `arm-none-eabi-gcc` n'est pas installé ; aucune image `.dfu` n'a
   donc été produite ni vérifiée.
4. **Couleurs quantifiées en RGB565** : `KDColor` stocke 16 bits. Le HEX
   enregistré est exact, l'écran arrondit au dessin ; les paires proches
   peuvent se confondre.
   reste de Paramètres suit la langue choisie. Seul le titre du menu est traduit
   (8 langues).

## Licences

- Le code original de ce dépôt (module, outils, tests, docs) : Apache 2.0, voir
  [LICENSE](LICENSE).
- `themes/omega_lite.json` et `integration/` dérivent d'Omega
  (CC BY-NC-SA 4.0, utilisation non commerciale). Détail dans
  [integration/README.md](integration/README.md).
- NumWorks et Epsilon sont des marques et œuvres de NumWorks SAS ; Omega est un
  projet communautaire distinct. Ce dépôt ne redistribue aucun binaire
  constructeur.
