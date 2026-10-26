# Stormwatch Headland — design proposal

Status: implemented as five playable stages east of Dawnmere. See the
[current area guide](stormwatch-headland.md) for controls, enemy stats and rewards.
The [native sprite review](art-sources/stormwatch/enemy-review.png) uses the actual
player and Dark Elf as scale references. The following records the approved
mechanics and visual direction.

## Landscape and identity

Wind-bent coastal grass, pale gray cliffs and an abandoned medieval watchfort
overlook open water. Weathered blue-gray masonry, warm straw grass, faded
ochre pennants, dark timber shutters and sparse sea-green moss distinguish the
headland. Water retains the established blue-teal coastal palette. Rain and
sea spray stay subtle enough to read enemies and attack warnings.

A compact ruined watchtower and curtain wall form the roadside entrance
landmark. Its doorway follows the established building scale. Transparent art
preserves the ground underneath. The interior adventure mixes open headland
with connected gatehouses, courtyards, stairs and accessible fort rooms.

Dawnmere remains coastal lowland. The eastern road climbs gradually onto the
headland. Saltwind keeps its sheltered sea-level inlet south of town, Goblin
Mountains stay north, and Ridgeshire remains the elevated mountain city. The
Stormwatch landmark and geographic label appear on the current atlas.

## Five stages

| Stage | Place | Exploration and encounters |
| --- | --- | --- |
| 1 | Windward Approach | A three-fingered headland with a sheltered inland loop. Broken watch huts and a side lookout introduce Harpy Raiders and one clearly marked gust lane. Two optional branches lead to supplies and a ruined signal post. |
| 2 | Broken Gatehouse | Two connected courtyards wrapped around a coastal cut. Guardrooms and wall stairs reconnect to the outer path. Cliff Gargoyles guard the shorter routes; the first storm shutters change which courtyard is sheltered. |
| 3 | Three Watchtowers | Three tower compounds form a triangle of routes, with rooms and supply pockets inside the walls. Harpy Stormcallers use exposed lanes, while sheltered passages let the player approach them from another direction. |
| 4 | Stormwall Fort | A larger fort with an outer patrol circuit, two accessible wings and a central crossing. Storm Elementals move through gust lanes while shutters let the player trade an open route for solid cover. Optional storehouses sit off the onward route. |
| 5 | The Matriarch's Court | An outer rampart loop reaches two side rooms and the final courtyard. The boss arena has connected sheltered and exposed ground. Defeating the Matriarch opens the final gate onto the return approach to Dawnmere. |

Every stage has a complete ordinary walking route, at least one reconnecting
loop and two optional branches with a cache, room or useful connection. Side
rooms belong to the same stage and minimap. The generator must vary these
compositions rather than reuse a room chain with a different floor color.

Entrances and exits terminate traversable ground at recognizable boundary
gateways or cliff stairs. They use weathered timber and local stone. A return
exit never floats in a clearing. Physical entry starts at stage 1, and the
adventure advances through stages 1–5 without reverse progression.

## Enemies, ordered by intended power

Stats remain subject to balancing during playable implementation.

| Rank | Enemy | First stage | Appearance and behavior | Player response |
| --- | --- | --- | --- | --- |
| 1 | Harpy Raider | 1 | Folded slate feathers, leather armor, talons and a short sword. A light melee skirmisher that approaches through exposed ground and uses a warned two-tile swoop. The swoop ends on reachable walking ground. | Sidestep the fixed lane, then attack during its one-action recovery. Ordinary gusts move Raiders too. |
| 2 | Cliff Gargoyle | 2 | Compact weathered stone body, folded wings and amber eyes. Strong front armor and a planted stance resist gusts. It commits to a warned straight lunge, then remains facing that direction during recovery. | Walk around it using a courtyard loop and attack its exposed back or recovery. It never perches on inaccessible cliff or water tiles. |
| 3 | Harpy Stormcaller | 3 | Pale feather mantle, blue cloth and a short rune staff. Alternates a warned fixed lightning strike with a call that changes the next gust's direction. New wind arrows and a full warning appear before that changed gust. | Use shelter to approach; freezing delays its preparation. A call never changes an already announced gust or attack mask. |
| 4 | Storm Elemental | 4 | A compact column of blue-gray cloud and pale wind ribbons. Immune to gust displacement, it approaches through exposed lanes and warns before a short crosswind burst. | Leave the marked fan or use masonry to break the burst's path. The Elemental itself occupies reachable land and can be fought with ordinary weapons. |
| 5 | Tempest Matriarch | 5 | Crowned harpy with broad pale wings, dark blue armor and a curved sword. Combines committed dives, wing sweeps and the courtyard's wind cycle. | Read its landing and gust direction together, use shelter or brace, and counter during the guaranteed landing recovery. |

Ordinary visible artwork is at most 24×24 pixels, including weapons and folded
wings. The Matriarch's spread wings are deliberately wider while her body
retains the same pixel density. No creature becomes unreachable merely because
its art depicts wings or swirling air.

