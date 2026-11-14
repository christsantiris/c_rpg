# Rosemoor: the manufactured crisis

Rosemoor is the western Act 5 hub, reached from Castle Grounds along Crown Road
West. Frostfell remains north of town and Ashen Hollow west. Both adventures
are accessible without quests, in either order, with retreat and final exits
returning toward Rosemoor. No world roads or region positions change.

Ridgeshire exposes the binding of the King. Rosemoor establishes how Malakar
engineered the influx of evil: dismantling ancient protections, suppressing
warnings, supplying creature allies and commissioning binding devices under
royal cover. Either town can be visited first. The Castle redesign is deferred.

## People and services

| Place | Person | Role |
| --- | --- | --- |
| Apothecary | Mira, repurposed from the retired swamp rescue | Potions, The Broken Ward, interpretation of Frostfell's evidence |
| Adventurer's Guild | Forgekeeper Hadrin | The Arcanist's Forge and its local reward |
| Adventurer's Guild | Huntmaster Selene | Hunt Rimefang and a potion reward choice |
| Spice Merchant | Suri | Trade-route testimony and reactions to reported anti-Crown actions |
| Castle Grounds | Courier Renn | Optional Rosemoor directions through his news topic |

The existing Apothecary art is retained. Its door now leads into a walkable
interior. Approach Mira and press Talk: **1: Buy / sell**, **2: Ask about
Frostfell**, **Enter / Esc: Close**. The second option changes to discussion or
reward collection with quest progress. Only choosing trading opens the existing
Apothecary buy/sell interface; closing it returns to gameplay beside Mira.
Opening the conversation or shop never assigns a quest or collects its reward.

Suri welcomes reported opponents of the Crown without clearing royal records.
Her conversation changes again after both regional victories. Mira recognizes
Oswin's proof if the player has already completed Ridgeshire's investigation.

## Three local quests

| Quest | Objective | Reward |
| --- | --- | --- |
| The Broken Ward | Defeat the Frost Kraken on Frostfell 5; its stolen wardstone and the wardkeepers' orders are recovered automatically | Select Mira's Frostfell topic in Rosemoor for 350 gold, once |
| The Arcanist's Forge | Defeat the Cinder Lord on Ashen Hollow 5, ending binding production and recovering the ledger | Hadrin pays 400 gold and gives all-class Emberforge Bracers: +2 defense, +30 maximum HP/MP |
| Hunt Rimefang | Defeat Rimefang and every pack member on Frostfell 3; the Kraken is optional | Selene pays 200 gold and offers ONE Strength or Intelligence potion |

The normal three-active-quest limit applies. Both boss objectives and the hunt
can complete without prior acceptance. Rimefang's existing four-enemy encounter
can be discovered before speaking to Selene; it retains its existing combat
behavior and leader health bonus. Completion requires the whole pack, including
members lured away. Unfinished groups replenish on a new expedition; completed
hunts never respawn their dedicated pack.

Completed objectives move straight to Completed and free their active slot.
Evidence needs no bag slot. NPC rewards are claimed locally in Rosemoor.
A full bag preserves Hadrin's bracers or Selene's potion choice, with the journal
still showing a pending reward after the gold is paid. Repeated conversations,
keys, pickup, travel and saving cannot duplicate payments or equipment.
Strength adds 1 permanent base attack; Intelligence adds 1 permanent maximum MP.

## Kraken weapon choice

The Frost Kraken's boss reward is separate from Mira's payment. Open its
reliquary with Interact, then Pick Up the released loot to choose ONE weapon.
The chest preserves the lake or snow beneath it; a full bag or Esc keeps the
choice pending.

| Weapon | Class | Bonuses |
| --- | --- | --- |
| Krakenbone Blade | Warrior | +11 attack, +25 maximum HP, 20% critical chance, one hand |
| Krakenbone Warbow | Rogue | +14 attack, range 7, piercing, two hands |
| Rimestaff | Mage | +10 attack, +10 spell power, +45 maximum MP, 10% cheaper spells, two hands |

These are modest upgrades over the corresponding shop weapons. Existing Kraken
gold, XP and score, and the Cinder Lord's Embercleaver/Strength-potion rewards,
remain separate. No new enemy balance or Castle mechanics are introduced.

## Evidence

- **Orders to the Wardkeepers:** Malakar ordered the wards dismantled, forbade
  repair after creatures emerged and kept warnings from the King's household.
- **Emberforge Production Ledger:** private commissions connect Malakar to
  creature bindings, the dragon's restraints and the King's ceremonial crown.
- **Rimefang's Binding Collar:** his maker's mark connects the pack attacks to
  protected royal shipping routes.

Each record uses the existing parchment reveal and Evidence journal. Either
main discovery implicates Malakar; both reveal the manufactured crisis.
Ridgeshire's separate evidence still establishes the King's magical control.
The wardstone's recovery and the forge's shutdown are recorded by permanent
boss victories. Their possible effects on the final battle await Castle design.

## Existing saves

Save version 252 stores Rosemoor's three quest states, held bracers and potion,
weapon choice, dialogue prompt and legacy Kraken-reward flag. Existing actor
wounds, exploration, loot, equipment, other quests and boss victories survive.
Existing Rosemoor doors become walk-in Apothecary doors in place. Hadrin appears
in the Guild only on clear floor away from the player; dropped loot is preserved.

Previously won or pending Kraken rewards retain the original Krakenbone Bow
instead of generating an additional weapon choice. Previously paid Great Hunts
contracts do not gain another local hunt payment or potion. Accepted Three Great
Hunts, Silent Expedition and Emberforge contracts remain completable through
their legacy conversations. Selene advertises a previous-contract topic even
before accepting the new hunt and when the active quest limit is reached.
Fresh games do not offer the distant Three Great Hunts or Brenna's rescue quest.

## Validation

Native game and test targets build successfully. The complete suite passes
**10,501/10,501** checks, including **185/185** Rosemoor checks.

Automated playthroughs cover both adventure orders across three seeds, using
real stage travel, retreat/re-entry, boss kills, all weapon choices and both
potion choices. Checks cover explicit trade/quest selection, three active slots,
Completed objectives, local payments, full bags, duplicate claims, evidence,
partial pack deaths, save/load, malformed fields and legacy contracts/rewards.

Native SDL views were inspected at 1280×720 and 640×480, including the existing
Apothecary beside the Guild, quest offers, reward menus and the frozen-lake
reliquary. A real version 240 save was loaded and rendered without writing to
its original file. README continues to embed and link the canonical atlas,
whose geography matches the unchanged routes.
