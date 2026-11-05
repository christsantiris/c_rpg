# Elderwood Forest and the eastern coastal road

Stillbury's east gate leads onto a safe cobblestone road through Elderwood.
A living woodland landmark beside the road starts its five-stage adventure.
Elderthorn blocks onward passage east to Sunken Coast. Defeating him opens
that road permanently; Blackwater's victory is independent.
Retreat from Elderwood or Sunken Coast returns to this eastern route and
Stillbury. The final Elderwood exit emerges north of its landmark onto a trail
that bends east toward the coast, preserving the crossing direction. The northern road instead passes through Blackwater Swamp and the
Royal Catacombs beneath the castle.

Elderwood's entrances and exits lie on the actual south and north map boundaries.
Travel and retreat keep the direction of the crossing step. Save version 243
extends old active and cached markers in place, preserving exploration,
brambles, wounds, loot and quest progress. The Elderwood story clue about
Malakar remains undecided; this geography change adds no new clue or quest.

Elderwood is drawn at the Crown Roads' 1.5× zoom: the camera shows about 30 × 18
tiles around the player, and its charges and roots, which reach at most seven
tiles, stay on screen.

## Five forward stages

Five stages of ancient emerald woodland use looping rounded clearings, as in the
Haunted Forest, with immense living trunks,
broad green canopies and warm golden highlights. Mossy roots and cuttable
brambles line the paths. Optional bramble passages give alternate
angles; a normal route around them remains available. Interact beside brambles
with the bound action key (A by default) to cut them. Fallen logs block movement
and projectiles and stop charging creatures. Floor layouts, exploration, cut
brambles and surviving enemies persist when moving between stages. Leaving and
re-entering the area begins a new expedition: ordinary enemies return on all
visited stages, while bosses, named quest guards and finite reinforcements keep
their existing state. Saving and loading alone does not respawn enemies.

| Enemy | Role |
| --- | --- |
| Blighted Wolf | Pack flanker; closes on ranged enemies' targets |
| Dark Elf Ranger | Bow skirmisher that repositions after shooting; shared with Haunted Forest |
| Thorn Dryad | Ranged thorn volleys and repositioning |
| Dark Elf Archer | The existing Haunted Forest archer, sharing its art and behavior |
| Treefolk | Toughest regular enemy; heavy bark armor and warned root attacks |
| Elderthorn | Stage 5 boss, an ancient stag armored in bark and living roots |

Elderthorn locks a charge lane or root target before attacking. Move off the
amber warning. Luring a charge into a trunk exposes its heart for three recovery
turns; its armor returns afterward. Below half health it calls one finite pair
of wolves. Defeating it awards 180 gold and 2,000 score and permanently opens the
Elderwood section of the road. The boss journal records the victory separately
from the older regions' integer boss mask.

Treefolk remain mobile, attackable enemies. Their marked footing identifies the
occupied tile beneath the canopy; their health bar sits above the canopy. Bump
that tile for melee or fire along its row or column. Root attacks crack the
warned ground for two turns, then sprout branching roots at that fixed location
and fade. The visual eruption adds no turns or lingering damage.

Save version 214 replaces retired wolf variants in active and cached encounters
with Blighted Wolf, retaining wounds, deaths, combat state and named quest guards.
Frostfell keeps its regional wolf difficulty; all wolves share the existing
Haunted Forest art.

Clearings, Elderthorn's arena and the Heartseed grove are carved as ellipses.
Save version **219** reshapes stages saved with rectangular clearings, keeping
their exploration, cut brambles, logs, quest objects and surviving enemies. A
short path stays open to any enemy, item, portal or player standing in a removed
corner. A three-tile trail links the rounded grove to its bramble flank.

## The Stolen Heartseed

In V2, Tamsin has left Oakhaven and no one offers this quest until a later act
places her. Existing quest progress is kept.

Forester Tamsin stands beside Oakhaven's northern road. Talk with her and accept
**The Stolen Heartseed**. Search the Dark Elf Archer camp in stage 2's western clearing
for orders identifying the **golden-root grove in stage 4's eastern clearing**.

Use **Interact** (A by default) beside each of the grove's three violet-bound
stakes. Each cut consumes an action while the guardian and Dark Elves remain
dangerous. Cutting the last binding frees the bound Treefolk, cancels its warned
attack, and awards its combat XP once as it leaves the grove. Defeating the
guardian first also works: cut the remaining bindings to release the heartseed,
without receiving a second guardian reward. Other Treefolk remain hostile.

Recover the seed from its stone resting place and bring it and the orders to
Tamsin for **120 gold and 800 score**, paid once. Quest discoveries are tracked
outside the pack, including discoveries made before acceptance. Neither this
quest nor its reward changes Elderthorn's road unlock.

Save version **204** preserves the orders, each severed binding, recovered seed,
guardian outcome, encounter placement, and paid reward. Existing saves gain the
NPC and quest locations while retaining their forest maps, cut paths, wounds,
loot, exploration, portals, and earned victories. Cached quest stages gain their
sites when next entered.

## Escaping and saves

Return to Town transports the player to the **last town actually entered**,
including when that town is outside the current region. Walking around a town
sprite does not change this destination. This is a one-way escape with no return
portal. Enter the forest again through its normal entrance. Castle Grounds does
not count as one of the five towns.

Save version 182 stores Elderwood's compact five-stage caches, boss victory,
maximum stage, physical entry towns, last town visited and return portal town.
Older saves keep character, equipment, quests, Catacombs floors, enemies,
braziers, rewards, earned victories and existing portal anchors. Existing
Catacombs victories satisfy that road lock; Elderthorn must still be defeated.
Older active Catacombs expeditions retreat toward Castle Grounds. Since older
saves did not record town visits, migration uses the current town or the known
regional entrance town where available, and otherwise starts with Oakhaven.

`./tools/debug_shortcuts.sh` bypasses both road locks and brambles for new and
loaded characters, without granting either boss victory.

