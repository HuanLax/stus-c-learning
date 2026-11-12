#include "app.h"
#include "ui.h"

#include <stdio.h>
#include <math.h>

void InitializeApp(App *app)
{
    *app = (App){ .language = LANGUAGE_ENGLISH, .screen = SCREEN_MENU, .nearbyInteraction = -1,
        .currentMap = MAP_TOWN, .lastSafeMap = MAP_TOWN, .inventoryMessage = TEXT_BAG_HELP,
        .interactionText = TEXT_QUEST_PREVIEW, .persistenceEnabled = true, .deathReturnToTown = true };
    InitializeQuest(&app->quest);
    for (int map = 0; map < MAP_COUNT; map++) InitializeWorld(&app->worlds[map], (MapId)map);
    InitializePlayer(&app->player, app->worlds[app->currentMap].spawn);
    InitializeCombat(&app->combat, MAP_TOWN);
    app->lastSafePosition = app->player.position;
    app->camera = GetWorldCamera(&app->worlds[app->currentMap], app->player.position, 1024, 768);
    LoadUiAssets(&app->assets);
    SetAppLanguage(app, LANGUAGE_CHINESE);
    int length = snprintf(app->savePath, sizeof(app->savePath), "%s../saves/save.txt", GetApplicationDirectory());
    SaveData probe;
    SaveResult result = length < 0 || (size_t)length >= sizeof(app->savePath) ? SAVE_IO_ERROR :
                        ReadSave(app->savePath, app->worlds, &probe);
    app->saveAvailable = result == SAVE_OK;
    app->saveExists = result != SAVE_MISSING;
    app->saveMessage = result == SAVE_OK ? TEXT_SAVE_FOUND : result == SAVE_MISSING ? TEXT_SAVE_NONE : TEXT_SAVE_BAD;
}

void SetAppLanguage(App *app, Language language)
{
    if (language < LANGUAGE_ENGLISH || language >= LANGUAGE_COUNT) return;
    if (language == LANGUAGE_CHINESE && !app->assets.supportsChinese) return;
    app->language = language;
}

void UpdateApp(App *app)
{
    if (IsKeyPressed(KEY_F2)) {
        SetAppLanguage(app, app->language == LANGUAGE_ENGLISH ?
                            LANGUAGE_CHINESE : LANGUAGE_ENGLISH);
    }
    AppScreen previousScreen = app->screen;
    MapId previousMap = app->currentMap;
    UpdateUi(app);
    if (app->sessionStarted && IsKeyPressed(KEY_F5) && app->screen != SCREEN_NEW_CONFIRM && app->screen != SCREEN_SAVE_ERROR)
        ManualSave(app);
    bool attackHeld = IsKeyDown(KEY_J) || (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !IsPointerOverHud(app));
    bool attackIntent = GetAttackIntent(&app->attackInputArmed, attackHeld,
        app->screen == SCREEN_PLAY && previousScreen == SCREEN_PLAY && previousMap == app->currentMap);
    if (previousScreen == SCREEN_PLAY && app->screen == SCREEN_PLAY && previousMap == app->currentMap) {
        Vector2 input = {
            (float)((IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) -
                    (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))),
            (float)((IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) -
                    (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)))
        };
        float seconds = GetFrameTime();
        /* Do not catch up a long focus/debug pause with a huge movement jump. */
        if (seconds > 0.05f) seconds = 0.05f;
        Rectangle obstacles[ENEMY_CAPACITY];
        int count = 0;
        for (int i = 0; i < app->combat.enemyCount; i++)
            if (app->combat.enemies[i].health > 0) obstacles[count++] = GetEnemyCollision(&app->combat.enemies[i]);
        MovePlayerWithObstacles(&app->player, &app->worlds[app->currentMap], input, seconds, obstacles, count);
        if (IsKeyPressed(KEY_Q)) app->player.weapon = app->player.weapon == WEAPON_SWORD ? WEAPON_STAFF : WEAPON_SWORD;
        int previousKills = app->player.defeatedGoblins;
        app->rewardNoticeTimer = fmaxf(0, app->rewardNoticeTimer - seconds);
        UpdateCombat(&app->combat, &app->player, &app->worlds[app->currentMap], seconds,
                      attackIntent);
        if (app->player.defeatedGoblins > previousKills) {
            app->rewardNoticeKills = app->player.defeatedGoblins - previousKills;
            app->rewardNoticeTimer = 2.5f;
        }
        UpdateAppQuest(app);
        if (app->player.health <= 0) {
            app->screen = SCREEN_DEATH;
            app->deathReturnToTown = true;
        }
        app->nearbyInteraction = FindWorldInteraction(&app->worlds[app->currentMap], app->player.position);
    }
    if (IsBodyInSafeArea(&app->worlds[app->currentMap], GetPlayerBody(app->player.position))) {
        app->lastSafeMap = app->currentMap;
        app->lastSafePosition = app->worlds[app->currentMap].spawn;
    }
    app->camera = GetWorldCamera(&app->worlds[app->currentMap], app->player.position, GAME_WIDTH, GAME_HEIGHT);
}

