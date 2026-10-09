# Castle of No Return

The six-floor castle interior is implemented and ready for play testing.
Reach the safe grounds through either Crown Road, approach the north castle
entrance, and confirm entry. The south gate still leads to the Royal Catacombs.

The grounds also contain the **Royal Jail** and a townsman who secretly reports
you to the Royal Guards when spoken to with `T`. This one-time arrest begins
an optional jail escape and prisoner escort to Ridgeshire; it does not reset
castle victories or equipment. See [Royal Jail](royal-jail.md).

## Floors and appearance

The player climbs from the gatehouse on floor 1 to the throne on floor 6.
Charcoal masonry, gold trim, crimson carpets and banners, stained glass,
torches, pillars, court tables, archives, and a royal throne establish the
fortress theme. Review sprites supply five regular enemies, two minibosses,
and three appearances of the final boss.

| Floor | Setting | Encounter and progress |
|---|---|---|
| 1 | Gatehouse | Two guarded approaches with independent portcullis levers and firing lanes |
| 2 | Iron Keep | Castellan with a Warden and Marksman guard detail; bait his charge into his guards |
| 3 | Forsaken Court | Protected Herald calls a capped three-guard patrol through a marked doorway |
| 4 | Crown Chapel | Disable two ward pedestals while fighting the Royal Arcanist and his guards |
| 5 | Royal Archives | Shelves narrow sightlines; roaming soldiers and warned fire lanes threaten the approaches |
| 6 | Throne of No Return | Veyr combines guarded approaches, ward pedestals, a throne-room Herald, fire lanes, and mixed escorts |

Each floor has six rooms linked by loops. Closed portcullises and magical
barriers retain alternate routes. Tables stay outside the carpeted corridors,
so every room is accessible without moving furniture. The entry room and stairs are free of traps;
bosses remain inside their arenas. Use the configured **Ascend stairs** command
(default `,`) to climb and **Descend stairs / exit** (default `.`) to descend.
Descending from floor 1 returns you to the castle grounds.

## Enemies and combat

| Enemy | Mechanic |
|---|---|
| Oathbound Soldier | Advances into melee; warns before striking a fixed tile |
| Iron Warden | Halves frontal damage and shields nearby allies behind it |
| Royal Marksman | Shoots through clear lanes; closing a portcullis provides cover |
| Court Hexer | Ranged magic and an attack bonus for nearby allies |
| Bell Herald | Announces one reinforcement call; Court and throne-room calls summon up to three guards at a marked doorway; killing or freezing interrupts or delays it |

Gold markers show shield facing; they dim during recovery. Flanking and
recovery attacks bypass shields. Frontal hits always deal at least one damage.
All three classes can fight the guards and operate levers. Press **Interact**
(default `A`) on or beside a lever to change its portcullis.

The Castellan marks a charge lane or a close frontal swing, then recovers.
Sidestepping avoids the charge; colliding with masonry briefly staggers him.
Colliding with one of his guards interrupts that guard's attack and disables
its shield for two turns. His guard detail does not return after his defeat.
The Royal Arcanist marks a small area and warns before switching ward barriers.
Occupied tiles stay open when a barrier closes.

The Gatehouse's east portcullis begins closed; its south portcullis begins open.
Each lever changes only its own approach. The throne floor repeats this setup.
Closing a gate blocks a Marksman's shot, including an already warned shot.

Two glowing pedestals stand at the chapel arena's southwest and southeast corners.
Use **Interact** on or beside either to disable it. Each active pedestal reduces
incoming boss damage by **20 percentage points** and adds **4 attack damage**.
The throne arena has the same pedestals. All classes can disable them. Disabled
pedestals persist through floor revisits and saving; retreat restores them around
a surviving boss. They never prevent damaging or defeating a boss.

In the Archives, ordinary soldiers patrol their rooms even before spotting the
player. Shelves leave the main carpeted corridors open but provide additional
cover and side approaches. Etched fire lanes pulse every four combat turns:
two warning turns, a damaging eruption, and a quiet turn. Orange overlays show
**2**, then **1**, turns until eruption. Leave the marked row before it strikes.
Fire deals **24 minus half defense**, with a minimum of **6 damage**. It remains
visible and dangerous beneath dropped loot. The throne arena repeats a fire lane
across its southern approach; entrances, stairs, and trapdoor landing tiles are safe.

Violet outlines mark attacks' fixed destinations before damage is applied.
Normal attacks and Castellan attacks allow one movement turn; Arcanist attacks
allow two. Freezing an enemy pauses its pending attack and its ward change.
Cover blocks ranged attacks even if the barrier closes after the warning starts.

