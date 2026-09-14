# Wargame AKA

Wargame médiéval fantastique au tour par tour, sur grille hexagonale, pour
la console **Gamebuino AKA** (ESP32-S3, écran 320x240).

Le royaume d'Aubevent est envahi par la Horde Grise -- puis, plus profond
dans la campagne, par la menace bien plus ancienne qui tirait ses ficelles
depuis l'ombre. 30 cartes réparties en trois actes, trois niveaux de
difficulté, une armée qui vous suit (et que vous devez soigner et renforcer)
d'une bataille à l'autre.

## Fonctionnalités

- 8 types d'unités (Infanterie, Archer, Cavalerie, Mage, Catapulte, Héros,
  Sapeur, Griffon), chacune avec son rôle tactique.
- Grille hexagonale avec bonus défensifs de terrain (forêt, colline,
  château, remparts), rivières franchissables par pont uniquement, routes
  qui s'enchaînent en un vrai chemin plutôt que des motifs isolés.
- Campagne de 30 cartes en 3 actes : les 20 premières accessibles à toute
  difficulté, les 10 dernières (et la fin héroïque) réservées à la
  difficulté Difficile.
- 3 niveaux de difficulté (Facile / Normal / Difficile), verrouillée en
  début de campagne, avec un combo de déblocage pour les tests.
- Boutique entre deux batailles : soigner l'armée existante ou recruter de
  nouvelles unités avec les points de victoire gagnés.
- Bilingue FR/EN, sélectionnable à l'écran titre.
- Sprites 16x16 pour le terrain et les unités, illustrations de briefing
  aux moments clés de l'histoire.

## Structure du dépôt

- `main/` -- code du jeu (logique, rendu, campagne, IA, données de niveaux).
- `components/gamebuino/` -- bibliothèque bas niveau de la console AKA
  (écran, boutons, son, carte SD). Sous licence LGPL, voir plus bas.
- `tools/pc_sdl/` -- port PC/SDL2 du jeu, pour tester sans flasher l'ESP32.
  Un binaire Linux précompilé est fourni dans `tools/pc_sdl/bin/`.
- `tools/sim/` -- simulateur headless pour tester l'équilibrage de la
  campagne (bot automatisé qui rejoue les niveaux).

## Compiler pour la console

Projet ESP-IDF standard :
```
idf.py build
idf.py -p <port> flash
```

## Tester sur PC sans flasher

```
cd tools/pc_sdl
make
./bin/wargame_pc
```
Voir `tools/pc_sdl/README.md` pour les détails (contrôles clavier, mode
script pour rejouer un scénario, compilation sous Windows/macOS).

## Licence

Le code de ce projet (`main/`, `tools/`) est sous licence **GPLv3** -- voir
le fichier `LICENSE` à la racine.

La bibliothèque `components/gamebuino/` (écran, boutons, son, carte SD) est
une bibliothèque tierce sous licence **LGPL v3**, Copyright (c) Gamebuino,
auteur Jean-Marie Papillon -- voir les en-têtes de ses fichiers sources pour
le texte complet. Elle reste sous sa propre licence, distincte de celle du
reste du projet.
