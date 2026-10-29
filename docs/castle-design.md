# Castle of No Return

The six-floor castle interior is implemented and ready for play testing.
Reach the safe grounds through either Crown Road, approach the north castle
entrance, and confirm entry. The south gate leads onto the northern road, past the Royal Catacombs
and Elderwood toward Oakhaven.

The grounds also contain the **Royal Dungeon** and a townsman who secretly reports
you to the Royal Guards when spoken to with `T`. This one-time arrest begins
an optional dungeon escape and prisoner escort to Ridgeshire; it does not reset
castle victories or equipment. See [Royal Dungeon](royal-dungeon.md).

The interior is drawn at the Crown Roads' 1.5× zoom: the camera shows about
30 × 18 tiles around the player. Ranged defenders fire from at most eight tiles,
so they stay on screen; a wounded Royal Arcanist or Lord Veyr retaliates from up to twelve, and the camera leans toward them to keep them on screen across their 22 × 14 arenas.

## Floors and appearance

The player climbs from the gatehouse on floor 1 to the throne on floor 6.
Charcoal masonry, gold trim, crimson carpets and banners, stained glass,
torches, pillars, court tables, archives, and a royal throne establish the
fortress theme. The approved roster includes shared Royal Guards and a Vampire
armory miniboss, alongside dedicated royal defenders and the final boss.

| Floor | Setting | Encounter and progress |
|---|---|---|
| 1 | Gatehouse | Two guarded approaches with independent portcullis levers and firing lanes |
| 2 | Iron Keep | Vampire miniboss, guard detail and side portcullis controls |
| 3 | Forsaken Court | Royal Guards defend the court alongside marksmen and hexers |
| 4 | Crown Chapel | Royal Arcanist's two phases, sequential ward lanes and pedestal counters |
| 5 | Royal Archives | Shelves narrow sightlines; roaming soldiers and warned fire lanes threaten the approaches |
| 6 | Throne of No Return | Veyr combines guarded approaches, ward pedestals, Royal Guards, fire lanes, and mixed escorts |

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
| Royal Guard | Shared armored melee defender from the Royal Dungeon and Crown Roads |
| Vampire | Existing melee enemy; the armory miniboss retains floor-3 progression and its unique reward |

Gold markers show shield facing; they dim during recovery. Flanking and
recovery attacks bypass shields. Frontal hits always deal at least one damage.
All three classes can fight the guards and operate levers. Press **Interact**
(default `A`) on or beside a lever to change its portcullis.

The armory Vampire uses ordinary castle melee warnings and recovery. It has no
charge, shield phase or reinforcement call. Its Warden and Marksman detail
remains, and its defeat retains the first castle miniboss completion flag.

Two side levers at **(37,51)** and **(53,51)** control their own three-stone
portcullises at **x=40** and **x=50**, **y=47..49**. Both begin open. Actor- or
loot-occupied cells remain open; paths around each gate preserve access to the
stairs and both levers.

Victory preserves the **100 gold** and floor-3 unlock and reserves **Castellan's
Bulwark** in the southeast armory chest at **(53,55)**. Interact on or beside it
to claim the rare Warrior shield: **+8 defense, 30% block**, with the usual 50%
block damage reduction. It requires a one-handed main weapon. A full inventory
leaves it reserved; maximum floor loot cannot prevent the award. The reservation
survives retreat, floor travel and save/load. Claiming, dropping or selling the
shield cannot award another copy.

Save version **222** migrates the replaced castle actors in active and cached
floors. Existing wounds, deaths, victories, armory reservations and claimed
items remain intact; retired attack and summon warnings are cleared.

The Royal Arcanist's **Warded Scholar** phase retains a two-action, five-stone
diamond blast. At or below half health, **Shattered Liturgy** alternates a
two-action **Chapel Cross** with a sequence of separately warned ward lanes.
The cross reaches three stones in each cardinal direction from the locked
target; its diagonals remain safe. The current warning and recovery finish
before a health-phase change, and healing cannot reverse the phase.

