# Quest reference and design proposals

Last checked against the game on **2026-10-27**.

The current implementation has **22 journal quests**. All five approved regional
quests below are implemented: **The Stolen Heartseed, The Missing Spice Skiff,
The Silent Stormbell, Break the War Machine, and Giant's Crown**.

## Current quest coverage

| Quest | Assigning NPC | NPC location | Area 1 | Area 2 | Area 3 |
| --- | --- | --- | --- | --- | --- |
| The Broken Seals | Elowen | Oakhaven — Tavern | Oakhaven Dungeon | — | — |
| Recover the Treasure Map | Dain | Dawnmere — Town center | Goblin Mountains | — | — |
| The Lost Wardens | Alder | Stillbury — Inn | Haunted Forest | — | — |
| Relight the Drowned Beacons | Mara | Oakhaven — Tavern | Sunken Coast | — | — |
| The Buried Sun | Nahla | Ruined Isle — Beside the temple | Ruined Temple | — | — |
| The Ivory Rook | Rook | Oakhaven — Tavern | Rook's Labyrinth | — | — |
| Bring Mira Home | Bram | Stillbury — Inn | Blackwater Swamp | — | — |
| The Dragon's Hoard | Ilya | Oakhaven — Tavern | Dragonspine | — | — |
| The Lost Magic Lamp | Zara | Stillbury — Inn | Sunscar Wastes | — | — |
| Reclaim the Emberforge | Steward Hadrin | Ridgeshire — Town Hall | Ashen Hollow | — | — |
| The Silent Expedition | Quartermaster Brenna | Stillbury — Inn | Frostfell Wastes | — | — |
| Reclaim the Quarry | Surveyor Orin | Rosemoor — Adventurer's Guild | Stonewake Quarry | — | — |
| Rest for the Forgotten | Brother Oswin | Ridgeshire — Town Hall | Royal Catacombs | — | — |
| Guide Tomas Home | Tomas | Royal Jail — Prison cell | Royal Jail | Escape Tunnel | Ridgeshire |
| Watchfires of Ridgeshire | Marshal Veyra | Ridgeshire — Town Hall | Goblin Mountains | Ashen Hollow | Dragonspine |
| The Three Great Hunts | Huntmaster Selene | Rosemoor — Adventurer's Guild | Frostfell Wastes | Stonewake Quarry | Sunscar Wastes |
| Recover Your Equipment | Automatic upon equipment confiscation | Royal Jail | Royal Jail — Guarded evidence room | — | — |
| The Stolen Heartseed | Forester Tamsin | Oakhaven — Town center, beside the north road | Elderwood Forest | — | — |
| The Missing Spice Skiff | Spice Merchant Corvin | Dawnmere — Spice Merchant shop | Saltwind Marsh | — | — |
| The Silent Stormbell | Watchkeeper Seren | Dawnmere — Town center, beside the east road | Stormwatch Headland | — | — |
| Break the War Machine | Marshal Veyra | Ridgeshire — Town Hall | Ironfang Mines | — | — |
| Giant's Crown | Antiquarian Edda | Rosemoor — Adventurer's Guild | Giant's Peaks | — | — |
| **No dedicated quest** | — | — | Crown Road East | — | — |
| **No dedicated quest** | — | — | Crown Road West | — | — |
| **No dedicated quest** | — | — | Crown Road North | — | — |
| Main objective: defeat Lord Veyr | — | — | Castle of No Return — Interior | — | — |

Defeating Lord Veyr is the main campaign objective and has no separate journal
entry. Ruined Isle is associated with The Buried Sun through Nahla. Castle
Grounds is associated with the jail, escape, and equipment recovery sequence.
Boss victories and road unlocks alone do not count as dedicated quests.

