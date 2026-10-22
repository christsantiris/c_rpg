# Haunted Forest

The Haunted Forest is a seven-stage adventure connecting OakHaven and Stillbury.
OakHaven's west forest gate enters stage 1; Stillbury's main east gate enters
stage 7. Either approach crosses three stages of increasing difficulty before
meeting the Necromancer on stage 4. Beyond the grove, the remaining three
stages become easier. Generated maps and enemies are cached independently from
other regions and retained when entering from either town.

Forest maps use rounded clearings connected by wandering trails with loops
and false branches. Stage passages vary between the east, west, north, and
south edges. The incoming passage remains open for retreat. Reaching the
glowing ancient landmark reveals the onward passage and hidden trails in
either direction. Defeating every regular enemy is never required.

| Stage | Region | Layout | Difficulty tier |
| --- | --- | --- | --- |
| 1 | Gloomwood Border | Two-route fork with a middle crossover | 1 |
| 2 | Spiderweb Thicket | Three pockets wrapped around a central trail | 2 |
| 3 | Dark Elf Territory | Three braided lanes with diagonal trails | 4 |
| 4 | Necromancer's Grove | Twin approaches narrowing into the boss grove | 8 |
| 5 | Sunken Grove | Twin approaches around corrupted clearings | 4 |
| 6 | Wurmwood | Interlocking trails with a false southern branch | 2 |
| 7 | Stillbury Border | Central hub splitting into three branches | 1 |

Difficulty controls the regular enemy roster, enemy count, and trap count.
Clearing dimensions vary when a stage is first generated. Completing other
regions can strengthen enemy rosters through the existing region-order scaling.
Travel score is awarded on first visits to stages from either direction.

## Enemy Roles

- **Pixie:** fragile skirmisher that moves twice per turn.
- **Blighted Wolf:** fast melee hunter that moves twice per turn.
- **Giant Spider:** melee controller whose bite applies poison.
- **Dark Elf:** archer that fires along an unobstructed row or column.
- **Giant Wurm:** armored heavy enemy that moves every other turn.
- **Forest Troll:** high-health bruiser that moves every other turn.

## Central Encounter and Shortcuts

The Necromancer waits in the stage-4 grove and remains dormant until the
player enters it or damages him from range. He stays in the grove and blocks
crossing while alive, but permits retreat toward the expedition's entry town.
His health, attack, defense and experience reward retain their existing scaling.

- **Grove Binder, above half health:** spirit bolts mark a fixed lane for one
  player turn. Step aside before the countdown ends. After the first bolt and
  recovery, one grave wave warns all unconsecrated patches for two turns.
- **Withered Grove, at/below half health:** one second finite grave wave is
  followed by alternating spirit bolts and two-turn crossing root lanes.
  Roots lock the player's row and column inside the grove when declared.
  Two ordinary cardinal steps can leave the crossing; no special ability is
  required. Roots deal one hit and do not leave impassable tiles or lingering
  damage. Roots and grave waves resolve separately.
- Every signature has a recovery turn without extra adjacent boss attacks.
  The panel shows phase, health, attack countdown, freeze and remaining graves.
  Numbered outlines show the declared footprint. Freezing pauses the countdown;
  crossing half health never changes an existing warning. Retreat beyond attack
  range pauses the warning and does not refund its wave.
- Three grave patches can be consecrated on or beside the patch with contextual
  **Interact (`A` by default)**. Consecration permanently prevents that patch
  from raising a servant. Consecrating any currently warned grave cancels the
  entire pending wave and grants two complete recovery turns. Loot on a grave
  stays available and does not prevent interaction. Every class can do this.

Each wave raises at most one **Grove Revenant** per unconsecrated grave. Recent
fallen forest servants return at half health; when no recent corpse is available,
a buried Blighted Wolf rises instead. Occupied patches and a full enemy roster
skip that spawn without moving the warning or replacing a living enemy. Servants
begin acting on the following turn and grant no repeat gold, items, experience
or score. Each health phase spends its wave when declared, even if interrupted.
Entering phase two before the first wave skips the unused first wave. Graves,
phase, exact attack targets and consumed waves survive saves and revisits.
Hidden random traps are excluded from the grove; previously revealed traps in
older saves remain intact.

The guaranteed **Necromancer's Cloak** keeps its +5 defense, 8% evasion and Rogue
restriction. It normally drops on the boss tile. If the floor loot list is full,
a named grove reliquary preserves it without deleting another item. An earned,
unclaimed cloak is restored there after backtracking or a map refresh. Interact
beside the reliquary or press `P` while standing on it; a full inventory leaves
it available. Claiming it once prevents later sale or dropping from producing
a replacement. The reward does not depend on consecrating the graves.

Defeating him displays a shortcut discovery prompt until a fresh press of Enter
acknowledges it. A separate marked trail appears beside the defeated boss,
matching the swamp and mountains. That shortcut returns onto the boss arm of the forest crossroads. Follow its signs to either
town, or turn back to the boss chamber. Players may instead
cross the remaining three stages to reach the same town. Surviving enemies do
not block either route.

Boss victory opens both side arms of one + shaped crossroads outside each town.
From the town gate, the regular forest is straight, the next town is left, and
the Necromancer's chamber is right. The side arms remain hidden before victory.
Each destination is reached directly from its arm, without a route menu or a
second connecting path. Returning from the boss uses the same crossroads.

Return to Town returns to the expedition's entry town. Its portal remembers
that town, the exact forest stage, and casting position, even if the forest is
subsequently entered from the other town.

## Forest Quest

Speak to Alder with `T` in **Stillbury's Inn** to accept **The Lost Wardens**.
Return to him there for 70 gold and 500 score once all three wardens are rescued.
The quest places captives in guarded groves on **stages 7, 6, and 5**. Enter
Stillbury's east forest gate to encounter all three before the stage-4
Necromancer. From Oakhaven, the wardens appear after crossing his grove. Their
hunting parties are led by a Giant Spider, Dark Elf, and
Forest Troll. Defeat or evade the guards, then stand beside a warden and press
`T` to rescue them. A full stage clear is never required. Accepting the quest
starts a fresh forest expedition, while a previously defeated Necromancer
remains dead and unlocked shortcuts stay open.
After defeating the Necromancer, take the grove shortcut to Oakhaven and the
unlocked town road back to Stillbury, or fight through stages 3, 2, and 1.

## Save Compatibility

Older saves migrate automatically when loaded. The old final boss stage moves
to stage 4, and saved maps and portal destinations move with the route. Warden
placements update to stages **7, 6, and 5** while retaining rescue progress.
Version **113** removes obsolete captives on stages 1–4, including loot and
portal underlays, without resetting explored maps, enemy damage, or coordinates.
Collected rescue bits keep their identity; unfinished rescues appear on the new
stages when visited. Boss victories and shortcut unlocks remain intact. Saving
writes the current format, including the expedition's entry town and portal anchor.

Version **125** adds the grave patches to active and cached living encounters
without regenerating the map or resetting boss health, enemy deaths, loot,
exploration, player position, portals or quests. Old unmarked attack timing
restarts with a visible warning. Previous victories stay defeated; only a known,
currently dropped legacy cloak is treated as unclaimed. New saves store cloak
claim state and both finite-wave flags, including when the player is elsewhere.

Renderer captures and focused play-testing notes are in
[the Necromancer encounter gallery](art-sources/necromancer-encounter/README.md).
