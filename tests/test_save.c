#include "app.h"
#include <assert.h>
#include <math.h>
#include <limits.h>
#include <share.h>
#include <stdio.h>
#include <string.h>

static void Setup(App *app, const char *path)
{
    *app = (App){ 0 };
    for (int i = 0; i < MAP_COUNT; i++) InitializeWorld(&app->worlds[i], (MapId)i);
    app->assets.supportsChinese = true; app->persistenceEnabled = true;
    snprintf(app->savePath, sizeof(app->savePath), "%s", path);
    StartNewGame(app);
}

static void UnlockBoss(App *app)
{
    assert(AdvanceQuest(&app->quest, &app->player));
    app->player.meleeDefeated = 4; app->player.rangedDefeated = 2; app->player.defeatedGoblins = 6;
    GrantPlayerRewards(&app->player, 150, 180);
    UpdateQuestProgress(&app->quest, &app->player);
    assert(AdvanceQuest(&app->quest, &app->player));
}

static bool SameFile(const char *left, const char *right)
{
    FILE *a = fopen(left, "rb"), *b = fopen(right, "rb"); assert(a && b);
    int x, y;
    do { x = fgetc(a); y = fgetc(b); } while (x == y && x != EOF);
    fclose(a); fclose(b);
    return x == y;
}

static void Corrupt(const char *source, const char *target, const char *find, const char *replacement)
{
    char text[16384];
    FILE *file = fopen(source, "rb"); assert(file);
    size_t size = fread(text, 1, sizeof(text) - 1, file); fclose(file); text[size] = '\0';
    char *match = strstr(text, find); assert(match);
    file = fopen(target, "wb"); assert(file);
    assert(fwrite(text, 1, (size_t)(match - text), file) == (size_t)(match - text));
    assert(fputs(replacement, file) >= 0 && fputs(match + strlen(find), file) >= 0);
    assert(fclose(file) == 0);
}

static void RejectFile(const char *path, const World worlds[MAP_COUNT])
{
    SaveData output = { 0 }; output.player.gold = 777;
    SaveData before = output;
    assert(ReadSave(path, worlds, &output) == SAVE_INVALID);
    assert(memcmp(&output, &before, sizeof(output)) == 0);
}

static void RoundTrip(App *app, const char *path, const char *copy)
{
    SaveData original = CaptureSave(app), loaded;
    assert(ValidateSave(&original, app->worlds));
    assert(WriteSave(path, app->worlds, &original) == SAVE_OK);
    assert(ReadSave(path, app->worlds, &loaded) == SAVE_OK && loaded.hasSnapshot);
    assert(WriteSave(copy, app->worlds, &loaded) == SAVE_OK && SameFile(path, copy));
}

