# Stillbury Act 4 geography

Stillbury's north exit is the sole first approach to Castle Grounds:

**Stillbury → Blackwater Swamp → Royal Catacombs → Crown Road North → Castle Grounds.**

The Swamp Demon opens the Catacombs section; the Grave Marshal opens Crown
Road North. That road retains its six occupied checkpoints and is the toughest
of the three Crown Roads. Retreat remains available through each adventure.
Neither optional quests nor victories in the eastern or western side adventures
are additional castle requirements.

The east exit reaches Elderwood, Sunken Coast and the island adventure. Cliffs
separate that route from Ridgeshire. The west road retains Giant's Bridge and
its quarry fork: Stonewake and the optional Labyrinth provide equipment and
experience, with a northern escarpment preventing passage into Rosemoor.
The same cliffs appear in the actual approach maps and their route notices.
Existing walking squares and combat-stage maps are preserved.

The Royal Dungeon's one-way escape tunnel now returns Tomas and the player to
Stillbury. Its escort, enemies, equipment recovery and one-time 100-gold,
750-score reward are unchanged. The final boundary is labeled STILLBURY and
marked on the discovered minimap. The barred hatch cannot bypass the northern
castle journey.

From Castle Grounds, the lateral Crown Roads lead to Rosemoor and Ridgeshire.
Rosemoor has Frostfell to its north and Ashen Hollow through its west gate.
Ashen's former castle gate is removed. Ridgeshire retains Dragonspine to its
north and Ironfang Mines to its east. Neither town has a southern connection
to Stillbury's optional adventures. These connections are established;
their Act 5 stories and the castle finale remain provisional.

## Saves and map

Save version **248** retains existing combat caches, exploration, wounds,
equipment, quests, earned boss victories and unclaimed loot. Old Ashen approaches
return to Rosemoor, saved Rosemoor exteriors gain the new road, and the old
castle gate closes with boundary loot moved onto accessible ground. Legacy
town-connecting roads migrate onto the current regional approaches. Pending
Tomas offers use Stillbury without accepting the quest or paying it again;
saved escorts retain their progress. Completed old escapes remain completed.

The permanent atlas remains [world-map.png](world-map.png), embedded and linked
from README with its SHA-256 cache key. It shows both escarpments, the sole
northern approach, the escape-only return, Rosemoor's Ashen access, the existing
river and sea voyages, and an Act V provisional legend. The edit specification
is recorded in [the map prompt](stillbury-act4-map-prompt.md).

## Validation — November 10, 2026

Debug `conr` and `test_runner` builds succeed; **9,459 / 9,459** gameplay checks
pass. The focused Stillbury and approach checks pass **887 / 887**.
Tests run in isolated temporary directories with the game's assets linked in;
the local saves and settings were backed up and their SHA-256 hashes remain
unchanged afterward.

Movement actions exercise all five Blackwater, Elderwood, Catacombs and Ashen
stages across seeds 19, 107 and 1301, including retreat, cached revisits and
their real exit tiles. Catacombs stair actions, the independent northern boss
gates, Crown Road North's living patrols, actual castle entry and retreat are
covered. The full suite retains quarry, Labyrinth, both coastal tide states,
island-voyage, equipment and companion checks.

Save fixtures cover a pending Tomas offer, an escort saved inside the tunnel,
completed rewards, the old castle gate with sharpened boundary loot, and active
and cached wounded Ashen enemies. These preserve progress while adopting the
new destinations. No new persistent game-state fields are introduced.

Native SDL software-renderer review at **1280×720** and **640×480** covers the
local cliffs and route notices, closed/open northern gates, Rosemoor's Ashen
gate, Ashen's inward arrival and exit label, the retired castle gate, the saved
tunnel exit and Tomas's Stillbury arrival. An isolated copy of the existing
local version-240 Sunscar save also loads and renders successfully. The actual
README-linked atlas was inspected against the playable routes; both map URLs
use SHA-256 prefix **acdca7c9a496**.

Review captures:

- [Quarry's northern boundary](stillbury-act4/quarry-cliffs.png)
- [Elderwood's northern cliffs](stillbury-act4/elderwood-cliffs-small.png)
- [Loaded legacy tunnel exit](stillbury-act4/tunnel-exit-small.png)
- [Tomas arrives in Stillbury](stillbury-act4/tomas-stillbury-small.png)
- [Rosemoor's Ashen gate](stillbury-act4/rosemoor-ashen-small.png)
- [Retired castle gate after migration](stillbury-act4/legacy-castle-gate-small.png)

## Next change sets

Stop for a manual commit after this geography change. Then expand the quarry
story around restoring Stillbury's quarry and earning equipment for the
northern journey. A second resurrection shrine on the western road near the
quarry/Labyrinth fork is also requested for follow-up. Neither feature is
implemented in this change. Act 5 towns and the castle finale follow later.
