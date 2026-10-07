# Royal Catacombs

The south road from the grounds of the Castle of No Return leads into the
Royal Catacombs. They open as soon as the castle grounds are reachable from
either Crown Road. The north entrance leads separately to the six-floor
[castle interior](castle-design.md).

## Exploration and travel

Five floors contain eight burial chambers each, with varied room sizes and
looping passages. Ivory burial niches, cracked stone, sarcophagi, and animated
blue ossuary flames distinguish the catacombs from OakHaven's early dungeon.
The castle's south gate uses an ivory area label.

Walk onto downstairs and press **D** to descend, even while enemies remain.
Walk onto upstairs and press **U** to ascend; upstairs on floor 1 return to the
castle's south gate. On floor 5, the cyan return passage remains sealed until
the Grave Marshal is defeated. Stand on it and press **D** to return directly
to the castle without retracing the other floors or clearing every enemy.

Each floor has its own persistent cache. Leaving and revisiting preserves
explored maps, enemy health, deaths, resurrection allowances, extinguished
braziers, and triggered traps. Return to Town takes the player to Rosemoor and
opens a portal beside its east Crown Road gate. That portal returns to the
recorded catacomb floor and position. Portal wards dissipate pending spike or
polearm warnings that would strike the landing position immediately.

## Ossuary braziers and resurrection

Selected chambers contain a blue-flame brazier. Stand directly beside it and
press **A** to extinguish it. This takes a normal combat turn and permanently
silences the chamber's resurrection magic. Cold braziers do not relight.

Ancient Skeletons are tougher than OakHaven's skeletons. When first defeated
in a chamber with a lit brazier, they leave visibly rattling bones. One warning
turn later they reassemble at their corpse position with full health. They
cannot attack during the phase in which they rise. Occupied corpse positions
postpone resurrection; enemies never appear on the player or another enemy.
Extinguishing the brazier cancels a pending resurrection.

Bone Cantors can also call fallen Ancient Skeletons or Bone Sentinels in their
own lit chamber. Automatic and Cantor resurrection share **one allowance per
enemy**. The second defeat grants no additional experience, score, gold, or
loot, and the enemy stays dead. Skeletons killed away from a lit burial chamber
do not automatically resurrect.

## Burial traps

Visible bronze pressure plates trigger a row or column of spikes extending
up to four tiles in each direction. Orange outlines mark the affected stones
for a full turn before the spikes erupt. Step perpendicular to the line to
dodge. Walls and tomb objects interrupt the pattern; stairs and return
passages are protected. Each plate fires once and remains spent on revisits.

Damage is `12 + 2 * floor - half player defense`, with a minimum of 3, before
normal blocking and evasion. Opening menus or leaving the area does not
advance trap or resurrection timers. Their warning states survive saving.

## Enemies and difficulty

| Enemy | First floor | Role |
|---|---|---|
| Ancient Skeleton | 1 | Armored melee enemy that can reassemble once |
| Bone Sentinel | 1 | Heavy melee guard that protects nearby ranged allies |
| Crypt Bat | 1 | Fast flanker, with stronger catacomb stats |
| Grave Archer | 2 | Fires on clear rows or columns every other attack cycle |
| Wraith | 2 | Pierces half defense and drains mana; stronger catacomb stats |
| Bone Cantor | 3 | Ranged grave bolts and resurrection support |
| Grave Marshal | 5 | Final tomb boss, protected by two braziers |

Regular enemies appear in pairs around individual chambers; the starting room
is safe and the final boss chamber excludes ordinary enemies. Mixed roles,
stronger undead, limited resurrection, and marked hazards make the area harder
than the initial dungeon. Stairs never require killing every enemy.

New enemy base stats, before shared regional-order and player-level scaling:

| Enemy | HP | Attack | Defense | XP |
|---|---:|---:|---:|---:|
| Ancient Skeleton | 48 | 13 | 4 | 45 |
| Bone Sentinel | 80 | 17 | 8 | 75 |
| Grave Archer | 40 | 15 | 3 | 55 |
| Bone Cantor | 58 | 16 | 4 | 90 |
| Grave Marshal | 280 | 26 | 8 | 750 |

Catacomb Crypt Bats and Wraiths have twice their dungeon base HP and XP,
six additional base attack, and two additional base defense. The existing
regional scaling applies when enemies spawn; cached enemies keep their stats.
Catacomb victory does not increase the four original regions' order tier.

## Grave Marshal and reward

The Grave Marshal occupies floor 5's royal tomb. It approaches the player,
marks a three-tile polearm sweep, strikes those fixed tiles on its next turn,
then spends one turn recovering. Moving after the warning avoids the strike;
freezing the boss pauses its attack sequence.

Two accessible braziers reduce incoming damage by 50%. With one extinguished,
the reduction is 25%; with both extinguished, the protection disappears. The
boss always takes at least one point from a damaging hit. The protection
applies consistently to melee, cleave, bows, Magic Arrow, Frost Bolt, and
Fireball. Every class can extinguish the braziers.

Victory opens the final return passage, completes the boss journal entry, and
drops **Gravekeeper's Mantle**: rare armor for all classes, granting **+6
defense and +20 maximum health**. An unclaimed mantle is restored on revisiting
the final floor; collecting it ends that restoration.

Save format **83** stores all catacomb progression, pending attacks, revival
state, trap mechanisms, and the unclaimed reward. Older saves receive a fresh
catacomb cache and the castle south exit without losing existing character,
quest, equipment, or regional progress. This pass adds no regional quest.
