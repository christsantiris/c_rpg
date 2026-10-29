#ifndef EQUIPMENT_TEST_UTILS_H
#define EQUIPMENT_TEST_UTILS_H

#include "game/game.h"
#include "cJSON.h"

// Historical fixtures must put worn items back in the old bag representation
// before changing their version. Production load migration then moves them out.
static void test_legacy_equipment_json(cJSON *root) {
    cJSON *worn = cJSON_GetObjectItem(root, "equipment");
    cJSON *bag = cJSON_GetObjectItem(root, "inventory");
    const char *fields[] = {"equipped_main_hand", "equipped_off_hand", NULL, "equipped_armor", NULL, NULL};
    for (int i = 0; i < EQUIPMENT_SLOT_COUNT; i++) {
        cJSON *item = cJSON_GetObjectItem(cJSON_GetArrayItem(worn, i), "item");
        const cJSON *active = cJSON_GetObjectItem(item, "active");
        if (fields[i] && cJSON_IsNumber(active) && active->valueint) {
            cJSON_ReplaceItemInObject(root, fields[i], cJSON_CreateNumber(cJSON_GetArraySize(bag)));
            cJSON_AddItemToArray(bag, cJSON_Duplicate(item, 1));
        }
    }
    cJSON_ReplaceItemInObject(root, "inventory_count", cJSON_CreateNumber(cJSON_GetArraySize(bag)));
    cJSON_DeleteItemFromObject(root, "equipment");
    cJSON_DeleteItemFromObject(root, "jail_extra_slots");
}

#endif
