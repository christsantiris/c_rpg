# Royal Catacombs

The Royal Catacombs now stand as a separate sprite along the northern road
between Oakhaven and Castle Grounds. From Oakhaven, defeat Elderthorn in
[Elderwood Forest](elderwood-forest.md) to reach them. Defeat the Grave Marshal
to unlock the remainder of this northern castle route. They can also be reached
from Castle Grounds' south exit after arriving by either Crown Road. The
castle's north entrance leads separately to the six-floor [castle interior](castle-design.md).

## Exploration and travel

Five floors contain eight burial chambers each, with varied room sizes and
looping passages. Ivory burial niches, cracked stone, sarcophagi, and animated
blue ossuary flames distinguish the catacombs from OakHaven's early dungeon.
The castle's south exit is signed NORTHERN ROAD.

Walk onto downstairs and press **D** to descend, even while enemies remain.
Walk onto upstairs and press **U** to ascend; upstairs on floor 1 return to the
road beside the Catacombs sprite. On floor 5, the cyan return passage remains sealed until
the Grave Marshal is defeated. Stand on it and press **D** to return directly
to that road without retracing the other floors or clearing every enemy. Walk
north for Castle Grounds or south through cleared Elderwood for Oakhaven.

Each floor has its own persistent cache. Leaving and revisiting preserves
explored maps, enemy health, deaths, resurrection allowances, extinguished
braziers, and triggered traps. Return to Town takes the player to the last town
actually entered and opens a portal there. That portal returns to the
recorded catacomb floor and position. Portal wards dissipate pending spike or
Marshal signature warnings that threaten the landing position. Non-damaging
honor-guard calls retain their fixed targets and countdown.

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

Crypt Conjurers can also call fallen Ancient Skeletons or Bone Sentinels in their
own lit chamber. Automatic and Conjurer resurrection share **one allowance per
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
| Vampire | 2 | Reuses the Blackwater Swamp melee enemy |
| Wraith | 2 | Pierces half defense and drains mana; stronger catacomb stats |
| Crypt Conjurer | 3 | Dungeon caster with ranged bolts and brazier-bound resurrection support |
| Grave Marshal | 5 | Final tomb boss, protected by two braziers |

Regular enemies appear in pairs around individual chambers; the starting room
is safe and the final boss chamber excludes ordinary enemies. Mixed roles,
stronger undead, limited resurrection, and marked hazards make the area harder
than the initial dungeon. Stairs never require killing every enemy.

Catacomb enemy base stats, before shared regional-order and player-level scaling:

| Enemy | HP | Attack | Defense | XP |
|---|---:|---:|---:|---:|
| Ancient Skeleton | 48 | 13 | 4 | 45 |
| Bone Sentinel | 80 | 17 | 8 | 75 |
| Vampire | 65 | 15 | 5 | 80 |
| Crypt Conjurer | 48 | 13 | 3 | 70 |
| Grave Marshal | 280 | 26 | 8 | 750 |

Catacomb Crypt Bats, Wraiths and Crypt Conjurers have twice their dungeon base HP and XP,
six additional base attack, and two additional base defense. The existing
regional scaling applies when enemies spawn; cached enemies keep their stats.
Catacomb victory does not increase the four original regions' order tier.

## Grave Marshal and reward

The Grave Marshal retains **280 base HP, 26 attack, 8 defense and 750 experience**,
with existing regional and player-level scaling. He stays in the royal tomb.
Entering it or damaging him from a nearby ranged approach starts the encounter;
retreat beyond 12 tiles pauses the exact warning without healing him.

- **Honored Commander, above half health:** retain ordinary approach movement
  and a fixed three-tile polearm front, warned for one complete player action.
- **Funeral March, at/below half health:** announce one two-action honor-guard
  call, then alternate a two-action cross sweep and a two-action funeral channel.
  The cross has radius two; the channel has radius three. Their centers lock
  when declared. Two ordinary cardinal moves leave a cross from its center.
  Walls and tomb objects clip the arms. No lingering damage patches are added.
- A phase change never widens an already queued attack. Every signature leaves
  one full recovery action with no extra adjacent Marshal retaliation. Freeze
  pauses the queued warning and recovery; ordinary burial traps retain their
  existing independent clock behavior.
- Numbered cyan outlines and slash/cross symbols show the exact damaging tiles.
  H2/H1 mark fixed honor-guard arrival tiles; the call itself deals no damage.
  The panel shows phase, attack, countdown, remaining recovery and lit braziers.

