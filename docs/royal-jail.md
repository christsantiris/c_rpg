# Royal Jail and Escape Tunnel

The castle grounds contain a stone jail southwest of the main entrance.
A brown-coated townsman east of the crossroads appears to be an ordinary NPC.
Talking to him with **T** triggers a one-time betrayal: he reports the player
to the Royal Guards, who imprison them. The arrest dialogue explains exactly
what happened. Walking nearby, bumping into him, or pressing Interact does not
cause arrest. The jail's front door is locked.

Equipment, money, and regional progress survive arrest. The player appears in a
barred stone cell with fellow prisoner **Tomas**; guards stand beyond the bars.
Speak to Tomas with **T**, then **Y** to accept **Guide Tomas Home**. He
explains that a loose flagstone conceals a tunnel to central Ridgeshire, but he
cannot escape alone because smugglers, spiders, and rats occupy it. Accepting
his offer reveals a visible ladder hatch in the cell's southeast corner.

Walking onto the hatch takes both characters into the **Escape Tunnel**.
Tomas closes it behind them to prevent pursuit, so there is no return to jail.
The tunnel is a single combat map with bends and room to maneuver, like the
Crown Roads. Twelve enemies impede the route; there is no boss or stair travel.

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

Save version **101** records the arrest/escort/reward state and companion
coordinates alongside the current map, enemies, and loot. Saving before the
hatch is revealed, after accepting, or mid-tunnel resumes that exact encounter.
Earlier saves gain an unstarted quest; saved castle grounds gain the jail and
informant. Players and loot overlapping the new footprint move to the jail's
front path while equipment, money, boss victories, and quest progress remain.

For play testing:

1. Talk to the townsman east of the castle crossroads and read the arrest reason.
2. Approach Tomas inside the cell and press T; locate the newly revealed hatch.
3. Save/load in the cell, then enter the tunnel and fight its mixed enemies.
4. Try Teleport, Space, and an early exit attempt while Tomas is behind you.
5. Save/load mid-escort; confirm enemy damage, loot, and Tomas's position persist.
6. Reach central Ridgeshire together and check the reward and Completed journal.
7. Return to the townsman and confirm the arrest and reward cannot repeat.
