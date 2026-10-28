# Docks, Ruined Isle, and Ruined Temple

The docks and island open after the Sunken Coast boss is defeated.

## Unlocking the Docks

Leave Oakhaven through its south gate and follow the Sunken Coast approach.
Captain Rowan stands on the beach beside the area entrance; the docks are
further east on the same coastal approach, outside town. Defeat the Drowned
Queen to open the dock landing. No other boss or Tavern quest is required.
Until then, Captain Rowan explains that the Queen blocks the sea route. Speak
with Rowan after defeating her to receive the Island Treasure Map. Inventory
space is required. If the dungeon and mountains remain uncleared, Rowan
recommends training in one of them before facing the Ruined Temple. Nahla
repeats the advice on the island. The advice does not block travel or entry.

Walk into the dock entrance to open the docks scene. Select **Board ship for
the Ruined Isle** and press `Enter`, or choose the return option to stay in
on the coastal approach. The first voyage uses the map to unlock the route and consumes it. Later
voyages remain available without carrying a map.

## Ruined Isle

The Ruined Isle is a safe tropical exploration map with jungle growth, beaches,
a lagoon, ruined statuary, an abandoned camp, and the gated temple approach.
Stand on or beside an island landmark and press `A` to inspect it.

Captain Rowan waits at the landing. Speak with him using `T` and choose the
ship option to return to the Sunken Coast docks. Casting Return to Town on the
island performs that same coastal return and does not leave a portal. Sailing
does not mark Oakhaven visited; enter its town sprite to update the last-town
destination.

Nahla waits near the temple approach. Speaking with her starts **The Buried
Sun**. Walk into the temple gate to enter the pyramid.

## Ruined Temple Structure

The temple is a five-tier stepped pyramid. Each tier is a separate combat map.
Use the upward stairs to climb and the downward stairs to revisit lower tiers.
The southern entrance on tier 1 returns to the island. Upward stairs stay
magically sealed until every enemy on that tier is defeated, including dormant
Moonbound Sentinels that must first be awakened at an altar. Downward stairs
and the island exit remain available for retreat. This rule applies only to
the Ruined Temple; other areas keep their existing travel rules.

| Tier | Encounter size | Notable addition |
| ---: | ---: | --- |
| 1 | 6 enemies | Core temple roster |
| 2 | 7 enemies | Lunar Effigy |
| 3 | 8 enemies | Treasure Wraith |
| 4 | 8 enemies | Solar traps, flooded galleries, and lunar defenders |
| 5 | 6 regular enemies and 1 boss | Fallen Sun Guardian and buried vault |

Walls and closed lunar doors block player and enemy ranged attacks.

## Sun and Moon Mechanic

Stand on or beside a solar altar and press `A` to change the temple alignment.

- **Sun:** lunar doors are closed and orange solar floor traps are active.
- **Moon:** lunar doors open and dormant Moonbound Sentinels awaken.

Switching to Sun waits until every open lunar doorway is clear of the player,
living enemies and loot. This prevents doors closing on occupants or becoming
hidden beneath walkable items. Loot covering an altar does not disable it.

Awakened sentinels remain active if the alignment returns to Sun. Alignment,
opened routes, awakened enemies, defeated enemies, and the treasure state are
cached separately for every tier and persist through saving and loading.
The seal counts dormant sentinels as remaining foes. A purple barrier marks
sealed stairs, and an on-screen notice shows the remaining count and reminds
you to use an altar while any sentinels are still dormant. The summit treasure
also remains sealed until both enemy groups are cleared, even after the
Guardian opens the vault. Clearing a tier keeps its stairs open on later visits.

## Enemy Roles

- **Relic Scarabs:** basic melee swarm.
- **Temple Stalker:** fast pursuer that moves twice when it advances.
- **Blowdart Hunter:** ranged attacker whose darts poison the player.
- **Vinebound Guardian:** slow, durable frontline enemy.
- **Sun Priest:** ranged support enemy that periodically heals a nearby ally.
- **Serpent Spirit:** ranged magical attacker.
- **Treasure Wraith:** strong late-temple attacker.
- **Lunar Effigy:** slow, armored temple construct.
- **Moonbound Sentinel:** durable ranged construct awakened by Moon alignment.
- **Fallen Sun Guardian:** summit boss with three phases, fixed solar and lunar
  warnings, alignment guards and sanctuary refuges.

## Fallen Sun Guardian

The Guardian wakes when the player enters the summit sanctuary or damages it
from range. It stays at its original position with its existing health,
attack, defense and experience. Adjacent and ranged attackers receive the
same scheduled warning opportunity; attacks never cause an immediate
unannounced retaliation.

