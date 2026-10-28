# Blackwater Swamp

Blackwater Swamp has five forward levels, with Mira on level 3 and the Swamp
Demon on level 5. Stillbury's north gate and Rosemoor's south gate each lead
onto a safe cobblestone road through grass. A mossy crypt framed by mangroves
stands beside the road; its south-facing doorway always starts Swamp 1.

Before the Demon is defeated, the road ends at the entrance. Victory opens
the full north–south road: Rosemoor north, Stillbury south. Leaving the final
clearing returns to that road so the player can walk home or onward. There is
no swamp shortcut, boss branch or route-choice menu. Ordinary backtracking
within the swamp remains available; retreat from level 1 returns to the entrance
path for the town used. Regular enemies do not need to be cleared to advance.

The swamp is drawn at the Crown Roads' 1.5× zoom: the camera shows about
30 × 18 tiles around the player. The Swamp Demon wakes within twelve tiles, so
it may begin its telegraphed attacks from just off screen.

Braided mudbanks connect irregular islands across broad blackwater channels.
Rounded cypress islets and reconnecting routes let players approach encounters
from different banks. Maps and enemies are cached independently of other regions.

## Blackwater channels and reed cover

Blackwater is impassable. Wide mudbanks cross between the islands, with loops
around the channels and alternate approaches to objectives. Three walkable reed
beds on each stage offer a way to approach or avoid a fight.

Reeds conceal every class from uninjured ordinary enemies more than three
cardinal steps away. Nearby enemies still notice the
player, and wounded enemies retain ordinary pursuit. Bosses ignore this
concealment. Leaving the reeds restores normal awareness immediately. No
interaction key, consumable or quest is needed. Loose loot and return portals
preserve the reeds beneath them.

Difficulty and first-visit travel rewards rise from levels 1 through 5.
The regular roster includes Giant Rats, Zombies, Bandits and Wraiths, with
Vampires on later levels. The final level adds the Swamp Demon and its guaranteed
Demonic Sword. A living Demon blocks the final exit but permits retreat to level 4.
Victory opens the road without creating a shortcut or discovery prompt.

Return to Town is a one-way escape to the last town actually visited. It leaves
no return portal. Physical re-entry starts through the normal swamp entrance.

## Bring Mira Home

At the Stillbury Inn, Bram asks the player to rescue his daughter Mira. She is
held by a named Vampire Captor in a clearing on level 3. Both towns start at
level 1, so Mira is always encountered before the final boss.

Defeat the captor, then speak to Mira to send her home. Return to Bram for a
one-time reward of 80 gold and 600 score. Accepting the quest starts a fresh
swamp expedition while retaining any boss victory and open town road.
Quest and rescue progress persist across backtracking and saves.

## Swamp Demon Encounter

The Demon retains **180 HP, 20 attack, 7 defense and 550 experience**, with
existing regional scaling. He stays at his arena position. Entering the final
clearing or damaging him from a clear nearby ranged lane starts the fight.
Retreat beyond 12 tiles outside the clearing pauses the exact queued attack;
active rot and vent cooldowns continue ticking on actions taken on stage 5.

- **Bog Tyrant, above half health:** alternate a one-action frontal cleave
  with a two-action aimed rot pool. The cleave covers the three tiles directly
  in front of the Demon; one sideways or backward step escapes. A pool covers
  its fixed center and four cardinal neighbors; two ordinary moves can leave it.
- **Unshackled Demon, at/below half health:** cycle cleave, twin rot pockets,
  then a two-action miasma fan. Pocket centers are at least four tiles apart,
  with at most ten lingering rot tiles. The fan reaches three tiles forward:
  widths three, three and five. Two sideways steps escape its widest approach.
- Targets and orientation lock when announced. Crossing half health during a
  warning does not widen it; the new phase begins at the next preparation.
  Walls and actors block targeting. A narrow position with no walking escape
  within the advertised countdown produces an empty swing.
- Each signature leaves one full recovery action with no extra adjacent
  retaliation. The Demon remains damageable throughout the encounter.

Pool eruption deals one signature hit, then leaves rot on ordinary mud for
**two further actions**. **R2/R1** outlines show its remaining lifetime; plain
numbered warnings show turns until the next signature. Lingering rot deals
8 damage before half-defense mitigation, with a minimum of 2. It inflicts no
hidden poison status, never blocks walking and does not replace exits, vents,
revealed traps or quest objects. Every loot copy retains its changing underlay.
Freeze pauses the Demon's warning and recovery; active rot and vent cooldowns
continue ticking independently, even if the Demon is dead.

Three **VENT** props offer an optional counter. Interact on or cardinally beside
one with contextual **`A` by default**. It drains rot within Manhattan distance
two, cancels the current signature and exposes the Demon for **two complete
counterattack actions**. It grants no healing, mana or signature immunity.
The vent then displays **4, 3, 2, 1** as it recovers; interacting while it cools
does not extend the opening. All classes can use it without gear or consumables.
Vents and pools preserve native paths and the Mira/Bram rescue quest.

The **Demonic Sword** remains guaranteed with its original stats and skill.
A full floor-loot list reserves it in a named root cache without deleting other
items. An earned, unclaimed sword returns there after a floor refresh. Interact
beside the cache or press **`P`** on it; full inventory leaves it available.
Claiming is permanent, so selling or deliberately dropping it grants no copy.
Victory opens the safe town road; the entrance remains available from either town.

See the [encounter screenshots and play-testing guide](art-sources/swamp-demon-encounter/README.md).

## Demonic Sword

The Demonic Sword is a one-handed weapon usable by Warriors, Mages, and Rogues.
It gives **+6 attack**, equal to the Long Sword. Press `F` to strike in the last
movement direction at an enemy one or two tiles away. The attack stops at the
first enemy or wall, and the sword keeps its full +6 bonus in ordinary melee.
It costs no mana.

## Save Compatibility

Save version **173** migrates the former seven-level route into five levels.
Old floors 1–7 map to 1, 2, 3, 5, 4, 2, 1. Mira's level-3 map and the former
level-4 boss arena retain their terrain and encounter state. Active and linked
portal snapshots take priority when old outer trails merge, with separate
slots reserved when needed. Obsolete shortcut markers, prompts and loot/portal
underlays are repaired. Health, equipment, gold, quest progress, exploration,
portal destinations and boss victories remain intact. The serialized cache
retains seven legacy slots for compatibility; only the first five are playable.
Older five-level saves migrate through the historical route conversion first.

Save version **175** also migrates saved side-area corridors to their new
entrance sprites and town paths, preserving accessible player positions and loot.

Save version **128** adds the Demonic Sword claim state. Existing enemy fields
save the phase, countdown, orientation and both locked pool targets. Appended
map tile IDs save rot expiry and vent cooldowns in active/cached maps and loot
underlays. Older living encounters gain vents in place and restart with a visible
warning, preserving health, deaths, position, exploration, gear, loot, rescue
progress and portal destinations. Existing victories remain completed; only
identifiable current, unsharpened floor rewards establish an unclaimed sword.

Save version **148** introduces the independent island layout for new stages.
Older active and cached stages gain wider banks, connecting routes, blackwater
and reed beds in place. Existing walking squares, actor positions, exploration,
loot, portal landings and quest progress remain usable. The final clearing's
terrain and the Demon's queued attacks are preserved. Ordinary enemies still
return only on a new expedition from town.