## Predictable gusts

Wind is a spatial movement mechanic. It does not change attack accuracy and
continues while the player stands still. Each exposed lane follows a visible cycle:
two calm seconds, two warning seconds, one gust second, then one recovery
second before the next cycle. Initial encounters use a single lane; later
stages combine two lanes with staggered cycles.

During the warning, arrows mark the exact affected ground and direction, and
the intended one-tile landing is shown beside the player. The warned cells
and direction lock until that gust resolves. An independent environmental clock
advances the cycle without granting enemy turns. Menus, loss of focus and
projectile/transition animations pause it.

A gust pushes an unbraced player or Harpy Raider one cardinal tile. A wall,
closed shutter, occupied square, water, cliff, exit or other unsafe destination
stops the push. Gusts do not throw actors off the map, jump obstacles or inflict
collision damage. Warning combinations must leave an ordinary walking escape;
wind cannot deliver an unavoidable hit from a simultaneous enemy attack.

Masonry shelters the cells immediately downwind. Enemy gusts and the regional
wind use the same cover rules. Gargoyles visibly plant their claws; Storm
Elementals move with the air. These two types resist displacement.

On an exposed tile without an adjacent interaction prop, **Interact** (`A` by
default) braces during the final warning second for the scheduled gust.
Bracing consumes a combat action and holds until the gust or another player
action. It prevents that phase's gust displacement and grants no attack
protection. It expires
after the phase, so it creates a deliberate choice between holding position,
attacking or moving into shelter. Every class can brace without equipment or
consumables.

## Storm shutters: route versus cover

Paired timber shutters stand at selected fort openings, with a reachable
hand crank on each side. Interact toggles their physical state:

- **Raised:** the opening is solid cover, blocking walking, projectiles and
  downwind gust propagation. The longer route around the room remains open.
- **Lowered:** the opening becomes a walkable timber threshold. It creates a
  shortcut or a second approach to a guardroom, but wind and projectiles can
  pass through it.

This makes the control useful before and during encounters: open a flank,
enter a side room, or close an opening to shelter a courtyard approach. The
art, collision and attack paths must change together. Controls cannot raise a
shutter through an actor, item or portal, or strand anyone behind it.

Closing a shutter can reduce a currently warned gust. Opening one affects the
next cycle; it cannot expand an existing warning into previously safe cells.
All essential destinations remain reachable in either state through a
permanent alternate route. Optional rooms contain visible supplies and space
to fight, rather than an empty branch with no purpose.

## Tempest Matriarch encounter

The final courtyard combines two staggered gust lanes with permanent stone
windbreaks. The Matriarch stays on reachable courtyard ground. A wing animation
can suggest flight without removing her ordinary combat target.

Above half health, alternate these attacks:

1. **Marked dive:** two actions of warning show a fixed approach lane and
   landing tile. The Matriarch commits to that tile, then grants two complete
   counterattack actions without an extra adjacent strike. Her landing must
   remain reachable by ordinary melee movement.
2. **Wing sweep:** one action of warning marks a short directional fan. One
   sideways or backward step escapes it. A hit deals normal encounter damage
   and can push one safe tile using the same visible wind rules.

At or below half health, she adds **Thunder Feathers**: two fixed lightning
patches announced for two actions. They cannot cover every sheltered escape
or change after the warning. She calls one finite pair of Harpy Raiders once;
they grant no repeat loot. The wind cycle remains predictable and keeps its
current phase when the boss changes phase.

Freezing the boss preserves her pending target and recovery. Environmental
wind continues on its independent clock. Landing recovery remains the player's
reliable opening in both phases. First victory grants 220 gold and 2,600 score; the Matriarch grants 450 combat XP.
A possible Dawnmere quest remains a separate feature.

## Presentation and implementation acceptance

Use a small contextual line such as `GUST EAST: 2` or `SHELTERED`, with clear
ground arrows and an Interact prompt near shutters or when bracing is useful.
Keep the adventure visible; a large tutorial panel must not cover the fort.
The minimap should show cliff silhouettes, courtyard walls, open thresholds,
explored rooms and discovered caches. Terrain outlines distinguish shelter
from open ground without relying only on color.

Generate the headland coastline and fort compounds together. Validate the
entire walking component, loop routes, both shutter configurations, control
access, all actor positions, loot, portals and boundary exits across varied
seeds. Enemies spawn only on connected land, including those with flying art.

Fresh physical entry respawns ordinary enemies on previously visited stages.
Backtracking and portals retain the expedition's wounds and deaths. Save and
load must retain shutter states, the exact wind cycle and declared warning,
boss phase, enemies, claimed supplies, exploration and return destination.
Older saves gain Stormwatch without resetting unrelated progress. The last
town actually visited remains the Return to Town destination.

Crown Road North's
goblin formations and wolf packs, and the Catacombs dungeon revamp, remain
separate later change sets.