| Phase | Threshold | Signatures |
| --- | --- | --- |
| Sunbound Armor | Above half health | Alternating one-action cardinal solar lance and two-action lunar burst |
| Cracked Core | Half health or below | Alternating lance and two-action sunburst ring |
| Eclipse | Quarter health or below | Alternating two-action solar lanes and lunar burst |

Phase changes latch at the next declaration. Crossing a threshold or freezing
the Guardian cannot replace an already announced strike. Each resolution
allows one full counterattack action before the next declaration. Frost Bolt
still damages the Guardian, but cannot freeze it. Existing saved freezes finish
normally; leaving its activation range pauses the encounter.
Phases never reverse if a Sun Priest heals the boss.

Before declaring a strike, the Guardian replaces lances aimed away from diagonal
attackers with targeted lunar bursts. Camping an M plate draws a lunar burst;
camping an S plate during a lunar declaration draws solar lanes. Each counter
receives its own full two-action warning. Moving onto a matching plate after
declaration still protects against that fixed strike. A provoked Guardian can
target lunar bursts in the approach corridor as well as inside the sanctuary.

Lances retain their declared direction and endpoint. Walls, closed doors and
living actors stop them. Removing cover or opening a door after declaration
cannot extend the original ray; new cover can shorten it. The sunburst ring
covers Manhattan distances 2–4 from the Guardian inside its chamber, leaving
the adjacent interior and outside edge safe. Solar lanes form a fixed cross
through the player's declared position, extending four cells along each arm.
The lunar burst covers that fixed target and its four cardinal neighbors.
These area attacks stay within the upper sanctuary, south of the vault wall.
Two ordinary moves suffice to escape the broader attacks. Outside the chamber,
the boss uses a lance instead, with two actions during Eclipse.

Two additional summit altars sit three tiles west and east of the Guardian.
Press **A** on or beside either to switch the existing alignment; this costs an
ordinary action and retains all door, solar-trap and sentinel behavior. The
warned footprint never changes when an altar is used:

- **Sun** reduces damage dealt to the Guardian by one third, with a minimum
  hit of one. It also halves incoming lunar-burst damage after defense.
- **Moon** exposes the Guardian to normal damage and halves incoming solar
  damage after defense. Its open doors and newly awakened sentinels remain
  part of the original temple challenge.

Sanctuary plates beside each altar provide complete protection from their
matching attack regardless of current alignment. Cyan **M** plates protect
against solar lances, rings and lanes; golden **S** plates protect against
lunar bursts. The western plates lie inside the ring's outer edge. Players
can use these refuges, change alignment for reduced damage, or escape by
walking. Neither a class ability nor a consumable is required.

Defeating the Guardian permanently records its boss victory and opens the
vault. **Sunwarden Mantle** goes directly into the inventory: rare all-class
light armor granting **+5 defense, +8 maximum HP and +7 maximum MP**. If the
pack is full, a summit reliquary reserves it independently of floor capacity.
Use **A** beside the cache or **P** on it, even beneath loot. Saving, backtracking
and portal travel preserve an unclaimed mantle. Claiming restores the native
floor; sold or discarded mantles never regenerate.

Nahla's quest remains separate. Clear the remaining Sun and Moon enemies,
then stand on or beside the treasure and press **A** to recover the Buried Sun.
Return to **Nahla near the Ruined Isle temple approach** for **150 gold and
2,500 score**. Receiving the mantle neither claims the treasure nor completes
her quest.

See the [native renderer gallery and play-test guide](art-sources/sun-guardian-encounter/README.md).

## Return to Town

Return to Town works from every temple tier and from the Ruined Isle. It is a
one-way escape to the last town visited and leaves no return portal. Walking
out through tier 1 still returns to the island. Sailing with Rowan remains
available for physical return voyages.

Save version 85 moves the old tier-4 summit and its portal destination to
tier 5, preserving enemy health, alignment, treasure, and boss completion.
The new tier 4 is inserted below the summit.

Save version **135** adds one initialized, serialized reward state. Existing
enemy fields preserve the phase, attack sequence, countdown, fixed target,
lance endpoint, freeze and recovery in active and cached summits. Appended
map IDs preserve sanctuary plates and the reliquary. Migration resets obsolete
boss timing and adds arena props around a living Guardian without healing it,
reviving enemies or overwriting exploration, loot, portal landings or treasure
progress. Earlier victories receive no newly manufactured mantle. Returning
through a portal wards any saved signature threatening its landing, including
a lance temporarily hidden by an actor that could move before the boss.
