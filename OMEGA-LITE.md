# Omega Lite — cible N0120

Cette branche est un arbre **Omega** (commit `b06f3a1`, Omega 2.0.6,
Epsilon 15.5.0) avec le module **Omega Lite** appliqué : page « Personnaliser
les couleurs » dans Paramètres, palette runtime générée depuis
`themes/themes/local/omega_lite.json`, persistance Ion `omegalite.clr`.

## 1. Cible N0120 dans les sources publiques d'Epsilon

Le dernier code source public de NumWorks est le tag **23.2.3**
(`numworks/epsilon`). Il contient bien une cible N0120 :

| Élément | Fichier public | Contenu |
|---|---|---|
| Plateforme | `build/platform.device.n0120.mak` | PCB 4.19, `SIGNATURE_INDEX = 1`, `TOOLCHAIN = arm-gcc-m7h` |
| Cibles | `build/targets.device.n0120.{kernel,bootloader,userland,flasher,bench}.mak` | dérivées de `targets.device.dual_slots.mak` (2 slots, 2 paires kernel+userland) |
| Debug | `build/device/openocd.n0120.cfg` | configuration SWD N0120 |
| Carte | `ion/src/device/include/n0120/config/` | `board.h` (flash externe 8 Mio @ `0x90000000`, slots A/B, kernel 64 Kio, data, userland), `usb.h`, `otg.h`, `timing.h`, `internal_flash.h`, `external_flash.h` |
| Drivers | `ion/src/device/shared/{drivers,flash,usb,regs}/` + `ion/src/device/userland/flash/*.ld` | drivers partagés et scripts de link userland |
| Chaîne | `build/toolchain.arm-gcc-m7h.mak` | `-mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard` |

## 2. Ce qui manque pour obtenir une image N0120

Le sous-module `ion/src/device/epsilon-core` (dépôt `numworks/epsilon-core`)
est **privé** : `git ls-remote` demande une authentification et l'API GitHub
répond 403. Or `ion/src/device/Makefile` en dépend directement :

- `-Iion/src/device/epsilon-core/device/include/$(MODEL)` et
  `-Iion/src/device/epsilon-core/device` ;
- `-include ion/src/device/epsilon-core/device/$(FIRMWARE_COMPONENT)/Makefile` ;
- kernel et bootloader : `kernel_shared.ld`, `kernel_A.ld`, `kernel_B.ld`,
  `bootloader_flash.ld` (via `build/targets.device.kernel.mak` et
  `targets.device.bootloader.mak`).

## 3. Tentative de build N0120 (22 novembre 2026)

Environnement : `arm-none-eabi-gcc` 13.2.1 (paquets Ubuntu extraits sous
`/tmp`, wrapper `-B` pour le sysroot newlib), freetype 2.13 + libpng 1.6 pour
l'outil hôte `kandinsky/fonts/rasterizer`, modèle `MODEL=n0120`.

Résultat : la chaîne ARM et l'outil de rasterisation passent, puis la
compilation s'arrête sur les fichiers fournis par le sous-module privé :

```
ion/src/device/include/n0120/config/serial_number.h:4: fatal error: regs/regs.h: No such file or directory
ion/src/device/shared/usb/calculator.h:6: fatal error: drivers/usb.h: No such file or directory
```

Les scripts de link du kernel (`kernel_shared.ld`…) étant eux aussi dans
`epsilon-core`, aucun `.elf` kernel ni `.dfu` N0120 ne peut être produit à
partir des sources publiques.

## 4. Installation

Même avec une image N0120 compilée, l'installation resterait impossible :

1. le noyau authentifié refuse une image non signée (« OFFICIAL UPDATE
   REQUIRED », issue #2347), y compris sur N0110 depuis les firmwares 24.x ;
2. la N0120 n'a jamais eu de version déverrouillable (issue #2358) ;
3. aucun tag 24.x/25.x/26.x n'est publié : Epsilon 26.4.0 n'est pas compilable
   du tout ;
4. le mode sombre officiel existe depuis Epsilon 26.3 (Paramètres →
   Accessibilité), c'est la voie disponible sur une N0120 à jour.

Aucune étape de ce dépôt ne contourne la signature, le bootloader ou une
protection matérielle.

## 5. Construire ce qui est constructible

```sh
# N0110 (cible réelle d'Omega)
make -j4 THEME_REPO=local THEME_NAME=omega_lite

# Simulateur Linux
make -j4 PLATFORM=simulator EPSILON_APPS="calculation settings" \
    THEME_REPO=local THEME_NAME=omega_lite
```

Vérifié sur cet arbre : les 14 sources de `apps/settings/custom_colors/`
passent une compilation syntaxique `-std=c++11` contre les en-têtes réels
(`escher`, `kandinsky`, `ion`, `poincare`).
