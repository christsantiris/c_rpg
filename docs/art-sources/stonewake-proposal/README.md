# Stonewake Quarry concept art

Generated with the built-in imagegen tool for user review only. These are
speculative visual proposals, not production sprites or gameplay screenshots.
The [proposal](../../stonewake-quarry-proposal.md) records scope and decisions.
No runtime asset is overwritten and the world atlas remains unchanged.

## Style references

- Existing cave/forest/inn comparison: `../glassdeep-entrance/scale-comparison.png`
- Existing enemy/player renderer capture: `../enemy-batch-12/glassdeep.png`

## Review

The initial area draft reused purple crystal spiders from its style reference;
the revised concept replaces those with the new quarry roster. The initial
enemy sheet also oversized the creatures relative to the player; a targeted
scale correction brings normal humanoids near player height and reserves the
larger silhouette for the Colossus. The Magic Stone Warhammer panel shows its
confirmed maximum-HP bonus while equipped, with no numeric bonus chosen yet.

The selected drafts and their final scale are subject to art approval. Further
[targeted correction prompts](corrections.md) record the refinements.

## Initial generation prompts

### entrance

```text
Use case: stylized-concept. Asset type: PROPOSED pixel-art area entrance sprite for approval, NOT a finished runtime asset. Input image 1 is a STYLE AND SCALE REFERENCE ONLY: existing game's crystal-cave landmark beside a medieval inn. Input image 2 shows the actual small silver-blue player sprite for scale. Create a single new STONEWAKE QUARRY entrance landmark on a genuinely TRANSPARENT background, with ample margins and no text. A compact medieval limestone quarry cut into a low rounded coastal rock shelf, about seven game tiles wide and six tiles tall (conceived at 168x144 native pixels), matching the reference's crisp pixel clusters, dark one-pixel outlines, stepped shading and frontal RPG perspective. Cream and warm gray sedimentary rock ledges cut into broad benches. A SMALL weathered timber lifting crane with rope, pulley and suspended stone block over a side excavation; simple sturdy medieval carpentry and iron bands. Timber-reinforced dark mine entry at the BOTTOM CENTER, with a short rail track ending at the central front edge so the entry is clear. A little turquoise quarry pool at one side, a few stacked cut stones, a mine cart and sparse green moss. Warm amber lamp at doorway. Keep the doorway at human scale like the inn reference. The rock formation should read as a quarry and worked stone, NOT a new mountain range. One coherent landmark silhouette, the timber crane integrated into it, contained entirely inside the sprite. True 2D pixel art without antialiasing, blur or painterly brushwork. No giant crystals, no purple ground, no guns, no industrial engines, no steam machinery, no photorealism, no isometric diamond ground tiles, no background rectangle or pasted ground patch, no town walls. Do not include the reference inn or forest or copy the old cave. No text, no UI.
```

### area

```text
Use case: ui-mockup. Asset type: PROPOSED Stonewake Quarry area gameplay art concept, NOT an actual game screenshot. The reference images establish this C/SDL fantasy RPG's actual pixel-art style, floor tiles, building shading and small player/enemy scale. Create a readable LANDSCAPE orthographic TOP-DOWN RPG play-area concept at roughly 1536x1024, flat square-grid geography with short frontal rock faces, never a diamond-grid isometric illustration. A single explorable Stonewake stage with warm pale LIMESTONE excavation benches, buff gravel walking ground, weathered brown timber cranes/gantries, dark iron fittings, muted moss and two turquoise flooded excavation pits. Design a clearly connected, branching network: a perimeter ramp CIRCUIT gives a permanent walking route from a timber-braced NORTH BOUNDARY entrance to a visible SOUTH BOUNDARY exit; two substantial side branches explore an abandoned stone workshop and a recessed supply yard, each reconnecting to the main circuit. Two medieval timber cranes can swing short wooden walkways across cuts, providing OPTIONAL cross-connections rather than blocking the only route. Show solid continuous walkway connections at every end. Clear traversable ramps, no disconnected islands, no floating exit portal, no cramped single-tile hallway winding from start to end, no enormous empty rectangular floor. At least two optional small treasure chests in reachable side pockets. Include one SMALL silver-blue armored player, referencing image2, and only a handful of small Quarry Constructs, Rockworms and a Stone Troll with sensible relative sizes: ordinary humanoid bodies approximately player height, troll about 1.4x taller. Their positions must all be on connected traversable terrain. Surrounding quarry rock walls and pits define the minimap outline. Make machinery legible at the same tile scale, warm limestone faces textured with layered strata, timber bridges with posts and ropes. A modest workshop matches the reference medieval building textures and normal door scale. No huge buildings dwarfing the view. Keep the region entirely land-based quarry, pools contained in excavations, no beachfront/ocean in this interior-stage concept. Simple small title 'STONEWAKE QUARRY — AREA CONCEPT' at the top. No fake health stats, no side HUD, no dense annotation, no player dialogue. Crisp restrained 24px-era pixel-art clusters and nearest-neighbor presentation, no smooth painting or cinematic 3D, no purple terrain, no firearms, no modern construction equipment.
```

