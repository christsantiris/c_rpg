# Castle of No Return

The six-floor castle interior is implemented and ready for play testing.
First reach the grounds from Stillbury through Blackwater Swamp, the Royal
Catacombs and Crown Road North, then approach the castle entrance and confirm
entry. The lateral Crown Roads reach Rosemoor and Ridgeshire for later
adventures. The south gate provides retreat along the northern route to Stillbury.

The grounds also contain the **Royal Dungeon** and a townsman who secretly reports
you to the Royal Guards when spoken to with `T`. This one-time arrest begins
an optional dungeon escape and prisoner escort to Stillbury; it does not reset
castle victories or equipment. See [Royal Dungeon](royal-dungeon.md).

The interior is drawn at the Crown Roads' 1.5× zoom: the camera shows about
30 × 18 tiles around the player. Ranged defenders fire from at most eight tiles,
so they stay on screen; a wounded Royal Arcanist or Lord Veyr retaliates from up to twelve, and the camera leans toward them to keep them on screen across their 22 × 14 arenas.

## Floors and appearance

The player climbs from the gatehouse on floor 1 to the throne on floor 6.
Charcoal masonry, gold trim, crimson carpets and banners, stained glass,
torches, pillars, court tables, archives, and a royal throne establish the
fortress theme. The approved roster includes shared Royal Guards and Royal
Alchemist Malakar in an early laboratory encounter, alongside dedicated royal
defenders and the current final boss.

| Floor | Setting | Encounter and progress |
|---|---|---|
| 1 | Gatehouse and Lower Keep | Ten connected rooms, two guarded approaches, an eastern gallery and a permanent service shortcut |
| 2 | Royal alchemy Wing | Early Malakar encounter, interrupted preparations, escape at 40% health, and ten explorable rooms |
| 3 | Court and Household | Great Hall, western and eastern flanking routes, a permanent service shortcut, and Chamberlain Edras |
| 4 | Crown Chapel | Royal Arcanist's two phases, sequential ward lanes and pedestal counters |
| 5 | Royal Archives | Shelves narrow sightlines; roaming soldiers and warned fire lanes threaten the approaches |
| 6 | Throne of No Return | Veyr combines guarded approaches, ward pedestals, Royal Guards, fire lanes, and mixed escorts |

Floors 4–6 retain six rooms linked by loops. Closed portcullises and magical
barriers retain alternate routes. Tables stay outside the carpeted corridors,
so every room is accessible without moving furniture. The entry room and stairs are free of traps;
bosses remain inside their arenas. Use the configured **Ascend stairs** command
(default `,`) to climb and **Descend stairs / exit** (default `.`) to descend.
Descending from floor 1 returns you to the castle grounds.

## V2 gatehouse and lower keep

Floor 1 now has a fixed architectural layout rather than six matching halls.
The entrance hall connects to the east watch through a guarded portcullis, or
south into the barracks through a separately controlled portcullis. Narrow,
offset doorways lead through the mess hall, kitchen, stores and armory. An
eastern guard gallery connects the watch to the upper stair hall, providing
another approach around the mess hall. Room names appear in the game renderer.

The existing entrance levers still operate their own portcullises. A third
lever in **Service Control**, reached from the lower keep, permanently opens
the central service passage to the entrance hall. It cannot be reached through
the sealed shortcut from the entrance side. Its three gate stones stay open
through floor travel, castle retreat, defender regrouping and save/load.

The stores contain one Health Potion; the kitchen contains one Mana Potion.
These supplies are placed only on first generation and do not replenish when
defenders regroup. The ten defenders occupy the rooms and gallery rather than
repeating a pair at every room center. There is no required boss on this floor.

The entry stair remains at **(15,11)** and is labeled **DOWN: CASTLE GROUNDS**.
The onward stair remains at **(45,55)** and is labeled **UP: alchemy WING**. Both
use the established castle stair commands, and discovered stairs retain their
minimap markers. The upper-floor trapdoor landing at **(8,15)** stays usable.

Save version **253** converts active and cached gatehouses in place. Exploration,
actor positions, wounds and deaths, dropped items, equipment, quests, boss
victories and operated entrance gates remain intact. If new masonry overlaps
an old occupant or item, a short connected alcove preserves its position. Old
gatehouses receive no replacement supply drops. Floors 4–6 keep their current
layouts and encounters for subsequent V2 changes.

## Enemies and combat

| Enemy | Mechanic |
|---|---|
| Oathbound Soldier | Advances into melee; warns before striking a fixed tile |
| Iron Warden | Halves frontal damage and shields nearby allies behind it |
| Royal Marksman | Shoots through clear lanes; closing a portcullis provides cover |
| Court Hexer | Ranged magic and an attack bonus for nearby allies |
| Royal Guard | Shared armored melee defender from the Royal Dungeon and Crown Roads |
| Malakar, Royal Alchemist | Warned concoctions, interruptible restorative, and a permanent escape at 40% health |

Gold markers show shield facing; they dim during recovery. Flanking and
recovery attacks bypass shields. Frontal hits always deal at least one damage.
All three classes can fight the guards and operate levers. Press **Interact**
(default `A`) on or beside a lever to change its portcullis.

## V2 Royal alchemy Wing

Malakar works in the laboratory immediately east of level 2's receiving hall.
The entry stair remains at **(15,11)**, with an uninterrupted retreat to the
gatehouse. A visible laboratory seal identifies the route into the lower wing;
it opens when Malakar flees. The onward stair at **(45,55)** leads to the
Court and Household and uses the existing encounter checkpoint.