Two accessible braziers retain **50%/25%/0% damage reduction**. Every damaging
hit still deals at least one point. Protection applies consistently to melee,
cleave, bows, Magic Arrow, Frost Bolt and Fireball. All classes can extinguish
flames without gear or consumables. Extinguishing a royal-tomb brazier while a
**funeral channel** is preparing cancels that channel and grants **two complete
counterattack actions**. It does not cancel a polearm/cross sweep or an honor
call, stop existing guards, heal the player or relight a spent flame.

Two tomb niches release at most **two Royal Honor Guards** after a full warning.
They use the existing Bone Sentinel stats and art, start acting on the following
action, and grant no experience, score, coins, loot or resurrection allowance.
Occupied arrival positions skip their guard instead of postponing an unannounced
spawn. A full enemy list consumes the finite call without replacing old corpses
or actors. The wave is never repeated on revisits or save/load.

Victory opens the final return passage even while guards or regular enemies
remain, completes the boss journal entry and guarantees **Gravekeeper's Mantle**:
rare all-class armor with **+6 defense and +20 maximum health**. Its stats and
art are unchanged. An unclaimed mantle still restores as a floor item on return
when space permits. Full loot capacity reserves it in a royal reliquary without
discarding other items. Interact on or beside that cache with contextual **A**,
or press **P** on it; full inventory leaves it available. First claim is permanent,
so selling or deliberately dropping it cannot create a replacement.

Save version **130** adds `grave_marshal_reward_state` (not earned, unclaimed,
claimed), preserving the old `catacombs_mantle_unclaimed` behavior. Existing
enemy fields save phase, attack, countdown, finite-wave use and both niche
positions in active/cached encounters. Appended tile IDs preserve niches and
reliquaries, including under loot. Older living fights migrate in place with
visible new warnings, preserving health, deaths, cold braziers, exploration,
portals, traps, gear and memorial/ledger progress. Known old unclaimed flags
remain recoverable; previous victories stay completed without fabricating gear.

See the [renderer captures and play-testing guide](art-sources/grave-marshal-encounter/README.md).

Save format **83** stores all catacomb progression, pending attacks, revival
state, trap mechanisms, and the unclaimed reward. Older saves receive a fresh
catacomb cache and the castle south exit without losing existing character,
quest, equipment, or regional progress.

## Rest for the Forgotten

**Brother Oswin**, a royal archivist in **Ridgeshire's Town Hall**, assigns
this optional quest when spoken to with **T**. He wants the royal dead's names
returned to their families and their disturbed graves put to rest. Take
Ridgeshire's west gate along **Crown Road West**, then enter the Catacombs
through the castle grounds' south gate.

| Floor | Objective | Additional defenders |
|---|---|---|
| 2 | Silence the Soldiers' memorial | Ancient Skeleton and Bone Sentinel |
| 3 | Silence the Watchers' memorial | Vampire and Wraith |
| 4 | Silence the Choir memorial | Crypt Conjurer and Ancient Skeleton |
| 5 | Recover the royal burial ledger | Existing Grave Marshal boss |

Each memorial is a marked blue brazier with an ivory plaque and name label.
Stand directly beside it and press **A**. Guards do not need to be defeated
first: silencing the flame prevents the chamber's resurrection magic and
cancels pending revivals. The brazier remains visibly cold. Other ossuary
braziers still work normally and do not count as quest objectives.

The ledger rests on a plinth in floor 5's royal tomb. Its seal remains locked
while the Grave Marshal lives. After victory, stand directly beside it and
press **A** to recover it. Memorials can be silenced in any order, and the
ledger can be collected before finishing the memorials. No quest object uses
an inventory slot. The existing Gravekeeper's Mantle reward is unchanged.

Return to Oswin for **150 gold and 1,500 score**, awarded once. The journal
tracks all four objectives separately and retains them in its Completed tab.
The quest never gates castle entry or victory.

Accepting preserves existing caches, portals, enemy damage, deaths, revival
allowances, and traps. Memorial braziers already extinguished before accepting
receive credit; their defenders are not added again. A previous Grave Marshal
victory unlocks the ledger without a second fight. Save version **97** stores
quest state, completed objectives, and encounter placement. Older saves gain
an unassigned quest and Oswin in the Town Hall without losing existing progress;
players or loot occupying his new position move safely beside him.