### enemies

```text
Use case: stylized-concept. Asset type: Stonewake Quarry ENEMY AND QUEST REWARD concept sheet for approval. Input images are existing game style/scale references only. Create a polished LANDSCAPE 1536x1024 pixel-art presentation on a plain dark desaturated green backdrop. Title at top: 'STONEWAKE QUARRY — ENEMIES'. Six well-spaced figures in ONE ROW on the SAME BASELINE, drawn with the SAME pixel density and SAME shared scale, with ample space below for readable name labels. From left to right: 1 'PLAYER' — the game's small silver-blue helmeted knight with blue cloth, a normal sword and shield, body conceived about24 pixels tall; 2 'ROCKWORM' — low gray-brown segmented burrowing worm, rocky plated back, warm ochre jaws, about1.4 player-body widths long but lower than the player; 3 'QUARRY CONSTRUCT' — compact articulated limestone laborer with timber shoulder bracing and muted green rune, approximately player height; 4 'STONE TROLL' — stocky rugged humanoid with beige stone hide, mossy shoulders and heavy knuckles, about1.4x player height, visibly a troll rather than another robot; 5 'IRON SENTINEL' — dark wrought-iron armored guardian with stone shoulder plates and a medieval polehammer, about1.15x player height; 6 'QUARRY COLOSSUS' — imposing boss built from massive quarried limestone blocks with old iron braces, a few carved green runes and a cracked central joint, ONLY about2x player height and broader, not skyscraper size. These are fantasy quarry creatures, no guns, cannons, explosives, modern robots, science-fiction armor, machinery exhaust or neon sci-fi effects. No reused crystal-spider designs. All five enemies have distinct silhouettes and fit the reference game's crisp 2D pixel style, dark outlines and textured stepped shading. IMPORTANT all art uses one shared pixel scale; do NOT enlarge each regular enemy independently to fill an equal-sized panel. Below the lineup, separated by a thin understated line, show a generous reward panel with ONE enlarged inventory-icon preview of a MAGIC STONE WARHAMMER: heavy rectangular weathered stone head with beveled striking faces, small glowing green carved runes, iron binding and a leather-wrapped wooden handle, strong readable medieval hammer silhouette, not an axe or futuristic weapon. Exact readable text beside it: 'QUEST REWARD', 'Magic Stone Warhammer', '+ Maximum HP while equipped', and small 'Bonus amount to be balanced'. A small inset of the same hammer at ordinary item-icon scale is welcome. No damage numbers, no invented equipment stats, no permanent-health text. No photorealism, no blur, no soft painted illustration. This is a speculative art concept, not an implemented game screenshot.
```


## Approved runtime implementation

The proposal is implemented through `tools/sprites/stonewake.py`, rather than
sampling or resizing these concept images into the game. Native pixelkit art
and actual SDL renderer captures are linked in the
[Stonewake guide](../../stonewake-quarry.md#art-and-compatibility). The atlas
uses a separate localized built-in imagegen edit; its exact prompt and
protected-coastline review are in [the atlas record](../world-map/stonewake.md).
