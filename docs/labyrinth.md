# Rook's Labyrinth — Minotaur encounter

Rook, in **Oakhaven's Tavern**, assigns The Ivory Rook quest. Accept with `Y`
to open the labyrinth gate beside the **Haunted Forest entrance on the road
between Oakhaven and Stillbury**. It is outside both towns, just west of the
forest sprite. Oakhaven's west gate or Stillbury's east gate leads onto this
safe road. No forest boss victory is needed to reach either entrance.

The labyrinth is visible but sealed before the quest is assigned. Once unlocked,
it remains open after retrieving the relic and completing the quest. Its existing
sealed/open sprite and five-floor adventure are retained. Physical retreat from
floor 1 returns beside the labyrinth doorway on the same approach; walk back to
the town used, enter the forest, or continue along the through road once the
Necromancer is defeated.

Five maze floors each hold a rune. All five runes open the final vault gate;
the Minotaur must fall before the player can take the ivory rook. Return to
**Rook in Oakhaven's Tavern** for the one-time 40 gold and 500 score reward.
True and false stairs, runes, gate, relic and regular maze enemies keep their
existing behavior.

## Phases and warnings

The Minotaur retains **120 HP, 18 attack, 7 defense and 250 experience**. Attacks
use normal defense mitigation, with a minimum of 1 damage. The closed rune gate
blocks engagement. A clear nearby line starts the encounter; damage from a
ranged approach also engages him. He charges along the last floor's walkable
lanes and never crosses stairs or overwrites runes, gate, relic or maze walls.

- **Vault Keeper, above half health:** a one-action fixed charge lane when
  farther away, or a one-action three-tile frontal axe sweep when close.
- **Bloodied Bull, at/below half health:** two charges, each with its own fresh
  one-action warning and recovery, followed by a two-action wide axe arc.
  Crossing half health during a warning never changes that queued attack.
- Charge lanes are horizontal or vertical and reach at most six tiles. Their
  endpoint and collision target lock before movement. A player standing in
  the lane takes one hit; the Minotaur stops before the player instead of
  sharing a tile. Newly entering actors can stop the charge early, but removing
  cover cannot extend an already warned endpoint.
- The short axe covers three tiles in front. The wide axe adds flanking tiles
  beside the Minotaur and expands the frontal row to five tiles. Both use the
  same fixed tile predicate for warnings and damage. Normal walking can escape
  within their advertised countdown.
- In single-file approaches, a charge that would prevent a one-action escape
  shortens to the aimed tile so the player can step back. If no safe walk exists,
  the attack becomes an empty swing. The map receives no invisible barriers.

Numbered outlines and directional arrows mark exact charge lanes; slash symbols
mark axe attacks. The panel shows the phase, attack countdown and remaining
recovery actions. Frozen warnings retain targets and pause their countdown;
freeze also pauses recovery. Retreat beyond 12 tiles pauses the exact pending
attack without healing or moving the boss. Opening inventory or the journal
advances no encounter clocks.

Every signature leaves **one full recovery action** without an extra adjacent
retaliation. The Minotaur remains damageable throughout. Baiting an announced
charge into an ordinary wall or reinforced pillar instead grants **two full
actions of stagger**. Players can counterattack during both actions.

## Reinforced pillar caps

Two vault pillars provide optional, equipment-free counters for every class.
Their solid caps block walking and targeting. A charge aimed at a reinforced
pillar displays **C1**, plus an explicit crack warning, before impact. The first
impact cracks its cap while keeping it solid. In Bloodied Bull, a later charge
into that cracked cap displays **B1** and an explicit break warning; its impact
breaks the cap into walkable rubble, opening a flank. **H1** marks other planned
stone impacts. Collision labels identify the obstacle; those solid tiles are
not themselves damage tiles.

Only these two designated caps can break. Ordinary maze walls, the rune gate,
stairs, switches and ivory rook remain intact. Cracked/broken cap state persists
through floor backtracking and saves. A living fifth-floor encounter also
survives retreat to the forest road and re-entry, retaining health, targets, freeze,
charge sequence and pillars. Other floors still start fresh on a new expedition;
completed expeditions retain the existing regular-enemy refresh behavior.

## Labyrinth Aegis and save compatibility

Future victories guarantee **Labyrinth Aegis**: a rare, **Warrior-only** shield
with **+9 defense, 25% block chance and value 1,250**, matching the old Magic
Shield. Its bronze rim, bull emblem and cyan runes appear in inventory, equipped
icons and the player's off hand. Existing Magic Shields retain their identity,
art and shop availability; no class or shop progression changes are introduced.

The reward normally drops with boss gold. Full floor-loot capacity reserves it
in the bull-marked vault cache without deleting other loot. If every uncovered
vault tile is occupied, the cache can safely sit beneath existing loot. Interact
on or cardinally beside it with contextual **`A`**, or press **`P`** on it.
Full inventory leaves the reward available. Leaving an earned reward unclaimed
restores it in the cache after a floor refresh. Claiming permanently prevents
replacement after sale or deliberate dropping.

Save version **129** adds `minotaur_reward_state`: 0 not earned, 1 unclaimed
Aegis, 2 claimed, and 3 identifiable unclaimed legacy Magic Shield. Existing
enemy fields store phase, sequence, countdown, orientation, endpoint and collision
coordinates in active/cached encounters. Appended tile IDs preserve pillar and
cache state; a new appended item visual ID preserves the Aegis artwork.

Older living fights migrate in place with visible new warnings, preserving
health, deaths, positions, exploration, gear, loot and all rune/quest progress.
Old victories remain completed without inventing another shield. An identifiable
current dropped legacy Magic Shield remains recoverable as **Magic Shield**,
including after saving again; it is never retroactively renamed to Aegis.

See the [renderer captures and play-testing guide](art-sources/minotaur-encounter/README.md).


## Entrance relocation and older saves

Save version **176** records `labyrinth_entry_town`, so loading inside the maze
retains the correct return approach. Older maze saves default to Stillbury's
forest approach. Their floors, enemies, Minotaur state, runes, relic, boss victory
and quest completion remain intact. Old Stillbury saves remove the gate and its
spur, retaining the square's cobblestones, player position, exploration and loot;
items covering the removed gate receive grass underlays. Saved forest approaches
gain the landmark, with blocked players and loot moved to accessible road tiles.

See [sealed/open entrance renderer previews](art-sources/labyrinth-entrance/README.md)
and the updated [world map](images/world-map.png).
