# Castle review artwork prompts

Generated with the built-in imagegen tool for visual review. These images are concept sheets, not integrated gameplay assets. The character sheets are RGBA PNGs with transparent backgrounds; production atlases will require conversion to gameplay sprite dimensions. The interior board is an opaque RGB concept illustration. Defeating Lord Veyr ends the game with a win condition.

## castle-enemies.png

```text
Use case: stylized-concept.
Asset type: transparent pixel-art enemy sprite review sheet for the C/SDL tile RPG Castle of No Return.
Primary request: EXACTLY FIVE distinct, isolated, full-body enemy sprites. Layout: three equally spaced sprites on the top row, two equally spaced sprites centered on the bottom row. No duplicates. No text or labels. Plenty of transparent gutter around each silhouette, no cropping, no ground scenery.
Subjects, in reading order: 1 Oathbound Soldier: weathered charcoal plate armor, faded crimson tabard, tarnished gold crest, closed helmet with two small violet eyes, straight sword and small round shield; lean upright disciplined silhouette. 2 Iron Warden: massive broad rectangular shield with a faded royal crown emblem, dark heavy plate armor, crimson shoulder cloth, squat broad silhouette, short mace visible at one side. 3 Royal Marksman: hooded crimson-and-charcoal leather uniform, pale face in shadow, clearly recognizable crossbow held ready across the chest, quiver and slim silhouette. 4 Court Hexer: hooded violet robes, gold collar, elongated narrow silhouette, staff with a small violet crystal, one hand casting a restrained angular purple rune. 5 Bell Herald: hunched crimson-robed royal servant with a bronze handbell raised and a tall narrow back-mounted banner bearing a broken crown.
Style/medium: authentic retro fantasy pixel sprites, visibly square hard-edged pixel clusters, dark one-pixel outlines, limited palette, 3 or 4 shades per material, roughly 48x48 logical pixels per regular enemy enlarged cleanly for review. Slightly overhead front three-quarter RPG view, with short chunky character proportions and feet visible; no isometric floor. Match a tile RPG with 24px gameplay tiles. Each enemy has an immediately distinct silhouette that will remain readable small.
Color palette: charcoal stone/steel, dull gold, faded crimson, muted violet accents; small bright highlights ensure armor is legible.
Constraints: actual transparent background, no checkerboard painted into image, no opaque canvas, no frames, no lettering, no shadows outside silhouettes, no painterly brushwork, no photorealistic rendering, no smooth gradients, no excessive glowing effects. These are design-review sprites, not animation frames.
```

## castle-bosses.png

```text
Use case: stylized-concept.
Asset type: transparent pixel-art miniboss and final boss sprite review sheet for Castle of No Return.
Primary request: EXACTLY FIVE isolated full-body figures with generous transparent space. Layout: top row TWO minibosses, bottom row THREE phases of the SAME final boss. Consistent recognizable crown and sword across the final boss phases. No text, no labels, no frames, no ground scenery.
Top left: THE CASTELLAN: massive fortress commander in square charcoal plate armor, broad shoulders, crimson cloak, thick tarnished-gold trims, oversized tower shield and heavy warhammer, closed helmet with cold violet eye slits. Taller and broader than normal guards.
Top right: THE ROYAL ARCANIST: imposing court sorceress in layered violet and black robes with crimson lining, high angular gold collar, pale face, crown-like headpiece, ornate staff, one geometric violet sigil near her free hand. Silhouette distinct from the regular hooded Court Hexer.
Bottom left: LORD VEYR phase 1, THE ARMORED RULER: dark plate armor with weathered gold royal decorations, broken gold crown on helmet, ragged crimson mantle, long royal sword held point-down, authoritative upright posture.
Bottom center: LORD VEYR phase 2, THE BROKEN THRONE: same figure, same broken crown, same sword, armor cracked apart revealing violet curse-light through chest and shoulders, torn mantle, more aggressive forward stance; retain corporeal armored legs.
Bottom right: LORD VEYR phase 3, THE UNBOUND CROWN: same recognizable broken crown suspended above a ruined royal helmet and remaining chest plate, spectral violet body with clearly readable arms and trailing lower silhouette, same sword held outward, angular violet shards around the crown, restrained effects preserving silhouette.
Style/medium: genuine crisp retro fantasy pixel-art sprites, roughly 64x64 logical pixels per boss enlarged with hard square pixel clusters for review, dark outlines, limited color palette, 3 or 4 shades per material, short chunky RPG proportions, slightly overhead front three-quarter view, consistent style across all figures. Human-scale ordinary enemies would be 48 logical pixels, so these bosses visibly larger.
Constraints: actual transparent background, no checkerboard painted into image, no opaque canvas, no typography, no cut-off weapons, no painterly or photorealistic illustration, no smooth gradients, no detailed realistic anatomy, no explosive particle clouds. Final boss dies to end the game; these are combat phase concepts, no post-victory roaming phase.
```