int main(void)
{
    const char *path = "bin/test-save.txt", *copy = "bin/test-save-copy.txt", *bad = "bin/test-save-bad.txt";
    remove(path); remove(copy); remove(bad);
    App app;
    Setup(&app, path);
    SaveData loaded;
    assert(ReadSave(path, app.worlds, &loaded) == SAVE_MISSING);
    UnlockBoss(&app);
    assert(BuyInventoryItem(&app.player.inventory, &app.player.gold, ITEM_STEEL_SWORD) == INVENTORY_OK);
    assert(BuyInventoryItem(&app.player.inventory, &app.player.gold, ITEM_RUNE_STAFF) == INVENTORY_OK);
    assert(EquipInventoryItem(&app.player.inventory, 2, false) == INVENTORY_OK);
    assert(EquipInventoryItem(&app.player.inventory, 3, false) == INVENTORY_OK);
    app.player.weapon = WEAPON_STAFF;
    app.player.position = (Vector2){ 480.375f, 528.125f }; app.player.health = 93;
    app.player.hurtTimer = 0.3f; app.language = LANGUAGE_CHINESE;
    RoundTrip(&app, path, copy);
    SaveData base = CaptureSave(&app);
    /* Legal long-running progress must remain writable beyond the old kill cap. */
    SaveData rich = base;
    rich.player.meleeDefeated = 1000001;
    rich.player.rangedDefeated = 1000001;
    rich.player.defeatedGoblins = 2000002;
    rich.player.gold = PLAYER_GOLD_LIMIT - 1;
    GrantPlayerRewards(&rich.player, INT_MAX, 0);
    assert(rich.player.gold == PLAYER_GOLD_LIMIT);
    assert(WriteSave(copy, app.worlds, &rich) == SAVE_OK);
    assert(ReadSave(copy, app.worlds, &loaded) == SAVE_OK);
    assert(loaded.player.defeatedGoblins == 2000002 && loaded.player.gold == PLAYER_GOLD_LIMIT);
    rich.player.meleeDefeated = INT_MAX;
    rich.player.rangedDefeated = INT_MAX;
    assert(!ValidateSave(&rich, app.worlds));
    /* A valid pre-obstacle field snapshot keeps progress and actor health.
       Only overlapping actors move; a shot inside a new rock is absorbed. */
    EnterMap(&app, MAP_FIELD, app.worlds[MAP_FIELD].spawn);
    SaveData fieldSave = CaptureSave(&app);
    World previous[MAP_COUNT];
    memcpy(previous, app.worlds, sizeof(previous));
    previous[MAP_FIELD].sceneryCount = 0;
    fieldSave.player.position = (Vector2){144,232};
    fieldSave.combat.enemies[0].position = (Vector2){344,440};
    fieldSave.combat.enemies[0].health = 45;
    fieldSave.combat.projectiles[0] = (Projectile){ .active = true, .position = {344,432},
        .direction = {1,0}, .remainingDistance = 100, .damage = 20 };
    assert(WriteSave(bad, previous, &fieldSave) == SAVE_OK);
    assert(ReadSave(bad, app.worlds, &loaded) == SAVE_OK);
    assert(ValidateSave(&loaded, app.worlds));
    assert(loaded.player.gold == base.player.gold && loaded.player.health == base.player.health);
    assert(loaded.combat.enemies[0].health == 45 && !loaded.combat.projectiles[0].active);
    assert(loaded.combat.enemies[1].position.x == fieldSave.combat.enemies[1].position.x &&
        loaded.combat.enemies[1].position.y == fieldSave.combat.enemies[1].position.y);
    assert(WriteSave(copy, previous, &fieldSave) == SAVE_OK && SameFile(bad, copy));
    Corrupt(bad, copy, "location 1 144 232", "location 1 -10 232");
    RejectFile(copy, app.worlds);
    EnterMap(&app, MAP_TOWN, base.player.position);
    /* Old decorative town scenery now blocks: migrate a scene snapshot. */
    Corrupt(path, bad, "location 0 480.375 528.125", "location 0 192 208");
    assert(ReadSave(bad, app.worlds, &loaded) == SAVE_OK && loaded.hasSnapshot);
    assert(CanStandInWorld(&app.worlds[MAP_TOWN], GetPlayerBody(loaded.player.position)));
    assert(loaded.player.gold == base.player.gold && loaded.quest.stage == base.quest.stage);
    Corrupt(bad, copy, "location 0 192 208", "location 0 -10 208");
    RejectFile(copy, app.worlds);
    ContinueGame(&app);
    assert(app.player.position.x == base.player.position.x && app.player.position.y == base.player.position.y);
    assert(app.player.health == 93 && app.player.hurtTimer == 0.3f && GetPlayerAttack(&app.player) == 40);
    assert(app.player.weapon == WEAPON_STAFF && app.language == LANGUAGE_CHINESE);
    for (int stage = QUEST_OFFER; stage <= QUEST_COMPLETE; stage++) {
        SaveData variant = base; variant.quest.stage = (QuestStage)stage;
        variant.quest.bossDefeated = stage >= QUEST_REPORT_BOSS;
        if (stage == QUEST_CLEAR_FIELD) { variant.quest.startingMelee = 4; variant.quest.startingRanged = 2; }
        assert(WriteSave(path, app.worlds, &variant) == SAVE_OK);
        assert(ReadSave(path, app.worlds, &loaded) == SAVE_OK && loaded.quest.stage == (QuestStage)stage);
    }
    for (int map = 0; map < MAP_COUNT; map++) for (int language = 0; language < LANGUAGE_COUNT; language++) {
        EnterMap(&app, (MapId)map, app.worlds[map].spawn); app.language = (Language)language;
        RoundTrip(&app, path, copy);
    }
    EnterMap(&app, MAP_TOWN, app.worlds[MAP_TOWN].spawn);
    const AppScreen panels[] = { SCREEN_PAUSE, SCREEN_INVENTORY, SCREEN_SHOP, SCREEN_QUEST, SCREEN_INTERACTION };
    for (int i = 0; i < 5; i++) {
        app.screen = panels[i]; app.selectedSlot = 3; app.selectedProduct = 1;
        app.dialoguePage = i == 3 ? 1 : 0; app.nearbyInteraction = 2; app.interactionText = TEXT_HEALED;
        RoundTrip(&app, path, copy); ContinueGame(&app);
        assert(app.screen == panels[i] && app.selectedSlot == 3 && app.selectedProduct == 1);
        assert(app.dialoguePage == (i == 3 ? 1 : 0));
    }
    app.dialoguePage = 0;
    /* A real shot and a dead enemy survive closing; no safe-area teleport. */
    EnterMap(&app, MAP_FIELD, (Vector2){ 280, 656 });
    app.player.staffFacing = (Vector2){ 1, 0 };
    UpdateCombat(&app.combat, &app.player, &app.worlds[MAP_FIELD], 0.05f, true);
    app.combat.enemies[1].health = 0; app.combat.enemies[1].state = ENEMY_DEAD;
    ClaimEnemyRewards(&app.combat, &app.player); UpdateAppQuest(&app);
    assert(app.combat.inCombat && app.combat.projectiles[0].active && app.combat.cooldown > 0);
    RoundTrip(&app, path, copy);
    float shotX = app.combat.projectiles[0].position.x, cooldown = app.combat.cooldown;
    ManualSave(&app); assert(app.saveMessage == TEXT_SAVE_BUSY);
    app.screen = SCREEN_PAUSE; RequestAppExit(&app); assert(app.exitRequested); ContinueGame(&app);
    assert(!app.exitRequested && app.screen == SCREEN_PAUSE && app.currentMap == MAP_FIELD && app.player.position.x == 280);
    assert(app.combat.inCombat && app.combat.cooldown == cooldown && app.combat.projectiles[0].position.x == shotX);
    assert(app.combat.enemies[1].health == 0 && app.combat.enemies[1].rewardGranted);
    int gold = app.player.gold;
    assert(ClaimEnemyRewards(&app.combat, &app.player) == 0 && app.player.gold == gold);
    /* Loading and then updating must match uninterrupted execution. */
    App uninterrupted = app;
    UpdateCombat(&app.combat, &app.player, &app.worlds[MAP_FIELD], 0.05f, false);
    UpdateCombat(&uninterrupted.combat, &uninterrupted.player, &uninterrupted.worlds[MAP_FIELD], 0.05f, false);
    SaveData resumed = CaptureSave(&app), continuous = CaptureSave(&uninterrupted);
    assert(WriteSave(path, app.worlds, &resumed) == SAVE_OK);
    assert(WriteSave(copy, app.worlds, &continuous) == SAVE_OK && SameFile(path, copy));
    for (int frame = 0; frame < 30 && app.player.health > 0; frame++) {
        UpdateCombat(&app.combat, &app.player, &app.worlds[MAP_FIELD], 0.05f, false);
        UpdateCombat(&uninterrupted.combat, &uninterrupted.player, &uninterrupted.worlds[MAP_FIELD], 0.05f, false);
        UpdateAppQuest(&app); UpdateAppQuest(&uninterrupted);
        if (app.player.health == 0) app.screen = uninterrupted.screen = SCREEN_DEATH;
        resumed = CaptureSave(&app); continuous = CaptureSave(&uninterrupted);
        assert(WriteSave(path, app.worlds, &resumed) == SAVE_OK);
        assert(WriteSave(copy, app.worlds, &continuous) == SAVE_OK && SameFile(path, copy));
    }
    /* Rebuild this controlled scenario for the single-swing regression below. */
    EnterMap(&app, MAP_FIELD, (Vector2){ 280, 656 });
    app.player.health = app.player.maxHealth;
    /* Preserve the sword's already-hit set, hostile shots and warnings. */
    app.player.weapon = WEAPON_SWORD; app.combat.cooldown = 0;
    UpdateCombat(&app.combat, &app.player, &app.worlds[MAP_FIELD], 0.01f, true);
    assert(app.combat.swingTimer > 0 && app.combat.swingHit[0]);
    Enemy *ranged = &app.combat.enemies[4];
    ranged->attackDirection = (Vector2){ -1, 0 }; ranged->state = ENEMY_WINDUP;
    ranged->windup = 0.25f; ranged->cooldown = 0.8f; FireEnemyProjectile(&app.combat, ranged);
    RoundTrip(&app, path, copy); ContinueGame(&app);
    assert(app.combat.swingHit[0] && app.combat.enemies[4].windup == 0.25f);
    bool hostile = false;
    for (int i = 0; i < PROJECTILE_CAPACITY; i++) hostile = hostile || (app.combat.projectiles[i].active && app.combat.projectiles[i].hostile);
    assert(hostile);
    int enemyHealth = app.combat.enemies[0].health;
    UpdateCombat(&app.combat, &app.player, &app.worlds[MAP_FIELD], 0.01f, false);
    assert(app.combat.enemies[0].health == enemyHealth);
    for (int town = 0; town <= 1; town++) {
        app.player.health = 0; app.screen = SCREEN_DEATH; app.deathReturnToTown = town != 0;
        RoundTrip(&app, path, copy); RequestAppExit(&app); ContinueGame(&app);
        assert(app.screen == SCREEN_DEATH && app.player.health == 0 && app.currentMap == MAP_FIELD);
        assert(app.deathReturnToTown == (town != 0));
    }
    RespawnAppPlayer(&app, false);
    assert(app.player.health == app.player.maxHealth && app.combat.enemies[1].health == 60);
    app.combat.enemies[0].health = 0; app.combat.enemies[0].state = ENEMY_DEAD;
    ClaimEnemyRewards(&app.combat, &app.player);
    EnterMap(&app, MAP_TOWN, app.worlds[MAP_TOWN].spawn); EnterMap(&app, MAP_FIELD, app.worlds[MAP_FIELD].spawn);
    assert(app.combat.enemies[0].health == 60);
    /* Validate real runtime snapshots over multiple AI and attack cycles. */
    EnterMap(&app, MAP_FIELD, (Vector2){ 280, 656 });
    for (int frame = 0; frame < 250; frame++) {
        UpdateCombat(&app.combat, &app.player, &app.worlds[MAP_FIELD], 0.05f, frame % 3 == 0); UpdateAppQuest(&app);
        if (app.player.health == 0) app.screen = SCREEN_DEATH;
        SaveData frameData = CaptureSave(&app); assert(ValidateSave(&frameData, app.worlds));
        if (app.player.health == 0) RespawnAppPlayer(&app, false);
    }
    EnterMap(&app, MAP_BOSS, (Vector2){ 468, 400 }); app.combat.enemies[0].cooldown = 0;
    UpdateCombat(&app.combat, &app.player, &app.worlds[MAP_BOSS], 0.01f, false);
    RoundTrip(&app, path, copy); assert(app.combat.enemies[0].state == ENEMY_WINDUP);
    app.combat.enemies[0].health = 0; app.combat.enemies[0].state = ENEMY_DEAD;
    ClaimEnemyRewards(&app.combat, &app.player); UpdateAppQuest(&app);
    RoundTrip(&app, path, copy); ContinueGame(&app);
    assert(app.screen == SCREEN_BOSS_VICTORY && app.combat.enemies[0].health == 0);
    EnterMap(&app, MAP_BOSS, app.worlds[MAP_BOSS].spawn);
    assert(app.combat.enemyCount == 1 && app.combat.enemies[0].health == 300 && app.quest.bossDefeated);
    RoundTrip(&app, path, copy); ContinueGame(&app);
    assert(app.combat.enemies[0].health == 300 && app.quest.stage == QUEST_REPORT_BOSS);
    EnterMap(&app, MAP_TOWN, app.worlds[MAP_TOWN].spawn);
    assert(AdvanceQuest(&app.quest, &app.player)); app.screen = SCREEN_COMPLETE;
    RoundTrip(&app, path, copy); ContinueGame(&app); assert(app.screen == SCREEN_COMPLETE);
    /* Version 1 preserves progress, rebuilding the current map. */
    SaveData legacy = base; legacy.hasSnapshot = false;
    assert(WriteSave(path, app.worlds, &legacy) == SAVE_OK);
    assert(ReadSave(path, app.worlds, &loaded) == SAVE_OK && !loaded.hasSnapshot);
    ContinueGame(&app); assert(app.screen == SCREEN_PLAY && app.player.gold == base.player.gold);
    Corrupt(path, bad, "480.375", "480"); Corrupt(bad, copy, "528.125", "624");
    assert(ReadSave(copy, app.worlds, &loaded) == SAVE_OK && loaded.player.position.y != 624);
    assert(CanStandInWorld(&app.worlds[MAP_TOWN], GetPlayerBody(loaded.player.position)));
    assert(WriteSave(path, app.worlds, &base) == SAVE_OK);
    SaveData invalid = base;
    invalid.player.position.x = NAN; assert(WriteSave(path, app.worlds, &invalid) == SAVE_INVALID);
    invalid = base; invalid.player.inventory.equippedSword = 99; assert(!ValidateSave(&invalid, app.worlds));
    invalid = base; invalid.combat.enemyCount = 999; assert(!ValidateSave(&invalid, app.worlds));
    invalid = base; invalid.inventoryMessage = TEXT_HP; assert(!ValidateSave(&invalid, app.worlds));
    invalid = base; invalid.player.health = 0; assert(!ValidateSave(&invalid, app.worlds));
    invalid = base; invalid.player.position = app.worlds[MAP_TOWN].interactions[2].position; assert(!ValidateSave(&invalid, app.worlds));
    const char *find[] = { "PROJECT_LAX 2", "motion ", "combat 0", "shot 0 0", "end_snapshot", "snapshot 1" };
    const char *replace[] = { "PROJECT_LAX 99", "motion nan ", "combat 99999999999999999999999", "shot 0 9", "INCOMPLETE", "snapshot 4" };
    for (int i = 0; i < 6; i++) { Corrupt(path, bad, find[i], replace[i]); RejectFile(bad, app.worlds); }
    Corrupt(path, bad, "END\n", "END\nTRAILING_DATA"); RejectFile(bad, app.worlds);
    snprintf(app.savePath, sizeof(app.savePath), "%s", bad);
    Player before = app.player; Combat combatBefore = app.combat;
    ContinueGame(&app);
    assert(app.saveMessage == TEXT_SAVE_BAD && memcmp(&before, &app.player, sizeof(before)) == 0);
    assert(memcmp(&combatBefore, &app.combat, sizeof(combatBefore)) == 0);
    app.sessionStarted = false; app.exitRequested = false; RequestAppExit(&app); assert(app.exitRequested); RejectFile(bad, app.worlds);
    FILE *lock = _fsopen(path, "rb", _SH_DENYRW); assert(lock);
    App failedExit; Setup(&failedExit, path); failedExit.player.gold = 999; failedExit.screen = SCREEN_PAUSE;
    RequestAppExit(&failedExit); assert(!failedExit.exitRequested && failedExit.screen == SCREEN_SAVE_ERROR);
    fclose(lock);
    assert(ReadSave(path, app.worlds, &loaded) == SAVE_OK && loaded.player.gold == base.player.gold);
    RequestAppExit(&failedExit); assert(failedExit.exitRequested);
    ContinueGame(&failedExit); assert(failedExit.screen == SCREEN_PAUSE && failedExit.player.gold == 999);
    assert(!FileExists("bin/test-save.txt.tmp"));
    remove(path); remove(copy); remove(bad);
    puts("PASS: exact scene snapshots, shots/cooldowns/hit flags, AI/loot persistence, UI/death restore, map refresh, v1 migration, corruption rejection and atomic replacement");
    return 0;
}
