# Goblin Mountains

The Goblin Mountains connect OakHaven's north gate and Ridgeshire's south mountain
gate and use an independent seven-level cache. Entering from OakHaven starts at
level 1; entering from Ridgeshire starts at level 7. Both routes climb toward
the Goblin King at Crown Peak on level 4, then descend in difficulty toward the
opposite town. Their black basalt, iron-red rock and ember-orange accents
separate the region visually from the dungeon and Haunted Forest. The Mountains
use seven authored route graphs rather than the Forest's clearing templates.

Wild Boars are deliberately excluded. The area is about an organized goblin
war host, its siege craft, guarded mines and enslaved cave creatures.

| Level | Region | Layout identity | Enemy roster |
| --- | --- | --- | --- |
| 1 | Blackrock Foothills | Open switchbacks and scout camps climbing to a north exit | Goblin Scouts |
| 2 | Raider Pass | Overlapping firing lanes leading east | Scouts and Goblin Archers |
| 3 | Cinder Mines | Braided mine routes descending to a south exit | Scouts, Archers, Bombers and Tunnel Spiders |
| 4 | Crown Peak | Twin approaches into the summit fortress and a southern descent | Full roster, Goblin Shamans and the Goblin King |
| 5 | Siege Camp | Split routes through abandoned positions; south pass | Scouts, Archers, Bombers and Tunnel Spiders |
| 6 | Collapsed Tunnels | Broken routes and side chambers leading east | Scouts and Goblin Archers |
| 7 | Ridgeshire Approach | Timber crossings and scout camps near the town | Goblin Scouts |

Approach stages do not require a full clear. Reaching the marked boundary pass
is enough to advance, whether it lies east, north or south. At the peak, only
the Goblin King blocks crossing to the other approach; retreat remains possible.
His defeat prompts discovery of a marked shortcut beside the defeated boss to
the opposite town. Press Enter to acknowledge the prompt; movement cannot
dismiss it, matching the forest and swamp. Players
can take it immediately or fight through the three remaining stages.

## Combat Roles

- **Goblin Scout:** basic melee pursuer.
- **Goblin Archer:** fires along clear rows and columns.
- **Goblin Bomber:** launches high-damage explosives along clear firing lines.
- **Tunnel Spider:** armored ambusher whose bite applies poison.
- **Cave Troll:** slow, high-health bruiser.
- **Hobgoblin Guard:** heavily armored frontline defender.
- **Goblin Shaman:** periodically heals a nearby wounded ally.
- **Goblin King:** remains at Crown Peak until confronted, then alternates a
  telegraphed wind-up with powerful thrown-axe attacks.

The route geometry and ground treatment vary by level. Foothill and ascent
stages use exposed basalt switchbacks. Raider Pass and Ridgeshire Approach contain
narrow timber bridge crossings. Cinder Mines and Collapsed Tunnels use dark
underground chambers, while Siege Camp and Crown Peak use paved
fortress courtyards and linked defensive positions.

## Terrain and Treasure

Stages 2 and 7 contain a weak bridge span that collapses after the player
crosses it. Stand beside the gap and press `A` to rebuild the bridge from either
side. The other mountain stages use open room routes without the old internal
toggle-gate chambers.

Stages 3–6 contain an optional rockfall passage. Press `A` beside either
end to expose it. The falling rocks deal `4 + difficulty` HP, but the chamber behind
them contains a one-time cache worth `15 + 4 × difficulty` gold. Difficulty follows
1, 2, 3, 8, 3, 2, 1, so hazards and rewards also decrease away from the peak. Stand on the cache
and press `A` to collect it. Bridge repairs, opened passages, claimed caches,
and minimap discoveries persist while the expedition is cached and through
save/load.

## Mountain Quest

Speak to Dain with `T` in **Rosemoor's Adventurer's Guild Hall** to accept
**Recover the Treasure Map**. His quest places guarded Map Bearers on stages
**7, 6, and 5**, entered through Ridgeshire's south mountain gate after traveling
west from Rosemoor along Crown Road West. The Archer, Bomber, and Shaman leaders
respectively carry one fragment and travel with a themed warband. Defeating the
leader recovers the fragment automatically. All three objectives precede the
stage-4 Goblin King from Ridgeshire; entering from Oakhaven finds them after him.
Accepting starts a fresh mountain expedition, while a previously defeated
Goblin King remains dead and permanent shortcuts stay open. After victory,
take the peak shortcut to Oakhaven and the unlocked town roads home, or battle
through stages 3, 2, and 1.

Save version **113** preserves collected fragments and explored maps. Old
bearers on incorrect stages become regular enemies with their existing health,
positions, and death state. Missing Archer and Bomber fragments gain bearers on
stages 7 and 6 when visited; the Shaman remains on stage 5. Loot, boss victories,
and portal anchors remain intact.

Return all three fragments to Dain in Rosemoor's Guild Hall for the one-time
reward of **60 gold and 400 score**.

Ridgeshire's east gate enters [Dragonspine](dragonspine.md) directly. Ilya
assigns its goblet quest in Oakhaven's Tavern. After the Goblin King's defeat, a separate south shortcut gate
at x=28 follows the safe High Pass to OakHaven's northeast road. Ridgeshire's
ordinary south mountain gate at x=20 stays open before and after the victory.
Return to Town anchors to the expedition's entry town, and its return portal
keeps that anchor if the mountains are later entered from the opposite town.

## Save Compatibility

Older saves migrate automatically. The old level 8 summit moves to level 4;
the former level 4 and level 7 approaches merge into level 7, preserving the
occupied or portal-linked map when needed. Character progress, boss victories,
partial map fragments, active loot, and portal coordinates remain intact. Old
map bearers become regular enemies, and missing fragments gain bearers on their
new stages when visited. Existing shortcut entrances relocate beside the saved
boss death position on load. Saving writes version 79 with both mountain travel anchors.