Lord Veyr has one continuous health bar and three phases: Armored Ruler above
two-thirds health, Broken Throne down to one-third, and Unbound Crown below it.
His frontal armor gives way to magic with expanding attack areas and changing
wards. The latter phases allow two and three movement turns to dodge.
Crossing a health threshold does not enlarge an already announced attack.
Veyr calls at most two escorts per attempt: a Warden and a Marksman. His
throne-room Herald can separately call one patrol of up to three guards through
the marked doorway. New reinforcements act on the following turn. Defeating him immediately records
victory, stops combat, and opens the ending and Hall of Fame flow.

## Progression

Defeating the Castellan and Royal Arcanist opens the route to the throne.
No regional collectibles are required; visiting other areas is optional.

## Trapdoors

One concealed trapdoor in an ordinary room on each of floors 2–5 drops the
player exactly one floor. There are no trapdoors in boss arenas, entrances,
stairs, or floor 6. An alternate route avoids every hole.

Triggered holes remain visibly open after revisiting, saving/loading, and
retreating. Entering an open hole causes another fall, but no turn can drop the
player multiple floors. Landing relocates enemies and cancels warnings covering
the arrival tile. The first fall rallies a small patrol farther along the route;
each trapdoor can call this patrol only once. Falls cause no separate damage
and preserve defeated minibosses, terrain, and dropped items.

## Retreat

Return to Town requests confirmation and becomes a one-way escape to Rosemoor.
It creates no return portal. Entering the castle also abandons any older portal.
Walking out through the floor 1 stairs likewise ends the current attempt.

Ordinary defenders respawn, surviving bosses regain full health, and gates and
wards reset. Maps, dropped items, defeated minibosses,
revealed trapdoors, and spent trapdoor/Herald calls persist. Defeated
minibosses never respawn or repeat their unique rewards.

Miniboss victories unlock the onward stairs. They do not create exit portals
or shortcut entrances on the grounds; entry uses the main castle entrance.

## Persistence and play testing

Save version 89 records castle floor caches, loot and item metadata, facing,
queued attack phases, wards, miniboss deaths, trapdoor reveals, pending
confirmations, and victory. Earlier saves retain their castle and regional
progress. Migration removes retired royal seal markers and their extra guardians,
including markers beneath loot and portals, without regenerating explored maps.

Version **119** removes the retired miniboss doorways and the tables that block
carpeted corridors on floors 3 and 4, in both the live map and saved floor caches.
Loot underlays are repaired too. The migration preserves player position,
enemy damage and defeats, miniboss victories, exploration, gates, and trapdoors.

Version **120** adds the floor encounters to live and cached castle maps without
regenerating them. Shelves skip tiles occupied by the player, living enemies, or
loot. Existing enemy damage, deaths, loot, exploration, and miniboss victories
remain intact; cleared floors gain no additional guards. Fire warning phases,
disabled pedestals, patrol directions, and each Herald's spent call survive
save/load. Migration runs once; resaving does not duplicate new defenders.

Suggested play test:

1. Enter through both Crown Road approaches and cancel/confirm the entry warning.
2. Fight the mixed guards; flank Wardens and use levers for ranged cover.
   Try both Gatehouse approaches and their independent levers.
3. Defeat the Castellan on floor 2 and the Arcanist on floor 4; climb onward and
   confirm the miniboss stays defeated after retreat, with no shortcut portals.
   Bait a charge into a guard, and disable both chapel pedestals.
4. Trigger a trapdoor, revisit its open hole, and save/load with loot over it.
5. Reach floor 6 without collecting objects from other regions.
6. Cancel and accept Return to Town; verify surviving defenders reset and
   permanent progress and dropped items survive.
7. Fight all three Veyr phases, dodge marked areas, and confirm the victory ending.
8. Interrupt a Court Herald and allow a separate call to observe its doorway patrol.
9. Walk the Archives' fire lanes during both countdown turns; dodge one eruption
   and deliberately take another. Save during a warning and confirm it resumes.

Automated tests cover all six floors' reachability, travel and boss gates,
combat warnings and cover, shield recovery, capped reinforcements, trapdoors,
retired portal removal, all three classes' victory paths, save round-trips, and legacy migration.
An SDL software-rendering check verifies sprite loading and attack overlays.
Enemy statistics and encounter pacing still need hands-on balance feedback.

## Review artwork

- [Regular enemies](art-sources/castle-review/castle-enemies.png)
- [Minibosses and Veyr phases](art-sources/castle-review/castle-bosses.png)
- [Interior room themes](art-sources/castle-review/castle-interiors.png)
- [Generation prompts](art-sources/castle-review/prompts.md)

The interior review board predates the reordered floors: its Iron Keep and
Crown Chapel panels correspond to floors 2 and 4; Forsaken Court and Archives
correspond to floors 3 and 5. Gameplay uses native tile rendering with these themes.
