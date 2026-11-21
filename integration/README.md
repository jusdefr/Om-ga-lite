# Intégration dans Omega

## Contenu

- `omega_lite_settings.patch` : diff des fichiers **suivis** d'Omega, généré avec
  `scripts/build_integration_patch.py` contre le commit `b06f3a1` (branche
  `master` d'Omega au moment de la génération). Il touche sept fichiers :
  - `apps/main.cpp` : appel de `OmegaLite::Colors::loadAtStartup()` au démarrage ;
  - `apps/settings/Makefile` : inclusion du module ;
  - `apps/settings/main_controller.h/.cpp` : membre du contrôleur et branche
    d'ouverture de la page ;
  - `apps/settings/main_controller_prompt_{none,beta,update}.cpp` : entrée de
    menu, ajoutée **en dernier** pour ne pas décaler les index utilisés par la
    cellule pop-up et la cellule À propos.
- Les traductions ne sont **pas** dans le patch : les fichiers `.i18n` d'Omega
  mélangent CRLF et LF, ce qui rend un hunk de fin de fichier fragile.
  `scripts/add_i18n_messages.py` ajoute la ligne `CustomColors` avec le bon
  saut de ligne, de façon idempotente.
- `scripts/apply_integration.sh` fait les trois étapes (copie du module, copie du
  thème dans `themes/themes/local/`, application du patch, traductions) et
  vérifie le patch avant de l'appliquer.

## Vérification effectuée

Le 21 novembre 2026, sur un clone neuf d'Omega (`--depth 1`, `master`) :

```
Translations: 8 file(s) updated, 0 already present
Omega Lite applied to /tmp/omega-verify
```

`git apply --check` passe, les 7 fichiers sont modifiés comme prévu, et
`main_controller.cpp` contient bien la branche `I18n::Message::CustomColors`.

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