## castle-interiors.png

```text
Use case: stylized-concept.
Asset type: six-panel pixel-art castle interior environment review board for the C/SDL tile RPG Castle of No Return.
Primary request: a clean wide art-direction board showing SIX DISTINCT PLAYABLE ROOMS in a 3-column by 2-row grid. Each panel is a readable top-down tile-based game room with a small single blue-robed player sprite to establish scale. Panels have narrow dark gutters and simple legible uppercase pixel-font titles at their top edge, exact titles in reading order: "GATEHOUSE", "FORSAKEN COURT", "IRON KEEP", "ROYAL ARCHIVES", "CROWN CHAPEL", "THRONE OF NO RETURN".
Shared visual language: charcoal and gray-violet masonry, aged gold trim, ragged crimson banners, isolated amber candlelight, restrained cold violet cursed light. Crisp visibly square pixel clusters, tiled 24px-gameplay-scale architecture, chunky small sprites, orthographic top-down RPG view with front faces on north walls, navigable floors are bright enough to read. Rich environmental variation without filling every walkable tile with props. No isometric perspective, no cinematic painting.
Panel 1 GATEHOUSE: heavy double portcullis, worn stone courtyard, guard alcoves, broken royal crest, two shield guards defending the approach.
Panel 2 FORSAKEN COURT: abandoned royal banquet hall, broken tables along sides, dusty crimson carpet route, stained glass and hanging banners; paired soldiers protecting a distant crossbowman, multiple clear paths around furniture.
Panel 3 IRON KEEP: square stone miniboss arena, stout stone buttresses, iron bars, Castellan standing with tower shield and warhammer; a clearly readable amber charge-warning lane on the floor and nearby safe route.
Panel 4 ROYAL ARCHIVES: dusty bookcases, magical apparatus and violet rune inlays, several iron barriers with visible lever mechanisms; a narrow outlined violet warning on one barrier, alternative open route. One robed Court Hexer.
Panel 5 CROWN CHAPEL: broken stained-glass windows, red aisle carpet, pews at sides, violet geometric ward gates, Royal Arcanist near the far altar. A clear greenish-blue open passage is visually distinct from solid closed iron barriers.
Panel 6 THRONE OF NO RETURN: largest open fighting floor, cracked black-and-gold throne at north, long crimson approach, columns and navigable cover at sides, armored crowned Lord Veyr facing player. Subtle violet tile-bound attack warning lanes preserve strong floor readability.
Mood: oppressive occupied royal fortress progressively more supernatural, distinct from a skeleton crypt; grand and tragic rather than grotesque. Preserve logical walkable tile scale in all rooms. No HUD, no health bars, no decorative fake UI. Avoid piles of skulls, noisy fog, smooth digital painting, tiny illegible microdetails, and nonfunctional labyrinth clutter. This is environment concept art for review, not a screenshot of implemented gameplay.
```