The west pedestal at **(37,54)** powers a horizontal lane through the player's
declared row across **x=34..55**. After it resolves, an active east pedestal at
**(53,54)** declares a new vertical lane through the player's then-current
column across **y=44..57**. Each lane has its own full two-action warning;
they never strike simultaneously. A previously spent source is skipped. With
both sources spent, normal blasts alternate with crosses, so the boss remains
dangerous and vulnerable. Completing a blast, cross or lane sequence leaves
one full recovery action. All classes can disable a lane's source during its
preparation to cancel it and its barrier change, ending the sequence and
leaving two full counterattack actions after Interact.

The same saved footprint drives warnings and damage. **W2/W1** and **E2/E1**
identify lanes; plain **2/1** labels blasts and crosses. Gold **C2/C1** and
**O2/O1** mark planned gate closures and openings. Combined **X2/X1** labels
indicate both a gate change and a damaging warning on that
stone. Gate plans lock at declaration; opening cover cannot expose extra
unwarned cells. Actor- and loot-occupied gates never close. Escape checks account
for planned closures; a trapped preparation misses and only opens gates.
Freezing pauses the warning, lane sequence and gate plan together. The HUD
shows phase, health, attack, remaining actions and recovery.

The Arcanist stays inside the chapel arena. After taking damage or engaging,
he can answer clear corridor fire at every bow's range with a marked blast or
cross, so longbows cannot attack an inert boss from a safe approach. Ward lanes
remain inside the arena. A distant or hidden retreat outside the arena cancels pending warnings;
ordinary floor travel and castle regrouping retain their existing rules.

Victory preserves **100 gold** and the floor-5 unlock, and reserves **Crown
Chapel Vestments** in the southeast chest at **(53,56)**. Interact on or beside
it to collect the rare Mage robes: **+4 defense**, **+45 maximum MP**, and
**15% spell-cost reduction**. Maximum floor loot and a full inventory cannot
erase the award. It persists through floor travel, retreat and save/load; a
claimed, sold or dropped copy never regenerates. Purple cloth, gold trim and
cyan accents appear in its inventory/equipment icon and equipped player art.

Save version **138** preserves both health phases, all four attack footprints,
barrier plans, sequence position, warning/recovery timers, freeze, spent wards
and reward claims in active and cached floors. Existing queued blasts retain
their timing and small footprint, even at low health. Migration never rebuilds
the map or resets player position, enemies, loot, exploration, traps or castle
progress. A historical victory reserves the new Vestments once without
repeating gold, XP or the boss fight. See the
[Arcanist renderer review and play testing](art-sources/royal-arcanist-encounter/README.md).

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
Normal attacks allow one movement turn; Arcanist attacks allow two. Freezing an enemy pauses its pending attack and its ward change.
Cover blocks ranged attacks even if the barrier closes after the warning starts.

Lord Veyr has one continuous health bar and three phases: Armored Ruler above
two-thirds health, Broken Throne down to one-third, and Unbound Crown below it.
Health, base attack, defense, XP and the 10,000-point victory bonus remain.
Phases latch forward and wait for queued attacks and recovery to finish.

- **Armored Ruler:** frontal armor halves incoming damage before ward protection.
  A fixed close Oathblade Swing alternates with a **Throne Advance**, moving up
  to three marked cardinal stones. These give one action of warning; the first
  attack is extended to at least two when announcing escorts. Sidestep the
  advance and flank during recovery. Veyr never moves onto actors or outside
  the arena. Corridor fire can provoke a two-action Royal Edict rather than
  allowing ranged attacks against an inert boss; ordinary cover still blocks
  incoming shots and engagement.
- **Broken Throne:** armor gives way to a two-action five-stone Royal Edict,
  then separately prepared west/east ward lanes. West marks a fixed row across
  x=34..55; east marks a fixed column across y=44..57. Spent sources are skipped.
  Disabling a prepared lane's pedestal cancels its damage and gate changes;
  its announced countdown still finishes, preserving any queued escort arrival.
  With both wards spent, the boss continues using Royal Edicts.
- **Unbound Crown:** a fixed, radius-two diamond gives three ordinary actions
  of warning. After its recovery, the existing nine-stone throne rune lane
  at y=51, x=41..49 receives its own two-action fire warning. The crown and fire
  never damage simultaneously. Fire retains 24 minus half defense damage,
  minimum 6, with one eruption and no repeated floor damage. The old independent
  throne-fire clock is suspended while Veyr lives; earlier phases leave the runes
  dormant. Archive fire remains unchanged.

