# Compatibilité matérielle — N0120

Document de référence du projet. Toutes les affirmations ci-dessous sont
vérifiables via les sources listées en fin de page ; rien n'est supposé.

## 1. La N0120 n'est pas une N0110 plus rapide

| | N0100 | N0110 | N0115 | N0120 |
|---|---|---|---|---|
| MCU | STM32F412 (Cortex-M4) | STM32F730 (Cortex-M7) | STM32F730 | **STM32H725 (Cortex-M7)** |
| Fréquence | 100 MHz | 216 MHz | 216 MHz | **550 MHz** |
| RAM | 256 Ko | 256 Ko | 256 Ko | **564 Ko** |
| Flash interne | 1 Mo | 64 Ko | 64 Ko | **512 Ko** |
| Flash externe | — | 8 Mo | 8 Mo | **8 Mo** |
| Connecteur | micro-USB | micro-USB | USB-C | **USB-C** |
| Firmware tiers | oui | oui (avant le verrou) | non | **non** |

Conséquences concrètes pour un firmware :

- **Carte mémoire différente** : la N0120 a 512 Ko de flash interne (contre
  64 Ko), et des zones de RAM supplémentaires que le noyau officiel n'expose
  pas (documenté par le portage Xcas/KhiCAS).
- **Drivers non publics** : NumWorks n'a pas publié les drivers ni les schémas
  de la N0120, contrairement aux N0100/N0110 de l'époque open source.
- Un binaire compilé pour STM32F730 ne démarre pas sur STM32H725 : ce ne sont
  ni le même cœur de périphériques, ni le même mapping mémoire, ni le même
  contrôleur USB.

## 2. Omega ne prend pas en charge la N0120

Vérifié directement dans le dépôt `Omega-Numworks/Omega` (commit `b06f3a1`,

- `build/device/openocd.n0100.cfg` et `build/device/openocd.n0110.cfg` :
  **aucune** configuration `n0120` ni `n0115`.
- `Makefile` racine : le modèle n'est testé que pour `n0110` et `bootloader`
  (`ifeq (${MODEL},n0110)`), il n'existe pas de cible N0120.
- Il n'y a donc ni mapping mémoire, ni driver d'écran/clavier, ni cible de
  compilation N0120 dans Omega.

Dans Omega d'origine, le thème est appliqué **à la compilation** : `Palette::`
est un ensemble de constantes `constexpr` générées par
`themes/themes_manager.py`. Omega Lite remplace ce générateur par une version
runtime (`scripts/generate_runtime_palette.py`) et convertit les quelques
usages qui exigeaient une constante de compilation ; c'est une modification du
code d'Omega, pas un contournement de la machine.

## 3. Pourquoi aucun firmware tiers ne démarre sur N0120

1. Depuis **Epsilon 16**, NumWorks ne permet plus de flasher un firmware
   personnalisé ; le chargeur vérifie la signature de l'image.
2. Les modèles **N0115 et N0120 sont sortis après ce verrou** : ils n'ont jamais
   eu de version déverrouillable.
3. Le guide communautaire Omega l'écrit noir sur blanc : pour N0115–N0120,
   « You can't install a Custom OS on it ».
4. L'équipe NumWorks le confirme (issue #2358) : « N0115 and N0120 models have
   been released after the lock update and there is no known method to flash a
   custom version of Epsilon on them ».
5. Aucun jailbreak public, aucun firmware tiers et aucun utilitaire de flash
   N0120 n'existe à ce jour. Les seules approches documentées passent par
   l'ouverture de la machine et le port SWD — hors périmètre de ce projet.

## 4. Ce que ce projet affirme, et ce qu'il n'affirme pas

**Affirmé et vérifié :**

- Le module compile syntaxiquement contre les en-têtes réels d'Omega
  (`scripts/check_omega_syntax.sh`, 14/14 sources, C++11, 0 erreur).
- Le thème est accepté par le moteur de thèmes d'Omega, qui en produit un
  `palette.h` valide.
- Les tests du cœur portable passent (44 fonctions, 16 893 vérifications) et
  les tests Python aussi (38 tests).
- Le patch d'intégration (18 fichiers) s'applique sur un clone neuf d'Omega.
- Le **simulateur Linux complet** se compile et se lie avec Omega Lite
  (`epsilon.bin`), apps `calculation` et `settings` incluses.

**Non affirmé :**

- Qu'Omega Lite démarre sur une N0120.
- Qu'un firmware ARM fonctionne : seule la compilation du **simulateur** a été
  réalisée, aucune image `.dfu` n'a été produite.
- Qu'une méthode d'installation N0120 existe ou soit en préparation.

## 5. Ce qui changerait la conclusion

La conclusion ne tomberait que si l'un de ces éléments devenait vrai :

1. NumWorks publie un mode développeur signé ou une procédure d'installation
   officielle pour N0120 ;
2. un chargeur alternatif N0120 documenté et reproductible apparaît (avec
   drivers et schémas) ;
3. le projet est porté sur une N0110/N0100 déverrouillable, ou exécuté dans le
   simulateur Omega.

Dans l'attente, la cible N0120 est traitée dans ce dépôt comme une **cible de
conception** (rôles, ergonomie 320×240, stockage, RAM), pas comme une cible
d'installation.

## Sources

Consultées le 21 novembre 2026 :

- Guide Omega, modèles et matériel : <https://guide.getomega.dev/docs/how-does-it-work>
- Guide Omega, installation et modèles verrouillés : <https://guide.getomega.dev/docs/intro>
- NumWorks Epsilon, issue #2358 « I can't build firmware for the N0120 » :
  <https://github.com/numworks/epsilon/issues/2358>
- Dépôt Omega (Makefile, configurations openocd, moteur de thèmes) :
  <https://github.com/Omega-Numworks/Omega>
- Portage KhiCAS, limites RAM/flash N0120 :
  <https://xcas.univ-grenoble-alpes.fr/nw/khicasnw.html>
- Wikipédia, tableau comparatif N0100/N0110/N0115/N0120 :
  <https://en.wikipedia.org/wiki/NumWorks>
