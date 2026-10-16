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
| 2 | Iron Keep | Castellan's two phases, guard detail, finite rally and side portcullis counters |
| 3 | Forsaken Court | Protected Herald calls a capped three-guard patrol through a marked doorway |
| 4 | Crown Chapel | Royal Arcanist's two phases, sequential ward lanes and pedestal counters |
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

The Castellan's **Shield Commander** phase marks a five-stone cardinal charge
or a three-stone frontal swing, each with one action of warning. At or below
half health, **Broken Formation** calls one two-action guard rally, then
alternates a wider **Formation Sweep** with separately warned charges. Sweeps
cover two rows of five stones in front of him and allow two actions to escape.
The current warning finishes before a phase change; healing cannot reverse it.
The HUD shows the phase, health, signature name, remaining actions and freeze.

Warnings lock their footprint at declaration. Opening a gate cannot extend a
queued attack. All signatures leave one full recovery action; masonry and guard
collisions expose the Castellan for two. A struck guard also loses its pending
attack and shield protection for two turns. Guards can intercept a charge before
it reaches a player farther down the lane. Charges remain inside the arena.
If cover or actors eliminate every ordinary walking escape, the attack is marked
blocked and misses rather than trapping the player in an unavoidable hit.

Two side levers at **(37,51)** and **(53,51)** control their own three-stone
portcullises at **x=40** and **x=50**, **y=47..49**. Both begin open. Every class
can Interact on or beside a lever. Closing a gate across a marked charge
interrupts it and leaves two full counterattack actions after Interact. Actor- or
loot-occupied cells stay open; operating a partially closed gate closes the remaining empty
cells, and operating a fully closed gate reopens it. Paths around each gate
remain walkable, including access to the stairs and both levers.

The rally warns the two muster stones at **(42,46)** and **(48,46)** with **G2**,
then **G1**. At most one Warden and one Marksman arrive on empty marked stones;
occupied arrivals are skipped. They act from the following turn and award no
gold, items or XP. The call is spent at declaration and cannot be replenished by
saving, floor travel or castle retreat. His original guard detail does not return
after his defeat.

Victory preserves the **100 gold** and floor-3 unlock and reserves **Castellan's
Bulwark** in the southeast armory chest at **(53,55)**. Interact on or beside it
to claim the rare Warrior shield: **+8 defense, 30% block**, with the usual 50%
block damage reduction. It requires a one-handed main weapon. A full inventory
leaves it reserved; maximum floor loot cannot prevent the award. The reservation
survives retreat, floor travel and save/load. Claiming, dropping or selling the
shield cannot award another copy.

Save version **137** retains phase, fixed warning bits, countdown, facing, spent
rally, recovery, gates and reward state in active and cached floors. Older saves
receive the open gates, levers and muster stones without regenerating enemies or
resetting damage, exploration, traps or loot. Prior victories remain complete
and reserve the newly introduced Bulwark once, without repeating gold or XP.
See [Castellan renderer review and play testing](art-sources/castellan-encounter/README.md).

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
Normal attacks and Castellan charges/swings allow one movement turn; Castellan
sweeps/rallies and Arcanist attacks allow two. Freezing an enemy pauses its pending attack and its ward change.
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