Implementation references:
[quest definitions](src/screens/quest_journal.c),
[NPC dialogue and quest acceptance](src/game/game.c),
[jail quests](src/game/jail.c), and
[Dawnmere's Spice Merchant Corvin](src/game/town_life.c).

## Regional quest designs

**All five designs are approved, including Break the War Machine.** Goblins are
villains; Ironfang's quest undermines their war supply operation.

Corvin, Marshal Veyra, Tamsin, Seren, and Edda are present in their listed locations.

| Quest | Design status | Area | Assigning NPC | NPC location | Main activity | Reward target |
| --- | --- | --- | --- | --- | --- | --- |
| The Stolen Heartseed | Implemented | Elderwood Forest | Forester Tamsin — existing | Oakhaven — Town center, near the north road | Follow a Dark Elf trail and free a grove from root bindings | 120 gold, 800 score |
| The Missing Spice Skiff | Implemented | Saltwind Marsh | Spice Merchant Corvin — existing | Dawnmere — Spice Merchant shop | Follow a wreck's cargo trail and recover spices from a remote island by skiff | 150 gold, 1,000 score |
| The Silent Stormbell | Implemented | Stormwatch Headland | Watchkeeper Seren — existing | Dawnmere — Town center, near the east road | Restore a stormbell and defend its courtyard using wind and shutters | 180 gold, 1,200 score |
| Break the War Machine | Implemented | Ironfang Mines | Marshal Veyra — existing | Ridgeshire — Town Hall | Destroy a goblin weapons shipment and defeat the Goblin Mine Boss | 160 gold, 1,000 score |
| Giant's Crown | Implemented | Giant's Peaks | Antiquarian Edda — existing | Rosemoor — Adventurer's Guild | Open a royal vault in the giant ruins and reclaim the Last Jarl's crown | 220 gold, 1,600 score, Stoneguard Shield |

Quest rewards are separate from existing boss rewards. Giant's Crown awards a
rare Warrior shield with **+8 defense, 20% block chance, 50% blocked damage
reduction, and +30 maximum HP while equipped**. The other four quests award
their listed gold and score.

### Elderwood Forest — The Stolen Heartseed

**Status:** Implemented. Save version 204 preserves quest and encounter progress.

**Premise:** Dark Elves stole a living heartseed and bound its Treefolk guardian
to a clearing. Tamsin asks the player to recover it before the grove withers.

1. Search an optional Dark Elf camp on **stage 2**. Orders describe a bound
   grove deeper in the forest and give a recognizable landmark to look for.
2. Find the grove on **stage 4**, reached by a looping woodland path or a
   bramble shortcut. Cut brambles directly or lure a boar charge through them.
3. Break three binding stakes around the clearing while dealing with Dark Elf
   archers and the Treefolk's warned root attacks. Each stake interaction costs
   an action. Moving between stakes gives the encounter a purpose beyond
   standing beside an enemy and trading attacks.
4. Recover the heartseed and return to Tamsin.

**Encounter choice:** Breaking all bindings frees this particular Treefolk and
ends its hostility. Fighting it first also leaves the heartseed recoverable;
the remaining bindings can be cut afterward. Freeing and killing cannot both
pay the same encounter reward. Other Treefolk retain their normal behavior.

**Behavior:** Read orders and cut bindings with Interact (A by default), then
recover the seed from its stone resting place. All discoveries count before
quest acceptance. Both the orders and the seed are required for Tamsin's bounty;
they are tracked without occupying inventory slots. The freed guardian grants
its combat XP once, cancels its root warning, and leaves the grove. Killing it
instead grants normal combat rewards; cutting its bindings afterward grants no
additional guardian XP. Other Treefolk retain their normal behavior.

Stage 2's orders and stage 4's golden-root arch, bindings, and heartseed have
local labels and contextual prompts. The journal preserves the stage and grove
directions. Backtracking, town portals, and saves retain the same encounter.
Older saves gain Tamsin and the quest sites without regenerating existing maps
or clearing previous victories. Elderthorn's road unlock stays independent.

### Saltwind Marsh — The Missing Spice Skiff

**Status:** Implemented. Save version 205 preserves acceptance, discoveries, and
the paid bounty alongside the existing boat and encounter records.

**Premise:** Corvin's latest shipment never reached Dawnmere. He knows its last
landing, but the channels split beyond it.

1. Paddle to a stranded cargo skiff beside an optional **stage 2** landing.
   Disembark and inspect its manifest. It identifies a distinctive root arch
   near the island where the crew unloaded the valuable spice casket.
2. Explore the side channels on **stage 3** to find that island. Its landing
   leads to a shore encounter with Giant Crabs and Lizardfolk.
3. Recover the sealed casket and return it to Corvin in the Spice Merchant shop.

**Exploration:** The manifest provides a landmark clue. The player discovers
the route on the minimap through normal exploration. The destination branches
away from the onward jetty, making the skiff useful for a deliberate expedition.
The casket becomes a tracked quest item when recovered and travels with the
player through boarding, stage transitions, and Return to Town.

**Behavior:** Corvin offers the quest when spoken to in Dawnmere's Spice
Merchant shop. Use Interact (A by default) ashore beside the stranded skiff to
read and retain its manifest. The clue identifies stage 3's western island and
its east-facing landing. Defeat that island's Giant Crab and Lizardfolk guards,
then recover the sealed spice casket beneath the arched mangrove root. Enemies
on other islands do not block recovery. Both the manifest and casket are
required for Corvin's **150 gold and 1,000 score** bounty, paid once.

Both discoveries are tracked outside the pack and count if found before
accepting the quest or in the opposite order. Recovery takes place ashore.
Returning through a town portal resumes the same stage, moored skiff, and
encounter. A fresh physical entry respawns ordinary guards while the recovered
casket stays empty. Existing ordinary supply caches remain separate.

The damaged skiff, root arch, sealed casket, and empty casket use native pixelkit
art with transparent backgrounds. Their placement preserves the existing
islands, channels, roots, landings, and exits. Each stage retains its own skiff;
Sszareth's victory is independent of the delivery. Older saves retain their
boats, terrain, exploration, wounds, loot, portals, and previous quest progress.

### Stormwatch Headland — The Silent Stormbell

**Status:** Implemented. Save version 206 preserves discoveries, finite defense
actors, and the one-time reward.

**Premise:** A coastal warning bell has fallen silent since harpies occupied
the fort. Seren wants it working again so approaching ships can hear the coast.

1. Recover the missing bell clapper from an optional tower room on **stage 3**.
   The route uses the area's staggered wind lanes and nearby shelter.
2. Fit the clapper to the bell in a side courtyard on **stage 4**. A clear prompt
   explains that ringing it will attract harpies, letting the player prepare.
3. Ring the bell and defeat a finite response: two Harpy Raiders, followed by
   one Harpy Stormcaller. The next group appears after the first is defeated.
4. Return to Seren after the courtyard is secure and the bell is restored.

**Combat:** Place the courtyard beside an existing shutter and exposed lane.
Raiders can be displaced by gusts, while the Stormcaller pressures sheltered
positions with warned lightning. The player can use the shutter, shelter,
bracing, and normal attacks to control the fight. Retreat remains available.

**Timing:** Enemy actions follow the existing combat turns. Environmental wind
keeps its existing clock; standing still never gives this encounter extra enemy
turns. There is no timed failure condition.

**Scope:** Reuse existing enemy types and wind behavior. The bell interaction
and finite encounter are quest-specific. Progress survives retreat and saves, and
ringing the bell again cannot produce repeated enemies or rewards. The Tempest
Matriarch remains a separate boss objective.

**Interaction and persistence:** Seren stands on grass beside Dawnmere's east
road. The clapper needs no pack slot and can be discovered before accepting.
The bell uses the configured Interact key, followed by Y to begin or N/Esc to
prepare first. This confirmation pauses wind. A separate wave counter leaves
wind warnings and enemy attack markers visible. Discovered objectives receive
gold minimap markers. Defeated quest harpies stay defeated, and survivors keep
their wounds and attack warnings through saves, portals, backtracking, and fresh
expeditions. Ordinary expedition enemies still respawn on physical re-entry.
The finite response grants no repeatable loot or XP; Seren awards 180 gold and
1,200 score once. Ringing after completion cannot summon another wave.

**Compatibility:** Older saves keep their existing Stormwatch terrain,
exploration, items, actors, wind, portals, boss victories, and earlier quests.
New expeditions add a ruined room to the stage 3 northeast lookout. Older cached
lookouts keep their layout and still contain the recoverable clapper. Seren is
added to existing Dawnmere saves when her grass tile is free; an occupied tile
is preserved until the next town entry. The Matriarch's spawn is reserved, with
a safe fallback. Older affected saves gain a missing boss without moving their
existing actors, items, or return portal. Completed boss victories remain intact.

Implementation: [quest logic](src/game/stormbell.c),
[save support](src/systems/save_load.c),
[native art and renderer review](docs/art-sources/stormbell/README.md).

### Ironfang Mines — Break the War Machine

**Status:** Implemented. Save version 207 preserves acceptance, the three
quest discoveries, and the one-time bounty.

**Premise:** Ironfang's goblins are forging swords, spearheads, and armor for
their soldiers on Crown Road North. Marshal Veyra asks the player to destroy
the next shipment and kill the Goblin Mine Boss directing the operation.

1. Recover shipment orders from a side office in the **stage 2 Goblin Warrens**.
   They identify a weapons stockpile in the Deep Ironworks.
2. On **stage 4**, repair the eastern track, set the junction, and ride the
   minecart over the flooded trestle to the isolated supply platform. Disembark
   and release a loading winch to dump the suspended weapons into the flooded
   shaft. Visible falling crates and a journal update confirm the sabotage.
3. Defeat the **Goblin Mine Boss on stage 5**, recover his command seal as proof,
   and report to Veyra in Ridgeshire's Town Hall.

**Exploration:** The rail-only crossing gives the cart a military objective.
The stockpile and winch sit beside the ordinary salvage cache, so a previously
claimed cache cannot erase the quest. Goblins defend the accessible workings;
enemies remain off the isolated platform. Sabotage preserves the cart's return
route and the mine's normal paths.

**Scope:** Reuse the cart, turnout, repairs, trestle, and existing Goblin roster.
The shipment orders, loading winch, suspended weapons, and command coffer are
quest objects independent of ordinary caches. This is a local sabotage mission; Crown Road North retains
its existing enemy population and replay rules. An existing Mine Boss victory
counts toward the quest, with the command seal recoverable from his gallery.

**Assignment:** Talk to Marshal Veyra, then press **2** for Break the War Machine.
Press **1** to return to Watchfires of Ridgeshire. Each offer uses the normal
Y/N acceptance. Both quests can be active together and retain their own progress
and rewards; completing one is not a prerequisite for the other.

**Interaction:** Orders sit on a desk in stage 2's northwest side office. Stage
4's winch is on the isolated supply platform, beside its existing salvage
cache. Set the junction east, repair the eastern rail, ride over the flooded
trestle, and disembark. Interact releases the winch: weapons crates fall into
the water and leave an empty gantry. The drop and splash are visual feedback;
standing still does not give enemies additional turns. The stockpile stays
destroyed, and the cart's return route stays usable. An already searched cache
does not remove the winch. Retrieve the command seal from the coffer in the
stage 5 boss gallery after the Mine Boss's defeat.

**Persistence and reward:** Discoveries use no inventory slots and retain credit
even before acceptance. Saves, portals, backtracking, and new expeditions
preserve the discoveries and spent stockpile. Ordinary enemies retain normal
replay behavior, while the existing Mine Boss victory remains permanent.
Veyra awards **160 gold and 1,000 score once**, separately from the boss reward.
The sabotage does not change Crown Road North's enemy population.

**Compatibility:** Quest props overlay existing floors and water. Older saves
keep their terrain, exploration, carts, rails, lamps, switch settings, wounds,
loot, portals, boss victories, and earlier quests. The command coffer can be
searched after an existing Mine Boss victory. Quest interactions remain usable
when a dropped item occupies an object tile, preserving the item and underlay.

Implementation: [quest logic](src/game/war_machine.c),
[save support](src/systems/save_load.c),
[native art and renderer review](docs/art-sources/war-machine/README.md).

### Giant's Peaks — Giant's Crown

**Status:** Implemented. Save version 208 preserves acceptance, discoveries,
the royal vault, the bounty, and equipment awaiting collection.

**Premise:** Edda believes the ruined kingdom's crown still lies in its royal
vault. Its recovery would preserve the history of the giants and give Rosemoor
proof of what happened to the mountain kingdom.

1. Read the royal inscription in **stage 2, The Fallen Village**. It
   points to a royal seal hidden among the aqueduct ruins.
2. Recover the seal from a chamber behind cracked masonry in **stage 3, Broken
   Aqueduct**. Bait a giant's warned strike to open it, or clear the masonry
   with Interact. Its placement supports both methods.
3. Defeat **the Last Jarl on stage 5** and use the seal to open a royal vault
   beside his court. Recover the intact crown from its stone resting place.
4. Return the crown to Edda in Rosemoor's Adventurer's Guild for **220 gold,
   1,600 score, and the Stoneguard Shield**.

**Exploration:** The seal chamber gives destructible ruins a quest purpose.
The royal vault is a side chamber with a visible doorway and a return route.
Its entrance can be opened after the Jarl is dead using the seal and Interact.

**Existing victories:** Players who already defeated the Jarl receive credit
for that objective when accepting the quest. They can still discover the seal
and crown in their existing region. The crown is a quest artifact; the equipment
reward is a separate item.

**Interaction:** Talk to Antiquarian Edda in the Guild, then use the normal Y/N
acceptance. Interact (A by default) reads the inscription in stage 2's northwest
village hall, takes the seal from stage 3's western chamber, opens stage 5's
western warded doorway, and collects the crown. The inscription, seal, Jarl
victory, and crown are four journal objectives. The seal chamber's cracked wall
opens with a warned heavy strike or Interact. The royal ward cannot be shattered;
opening it requires the seal and the Jarl's defeat. Ordinary stone coffers stay
separate from both relic pedestals.

**Persistence and reward:** Discoveries count before accepting and need no pack
slots. Saves, portals, backtracking, and new expeditions preserve all four
objectives and the open vault. Ordinary enemies retain normal replay behavior.
Edda pays the gold and score once. If the pack is full, she reserves the shield
until the player makes room and speaks to her again. Claiming, dropping, or
selling it cannot produce another reward. Its +30 maximum HP applies only while
equipped; equipping does not heal, and removing it clamps current HP to the
remaining maximum.

**Compatibility:** Existing stages keep their terrain, broken masonry, searched
coffers, exploration, actors, wounds, loot, portals, and previous victories.
Older opened vaults stay accessible; crown recovery still requires the seal and
Jarl victory. A legacy player or return portal inside the vault retains an
escape route. Edda is added to an existing Guild save when her floor tile is
free; an occupied tile is preserved until the next Guild entry. Debug shortcuts
open the physical passage without granting quest discoveries or boss victory.

Implementation: [quest logic](src/game/crown_quest.c),
[save support](src/systems/save_load.c),
[native art and renderer review](docs/art-sources/crown-quest/README.md).

## Shared quest behavior

- All five designs describe optional, one-time quests. Ordinary regional enemies
  continue to respawn under the existing expedition rules.
- New objects occupy reachable side spaces. NPCs leave roads, shop doors, town
  gates, and cardinal interaction spaces accessible.
- Important clues remain readable in the quest journal. Quest objects receive
  clear local labels, interaction prompts, and progress messages.
- Discoveries made before accepting a quest retain credit. Quest artifacts
  survive travel and cannot be sold or discarded. A full pack must not lose a
  quest artifact or an equipment reward awaiting collection.
- Save/load must preserve acceptance, clues, objects, completed encounters,
  guardian state, and claimed rewards. Migrations must add quest objects to
  existing stages while preserving terrain, exploration, loot, and victories.
- These quests preserve region geography and existing road and boss unlocks.

## Keeping this reference current

Keep design approval separate from implementation status. When a quest is
implemented, update its current-coverage row with the final NPC,
location, and areas, then mark its design section implemented. Keep multi-area
quests split across Area 1, Area 2, and Area 3. Update the implementation count
and verification date when the journal changes.

Regional references:
[Elderwood Forest](docs/elderwood-forest.md),
[Saltwind Marsh](docs/saltwind-marsh.md),
[Stormwatch Headland](docs/stormwatch-headland.md),
[Ironfang Mines](docs/ironfang-mines.md), and
[Giant's Peaks](docs/giants-peaks.md).
