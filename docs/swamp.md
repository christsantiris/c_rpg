# Blackwater Swamp

Blackwater Swamp connects Stillbury and Rosemoor across seven levels.
Stillbury's main north gate enters level 1; Rosemoor's main south gate enters
level 7. Either approach crosses three levels of increasing difficulty before
meeting the Swamp Demon on level 4. Beyond the Demon, the remaining three
levels become easier. Dark water, cypress trees, and swaying reeds border
nine uneven muddy clearings on each level, connected by winding causeways.

The west-edge trail moves toward lower level numbers and Stillbury; the
east-edge trail moves toward higher numbers and Rosemoor. Either border level
allows immediate retreat to its town. Maps and enemies are cached independently
from other regions and retained when entering from either town. Ordinary
enemies do not need to be cleared to advance.

| Levels | Regular enemies | Difficulty |
| --- | --- | --- |
| 1 and 7 | Giant Rats, Zombies, Bandits, Wraiths | Border levels |
| 2 and 6 | Giant Rats, Zombies, Bandits, Wraiths | More enemies |
| 3 and 5 | The same roster, plus Vampires | Stronger regular enemies |
| 4 | Full roster and Swamp Demon | Central boss encounter |

Giant Rats are the weakest swamp enemies; Bandits are tougher and better
defended; Vampires have the most health and attack of the regular roster.
Zombies and Wraiths retain their dungeon enemy rules. The Demon remains a
separate boss and drops a fixed Demonic Sword reward. Travel score is awarded
on first visits to levels from either direction.

## Demon and Shortcuts

The living Demon blocks crossing level 4 toward the opposite town, but allows
retreat toward the expedition's entry town. Defeating him displays a shortcut
discovery prompt that stays visible until a fresh press of Enter acknowledges
it, matching the forest and mountains. A separate marked trail appears beside
the defeated Demon, even if he was lured away from his clearing. That trail
goes directly to Rosemoor when entering from Stillbury, or to Stillbury when
entering from Rosemoor. Players can instead cross the three remaining levels.
Surviving regular enemies do not block either exit choice.

Victory also permanently unlocks a safe swamp road between the towns. The
second north gate in Stillbury and separate south gate east of Rosemoor's main
swamp gate enter this enemy-free, one-tile-wide road. Both ordinary swamp gates remain open.
The Demon does not respawn on later visits or after accepting Bram's quest.

Return to Town returns to the expedition's entry town. Its portal preserves
the exact swamp level, casting position, and original town, even after the
swamp is subsequently entered from the other town. From Stillbury, the town
portal appears beside the north swamp gate; from Rosemoor, beside the south
swamp gate.

## Bring Mira Home

At the Stillbury Inn, Bram asks the player to rescue his daughter Mira. She is
held by a named Vampire Captor in a clearing on level 3, on the Stillbury side
of the Demon. Starting from Stillbury encounters Mira before level 4. Starting
from Rosemoor with the quest active encounters her after defeating the Demon
and continuing through level 3. Taking the central shortcut from Rosemoor
skips her clearing; the ordinary Stillbury swamp gate permits rescuing her later.

Defeat the captor, then speak to Mira to send her home. Return to Bram for a
one-time reward of 80 gold and 600 score. Accepting the quest starts a fresh
swamp expedition while retaining any boss victory and unlocked shortcuts.
Quest and rescue progress persist across backtracking and new saves.

## Swamp Demon Encounter

The Demon retains **180 HP, 20 attack, 7 defense and 550 experience**, with
existing regional scaling. He stays at his arena position. Entering the final
clearing or damaging him from a clear nearby ranged lane starts the fight.
Retreat beyond 12 tiles outside the clearing pauses the exact queued attack;
active rot and vent cooldowns continue ticking on actions taken on stage 4.

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
The central shortcut, safe swamp road and both town approaches remain available
under the existing victory rules.

See the [encounter screenshots and play-testing guide](art-sources/swamp-demon-encounter/README.md).

## Demonic Sword

The Demonic Sword is a one-handed weapon usable by Warriors, Mages, and Rogues.
It gives **+6 attack**, equal to the Long Sword. Press `F` to strike in the last
movement direction at an enemy one or two tiles away. The attack stops at the
first enemy or wall, and the sword keeps its full +6 bonus in ordinary melee.
It costs no mana.

## Save Compatibility

Older saves migrate automatically when loaded. The old Demon stage moves from
level 5 to level 4, and Mira's clearing moves from level 4 to level 3. Saved maps,
enemies, and portal destinations move with those levels; the two additional
stages start unexplored. Character progress, rescue progress, and boss victories
remain intact. Saving writes the current format with seven cached levels and
the expedition's entry town and portal anchor.

Save version **128** adds the Demonic Sword claim state. Existing enemy fields
save the phase, countdown, orientation and both locked pool targets. Appended
map tile IDs save rot expiry and vent cooldowns in active/cached maps and loot
underlays. Older living encounters gain vents in place and restart with a visible
warning, preserving health, deaths, position, exploration, gear, loot, rescue
progress and portal destinations. Existing victories remain completed; only
identifiable current, unsharpened floor rewards establish an unclaimed sword.
