# Stillbury geography validation — 2026-11-05

Debug `conr` and `test_runner` builds succeed. The full gameplay suite passes
**8,513 / 8,513** checks.

The new route checks use actual movement through transitions across seeds
19, 107 and 1301. They cover all five Elderwood and Blackwater stages, repeated
retreats and cached-stage revisits, independent boss gates, and onward travel
to the coast and Catacombs. Final exits retain their crossing direction and
provide a forward trail before its first bend.

The western bridge road forks north toward the quarry and optional labyrinth.
Its real boundaries, quarry entrance and maze retreat work with both the Jarl
and Minotaur alive. The north gate now leads to Blackwater; the new east gate
retains Stillbury's existing wanted-player checkpoint.

Save version 243 appends route IDs without reordering existing IDs. Tests cover
legacy coastal and swamp approaches, active and cached Elderwood boundaries,
wounded enemies, recoverable stacked loot and item metadata, current boundary
loot, and exact player/loot positions on both final exit trails. No new quest,
reward or reputation state is introduced.

Native SDL software-renderer review at 1280×720 and 640×480 covers the swamp
and Catacombs road, Elderwood and coast entrances, western fork, quarry/maze,
outdoor boundary labels, and final exit trails. A real existing save and an
affected legacy swamp fixture were loaded and rendered. Original local saves
and settings remain byte-for-byte unchanged; backups are outside the repository.

The inspected final atlas is `docs/world-map.png`, SHA-256 prefix
`397df5e09b72`. Both README image references use that asset and cache key.
The built-in imagegen edit prompt is recorded beside this file.

Mara's move, Rowan's new dock, the island amulet, coastal-trade reputation and
the Demon victory's reception change remain subsequent story work. Elderwood's
evidence against Malakar remains undecided.
