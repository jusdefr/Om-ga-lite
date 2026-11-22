# Omega Lite sur cette branche

Cette branche `master` est un arbre **Omega** (commit `b06f3a1`, Omega 2.0.6,
Epsilon 15.5.0) avec le module **Omega Lite** déjà appliqué : palette runtime
générée depuis `themes/themes/local/omega_lite.json`, page « Personnaliser les
couleurs » dans Paramètres, et persistance Ion `omegalite.clr`.

## Cible réelle de cette branche

| Cible | État |
|---|---|
| N0110 (firmware ARM) | Compilable (`make`), image non signée : flashable seulement sur une N0110 déverrouillée |
| Simulateur Linux | Compilable et exécutable |
| **N0120 / Epsilon 26.4.0** | **Ni compilable, ni installable** — voir ci-dessous |

## N0120 et Epsilon 26.4.0 : état vérifié le 22 novembre 2026

1. **Aucune source 26.4.0.** Le dépôt public `numworks/epsilon` s'arrête au tag
   `23.2.3` ; il n'existe aucun tag 24.x, 25.x ni 26.x. On ne peut donc pas
   compiler Epsilon 26.4.0, ni avec Omega, ni sans.
2. **Le noyau authentifié refuse les images non signées.** Depuis les firmwares
   24.x, une image recompilée est rejetée avec « OFFICIAL UPDATE REQUIRED »
   (issue #2347), y compris sur N0110. La N0120 n'a jamais eu de version
   déverrouillable (issue #2358).
3. **Le mode sombre est officiel depuis Epsilon 26.3** (29 juillet 2026) :
   Paramètres → Accessibilité. C'est la seule voie disponible pour changer les
   couleurs sur une N0120 à jour, sans rien flasher.

Conclusion : ce dépôt ne peut pas produire de firmware pour une N0120 en
26.4.0, et ne tente aucun contournement de signature ou de bootloader.

## Construire

```sh
# N0110
make -j4 THEME_REPO=local THEME_NAME=omega_lite

# Simulateur Linux
make -j4 PLATFORM=simulator EPSILON_APPS="calculation settings" \
    THEME_REPO=local THEME_NAME=omega_lite
```

Vérifié sur cet arbre : les 14 sources de `apps/settings/custom_colors/`
passent une compilation syntaxique `-std=c++11` contre les en-têtes réels
(`escher`, `kandinsky`, `ion`, `poincare`).
