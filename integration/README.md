# Intégration dans Omega

## Contenu

- `omega_lite_settings.patch` : diff des fichiers **suivis** d'Omega, généré avec
  `scripts/build_integration_patch.py` contre le commit `b06f3a1` (branche
  `master` d'Omega au moment de la génération). Il touche **18 fichiers** :
  - `apps/main.cpp` : appel de `OmegaLite::Colors::loadAtStartup()` au démarrage ;
  - `apps/settings/Makefile` : inclusion du module ;
  - `apps/settings/main_controller.h/.cpp` : membre du contrôleur et branche
    d'ouverture de la page ;
  - `apps/settings/main_controller_prompt_{none,beta,update}.cpp` : entrée de
    menu, ajoutée **en dernier** pour ne pas décaler les index utilisés par la
    cellule pop-up et la cellule À propos ;
  - `escher/Makefile` : la règle de `palette.h` appelle
    `scripts/generate_runtime_palette.py` au lieu de `themes_manager.py` ;
  - `escher/src/palette.cpp` : inclut les définitions générées
    (`palette_definitions.inc`) ;
  - `escher/include/.../modal_view_empty_controller.h`,
    `apps/shared/banner_view.h`, `apps/probability/distribution_curve_view.*`,
    `apps/solver/solutions_controller.*`, `apps/code/python_text_area.cpp`,
    `python/port/port.cpp` : les rares usages qui exigeaient une constante de
    compilation sont convertis en accès runtime (voir
    [docs/ARCHITECTURE.md](../docs/ARCHITECTURE.md)).
- Les traductions ne sont **pas** dans le patch : les fichiers `.i18n` d'Omega
  mélangent CRLF et LF, ce qui rend un hunk de fin de fichier fragile.
  `scripts/add_i18n_messages.py` ajoute la ligne `CustomColors` avec le bon
  saut de ligne, de façon idempotente.
- `scripts/apply_integration.sh` fait toutes les étapes (copie du module, copie
  du thème dans `themes/themes/local/`, copie du générateur de palette runtime,
  application du patch, traductions) et vérifie le patch avant de l'appliquer.

## Vérification effectuée

Le 21 novembre 2026, sur un clone d'Omega `b06f3a1` :

```
Translations: 8 file(s) updated, 0 already present
Wrote integration/omega_lite_settings.patch (618 lines)
Omega checkout patched
```

`git apply --check` passe, les 18 fichiers sont modifiés comme prévu, et
`main_controller.cpp` contient bien la branche `I18n::Message::CustomColors`.

Le même arbre patché a ensuite été **compilé et lié** pour le simulateur Linux
(`PLATFORM=simulator EPSILON_APPS="calculation settings"`) : `epsilon.bin`
est produit, avec les 14 objets de `custom_colors`. Les dépendances manquantes
de la machine (freetype, libpng, en-têtes X11/jpeg) ont été fournies sous `/tmp`
sans modifier Omega.

## Licence du patch

Omega et Epsilon sont publiés sous **CC BY-NC-SA 4.0** (usage non commercial,
partage à l'identique). Ce patch contient des lignes de contexte issues de ces
fichiers : il est donc un travail dérivé, distribué sous la même licence
CC BY-NC-SA 4.0, et il n'est **pas** couvert par l'Apache 2.0 du reste du dépôt.
Les deux licences ne sont pas compatibles entre elles ; elles sont gardées dans
des répertoires distincts pour que cette frontière reste explicite.

Le module `apps/settings/custom_colors/`, les scripts, les tests et la
documentation sont du code original sous Apache 2.0. Ils n'incluent aucun code
d'Omega, seulement ses en-têtes publics au moment de la compilation.

NumWorks est une marque déposée de NumWorks SAS. Ce dépôt ne distribue ni
firmware constructeur, ni clé, ni binaire signé.
