# Wargame AKA — port PC/SDL2

Fait tourner le **vrai code du jeu** (`main/game/*`, `main/core/*`,
`main/language.cpp`, `main/difficulty.cpp` — rien de modifié) sur PC, dans
une fenêtre SDL2, pour voir et jouer sans flasher l'ESP32 à chaque essai.

Seule la couche « matériel » (boutons, écran, NVS) est remplacée par ce
dossier. Le firmware réel n'est pas touché par ces fichiers.

## Binaire Linux fourni (`bin/wargame_pc`)

Un exécutable **Linux x86-64** précompilé est inclus dans ce dossier —
pratique pour le partager directement avec quelqu'un sous Linux sans lui
demander de compiler. Il lui faut seulement la bibliothèque SDL2 (pas les
en-têtes de dev, juste la lib partagée) :
```
sudo apt install libsdl2-2.0-0     # Debian/Ubuntu — souvent déjà présente
```
Puis lancer directement :
```
./bin/wargame_pc
```
Ce binaire ne tourne **pas** sous Windows ni macOS (format ELF Linux) — pour
ces plateformes, ou si tu modifies le code, recompile (voir plus bas).

## Compiler (si tu modifies le code, ou sous Windows/macOS)

**Linux / WSL :**
```
sudo apt install libsdl2-dev
cd tools/pc_sdl
make
./bin/wargame_pc
```

**macOS (Homebrew) :**
```
brew install sdl2
cd tools/pc_sdl
make
./bin/wargame_pc
```

**Windows (MSYS2/MinGW64)** — le plus simple si tu restes sous Windows :
```
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-SDL2 make
cd tools/pc_sdl
make
./bin/wargame_pc.exe
```
(Sinon, WSL + les instructions Linux ci-dessus fonctionnent aussi et évitent
d'installer MSYS2.)

## Commandes clavier

| AKA          | Clavier |
|--------------|---------|
| D-pad        | Flèches |
| A            | Z       |
| B            | X       |
| C            | C       |
| D            | V       |
| L1 / R1      | Q / W   |
| MENU         | Entrée  |
| RUN          | Tab     |
| Quitter      | Échap / fermer la fenêtre |

## Fichiers de jeu (sprites, illustrations)

Le port lit `/sdcard/Wargame_Aka/...` comme le firmware réel (même chemin en
dur, `SD_GAME_DIR` dans `core/filesystem.h`). Sur PC, ce chemin doit donc
exister sur le disque :

- **Linux/macOS/WSL** : `sudo mkdir -p /sdcard/Wargame_Aka` puis copier
  `sprites/` et `story/` dedans (les `.bin` déjà livrés pour la carte SD
  conviennent tels quels — même format).
- **Windows natif (MinGW, hors WSL)** : `/sdcard/...` n'existe pas comme
  chemin Windows valide ; le plus simple est de lancer via WSL, ou de créer
  un lecteur/dossier `C:\sdcard\Wargame_Aka\...` et d'adapter temporairement
  `SD_GAME_DIR` dans `core/filesystem.h` le temps du test PC (à ne pas
  committer, ce chemin doit rester `/sdcard/Wargame_Aka` pour la vraie
  console).

Sans ces fichiers, le jeu tourne quand même (repli automatique sur le rendu
couleur/glyphe existant, cf. `sprites.cpp`) — juste sans les images.

## Mode script (tests automatisés)

Pour rejouer un scénario sans clavier (utile en CI ou pour comparer un
rendu avant/après un changement) :
```
WARGAME_SCRIPT=mon_script.txt ./bin/wargame_pc
```
Fichier texte, une instruction par ligne :
```
WAIT 200                    # attendre N ms sans rien appuyer
PRESS A                     # une pression d'une frame (A, B, C, D, UP,
                             # DOWN, LEFT, RIGHT, L1, R1, MENU, RUN)
HOLD MENU 300                # maintenir une touche N ms (ex: l'aide MENU)
SCREENSHOT chemin.bmp        # capture le framebuffer tel quel
QUIT                         # ferme la fenêtre
```
À la fin du script (sans QUIT), le jeu repasse en clavier interactif normal.

## Limites connues

- **Pas de son** : toutes les fonctions `sfx_*` sont des no-op. Le son n'a
  pas été dans le périmètre de cette vérification (focus sur rendu +
  logique de jeu).
- **Pas de vraie combo retour-loader** (RUN+MENU) : sans effet sur PC,
  puisqu'il n'y a pas de partition loader à activer.
- Rendu logiciel (`SDL_RENDERER_SOFTWARE`) plutôt qu'accéléré — choix
  volontaire pour rester fiable même sans GPU (utile en CI/VM) ; sur une
  machine de dev normale, ça reste largement assez rapide pour un jeu à
  tour par tour.
