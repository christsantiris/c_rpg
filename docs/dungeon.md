# Castle Dungeon

Enter the five-floor dungeon from the east road of OakHaven. Its rooms and
connecting corridors are generated for each new expedition, then cached while
that expedition is in progress. Backtracking preserves explored rooms, enemies,
opened doors, and collected treasure. A later expedition generates new maps
and regular enemies, but a defeated Lich King does not return.

The dungeon targets 6–10 rooms per floor. Descend at the marked stairs and
ascend to revisit an earlier floor; the first floor's upward stairs return to
town. Floors 1–4 do not require defeating every enemy before descending. Traps
begin hidden, and deeper floors contain more of them. If other regional bosses
have already fallen, newly generated dungeon enemies become stronger and
advanced undead can appear earlier.

| Floor | Distinct feature | Baseline encounter progression |
| --- | --- | --- |
| 1 | Entrance rooms and the route back to town | Skeletons |
| 2 | Possible locked side crypt; Elowen's first seal during her quest | Zombies and Crypt Bats join |
| 3 | Possible locked side crypt; Elowen's second seal | Wraiths join |
| 4 | Possible locked side crypt; Elowen's third seal | Crypt Conjurers join |
| 5 | Golden key, locked boss chamber, and final return passage | Full roster and the Lich King |

Every generated floor is checked from its entrance before play. All rooms,
corridors, and side crypts must connect through walkable passages or their
intended locked doors. Disconnected sections receive connecting corridors
without bypassing locks or changing existing items and traps. A layout that
cannot be repaired is generated again. Cached floors are checked when revisited.

Save version 107 repairs disconnected sections on every active and cached
dungeon floor, preserving the player's position, exploration, enemies, loot,
and quest progress. Tests check every walkable tile across 5,120 layouts and
verify that keys can be reached before opening their doors.

## Boss Chamber

On floor 5, collect the golden key in the penultimate room with `P`, then walk
into the Lich King's locked door to open it. Corridors around the outside of
the sealed chamber keep earlier rooms and the door accessible. Its entrance
is placed away from corners so unlocking it leads into the chamber.

Older boss floors also receive the corridor and corner-door repairs introduced
in save version 106. Doors already opened remain open.

## Enemy Roles

- **Skeleton:** straightforward melee pursuer.
- **Zombie:** tougher melee enemy that moves every other turn.
- **Crypt Bat:** fast flanker that can move twice in a turn.
- **Wraith:** melee attacker that ignores half of the player's defense and
  drains MP.
- **Crypt Conjurer:** fires along a clear row or column and can revive a fallen
  Skeleton nearby. Walls and other enemies block its shot.

## Crypts and Traps

On floors 2, 3, and 4, generation can add an optional crypt where space allows.
Stand on its key and press `P`, then walk into the locked crypt door to spend
the key. Stand on the cache inside and press `P` to collect its gold. Each
generated crypt cache holds `10 + 2 × floor` gold.

Hidden traps may become spikes, fire, or poison when stepped on; the starting
room is excluded from trap placement.

## The Lich King's Chamber

The final floor places a golden key in the room before the Lich King's sealed
chamber. Stand on the key and press `P`, then walk into the chamber door to
unlock it. The Lich stays dormant until the player enters the chamber. Once
engaged, he alternates a warning turn with a ranged necrotic bolt. Two Skeleton
minions named **Lich Guard** flank him inside the locked chamber and pursue
the player in melee. They count toward the floor's normal enemy limit. Defeating
him opens the glowing return passage; other surviving
enemies do not block the trip back to OakHaven. The Lich King's victory and boss
reward are one-time.

Bram stands near Cain in Oakhaven's central square. Speak to him with `T` for
a warning about the Lich King in the eastern dungeon and advice to buy supplies
from the blacksmith and alchemist. His warning changes after the Lich is defeated.
Save version 108 adds Bram and the guards to older saves without resetting
progress; defeated bosses and fallen guards remain defeated.

## Elowen's Quest

Speak with Elowen in **Oakhaven's Tavern** to begin **The Broken Seals** in
**Oakhaven's dungeon**. Her three burial seals appear on floors 2, 3, and 4,
each with an undead guard group.
Stand on a seal and press `A` to restore it. Return to Elowen in Oakhaven's Tavern
after all three are restored for 40 gold and 300 score. Her quest does not require defeating
the Lich or clearing every floor. Accepting it starts a fresh expedition so
the objectives can appear even if the dungeon was visited earlier.

Cain's Scroll of Return to Town teaches a zero-MP spell. Casting it in the
dungeon places a portal in town that returns to the exact floor and tile where
it was cast, allowing a resupply trip without replaying the route.

Save version 85 merges the old eight floors into five, retaining the Lich
chamber on floor 5 and preserving seal completion, character progress, active
terrain, enemies, loot, and portal access.
