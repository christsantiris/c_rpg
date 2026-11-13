# Ridgeshire story arc

Ridgeshire is the eastern Act 5 base: a mountain garrison town whose people
remain loyal to the King while questioning Malakar's private commands.
Courier Renn on Castle Grounds directs the player along Crown Road East to
Marshal Veyra. Dragonspine and Ironfang remain open without accepting quests,
and can be explored in either order. Returning to the castle is always an
option; no new story gate blocks it.

## Buildings and services

| Place | People | Purpose |
| --- | --- | --- |
| Town Hall | Marshal Veyra, Archivist Oswin | Ironfang contract and royal records |
| Workshop | Garrick, Tessa | Existing sharpening service; arrows and bows |
| Runewright's Lodge | Runewright Edric | Dragon investigation; spells and health/mana potions after talking |
| Tavern & lodgings | Innkeeper, Ilya | Rest for 100 gold; local goblet quest and payout |

The Lodge replaces the monastery using the existing pixelkit building pipeline,
with purple slate and turquoise rune details. The Tavern reuses the established
alehouse artwork. Both have walk-in doors, explicit Talk interactions and real
retreat transitions. Paid rest restores the player and living companions who
are following; it never resurrects anyone or heals companions waiting elsewhere.

The Innkeeper acknowledges reported anti-Crown actions, while Veyra judges the
player's work for Ridgeshire. This does not clear the Crown's record or add a
new arrest. Earlier towns' reactions remain intact.

## Three local quests

| Quest | Assignment and objective | Reward and collection |
| --- | --- | --- |
| Break the War Machine | Veyra: read Ironfang 2's shipment orders, sabotage the weapons on 4, defeat the Mine Boss and take his command seal on 5 | Veyra pays 400 gold and 1,000 score and gives a Royal Ward Ring: all-class, +2 defense, +25 maximum HP/MP |
| The Dragon's Binding | Edric: defeat the Red Dragon on Dragonspine 5, recovering its runic shackle | Return to Edric's investigation topic for 350 gold |
| The Dragon's Hoard | Ilya: collect the golden goblet in the stage-5 dragon lair | Return to Ilya in Ridgeshire for 300 gold |

The normal three-active-quest limit applies. Objectives can be discovered before
acceptance. Completed objectives move straight to Completed and release their
slot before a local reward is collected. None of these quest objects consumes
bag space. Accepting a quest preserves existing expeditions. Payments are
one-time, and a full bag leaves Veyra's ring reserved without repeating gold.

The Red Dragon separately offers **one** armor reward, without requiring either
dragon quest. Open the reliquary with Interact and Pick Up its released loot to
choose Dragonplate (Warrior: +12 defense, +60 HP), Dragonscale Leathers (Rogue:
+8 defense, +25 HP, 15% evasion), or Emberweave Robes (Mage: +5 defense, +70 MP,
20% cheaper spells). Esc or a full pack leaves the choice pending. Existing
boss gold, XP, score and combat patterns are unchanged. The armor's item value
is added to score when claimed, following equipment pickup rules.

## Evidence and payoff

Ironfang's requisitions identify Malakar's orders diverting royal steel to
goblins and components to a binding apparatus, with reports withheld from the
King's household. The dragon's shackle demonstrates magic suppressing another
creature's will. Either discovery leads to Oswin's royal inventory recording
similar runes commissioned for the King's crown.

One clue raises suspicion; Oswin asks for the other. With both clues and the
inventory, Oswin and Veyra recognize that Malakar controls the King. Each record
is kept in the existing Evidence journal, with a parchment reveal, independently
of quest acceptance, inventory capacity or reward claims. The castle finale
and Rosemoor's story are separate changes.

## Saves and retired contracts

Save version 251 stores all Ridgeshire quest, evidence, prompt and reward states.
Previous saves retain exploration, wounds, loot, equipment, other quests and
boss victories. A new Tavern footprint moves an overlapping player and loose
loot onto its accessible doorstep. Old Dragon victories or pending rewards
retain the Dragon Scale Mantle; they cannot generate new armor choices. Already
paid Ironfang rewards do not gain a duplicate payment or ring.

New characters are no longer offered Watchfires, Reclaim the Emberforge or Rest
for the Forgotten in Ridgeshire. Already accepted legacy contracts retain their
turn-ins and rewards. Hadrin remains present only for an existing Emberforge
contract; Veyra and Oswin expose previous-contract topics when applicable.

## Validation

The complete test suite passes **10,316/10,316** checks, including
**184/184** Ridgeshire checks. Native game and test targets build successfully.

The Ridgeshire tests exercise both area orders across three seeds, using real
movement through every stage's onward transition, real minecart travel and
sabotage, boss kills, reliquary opening and pickup, armor choices, local reward
claims, active-slot release and save/load. They also cover full bags, repeated
claims, partial and combined evidence, old pending mantle rewards, exterior
migration with wounds and loot, and accepted legacy contracts.

Native SDL captures were reviewed at 1280×720 and 640×480, including NPC offers,
services, evidence payoff and the armor menu. The new lodge was inspected beside
the existing Workshop, Town Hall and Tavern at the same scale. A real version
240 save was loaded and rendered without writing to the original save files.

World geography and roads are unchanged. README continues to embed and link to
the canonical `docs/world-map.png`; the displayed atlas matches Castle Grounds
→ Crown Road East → Ridgeshire, with Dragonspine north and Ironfang east.
