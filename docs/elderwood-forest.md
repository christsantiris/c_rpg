# Elderwood Forest and the northern castle road

Oakhaven's north exit reaches a safe cobblestone road. A living woodland sprite
stands beside it; touching the landmark starts Elderwood at level 1. Elderthorn
blocks the road farther north. Defeating it opens the route to a separate Royal
Catacombs sprite. Defeating the Grave Marshal opens the remaining passage into
Castle Grounds. The existing Crown Roads remain available.

The same road connects both directions without reversing or flipping its map.
Physical retreat from either adventure returns beside its own entrance. Entering
Elderwood or Catacombs from either end always starts at level 1. Returning to the
Castle end from a Castle-origin expedition remains possible before either boss
is defeated. Reaching Oakhaven from that end requires clearing Elderwood.

## Five forward stages

Five stages of ancient emerald woodland use looping clearings, immense living trunks,
broad green canopies and warm golden highlights. Mossy roots and cuttable
brambles line the paths. Optional bramble passages give alternate
angles; a normal route around them remains available. Interact beside brambles
with the bound action key (A by default) to cut them. Fallen logs block movement
and projectiles and stop charging creatures. Floor layouts, exploration, cut
brambles and surviving enemies persist when moving between stages.

| Enemy | Role |
| --- | --- |
| Briar Wolf | Pack flanker; closes on ranged enemies' targets |
| Ironhide Boar | Warns an aligned charge, then smashes through brambles |
| Thorn Dryad | Ranged thorn volleys and repositioning |
| Dark Elf | The existing Haunted Forest archer, sharing its art and behavior |
| Treefolk | Toughest regular enemy; heavy bark armor and warned root attacks |
| Elderthorn | Stage 5 boss, an ancient stag armored in bark and living roots |

Elderthorn locks a charge lane or root target before attacking. Move off the
amber warning. Luring a charge into a trunk exposes its heart for three recovery
turns; its armor returns afterward. Below half health it calls one finite pair
of wolves. Defeating it awards 180 gold and 2,000 score and permanently opens the
Elderwood section of the road. The boss journal records the victory separately
from the older regions' integer boss mask.

## The Stolen Heartseed

Forester Tamsin stands beside Oakhaven's northern road. Talk with her and accept
**The Stolen Heartseed**. Search the Dark Elf camp in stage 2's western clearing
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
sites when next entered. See the [quest reference](../quests.md).

## Return portals and saves

Return to Town transports the player to the **last town actually entered**,
including when that town is outside the current region. Walking around a town
sprite does not change this destination. The portal resumes the same expedition,
floor, position and physical entrance direction. A portal in an unrelated town
stands in its central plaza; familiar regional portal positions remain in use
when returning to the usual town. Castle Grounds does not count as one of the
five towns.

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

See the [sprite comparison and SDL captures](art-sources/elderwood/README.md).
