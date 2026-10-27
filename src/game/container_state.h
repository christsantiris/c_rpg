#ifndef CONTAINER_STATE_H
#define CONTAINER_STATE_H

#define MAX_LOOT_CONTAINERS 512

// World loot is independent of the active floor's limited dropped-item buffer.
// Opening, closing, travelling, or saving cannot recreate or discard its contents.
typedef struct {
    int location;
    int level;
    int x;
    int y;
    int kind;
    int opened;
    int released;
    int gold;
    int gold_score;
    int rewards;
    int variant;
    int loot_x;
    int loot_y;
} LootContainer;

#endif
