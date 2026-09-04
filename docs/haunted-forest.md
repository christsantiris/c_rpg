# Haunted Forest

The Haunted Forest is a five-level adventure entered from the west end of the
town crossroad. Its progress and generated floors are cached independently from
the dungeon, so changing regions never erases progress in the other adventure.

Unlike the dungeon, the forest has no stairs. The player enters through the
opening on the left edge, then the entrance clearing splits into upper and
lower woodland routes. Cross-connections allow route changes before both paths
rejoin near the destination grove. Stage exits move between the east, north and
south map edges so that every route is not a west-to-east crossing. Levels 1-4
do not require every enemy to be defeated. The west opening supports
backtracking and returns to town from level 1.

| Level | Region | Layout | Roster |
| --- | --- | --- | --- |
| 1 | Gloomwood Border | Two-route fork with a middle crossover; east exit | Pixies and Blighted Wolves |
| 2 | Spiderweb Thicket | Three pockets wrapped around a central trail; north exit | Pixies, Wolves and Giant Spiders |
| 3 | Dark Elf Territory | Three braided lanes with diagonal shortcuts; south exit | Pixies, Wolves, Spiders and Dark Elves |
| 4 | Buried Wilds | Central hub splitting into three dangerous branches; north exit | Spiders, Dark Elves, Giant Wurms and Forest Trolls |
| 5 | Necromancer's Grove | Twin approaches narrowing into an antechamber and eastern boss grove | Complete roster plus the Necromancer |

Clearing dimensions and positions receive additional random variation every
time a new stage is generated, while each level retains its recognizable macro
structure.

## Enemy Roles

- **Pixie:** fragile skirmisher that moves twice per turn.
- **Blighted Wolf:** fast melee hunter that moves twice per turn.
- **Giant Spider:** melee controller whose bite applies poison.
- **Dark Elf:** archer that fires along an unobstructed row or column.
- **Giant Wurm:** armored heavy enemy that moves every other turn.
- **Forest Troll:** high-health bruiser that moves every other turn.

## Final Encounter

The Necromancer waits in the final grove and remains dormant until the player
enters it. He alternates a telegraphed invocation with a spirit bolt. Every
fourth action, he can return one defeated forest servant at half health. The
number of enemies remains bounded by the floor's original roster.

After defeating the Necromancer on level 5, the eastern passage sends the
player to the west town road. Other surviving enemies do not block it. Return
to Town also works inside the forest;
its portal preserves the exact forest level and casting position.
