# Quest reference and design proposals

Last checked against the game on **2026-10-26**.

The current implementation has **18 journal quests**. All five regional designs
below are approved. **The Stolen Heartseed is implemented**; the remaining four
will be implemented individually, with a pause between features for source
control review and a manual commit.

## Current quest coverage

| Quest | Assigning NPC | NPC location | Area 1 | Area 2 | Area 3 |
| --- | --- | --- | --- | --- | --- |
| The Broken Seals | Elowen | Oakhaven — Tavern | Oakhaven Dungeon | — | — |
| Recover the Treasure Map | Dain | Ridgeshire — Town center | Goblin Mountains | — | — |
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
| **No implemented quest — design approved** | — | — | Saltwind Marsh | — | — |
| **No implemented quest — design approved** | — | — | Stormwatch Headland | — | — |
| **No implemented quest — design approved** | — | — | Ironfang Mines | — | — |
| **No implemented quest — Giant's Crown approved** | — | — | Giant's Peaks | — | — |
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

Corvin, Marshal Veyra, and Tamsin exist. Seren and Edda will be new NPCs.

| Quest | Design status | Area | Assigning NPC | NPC location | Main activity | Reward target |
| --- | --- | --- | --- | --- | --- | --- |
| The Stolen Heartseed | Implemented | Elderwood Forest | Forester Tamsin — existing | Oakhaven — Town center, near the north road | Follow a Dark Elf trail and free a grove from root bindings | 120 gold, 800 score |
| The Missing Spice Skiff | Approved | Saltwind Marsh | Spice Merchant Corvin — existing | Dawnmere — Spice Merchant shop | Follow a wreck's cargo trail and recover spices from a remote island by skiff | 150 gold, 1,000 score |
| The Silent Stormbell | Approved | Stormwatch Headland | Watchkeeper Seren — new | Dawnmere — Town center, near the east road | Restore a stormbell and defend its courtyard using wind and shutters | 180 gold, 1,200 score |
| Break the War Machine | Approved | Ironfang Mines | Marshal Veyra — existing | Ridgeshire — Town Hall | Destroy a goblin weapons shipment and defeat the Goblin Mine Boss | 160 gold, 1,000 score |
| Giant's Crown | Approved | Giant's Peaks | Antiquarian Edda — new | Rosemoor — Adventurer's Guild | Open a royal vault in the giant ruins and reclaim the Last Jarl's crown | 220 gold, 1,600 score, equipment reward to be designed |

Reward amounts are balancing targets, separate from the existing boss rewards.
Giant's Crown's equipment reward still needs a type and statistics. Additional
equipment rewards for the other quests have not been specified.

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

**Status:** Approved design; not implemented.

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

**Scope:** Reuse the current skiff and shore combat rules. The wreck, manifest,
and casket would be new quest objects. Each stage keeps its own existing skiff;
the quest does not require moving a boat between stages. Sszareth's victory is
independent of the delivery.

### Stormwatch Headland — The Silent Stormbell

**Status:** Approved design; not implemented.

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
and finite encounter would be new. Progress survives retreat and saves, and
ringing the bell again cannot produce repeated enemies or rewards. The Tempest
Matriarch remains a separate boss objective.

### Ironfang Mines — Break the War Machine

**Status:** Approved design; not implemented.

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
The shipment orders, loading winch, weapons display, and command seal would be
new quest objects. This is a local sabotage mission; Crown Road North retains
its existing enemy population and replay rules. An existing Mine Boss victory
counts toward the quest, with the command seal recoverable from his gallery.

### Giant's Peaks — Giant's Crown

**Status:** Approved design; not implemented.

**Premise:** Edda believes the ruined kingdom's crown still lies in its royal
vault. Its recovery would preserve the history of the giants and give Rosemoor
proof of what happened to the mountain kingdom.

1. Read an optional royal inscription in **stage 2, The Fallen Village**. It
   points to a royal seal hidden among the aqueduct ruins.
2. Recover the seal from a chamber behind cracked masonry in **stage 3, Broken
   Aqueduct**. Bait a giant's warned strike to open it, or clear the masonry
   with Interact. Its placement supports both methods.
3. Defeat **the Last Jarl on stage 5** and use the seal to open a royal vault
   beside his court. Recover the intact crown from its stone resting place.
4. Return the crown to Edda in Rosemoor's Adventurer's Guild for the bounty and
   an equipment reward whose type and statistics still need design.

**Exploration:** The seal chamber gives destructible ruins a quest purpose.
The royal vault is a side chamber with a visible doorway and a return route.
Its entrance can be opened after the Jarl is dead using the seal and Interact.

**Existing victories:** Players who already defeated the Jarl receive credit
for that objective when accepting the quest. They can still discover the seal
and crown in their existing region. The crown is a quest artifact; the equipment
reward is a separate item.

## Shared behavior for later implementation

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
- No proposal changes region geography or existing road and boss unlocks.

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
