# Pirate Coast art review

The deterministic source is `tools/sprites/pirate.py`, using the established
`pixelkit.py` pipeline. Runtime assets are `assets/images/pirate-entrance.bmp`
(168×144) and `assets/images/pirate-enemies.bmp` (six 64×64 cells). The fort
shares pixelkit masonry, tiled roofs, timber, outlines and ship textures with
the existing harbor. Its central arch is one native 24-pixel tile wide.

- [Native-scale fort beside the existing harbor](building-comparison.png)
- [Enemy atlas at 2× nearest-neighbor scale](enemy-atlas.png)
- [Pirates beside the player, an existing bandit and a dark elf in SDL](scale-comparison.png)
- [Stage 1's northern return exit embedded in the cliff boundary](pirate-north-exit.png)
- [Stage 1's eastern boundary continuation to stage 2](pirate-east-exit.png)
- [Dawnmere's south exit](pirate-dawnmere.png)
- [Coastal approach and entrance](pirate-approach.png)
- [Wreckers' Strand](pirate-beach.png)
- [Rounded shorelines without an outline, with shimmering water while idle](pirate-water.gif)
- [Cutlass Stockade](pirate-stage-2.png)
- [Smugglers' Piers](pirate-stage-3.png)
- [Shipwreck Reach](pirate-stage-4.png)
- [Flagship, blade-wave warnings and captain status](pirate-stage-5.png)

The game captures use the actual SDL software renderer at 1280×720. Review
checks include transparent art edges, a visible doorway aligned to the path,
water extending across the southern approach, connected coves, branching piers, open wreck railings, visible
pirates at the established scale, exact numbered attack warnings and a readable captain status panel.

The 64×64 enemy atlas cells render into 40×40 frames, centered over their
24-pixel map tiles with boots aligned to the player's baseline. Actual opaque
body heights measured through SDL are 22–23 pixels for ordinary pirates and
25 for Voss's tricorn, compared with the player's 24 and existing bandit/dark
elf sprites at 23. The larger atlas frame includes transparent padding; it is not the body size.

The layout review replaced the original open beach and isolated wreck interiors.
Cliffs, sea and bulkheads now define the playable spaces. Both exit markers sit
on actual map boundaries. The full-stage captures below use the same SDL tile
renderer at native scale with exploration revealed for inspection:

- [Wreckers’ Strand overview](pirate-overview-1.png)
- [Cutlass Stockade overview](pirate-overview-2.png)
- [Smugglers’ Piers overview](pirate-overview-3.png)
- [Shipwreck Reach overview](pirate-overview-4.png)
- [Black Flagship overview](pirate-overview-5.png)

The Buccaneer wears brown leather and carries a curved cutlass. The Swashbuckler
wears navy and carries two swords. The fort uses narrow arrow slits. Voss's
ranged warning represents a magical blade wave from his sword. Pirate equipment
uses medieval blades, axes and a crossbow throughout.

Quest review uses the same SDL renderer at 1280×720:

- [Dawnmere merchant doorway](merchant-exterior.png)
- [Corvin's complete map offer and bounty](merchant-offer.png)
- [Quest journal with the map clue and both rewards](merchant-journal.png)
- [Silver coffer beside ordinary gold plunder on stage 3](merchant-coffer.png)
- [Returning the coffer for the bounty](merchant-reward.png)

Corvin uses an existing NPC sprite at the shop counter. The quest coffer reuses
the native chest silhouette and wood textures with silver metal fittings.
The merchant exterior keeps its established building sprite and doorway scale.
