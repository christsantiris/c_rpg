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
| 1 | Gatehouse | Guards, shield formations, and portcullis controls |
| 2 | Iron Keep | Castellan miniboss; permanent passage to the grounds |
| 3 | Forsaken Court | Court Hexers, Bell Heralds, flanking routes, and trapdoors |
| 4 | Crown Chapel | Royal Arcanist miniboss; shifting wards and a second permanent passage |
| 5 | Royal Archives | Mixed guards and bookcases on the approach to the throne |
| 6 | Throne of No Return | Lord Veyr; his defeat ends the campaign with victory |

Each floor has six rooms linked by loops. Closed portcullises and magical
barriers retain alternate routes. The entry room and stairs are free of traps;
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
| Bell Herald | Announces one reinforcement call; killing or freezing interrupts or delays it |

Gold markers show shield facing; they dim during recovery. Flanking and
recovery attacks bypass shields. Frontal hits always deal at least one damage.
All three classes can fight the guards and operate levers. Press **Interact**
(default `A`) on or beside a lever to change its portcullis.

The Castellan marks a charge lane or a close frontal swing, then recovers.
Sidestepping avoids the charge; colliding with masonry briefly staggers him.
The Royal Arcanist marks a small area and warns before switching ward barriers.
Occupied tiles stay open when a barrier closes.

Violet outlines mark attacks' fixed destinations before damage is applied.
Normal attacks and Castellan attacks allow one movement turn; Arcanist attacks
allow two. Freezing an enemy pauses its pending attack and its ward change.
Cover blocks ranged attacks even if the barrier closes after the warning starts.

Lord Veyr has one continuous health bar and three phases: Armored Ruler above
two-thirds health, Broken Throne down to one-third, and Unbound Crown below it.
His frontal armor gives way to magic with expanding attack areas and changing
wards. The latter phases allow two and three movement turns to dodge.
Crossing a health threshold does not enlarge an already announced attack.
Veyr calls at most two escorts per attempt. Defeating him immediately records
victory, stops combat, and opens the ending and Hall of Fame flow.

## Progression

Defeating the Castellan and Royal Arcanist opens the route to the throne.
No regional collectibles are required; visiting other areas is optional.

## Trapdoors

One concealed trapdoor in an ordinary room on each of floors 2–5 drops the
player exactly one floor. There are no trapdoors in boss arenas, entrances,
stairs, checkpoint passages, or floor 6. An alternate route avoids every hole.

Triggered holes remain visibly open after revisiting, saving/loading, and
retreating. Entering an open hole causes another fall, but no turn can drop the
player multiple floors. Landing relocates enemies and cancels warnings covering
the arrival tile. The first fall rallies a small patrol farther along the route;
each trapdoor can call this patrol only once. Falls cause no separate damage
and preserve defeated minibosses, passages, terrain, and dropped items.

## Retreat and passages

Return to Town requests confirmation and becomes a one-way escape to Rosemoor.
It creates no return portal. Entering the castle also abandons any older portal.
Walking out or using an earned passage likewise ends the current attempt.

Ordinary defenders respawn, surviving bosses regain full health, and gates and
wards reset. Maps, dropped items, defeated minibosses, unlocked
passages, revealed trapdoors, and spent trapdoor/Herald calls persist. Defeated
minibosses never respawn or repeat their unique rewards.

Miniboss deaths reveal an **Interact** passage in the arena. Corresponding
**KEEP** and **CHAPEL** passage markers appear on the grounds, letting the player
resume at the cleared floor instead of repeating the lower floors.

## Persistence and play testing

Save version 89 records castle floor caches, loot and item metadata, facing,
queued attack phases, wards, miniboss deaths, trapdoor reveals, pending
confirmations, and victory. Earlier saves retain their castle and regional
progress. Migration removes retired royal seal markers and their extra guardians,
including markers beneath loot and portals, without regenerating explored maps.

Suggested play test:

1. Enter through both Crown Road approaches and cancel/confirm the entry warning.
2. Fight the mixed guards; flank Wardens and use levers for ranged cover.
3. Defeat the Castellan on floor 2 and the Arcanist on floor 4; use each passage
   in both directions and confirm the miniboss stays defeated after retreat.
4. Trigger a trapdoor, revisit its open hole, and save/load with loot over it.
5. Reach floor 6 without collecting objects from other regions.
6. Cancel and accept Return to Town; verify surviving defenders reset and
   permanent progress and dropped items survive.
7. Fight all three Veyr phases, dodge marked areas, and confirm the victory ending.

Automated tests cover all six floors' reachability, travel and boss gates,
combat warnings and cover, shield recovery, capped reinforcements, trapdoors,
passages, all three classes' victory paths, save round-trips, and legacy migration.
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
