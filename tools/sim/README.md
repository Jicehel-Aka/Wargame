# Wargame AKA — simulateur de campagne (headless)

Joue automatiquement les 30 niveaux, à chaque difficulté, avec un bot simple
côté joueur (attaque si une cible est à portée, sinon avance vers l'ennemi
le plus proche ou vers le château) et la **vraie IA ennemie** (`ai_play_turn`,
non modifiée). But : repérer une carte injouable (trop dur, objectif
inatteignable, carte cassée) sans avoir à tout rejouer à la main.

Pas de fenêtre, pas de SDL2 — juste la logique de jeu (`main/game/*`,
`language.cpp`, `difficulty.cpp`, non modifiés) compilée en ligne de commande.

## Compiler et lancer

```
cd tools/sim
make run
```
(ou `make` puis `./bin/campaign_sim`)

Nécessite que `/sdcard/Wargame_Aka/` existe sur le disque (même chemin en
dur que le jeu réel, cf. `core/filesystem.h`) — peut rester vide, le
simulateur n'a pas besoin des sprites/illustrations.

## Diagnostic détaillé d'une carte

```
./bin/campaign_sim verbose1
```
Rejoue uniquement la Carte 1 en Difficile, tour par tour, en affichant le
nombre d'unités vivantes de chaque camp après chaque phase. Pratique pour
comprendre *comment* une défaite arrive (usure lente vs anéantissement
brutal) plutôt que juste *si* elle arrive. Pour diagnostiquer une autre
carte/difficulté, dupliquer ce bloc dans `main()` (`campaign_sim.cpp`) en
changeant l'index de niveau et `difficulty_set(...)`.

## Important : ce que ce bot NE fait PAS

Le bot est volontairement simple, pas malin :
- **Ne bat jamais en retraite** — une unité blessée continue d'avancer/
  attaquer comme si de rien n'était.
- **Ne se regroupe pas** — pas de recherche de flanquement ni de synergie
  entre unités, chacune agit isolément.
- **N'utilise pas le bonus défensif du terrain** — fonce vers l'ennemi le
  plus proche en ligne droite plutôt que de privilégier forêt/colline.
- **Ne concentre pas le feu d'une unité sur l'autre entre ses différentes
  unités** — chaque unité choisit sa propre cible "la plus faible", pas
  forcément la même d'un tour à l'autre.

Un niveau qui échoue contre CE bot n'est donc pas forcément injouable pour
un vrai joueur qui retraite, regroupe ses troupes et utilise le terrain.
Mais un **contraste très net entre difficultés** (Facile large succès,
Difficile écrasé au niveau 1) reste un signal utile — ça vaut le coup d'y
regarder de plus près, même en gardant cette réserve en tête.

## Idées d'amélioration si tu veux un signal plus fin

- Faire reculer une unité sous ~30% PV vers une case alliée plutôt que de
  continuer à avancer.
- Donner un léger bonus au choix de case lors d'un déplacement (tie-break
  vers le meilleur `def_bonus` de terrain plutôt que la distance seule).
- Concentrer le tir : mémoriser la cible la plus endommagée du tour et la
  proposer en priorité aux unités suivantes.
