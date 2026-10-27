# Giant's Crown — native art and renderer review

Reviewed on 2026-10-27 in the actual SDL game renderer at 1280 × 720, using
its normal viewport and native 24-pixel tile scale.

The runtime atlas is
[crown-quest-props.bmp](../../../assets/images/crown-quest-props.bmp), generated
by [crown_quest.py](../../../tools/sprites/crown_quest.py) with the established
[pixelkit.py](../../../tools/sprites/pixelkit.py) pipeline and Giant's Peaks'
existing stone and outline palettes. The 144 × 48 transparent atlas contains
six 24 × 48 frames: inscription, seal pedestal, crown pedestal, empty pedestal,
warded doorway, and open doorway. Frames are drawn at native size, with
transparent space above shorter props. The ordinary stone coffer remains a
separate object beside each relic. The open doorway reveals the real paving.

![Native quest prop atlas](native-props.png)

Edda uses the existing native NPC drawing pipeline: grey hair, a plum robe, and
a book. Her transparent sprite preserves the Guild floor. The Stoneguard Shield
uses the existing Tower Shield silhouette with a moss-grey stone and gold
finish in both the equipped sprite and inventory icon. Existing shield visual
IDs remain unchanged.

Captures come from `test_crown_quest_renderer` in
[debug_shortcuts_test.c](../../../tools/tests/debug_shortcuts_test.c). They were
inspected beside the existing player, Guild residents, regional enemies,
coffers, and chamber walls at the same rendering scale. No buildings, enemies,
or world-map art were changed.

| View | Capture |
| --- | --- |
| Edda beside existing Guild residents and furnishings | [Guild](edda-guild.png) |
| Complete quest offer and Y/N acceptance | [Quest offer](edda-offer.png) |
| Inscription in the northwest village hall | [Royal inscription](royal-inscription.png) |
| Seal pedestal and separate ordinary stone coffer | [Seal chamber](seal-chamber.png) |
| Closed ward integrated with the royal chamber | [Warded vault](warded-vault.png) |
| Open passage, intact crown, and clear return route | [Open vault](open-vault.png) |
| Four completed objectives and equipment reward | [Quest journal](journal.png) |
| Stoneguard Shield equipped at the player's native scale | [Equipped shield](stoneguard-equipped.png) |
| Shield icon, equipment comparison, block and HP statistics | [Inventory](stoneguard-inventory.png) |

Reproduce the art with `.venv/bin/python tools/sprites/crown_quest.py`. Build and
run `build/debug-shortcuts/debug_shortcuts_test` from the project root to
regenerate `/tmp/crown-quest-*.bmp`. The harness keeps saves in an isolated
temporary directory. Quest, passage, reward, equipment, and save compatibility
checks are in [test_crown_quest.c](../../../tests/test_crown_quest.c).
