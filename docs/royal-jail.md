# Royal Jail and Escape Tunnel

The castle grounds contain a stone jail southwest of the main entrance.
A brown-coated townsman east of the crossroads appears to be an ordinary NPC.
Talking to him with **T** triggers a one-time betrayal: he reports the player
to the Royal Guards, who imprison them. The arrest dialogue explains exactly
what happened. Walking nearby, bumping into him, or pressing Interact does not
cause arrest. The jail's front door is locked.

Guards confiscate **all weapons, armor, and shields**, including spare items in
the pack. Their exact upgrades and bonuses are stored for recovery. Equipped
bonuses are removed normally. Gold, consumables, quest items, and regional
progress stay with the player. Arrest assigns **Recover Your Equipment**.
The player appears in a barred stone cell with fellow prisoner **Tomas**;
guards stand beyond the bars.
Speak to Tomas with **T**, then **Y** to accept **Guide Tomas Home**. He
explains that a loose flagstone conceals a tunnel to central Ridgeshire, but he
cannot escape alone because smugglers, spiders, and rats occupy it. Accepting
his offer reveals a visible ladder hatch in the cell's southeast corner. Before
acceptance, the flagstone looks like ordinary floor and cannot enter the tunnel.
This remains true when using `./tools/debug_shortcuts.sh`.

Walking onto the hatch takes both characters into the **Escape Tunnel**.
Tomas closes it behind them to prevent pursuit, so there is no return to the cell.
After confiscation he supplies a Smuggled Dagger for the escape, small enough
to hide in a cell. Every class can wield it.
This does not return any confiscated equipment. The first enemies stand farther
along the passage, leaving space to prepare after arriving.
The tunnel is a single 190-tile combat map with thirty enemies. Its original
passage has room to maneuver; deeper stretches narrow to three tiles and add
three larger bends. Eighteen stronger enemies guard the extension, including
archers, shielding bodyguards, smugglers, and poisonous spiders. There is no
boss or stair travel.

Tomas visibly follows one tile per combat turn along walkable, unoccupied
terrain. He avoids enemies and does not teleport to the player. Clear the way
or bypass enemies together. He is a noncombat companion without a separate
health meter. Passing through his position swaps places, so he cannot block
backtracking. The player can still be hurt or killed normally.

**Space** spends a turn waiting for Tomas without changing weapon/spell facing;
enemies take their normal turn. A custom command assigned to Space takes
precedence. Moving back toward Tomas or attempting the eastern exit again also
lets him catch up. The exit refuses departure while he is more than one tile
away. Town portals are blocked by the royal wards in both jail and tunnel,
with an explanatory message and no mana cost. Ordinary Teleport remains usable,
but cannot transport Tomas; he must walk and catch up.

Reaching the eastern exit together brings both characters to the center of
**Ridgeshire**. Tomas automatically gives **100 gold and 750 score**. His quest
moves to the Completed journal tab. Speaking to him again, saving/loading, and
returning to the informant cannot repeat the reward or arrest.

## Recovering equipment

After escaping with Tomas, return to the existing **jail building southwest of
the castle entrance**, outside the castle walls. Walking through its front door
enters the evidence room. **Three Royal Guards** defend the chest; all three
must be defeated before it opens. They are combat enemies who can damage and
kill the player. Leave through the southern door to retreat to Castle Grounds.
Town portals are blocked inside the evidence room, with a message directing
the player to that door.

Stand beside the evidence chest and press **Interact** to reclaim equipment.
Original equipped items are returned first. Empty equipment slots are restored
where possible; current equipment choices are retained. If the pack fills,
unclaimed items stay in the chest. Make space and interact again. The recovery
quest completes only after every confiscated item has been reclaimed. Repeated
interaction cannot duplicate items or grant another reward.

Guard wounds and defeats, dropped loot, confiscated item metadata, and partial
recovery persist through retreat, reentry, and save/load. The evidence room
remains accessible after recovery.

Save version **101** records the arrest/escort/reward state and companion
coordinates alongside the current map, enemies, and loot. Saving before the
hatch is revealed, after accepting, or mid-tunnel resumes that exact encounter.
Earlier saves gain an unstarted quest; saved castle grounds gain the jail and
informant. Players and loot overlapping the new footprint move to the jail's
front path while equipment, money, boss victories, and quest progress remain.

Version **118** extends older tunnel saves beyond their original eastern exit.
The existing passage, revealed terrain, Tomas's position, enemy damage and
defeats, and dropped loot stay intact. The additional enemies occupy only the
new passage. Saving again retains the extension and its combat progress.

Version **183** adds confiscated equipment, original equipment slots, escape
supplies, claimed-item tracking, and the evidence room's guards and loot. Older
saves already in jail or the tunnel keep their equipment and escort progress;
the migration does not confiscate items retrospectively. An old visible hatch
is hidden again if Tomas's quest has not been accepted.

For play testing:

1. Talk to the townsman east of the castle crossroads and read the arrest reason.
2. Check that weapons, armor, and shields are gone but gold and other items remain.
3. Confirm the hatch is hidden, including in debug mode. Talk to Tomas with T
   and accept with Y; locate the newly revealed hatch.
4. Save/load in the cell, enter the tunnel, and check the supplied class weapon.
5. Try Teleport, Space, and an early exit attempt while Tomas is behind you.
6. Save/load mid-escort; confirm enemy damage, loot, and Tomas's position persist.
7. Reach central Ridgeshire together and check the reward and Completed journal.
8. Return to the jail's front door. Fight a guard, retreat, and save/load; verify
   that its damage and any dropped loot remain when reentering.
9. Defeat all three guards, then interact with the chest with a full pack and
   with one free slot. Save/load during partial recovery, then reclaim the rest.
10. Confirm every original item retains its bonuses and sharpening, recovery
    completes in the journal, and neither arrest nor rewards can repeat.

SDL renderer checks: [hidden hatch](art-sources/jail-recovery/jail-hidden.png),
[revealed hatch](art-sources/jail-recovery/jail-revealed.png), and
[guarded evidence room](art-sources/jail-recovery/jail-evidence-room.png).
