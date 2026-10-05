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
player enters it or attacks him from range. He alternates a telegraphed
invocation with a spirit bolt. Every fourth action, he can return one defeated
forest servant at half health. The roster remains bounded by the floor's
original enemies. He blocks crossing the grove while alive, but permits retreat
toward the town where the expedition began.

Defeating him displays a shortcut discovery prompt until a fresh press of Enter
acknowledges it. A separate marked trail appears beside the defeated boss,
matching the swamp and mountains. That shortcut goes directly to Stillbury when entering
from OakHaven, or to OakHaven when entering from Stillbury. Players may instead
cross the remaining three stages to reach the same town. Surviving enemies do
not block either route.

Boss victory also permanently unlocks the safe forest road between the towns.
OakHaven's separate west shortcut gate and Stillbury's upper east shortcut gate
enter that enemy-free road. Their ordinary forest gates remain available.

Return to Town returns to the expedition's entry town. Its portal remembers
that town, the exact forest stage, and casting position, even if the forest is
subsequently entered from the other town.

## Forest Quest

Alder's **The Lost Wardens** quest places captives in guarded groves on stages
1, 2, and 3 on the OakHaven side of the Necromancer. Entering from Stillbury
encounters them on stages 3, 2, and 1 after crossing the boss grove. Their
hunting parties are led by a Giant Spider, Dark Elf, and
Forest Troll. Defeat or evade the guards, then stand beside a warden and press
`T` to rescue them. A full stage clear is never required. Accepting the quest
starts a fresh forest expedition, while a previously defeated Necromancer
remains dead and unlocked shortcuts stay open.
Taking the grove shortcut from Stillbury skips the warden stages; enter the
ordinary forest gate from OakHaven to finish those rescues later.

## Save Compatibility

Older saves migrate automatically when loaded. The old final boss stage moves
to stage 4, and saved maps and portal destinations move with the route. Warden
placements update to stages 1, 2, and 3 while retaining rescue progress. Character
progress, boss victories, and shortcut unlocks remain intact. Saving writes the
current format, including the expedition's entry town and portal anchor.