void EnterMap(App *app, MapId map, Vector2 position)
{
    if (map < MAP_TOWN || map >= MAP_COUNT || !CanStandInWorld(&app->worlds[map], GetPlayerBody(position))) return;
    app->currentMap = map;
    app->player.position = position;
    InitializeCombat(&app->combat, map);
    app->attackInputArmed = false;
    app->nearbyInteraction = -1; /* do not reuse an index from the old map */
    app->screen = SCREEN_PLAY;
    app->camera = GetWorldCamera(&app->worlds[map], position, GAME_WIDTH, GAME_HEIGHT);
    if (IsBodyInSafeArea(&app->worlds[map], GetPlayerBody(position))) {
        app->lastSafeMap = map;
        app->lastSafePosition = app->worlds[map].spawn;
    }
}

void ActivateNearbyInteraction(App *app)
{
    const World *world = &app->worlds[app->currentMap];
    int index = app->nearbyInteraction;
    if (index < 0 || index >= world->interactionCount) return;
    Interaction object = world->interactions[index];
    if (object.kind == INTERACTION_QUEST) {
        UpdateQuestProgress(&app->quest, &app->player);
        app->dialoguePage = 0;
        app->screen = SCREEN_QUEST;
        return;
    }
    if (object.kind == INTERACTION_SHOP) {
        app->screen = SCREEN_SHOP;
        app->selectedProduct = 0;
        app->inventoryMessage = TEXT_SHOP_HELP;
        return;
    }
    const TextId descriptions[] = { TEXT_QUEST_PREVIEW, TEXT_SHOP_PREVIEW, TEXT_HEAL_PREVIEW, TEXT_EXIT_ERROR };
    app->interactionText = descriptions[object.kind];
    if (object.kind == INTERACTION_HEAL && IsBodyInSafeArea(world, GetPlayerBody(app->player.position))) {
        app->player.health = app->player.maxHealth;
        app->interactionText = TEXT_HEALED;
    }
    if (object.kind == INTERACTION_EXIT) {
        MapId destination;
        Vector2 arrival;
        if (TryMapExit(app->worlds, app->currentMap, index, IsBossUnlocked(&app->quest), &destination, &arrival)) {
            EnterMap(app, destination, arrival);
            return;
        }
        if (object.destination == MAP_BOSS && !IsBossUnlocked(&app->quest)) app->interactionText = TEXT_GATE_LOCKED;
    }
    app->screen = SCREEN_INTERACTION;
}

void RespawnAppPlayer(App *app, bool returnToTown)
{
    MapId map = returnToTown ? MAP_TOWN : app->currentMap;
    EnterMap(app, map, app->worlds[map].spawn);
    RevivePlayer(&app->player);
}

void UpdateAppQuest(App *app)
{
    UpdateQuestProgress(&app->quest, &app->player);
    /* Only the first kill advances the story and opens its victory panel.
       Refreshed bosses still pay loot through ClaimEnemyRewards. */
    if (app->currentMap != MAP_BOSS || app->quest.bossDefeated) return;
    for (int i = 0; i < app->combat.enemyCount; i++) {
        if (app->combat.enemies[i].kind == ENEMY_BOSS && app->combat.enemies[i].health <= 0) {
            app->quest.bossDefeated = true;
            app->combat.inCombat = false;
            app->combat.lostTimer = 0;
            app->combat.swingTimer = 0;
            for (int shot = 0; shot < PROJECTILE_CAPACITY; shot++) app->combat.projectiles[shot].active = false;
            UpdateQuestProgress(&app->quest, &app->player);
            if (app->player.health > 0) app->screen = SCREEN_BOSS_VICTORY;
            return;
        }
    }
}

void ConfirmQuestDialogue(App *app)
{
    if (app->dialoguePage + 1 < QuestDialoguePages(&app->quest)) { app->dialoguePage++; return; }
    QuestStage before = app->quest.stage;
    if (AdvanceQuest(&app->quest, &app->player)) {
        app->dialoguePage = 0;
        app->screen = app->quest.stage == QUEST_COMPLETE ? SCREEN_COMPLETE :
                      before == QUEST_OFFER ? SCREEN_PLAY : SCREEN_QUEST;
    } else app->screen = SCREEN_PLAY;
}

void StartNewGame(App *app)
{
    InitializePlayer(&app->player, app->worlds[MAP_TOWN].spawn);
    InitializeQuest(&app->quest);
    app->lastSafeMap = MAP_TOWN;
    app->lastSafePosition = app->worlds[MAP_TOWN].spawn;
    app->sessionStarted = true;
    app->dialoguePage = 0;
    app->rewardNoticeTimer = 0;
    app->rewardNoticeKills = 0;
    app->selectedSlot = 0;
    app->selectedProduct = 0;
    app->inventoryMessage = TEXT_BAG_HELP;
    app->interactionText = TEXT_QUEST_PREVIEW;
    app->saveMessage = TEXT_SAVE_NEW;
    EnterMap(app, MAP_TOWN, app->worlds[MAP_TOWN].spawn);
}