Malakar introduces himself when engaged. His **Bursting Vial** marks a small
five-stone diamond for **two player actions**. **Volatile Overpressure**, powered
by a glowing condenser, marks a larger radius-two diamond for **three actions**.
Footprints lock at declaration, respect masonry and furnishings, and resolve
once before a full recovery action. Freezing pauses the countdown. If walls
and actors leave no walking escape within the warning time, the mixture vents
harmlessly. Breaking line of sight cancels a pending preparation.

At or below 70% health, Malakar can prepare **one restorative per attempt**.
The HUD and an R-marked source announce its two-action preparation. Damage him,
or use **Interact** beside its linked condenser at **(38,16)** or **(51,16)**,
to interrupt it and create two counterattack opportunities. An uninterrupted
restorative heals 25% of maximum health, capped at full health. Interacting with
the source of a warned overpressure also cancels that mixture. All classes can
operate the apparatus. Drained condensers remain spent through floor revisits,
retreat and save/load.

Any damage that would reduce Malakar to **40% health or lower** instead stops
at that threshold and makes him flee immediately, including an oversized melee,
arrow, spell or companion hit. He remains alive, disappears from the room, and
cannot be fought here again. The escape clears his warnings, breaks the lab seal,
awards **100 gold and 300 XP**, and reserves **Castellan's Bulwark** in the chest
at **(53,55)**. Existing chest and inventory-capacity rules still apply. It is
not campaign victory. A surviving Malakar regroups when the player leaves the
castle, following the existing castle retreat rule.

The opened wing contains a specimen room, infirmary, reagent vault, distillery,
experiment room and private study, connected by offset doors and looping routes.
Malakar's ledger in the study explains that the crown's binding can restore him
in the King's chamber. The royal passage behind his laboratory shelves is sealed
from within; the player continues through the upper castle. Both discoveries
use **Interact** beside their labeled shelves. Room and transition labels remain
readable in small windows, and discovered stairs retain minimap markers.

Save version **254** migrates active and cached Iron Keeps. The old boss becomes
Malakar while retaining its position, health, wounds or death; incompatible old
attack warnings are cleared. Existing completed encounters and claimed or reserved
equipment remain completed without another payout. Exploration, other defenders,
loot, player position, equipment, quests and other castle victories are retained.
A migrated wing keeps the laboratory seal open so actors already south of it
remain reachable; the onward stair still requires the encounter checkpoint.
Old positions covered by new masonry receive connected alcoves. Encounter warning
masks, countdowns, restorative allowance and escape progress use the existing saved
actor and castle records. The later castle stages and final Malakar confrontation
remain subsequent V2 changes.

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
repeating gold, XP or the boss fight.

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
their names/scores and receive no invented victory status.

## V2 Court and Household

Floor 3 surrounds a larger **Great Hall** with a receiving hall, council chamber,
guest chambers, west and east galleries, servants' quarters, linen store,
household office and upper stair hall. Long banquet tables and pillars divide
the Great Hall into fighting lanes. Royal Guards and a Court Hexer hold the
hall while marksmen cover the galleries; guest pallets, service bedding and
linen stores give the surrounding rooms distinct uses.

The **western route** runs through the west gallery and servants' quarters to
the linen store and upper stair. The **eastern route** crosses the council and
guest chambers, household office and east gallery. Either route can reach the
chapel stair without crossing the Great Hall or defeating every defender.
The household lever at **(19,49)** permanently opens the service gate at
**(19..20,43)**, shortening retreat to the receiving hall. Its state survives
floor travel, castle regrouping and save/load; dropped loot cannot be erased
by operating it. There is no new boss or collection requirement on this floor.

**Chamberlain Edras**, a noncombat resident in the household office, explains
Malakar's dismissal of the royal attendants and points toward the Crown Chapel.
Stand beside him and use **Talk**; **Enter / Esc** closes the conversation.
Other keys are consumed while it is open. Existing Rosemoor or Ridgeshire
evidence adds context, but the conversation assigns no quest, consumes no
evidence and gives no repeatable reward. Council orders at **(29,6)** can be
read with **Interact** for another account of the royal household's isolation.

The return stair remains at **(15,11)**, labeled **DOWN: ALCHEMY WING**. The
onward stair remains at **(45,55)**, labeled **UP: CROWN CHAPEL**. Both use the
existing castle stair commands and discovered minimap markers. The optional
trapdoor at **(17,32)** retains a path around it and its existing fall rules.

Save version **255** converts active and cached Forsaken Courts in place,
preserving player and enemy positions, wounds, deaths, exploration, loot,
equipment, quests, castle victories and spent trapdoors. New masonry around
an old position receives a connected alcove. If an existing occupant or loose
item overlaps Edras's initial position, he moves to a nearby free office tile.
His location uses the existing serialized map; the service gate uses its saved
tiles, and conversation state uses the existing dialogue records. No new
Player or GameState fields are introduced. The Crown Chapel and later stages
remain subsequent V2 changes.

## Progression

Forcing Malakar to flee and defeating the Royal Arcanist opens the route to the throne.
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
3. Force Malakar to flee on floor 2 and defeat the Arcanist on floor 4; climb
   onward and confirm both encounters stay completed after retreat, with no
   shortcut portals. Interrupt Malakar's restorative and disable the chapel pedestals.
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

## Visual validation

Gameplay uses native tile rendering. The V2 gatehouse and alchemy wing are
checked in the game renderer at 1280×720 and 640×480, including room labels,
stairs, Malakar's preparations and escape, and migrated legacy maps.