Every signature or fire eruption leaves one full recovery action. Completing
a crown with **both pedestals spent** instead leaves **two** full counterattack
actions. Pedestals retain their existing damage/protection effects and are
available to every class. They stay spent for the attempt and reset around a
surviving boss only when the castle regroups on retreat. Freeze pauses a queued
attack, escort preparation, fire warning and gate plan together. A resolved fire
impact clears visually on the following action, including while Veyr is frozen.

Footprints and barrier plans lock at declaration. C/O previews distinguish gate
closures/openings; W/E identify ward lanes; F identifies throne fire; X identifies
a damaging cell with a gate change. Numbers count remaining player actions.
Occupied and loot-covered gate cells stay open. Walking escape checks include
planned closures; an inescapable preparation misses and only opens gates.

Veyr announces at most two escorts per attempt, a Warden and a Marksman, at
G-marked stones (42,46) and (48,46). They arrive on empty stones when the warning
resolves and act from the following turn. Occupied arrivals or a full roster
consume the call without retrying. New escorts grant no gold, items, XP or score.
Royal Guards replace the former reinforcement callers. Returning between
castle floors preserves the attempt; retreat resets a living Veyr and his escort
allowance according to existing castle rules.

Defeating Veyr awards **Crown of the Unbound** before setting campaign victory,
stops combat/fire immediately and grants the existing 10,000 score once. The crown
is a permanent trophy, requiring no inventory slot, loot space or post-combat
pickup. The boss journal records it; the ending displays it and allows the bound
journal key (default Q) to inspect the completed boss record. Closing that journal
returns to the ending. Enter retains the original Hall of Fame flow.

Save version **139** preserves the trophy, active/cached signature masks, phases,
gate plans, escort flags, sequence state, warning clocks and long recovery. Older
pending attacks retain their small/large geometry and remaining warning time.
An old overlapping fire warning is deferred until that signature finishes, then
receives a fresh two-action warning. No maps, enemies, loot, exploration, player
position or progression are regenerated. Historical campaign victories receive
the trophy without another fight or score bonus.

Hall of Fame records now store the trophy per ranked run and retain a permanent
earned flag even if a victory does not reach the top ten. The application records
that history on victory before leaving the ending. Legacy score rows preserve
their names/scores and receive no invented victory status. See the
[Veyr renderer review and play testing](art-sources/lord-veyr-encounter/README.md).

## Progression

Defeating the armory Vampire and Royal Arcanist opens the route to the throne.
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

Return to Town requests confirmation and becomes a one-way escape to Castle
Grounds beside the castle entrance. This is the exception to the usual last-town
destination.
It creates no return portal. Entering the castle also abandons any older portal.
Walking out through the floor 1 stairs likewise ends the current attempt.

Ordinary defenders respawn, surviving bosses regain full health, and gates and
wards reset. Maps, dropped items, defeated minibosses,
revealed trapdoors, and spent trapdoor calls persist. Defeated
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
disabled pedestals, patrol directions, and Royal Guard positions survive
save/load. Migration runs once; resaving does not duplicate new defenders.

Suggested play test:

1. Enter through both Crown Road approaches and cancel/confirm the entry warning.
2. Fight the mixed guards; flank Wardens and use levers for ranged cover.
   Try both Gatehouse approaches and their independent levers.
3. Defeat the Vampire on floor 2 and the Arcanist on floor 4; climb onward and
   confirm the miniboss stays defeated after retreat, with no shortcut portals.
   Bait a charge into a guard, and disable both chapel pedestals.
4. Trigger a trapdoor, revisit its open hole, and save/load with loot over it.
5. Reach floor 6 without collecting objects from other regions.
6. Cancel and accept Return to Town; verify surviving defenders reset and
   permanent progress and dropped items survive.
7. Fight all three Veyr phases, dodge marked areas, and confirm the victory ending.
8. Fight Royal Guards in the court and throne room; verify they cannot summon patrols.
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