void ContinueGame(App *app)
{
    SaveData data;
    SaveResult result = ReadSave(app->savePath, app->worlds, &data);
    if (result != SAVE_OK) {
        app->saveAvailable = false;
        app->saveExists = result != SAVE_MISSING;
        app->saveMessage = result == SAVE_MISSING ? TEXT_SAVE_NONE : TEXT_SAVE_BAD;
        return;
    }
    app->player = data.player;
    app->quest = data.quest;
    app->lastSafeMap = data.safeMap;
    app->lastSafePosition = data.safePosition;
    SetAppLanguage(app, data.language);
    app->sessionStarted = true;
    app->rewardNoticeTimer = 0;
    app->dialoguePage = 0;
    app->saveMessage = TEXT_SAVE_LOADED;
    EnterMap(app, data.map, data.player.position);
    if (data.hasSnapshot) {
        app->combat = data.combat;
        app->screen = data.screen;
        app->dialoguePage = data.dialoguePage;
        app->interactionText = data.interactionText;
        app->deathReturnToTown = data.deathReturnToTown;
        app->nearbyInteraction = data.nearbyInteraction;
        app->selectedSlot = data.selectedSlot;
        app->selectedProduct = data.selectedProduct;
        app->inventoryMessage = data.inventoryMessage;
        app->rewardNoticeTimer = data.rewardNoticeTimer;
        app->rewardNoticeKills = data.rewardNoticeKills;
    }
    app->exitRequested = false;
}

SaveData CaptureSave(const App *app)
{
    SaveData data = { .player = app->player, .quest = app->quest, .map = app->currentMap,
        .language = app->language, .safeMap = app->lastSafeMap, .safePosition = app->lastSafePosition,
        .hasSnapshot = true, .combat = app->combat,
        .screen = app->screen == SCREEN_SAVE_ERROR ? app->beforeSaveError : app->screen,
        .dialoguePage = app->dialoguePage, .interactionText = app->interactionText,
        .deathReturnToTown = app->deathReturnToTown, .nearbyInteraction = app->nearbyInteraction,
        .selectedSlot = app->selectedSlot, .selectedProduct = app->selectedProduct,
        .inventoryMessage = app->inventoryMessage, .rewardNoticeTimer = fmaxf(0, app->rewardNoticeTimer),
        .rewardNoticeKills = app->rewardNoticeKills };
    return data;
}

static SaveResult SaveAppProgress(App *app)
{
    /* The folder follows the executable; unrelated terminal working directories are harmless. */
    const char *directory = GetDirectoryPath(app->savePath);
    if (!DirectoryExists(directory) && MakeDirectory(directory) != 0) return SAVE_IO_ERROR;
    SaveData data = CaptureSave(app);
    SaveResult result = WriteSave(app->savePath, app->worlds, &data);
    if (result == SAVE_OK) { app->saveAvailable = true; app->saveExists = true; }
    return result;
}

void ManualSave(App *app)
{
    if (!app->persistenceEnabled) { app->saveMessage = TEXT_SAVE_PREVIEW; return; }
    if (!app->sessionStarted || app->player.health <= 0 || app->combat.inCombat) {
        app->saveMessage = TEXT_SAVE_BUSY;
        return;
    }
    app->saveMessage = SaveAppProgress(app) == SAVE_OK ? TEXT_SAVE_OK : TEXT_SAVE_FAILED;
}

void RequestAppExit(App *app)
{
    if (!app->sessionStarted || !app->persistenceEnabled) { app->exitRequested = true; return; }
    if (SaveAppProgress(app) == SAVE_OK) { app->exitRequested = true; return; }
    if (app->screen != SCREEN_SAVE_ERROR) app->beforeSaveError = app->screen;
    app->saveMessage = TEXT_SAVE_FAILED;
    app->screen = SCREEN_SAVE_ERROR;
}

static TextId InventoryResultMessage(InventoryResult result)
{
    const TextId messages[] = { TEXT_EQUIP_SUCCESS, TEXT_EQUIP_BUSY, TEXT_BAG_FULL,
        TEXT_GOLD_SHORT, TEXT_ALREADY_BOUGHT, TEXT_EMPTY_SLOT, TEXT_ALREADY_EQUIPPED, TEXT_ITEM_INVALID };
    return messages[result];
}

void EquipSelectedItem(App *app)
{
    InventoryResult result = EquipInventoryItem(&app->player.inventory, app->selectedSlot, app->combat.inCombat);
    app->inventoryMessage = InventoryResultMessage(result);
}

void BuySelectedProduct(App *app)
{
    if (app->selectedProduct < 0 || app->selectedProduct > 1) { app->inventoryMessage = TEXT_ITEM_INVALID; return; }
    if (app->combat.inCombat) { app->inventoryMessage = TEXT_EQUIP_BUSY; return; }
    ItemId item = app->selectedProduct == 0 ? ITEM_STEEL_SWORD : ITEM_RUNE_STAFF;
    InventoryResult result = BuyInventoryItem(&app->player.inventory, &app->player.gold, item);
    app->inventoryMessage = result == INVENTORY_OK ? TEXT_BUY_SUCCESS : InventoryResultMessage(result);
}

void UnloadApp(App *app)
{
    UnloadUiAssets(&app->assets);
}
