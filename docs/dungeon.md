# Castle Dungeon

Enter the five-floor dungeon from Oakhaven's south road. The lightly wooded
approach is a local cul-de-sac, with no onward connection to Dawnmere. See
[the approved V2 layout](design/chapter-1-oakhaven.md). Entry always starts at
floor 1 and proceeds forward to floor 5. Its rooms and
connecting corridors are generated for each new expedition, then cached while
that expedition is in progress. Backtracking preserves explored rooms, enemies,
opened doors, and collected treasure. A later expedition generates new maps
and regular enemies, but a defeated Lich King does not return.

The dungeon is drawn at the Crown Roads' 1.5× zoom: the camera shows about 30 ×
18 tiles around the player. The Lich King acts within twelve tiles or whenever the player is in his chamber; the camera leans toward him to keep him on screen as his warned lances and rings begin.

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
local path south of Oakhaven. Follow it north to the town's south gate.
The dungeon remains revisitable after victory and never unlocks a road to
Dawnmere. The stairs-down key (`.`) does not activate the exit; surviving
enemies do not block it. Entering the crypt always starts at floor 1.

The Lich King guarantees the all-class **Crypt Ring**, an accessory granting
**+5 maximum HP and +5 maximum MP**, with no attack or defense bonus. It uses
either accessory slot and may be worn alongside another ring. Equipping it
does not restore current HP or MP. It replaces the former Cryptblade reward.

Normally the ring drops on the floor. If the floor loot list is full, it remains
in a named chamber reliquary without deleting other items. An unclaimed ring
lost to a floor refresh is restored there on revisiting, even with the Lich
already defeated. Interact beside the reliquary or press `P` while standing on
it; a full inventory leaves it available. After the first claim, dropping or
selling the ring does not create another. Its bonuses and claimed status persist
through saving and loading. [Reward review](art-sources/crypt-ring/README.md).

The following save-version notes describe historical V1 behavior. The V2
geography uses fresh saves, with no migration to the new routes.

Return-to-town spells and their portals remember your entrance town. Save
version **167** preserves that entrance and portal destination; older saves
default to Oakhaven. Older completed dungeon approaches gain the connecting
road while preserving exploration, position and dropped items. Save version
**168** narrowed existing road maps; version **169** straightens the road and
raises the crypt, safely moving any overlapping player or loot onto the paving.

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

Bram is Oakhaven's Innkeeper. Approach his counter inside the Inn and press `T`
for a warning about the Lich King in the dungeon south of town and advice to
buy supplies from the blacksmith and alchemist. His warning changes after the
Lich is defeated. Bram no longer stands in the town square or Stillbury's Tavern.
He now offers Save Elowen and rewards their reunion. Historical save
version 108 still repairs Lich guards; it no longer inserts a town-square Bram.

## Save Elowen

Bram offers **Save Elowen** at Oakhaven's Inn. Accept with `Y` or decline with
`N`; accepting preserves an expedition already in progress. Elowen is a captive
in the Lich King's final chamber, whether or not Bram has assigned the quest.
Defeat the Lich to free her automatically. Other surviving enemies do not delay
the rescue. The quest moves straight to **Completed** and releases its active
slot; a full journal never prevents an unassigned rescue.

Elowen follows the player without fighting, blocking movement or taking damage.
She accompanies stairs, roads, buildings and the one-way Return to Town spell.
Use the opened return stairs and the northward path to Oakhaven, or escape by
spell, then speak to Bram at the Inn. He pays **80 gold once**, including with
a full inventory or journal. Elowen then becomes a permanent Inn resident.
Speak to her to hear that the Lich King claimed to be in league with the King;
she does not yet know about Malakar.

The former Broken Seals assignment, floor objectives and guardian groups are
retired. The Lich encounter's two combat seals remain usable. Save version 229
records Elowen's escort position as well as assignment, rescue and reunion state.
This V2 rewrite uses fresh saves without a new migration of old quest progress.

Cain's Scroll of Return to Town teaches a zero-MP, one-way escape to the last
town visited. It leaves no return portal; re-enter through the normal dungeon
entrance. See [Return to Town](spells.md#return-to-town).

Save version 85 merges the old eight floors into five, retaining the Lich
chamber on floor 5 and preserving seal completion, character progress, active
terrain, enemies, loot, and portal access.
