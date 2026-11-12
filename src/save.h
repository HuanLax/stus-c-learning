#ifndef PROJECT_LAX_SAVE_H
#define PROJECT_LAX_SAVE_H

#include "quest.h"
#include "combat.h"

typedef enum AppScreen { SCREEN_MENU, SCREEN_PLAY, SCREEN_PAUSE, SCREEN_INTERACTION,
    SCREEN_DEATH, SCREEN_INVENTORY, SCREEN_SHOP, SCREEN_QUEST, SCREEN_BOSS_VICTORY,
    SCREEN_COMPLETE, SCREEN_NEW_CONFIRM, SCREEN_SAVE_ERROR } AppScreen;

typedef enum SaveResult { SAVE_OK, SAVE_MISSING, SAVE_INVALID, SAVE_IO_ERROR } SaveResult;

/* Version 2 keeps the current scene, but never pointers or GPU resources. */
typedef struct SaveData {
    Player player;
    Quest quest;
    MapId map;
    Language language;
    MapId safeMap;
    Vector2 safePosition;
    bool hasSnapshot; /* Version 1 files contain character progress only. */
    Combat combat;
    AppScreen screen;
    int dialoguePage;
    TextId interactionText;
    bool deathReturnToTown;
    int nearbyInteraction;
    int selectedSlot;
    int selectedProduct;
    TextId inventoryMessage;
    float rewardNoticeTimer;
    int rewardNoticeKills;
} SaveData;

bool ValidateSave(const SaveData *data, const World worlds[MAP_COUNT]);
SaveResult ReadSave(const char *path, const World worlds[MAP_COUNT], SaveData *output);
SaveResult WriteSave(const char *path, const World worlds[MAP_COUNT], const SaveData *data);

#endif
