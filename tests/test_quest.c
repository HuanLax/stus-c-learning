#include "app.h"
#include "boss.h"
#include "enemy_ai.h"

#include <assert.h>
#include <stdio.h>

static void Setup(App *app)
{
    *app = (App){ 0 };
    for (int map = 0; map < MAP_COUNT; map++) InitializeWorld(&app->worlds[map], (MapId)map);
    InitializePlayer(&app->player, app->worlds[MAP_TOWN].spawn);
    InitializeQuest(&app->quest);
    EnterMap(app, MAP_TOWN, app->worlds[MAP_TOWN].spawn);
}

int main(void)
{
    App app;
    Setup(&app);
    app.player.meleeDefeated = 4;
    app.player.rangedDefeated = 2;
    assert(AdvanceQuest(&app.quest, &app.player));
    UpdateQuestProgress(&app.quest, &app.player);
    assert(app.quest.stage == QUEST_CLEAR_FIELD);
    assert(QuestMeleeProgress(&app.quest, &app.player) == 0 && QuestRangedProgress(&app.quest, &app.player) == 0);
    assert(!AdvanceQuest(&app.quest, &app.player));

    Setup(&app);
    app.nearbyInteraction = 0;
    ActivateNearbyInteraction(&app);
    assert(app.screen == SCREEN_QUEST && app.quest.stage == QUEST_OFFER);
    ConfirmQuestDialogue(&app);
    ConfirmQuestDialogue(&app);
    assert(app.dialoguePage == 2 && app.quest.stage == QUEST_OFFER);
    ConfirmQuestDialogue(&app);
    assert(app.quest.stage == QUEST_CLEAR_FIELD && app.screen == SCREEN_PLAY);
    MapId destination = MAP_TOWN;
    Vector2 arrival = { 0 };
    assert(!TryMapExit(app.worlds, MAP_FIELD, 1, IsBossUnlocked(&app.quest), &destination, &arrival));
    EnterMap(&app, MAP_FIELD, app.worlds[MAP_FIELD].spawn);
    for (int i = 0; i < app.combat.enemyCount; i++) app.combat.enemies[i].health = 0;
    assert(ClaimEnemyRewards(&app.combat, &app.player) == 6);
    UpdateAppQuest(&app);
    assert(app.quest.stage == QUEST_REPORT_FIELD && app.player.gold == 150);
    assert(QuestMeleeProgress(&app.quest, &app.player) == 4 && QuestRangedProgress(&app.quest, &app.player) == 2);
    app.player.health = 0;
    RespawnAppPlayer(&app, true);
    assert(app.quest.stage == QUEST_REPORT_FIELD && app.player.gold == 150 && app.player.health == 140);
    app.nearbyInteraction = 0;
    ActivateNearbyInteraction(&app);
    ConfirmQuestDialogue(&app);
    assert(app.quest.stage == QUEST_REPORT_FIELD && app.player.gold == 150);
    ConfirmQuestDialogue(&app);
    assert(IsBossUnlocked(&app.quest) && app.quest.stage == QUEST_HUNT_BOSS && app.player.gold == 200);
    assert(app.player.level == 3 && app.player.experience == 80);
    int paid = app.player.gold;
    ConfirmQuestDialogue(&app);
    ConfirmQuestDialogue(&app);
    assert(app.player.gold == paid);
    assert(TryMapExit(app.worlds, MAP_FIELD, 1, IsBossUnlocked(&app.quest), &destination, &arrival));
    assert(destination == MAP_BOSS);
    BuyInventoryItem(&app.player.inventory, &app.player.gold, ITEM_STEEL_SWORD);
    EquipInventoryItem(&app.player.inventory, 2, false);
    assert(GetPlayerAttack(&app.player) == 40);
    EnterMap(&app, destination, arrival);
    Enemy *boss = &app.combat.enemies[0];
    assert(boss->kind == ENEMY_BOSS && CanStandInWorld(&app.worlds[MAP_BOSS], GetEnemyCollision(boss)));

    /* Run the real battle pipeline with an upgraded sword; no fake boss damage. */
    app.player.position = (Vector2){ boss->position.x - 60, boss->position.y };
    app.player.swordFacing = (Vector2){ 1, 0 };
    for (int frame = 0; frame < 300 && boss->health > 0 && app.player.health > 0; frame++) {
        UpdateCombat(&app.combat, &app.player, &app.worlds[MAP_BOSS], 0.05f, true);
        UpdateAppQuest(&app);
    }
    assert(boss->health == 0 && app.player.health > 0 && app.quest.bossDefeated);
    assert(app.quest.stage == QUEST_REPORT_BOSS && app.screen == SCREEN_BOSS_VICTORY);
    assert(app.player.defeatedGoblins == 6 && app.player.gold == 275);
    assert(!app.combat.inCombat && ClaimEnemyRewards(&app.combat, &app.player) == 0);
    EnterMap(&app, MAP_TOWN, app.worlds[MAP_TOWN].spawn);
    app.nearbyInteraction = 0;
    ActivateNearbyInteraction(&app);
    ConfirmQuestDialogue(&app);
    ConfirmQuestDialogue(&app);
    assert(app.quest.stage == QUEST_COMPLETE && app.screen == SCREEN_COMPLETE && app.player.gold == 375);
    paid = app.player.gold;
    app.nearbyInteraction = 0;
    ActivateNearbyInteraction(&app);
    ConfirmQuestDialogue(&app);
    assert(app.player.gold == paid);
    EnterMap(&app, MAP_BOSS, app.worlds[MAP_BOSS].spawn);
    assert(app.combat.enemyCount == 1 && app.combat.enemies[0].health == 300);
    app.combat.enemies[0].health = 0;
    app.combat.enemies[0].state = ENEMY_DEAD;
    ClaimEnemyRewards(&app.combat, &app.player);
    UpdateAppQuest(&app);
    assert(app.quest.stage == QUEST_COMPLETE && app.player.gold == paid + 150);
    ClaimEnemyRewards(&app.combat, &app.player);
    assert(app.player.gold == paid + 150); /* One loot reward per spawned boss, no repeated story reward. */

    Setup(&app);
    app.quest.stage = QUEST_HUNT_BOSS;
    EnterMap(&app, MAP_BOSS, (Vector2){ 468, 400 });
    boss = &app.combat.enemies[0];
    boss->cooldown = 0;
    UpdateCombat(&app.combat, &app.player, &app.worlds[MAP_BOSS], 0.01f, false);
    assert(boss->state == ENEMY_WINDUP && boss->attackVariant == 0);
    assert(BossSlashHits(boss, GetPlayerBody(app.player.position)));
    app.player.position = (Vector2){ 620, 400 };
    assert(!BossSlashHits(boss, GetPlayerBody(app.player.position)));
    UpdateCombat(&app.combat, &app.player, &app.worlds[MAP_BOSS], 0.9f, false);
    assert(app.player.health == 100);
    boss->cooldown = 0;
    app.player.position = (Vector2){ 468, 400 };
    UpdateCombat(&app.combat, &app.player, &app.worlds[MAP_BOSS], 0.01f, false);
    assert(boss->attackVariant == 1 && boss->state == ENEMY_WINDUP);
    Vector2 locked = boss->attackDirection;
    app.player.position.y += 100;
    UpdateCombat(&app.combat, &app.player, &app.worlds[MAP_BOSS], 1.1f, false);
    int shots = 0;
    for (int i = 0; i < PROJECTILE_CAPACITY; i++) if (app.combat.projectiles[i].active && app.combat.projectiles[i].hostile) {
        shots++;
        assert(app.combat.projectiles[i].position.x == boss->position.x);
        assert(app.combat.projectiles[i].position.y == boss->position.y - 24);
    }
    assert(shots == 3 && boss->attackDirection.x == locked.x && boss->attackDirection.y == locked.y);
    app.player.position = app.worlds[MAP_BOSS].spawn;
    UpdateCombat(&app.combat, &app.player, &app.worlds[MAP_BOSS], 0.05f, false);
    for (int i = 0; i < PROJECTILE_CAPACITY; i++) assert(!app.combat.projectiles[i].active);
    assert(!app.combat.inCombat && app.player.health == 100);

    /* A lethal frame may award progress, but must still require respawn. */
    boss->health = 0;
    app.player.health = 0;
    ClaimEnemyRewards(&app.combat, &app.player);
    UpdateAppQuest(&app);
    assert(app.player.health == 0 && app.quest.bossDefeated && app.quest.stage == QUEST_REPORT_BOSS);
    RespawnAppPlayer(&app, false);
    assert(app.player.health == app.player.maxHealth && app.combat.enemyCount == 1 && app.combat.enemies[0].health == 300);
    puts("PASS: accepted-only typed objectives, report gates, one-time rewards, playable boss kill, both boss attacks, death retry and complete quest loop");
    return 0;
}
