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
brambles, wounds, loot and quest progress. Ranger Tamsin offers the boss-only **Clear Elderwood** quest in Stillbury.
This quest adds no new Malakar clue; later story evidence remains a separate change.

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

## Clear Elderwood

**Ranger Tamsin** stands near Stillbury's eastern gate at `(34,13)`. She asks
the player to defeat Elderthorn in Elderwood 5 and clear the forest of evil.
Accepting uses one of the three active quest slots. There are no collection,
binding, or rescue objectives. Elderthorn is reachable without accepting.

Defeating Elderthorn opens the road to Sunken Coast and immediately moves the
quest to **Completed**, freeing its active slot. This also works if the player
never spoke to Tamsin. Elderthorn retains his existing 180-gold and 2,000-score
boss reward. Speak to Tamsin in Stillbury afterward to claim her separate
**120 gold and 800 score**, paid once. The journal shows the unclaimed reward;
no trip back to Oakhaven is required.

Save version **246** stores the new quest state and restores Tamsin to existing
Stillbury maps. Existing Elderthorn victories receive an unclaimed quest reward
without repeating the boss's loot. Pending offers, completion and payment
survive save/load.

**The Stolen Heartseed is retired.** New stages no longer create its orders,
bindings, altar, or special guardians. Existing active and cached stages lose
these props in place; saved enemies remain with their wounds, deaths, positions
and XP values, using ordinary enemy names. Exploration, cut paths, loot and
portal underlays are preserved. Old quest counters remain serialized solely
for compatibility and do not occupy a journal entry or an active quest slot.

See [quest validation](elderwood-quest-validation.md).

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

