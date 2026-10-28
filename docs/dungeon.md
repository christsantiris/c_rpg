# Castle Dungeon

Enter the five-floor dungeon from Oakhaven's east road or Dawnmere's west road.
Both entrances always start at floor 1 and proceed forward to floor 5. Its rooms and
connecting corridors are generated for each new expedition, then cached while
that expedition is in progress. Backtracking preserves explored rooms, enemies,
opened doors, and collected treasure. A later expedition generates new maps
and regular enemies, but a defeated Lich King does not return.

The dungeon targets 6–10 rooms per floor. Descend at the marked stairs and
ascend to revisit an earlier floor; the first floor's upward stairs return to
the safe approach for the town you entered from. Floors 1–4 do not require defeating every enemy before descending. Traps
begin hidden, and deeper floors contain more of them. If other regional bosses
have already fallen, newly generated dungeon enemies become stronger and
advanced undead can appear earlier.

| Floor | Distinct feature | Baseline encounter progression |
| --- | --- | --- |
| 1 | Entrance rooms and the route back to town | Skeletons |
| 2 | Possible locked side crypt; Elowen's first seal during her quest | Zombies and Crypt Bats join |
| 3 | Possible locked side crypt; Elowen's second seal | Wraiths join |
| 4 | Possible locked side crypt; Elowen's third seal | Crypt Conjurers join |
| 5 | Golden key, locked boss chamber, and sealed return stairs | Full roster and the Lich King |

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
unlock it. The Lich stays dormant until you enter; damaging him from outside
also starts his warned attacks. He remains inside the chamber.

- **Crypt Sovereign, above half health:** a necrotic lance marks a fixed lane
  for one player turn. Step aside or use the chamber's stone pillars as cover.
- **Unbound Soul, at/below half health:** one finite wave of up to two Skeletons
  rises at marked positions after a one-turn warning. Subsequent attacks
  alternate lances with two-turn soul rings. The diamond-shaped ring is three
  cardinal steps from the Lich; its inner and outer floor are safe from the burst.
- Every resolved signature has a recovery turn. Attack during the green
  **RECOVERING** opening. There is no extra adjacent Lich attack during it.
- The boss panel shows the health phase, attack name, countdown and active seals.
  Outlined numbered tiles show the exact footprint. Freezing pauses a pending
  attack; crossing half health never changes a warning already on the floor.
- Use your contextual **Interact key (`A` by default)** on or beside either
  burial seal. Each active seal reduces damage dealt to the Lich by 15 percentage
  points and adds 2 damage to his soul burst. Silencing both removes protection,
  cancels his current warning and grants two complete recovery turns. Seals stay
  silenced through backtracking, portals and saves. They are separate from
  Elowen's quest seals; no quest or class-specific ability is required.

Two existing Skeleton minions named **Lich Guard** flank him inside the locked
chamber. The extra phase-two **Lich Shades** are summoned only once per attempt,
start acting on the following turn, and award no gold, loot, score or experience.
Hidden random traps are excluded from the chamber; previously revealed traps
in older saved chambers retain their state.

Defeating the Lich unseals the pale stone return staircase. Stand on it and use
the **stairs-up key (`<`, the comma key by default)** to emerge onto the safe
straight, single-tile cobblestone road between the town gates. The crypt stands
above the road, reached by a short path. Separate signs beside the town
entrances read **OAKHAVEN** and **DAWNMERE**. Walk **west to Oakhaven** or **east to
Dawnmere**. The new town connection stays hidden until the Lich is defeated.
The stairs-down key (`.`) does not activate the exit; surviving enemies do not
block it. Entering the crypt from either town always starts at floor 1.

Return-to-town spells and their portals remember your entrance town. Save
version **167** preserves that entrance and portal destination; older saves
default to Oakhaven. Older completed dungeon approaches gain the connecting
road while preserving exploration, position and dropped items. Save version
**168** narrowed existing road maps; version **169** straightens the road and
raises the crypt, safely moving any overlapping player or loot onto the paving.

The guaranteed **Cryptblade** grants +2 attack and 10% critical chance and remains
Warrior-only. Normally it drops on the floor. If the floor loot list is
full, it remains in a named chamber reliquary without deleting other items.
An unclaimed blade lost to a floor refresh is restored there on revisiting,
even in a fresh expedition with the Lich already defeated. Interact beside the
reliquary or press `P` while standing on it; a full inventory leaves it available.
After the first claim, dropping or selling the blade does not create another.

Save version **124** stores pending attacks, phase, finite-wave use, both warned
summon coordinates and reward claim state in active/cached encounters. Older
living encounters gain seals and safe cover in place, preserving boss health,
existing enemies, exploration, player position, quests and loot. Old unmarked
bolt timing restarts with a visible warning. Previous victories remain defeated;
only a known, currently dropped legacy Cryptblade is treated as unclaimed.

[Renderer review and play-test checklist](art-sources/lich-encounter/README.md).

Save version 109 converts old boss-room return doors and downward stair markers
into the special staircase, including cached floors, loot underlays, and portal
destinations. The stairs remain sealed if the Lich King is still alive.

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

Cain's Scroll of Return to Town teaches a zero-MP, one-way escape to the last
town visited. It leaves no return portal; re-enter through the normal dungeon
entrance. See [Return to Town](spells.md#return-to-town).

Save version 85 merges the old eight floors into five, retaining the Lich
chamber on floor 5 and preserving seal completion, character progress, active
terrain, enemies, loot, and portal access.
