# Boss gates and regional travel

Travel policy is declared in [`rules`](../src/game/boss_gates.c), independently
of quest acceptance. Road access checks use the same saved boss victory from
both approaches. A player can enter an adventure from either side and retreat
to the same side. Defeating its boss permanently secures the passage in both
directions; ordinary enemy respawns never close it again.

| Area | Policy | Requirement or purpose |
| --- | --- | --- |
| Oakhaven Dungeon | Boss-gated road | Lich King; Oakhaven ↔ Dawnmere |
| Haunted Forest | Boss-gated road | Necromancer; Oakhaven ↔ Stillbury |
| Goblin Mountains | Boss-gated road | Goblin King; Dawnmere ↔ Ridgeshire |
| Blackwater Swamp | Boss-gated road | Swamp Demon; Stillbury ↔ Rosemoor |
| Elderwood Forest | Boss-gated road | Elderthorn; Elderwood section of the northern castle connection |
| Royal Catacombs | Boss-gated road | Grave Marshal; Catacombs section of the northern castle connection |
| Sunken Coast | Boss-gated destination | Drowned Queen unlocks the docks; existing chart and sailing rules remain |
| Castle interior | Internal progression | Existing Castellan, Royal Arcanist and Lord Veyr encounter rules |
| Rook's Labyrinth | Free travel | Minotaur still guards the Ivory Rook; Rook's quest opens the entrance |
| Stonewake Quarry | Free travel | Quarry Colossus and quarry quest remain optional |
| Saltwind Marsh | Free travel | Sszareth does not seal the final departure |
| Stormwatch Headland | Free travel | Tempest Matriarch does not seal the final departure |
| Ironfang Mines | Free travel | Goblin Mine Boss does not seal the final departure |
| Giant's Peaks | Free travel | The Last Jarl still guards the royal vault and crown |
| Sunscar Wastes | Free travel | Desert Pharaoh remains an optional boss |
| Frostfell Wastes | Free travel | Frost Kraken remains an optional boss |
| Dragonspine | Free travel | Red Dragon remains an optional boss |
| Ashen Hollow | Free travel | Cinder Lord remains an optional boss |
| Ruined Temple | Free travel | Existing alignment puzzles and Guardian-protected treasure remain |
| Ruined Isle | Free travel | Landing and temple approach |
| Crown Road East | Free travel | Ordinary encounters do not lock travel |
| Crown Road West | Free travel | Ordinary encounters do not lock travel |
| Crown Road North | Free travel | No additional boss requirement; regional checkpoints control the approach |
| Castle Grounds | Free travel | Exterior connections retain their normal behavior |

## Checkpoints

Compact posts and a lowered beam identify closed road connections. The beam
stays raised after victory. Regional textures distinguish crypt masonry,
woodland checkpoints, goblin timber and the swamp causeway. Banks frame the
crossings and prevent walking around a closed gate. Every gated road draws its
banks as the woods, bog or rock that the road passes through.

Nearby labels name the boss controlling a closed road. Bumping the beam or
using the configured Interact key explains the boss and area requirement.
The approach panel reports passage status. Discovered connections on the
minimap use a lock before victory and a check mark afterward. Entrance sprites
remain available to challenge the boss. Single-tile roads stay single tile.

From Oakhaven, the northern connection encounters Elderwood before Catacombs.
From Castle Grounds, it encounters Catacombs before Elderwood. Each checkpoint
uses its own victory; both are needed to complete the entire journey.
Retreat from a castle-side Catacombs attempt returns north of its closed gate.

## Saves and debug play

Save version **211** appends checkpoint and bank tile IDs, keeping all previous
IDs stable. The policy itself is static; existing serialized boss victories
remain authoritative. Older safe approach maps acquire checkpoints while
retaining exploration, loot, money, quests and all combat caches. A player
stranded beyond a new closed checkpoint is placed on the original approach's
accessible landing. Loot covered by a new bank is moved onto accessible paving,
without covering the town transition.

Gated roads cut through scenery and narrow to a single-tile verge at their
checkpoint. See [road scenery](road-scenery.md), including how older saves on
these roads are rebuilt.

`--unlock-shortcuts`, including `tools/debug_shortcuts.sh`, bypasses these
checkpoints without granting boss victories or quest completion. Use a normal
launch with the saved debug override disabled to test closed gates.

The one-way Return to Town spell, castle escape exception, Rowan dialogue,
quest doors, local puzzles and reward eligibility retain their existing rules.

## Verification

`tests/test_boss_gates.c` searches actual walkable terrain and exercises both
approaches, all four northern victory combinations, physical crossings, retreat,
debug overrides, explanations, and save migrations. Existing regional tests
cover free departures before optional bosses die and retain combat/reward checks.
`tools/tests/debug_shortcuts_test.c` captures closed/open checkpoints using the
actual SDL game renderer beside existing landmarks and the player.

Art source: [`road_checkpoints.py`](../tools/sprites/road_checkpoints.py), using
[`pixelkit.py`](../tools/sprites/pixelkit.py) at native 24px tile scale.
