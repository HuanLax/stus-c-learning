#include "app.h"
#include "enemy_ai.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

static void CheckEnemyLanguageSwitch(void)
{
    App app = { 0 };
    app.assets.supportsChinese = true;
    const EnemyKind kinds[] = { ENEMY_MELEE, ENEMY_RANGED, ENEMY_BOSS };
    const char *const english[] = { "Melee goblin", "Ranged goblin", "Goblin King Grukk" };
    const char *const chinese[] = { "近战哥布林", "远程哥布林", "哥布林王格鲁克" };
    for (int cycle = 0; cycle < 4; cycle++) {
        Language language = cycle % 2 ? LANGUAGE_ENGLISH : LANGUAGE_CHINESE;
        SetAppLanguage(&app, language);
        for (int i = 0; i < 3; i++)
            assert(strcmp(GetEnemyName(kinds[i], app.language), language == LANGUAGE_ENGLISH ? english[i] : chinese[i]) == 0);
    }
    for (int language = 0; language < LANGUAGE_COUNT; language++) {
        for (int id = 0; id < TEXT_COUNT; id++) {
            const char *text = GetUiText((Language)language, (TextId)id);
            assert(text && text[0] && strcmp(text, "[invalid text]") != 0);
        }
    }
    assert(strcmp(GetUiText((Language)-1, TEXT_MELEE_GOBLIN), "[invalid text]") == 0);
    assert(strcmp(GetUiText(LANGUAGE_ENGLISH, TEXT_COUNT), "[invalid text]") == 0);
}

static void Setup(App *app)
{
    *app = (App){ 0 };
    for (int map = 0; map < MAP_COUNT; map++) InitializeWorld(&app->worlds[map], (MapId)map);
    app->currentMap = MAP_FIELD;
    app->screen = SCREEN_PLAY;
    InitializePlayer(&app->player, (Vector2){ 280, 656 });
    app->player.swordFacing = (Vector2){ 1, 0 };
    app->player.staffFacing = (Vector2){ 1, 0 };
    InitializeCombat(&app->combat, MAP_FIELD);
    app->combat.enemyCount = 1;
    app->combat.rules.enemySpeed = 0; /* isolate hit/health tests from movement */
}

static void CheckActorBlocking(void)
{
    App app;
    Setup(&app);
    World *world = &app.worlds[MAP_FIELD];
    for (int kind = ENEMY_MELEE; kind <= ENEMY_BOSS; kind++) {
        Enemy *enemy = &app.combat.enemies[0];
        enemy->kind = (EnemyKind)kind;
        enemy->health = 60;
        enemy->position = (Vector2){ 400, 400 };
        Rectangle obstacle = GetEnemyCollision(enemy);
        app.player.position = (Vector2){ 300, 400 };
        MovePlayerWithObstacles(&app.player, world, (Vector2){ 1, 0 }, 2, &obstacle, 1);
        assert(!CheckCollisionRecs(GetPlayerBody(app.player.position), obstacle));
        assert(app.player.position.x < enemy->position.x);
        /* Live enemies cannot walk through the player or another live enemy. */
        app.player.position = (Vector2){ 300, 400 };
        for (int i = 0; i < 70; i++) MoveEnemyTowards(enemy, world, app.player.position, 96, 0.05f, &app.combat, &app.player);
        assert(!CheckCollisionRecs(GetPlayerBody(app.player.position), GetEnemyCollision(enemy)));
        assert(enemy->position.x > app.player.position.x);
        app.combat.enemyCount = 2;
        app.combat.enemies[1] = *enemy;
        app.combat.enemies[1].position = (Vector2){ 450, 400 };
        enemy->position = (Vector2){ 540, 400 }; enemy->pathTimer = 0;
        app.player.position = (Vector2){ 700, 600 };
        for (int i = 0; i < 50; i++) MoveEnemyTowards(enemy, world, (Vector2){ 350, 400 }, 96, 0.05f, &app.combat, &app.player);
        assert(!CheckCollisionRecs(GetEnemyCollision(enemy), GetEnemyCollision(&app.combat.enemies[1])));
        app.combat.enemyCount = 1;
        /* Zero obstacles models a dead enemy, whose body no longer blocks. */
        app.player.position = (Vector2){ 300, 400 };
        MovePlayerWithObstacles(&app.player, world, (Vector2){ 1, 0 }, 2, &obstacle, 0);
        assert(app.player.position.x > 400);
        app.player.position = (Vector2){ 400, 400 };
        MovePlayerWithObstacles(&app.player, world, (Vector2){ 1, 0 }, 1, &obstacle, 1);
        assert(!CheckCollisionRecs(GetPlayerBody(app.player.position), obstacle));
    }
}

int main(void)
{
    CheckActorBlocking();
    CheckEnemyLanguageSwitch();
    App app;
    Setup(&app);
    World *world = &app.worlds[MAP_FIELD];
    assert(StartPlayerAttack(&app.combat, &app.player, world));
    UpdateCombat(&app.combat, &app.player, world, 0.01f, false);
    assert(app.combat.enemies[0].health == 40);
    for (int i = 0; i < 4; i++) UpdateCombat(&app.combat, &app.player, world, 0.02f, false);
    assert(app.combat.enemies[0].health == 40); /* no per-frame damage */
    app.player.weapon = WEAPON_STAFF;
    assert(!StartPlayerAttack(&app.combat, &app.player, world)); /* switching cannot bypass cooldown */

    Setup(&app);
    app.player.swordFacing = (Vector2){ -1, 0 };
    UpdateCombat(&app.combat, &app.player, world, 0.01f, true);
    assert(app.combat.enemies[0].health == 60); /* target behind attacker */
    Setup(&app);
    world->tiles[20][9] = TILE_WALL;
    UpdateCombat(&app.combat, &app.player, world, 0.01f, true);
    assert(app.combat.enemies[0].health == 60); /* cannot strike through wall */

    Setup(&app);
    app.combat.enemies[0].health = 20;
    UpdateCombat(&app.combat, &app.player, world, 0.01f, true);
    assert(app.combat.enemies[0].health == 0 && app.combat.enemies[0].windup == 0);

    Setup(&app);
    app.player.weapon = WEAPON_STAFF;
    app.player.position = (Vector2){ 248, 656 };
    assert(StartPlayerAttack(&app.combat, &app.player, world));
    for (int i = 0; i < 10; i++) UpdateCombat(&app.combat, &app.player, world, 0.04f, false);
    assert(app.combat.enemies[0].health == 40);
    assert(!app.combat.projectiles[0].active);
    Setup(&app);
    app.player.weapon = WEAPON_STAFF;
    app.player.position = (Vector2){ 248, 656 };
    app.combat.enemyCount = 2;
    app.combat.enemies[1] = app.combat.enemies[0];
    app.combat.enemies[1].position.x = 344;
    StartPlayerAttack(&app.combat, &app.player, world);
    UpdateCombat(&app.combat, &app.player, world, 0.4f, false);
    assert(app.combat.enemies[0].health == 40 && app.combat.enemies[1].health == 60);
    Setup(&app);
    app.player.weapon = WEAPON_STAFF;
    app.player.position = (Vector2){ 248, 656 };
    world->tiles[20][9] = TILE_WALL;
    StartPlayerAttack(&app.combat, &app.player, world);
    UpdateCombat(&app.combat, &app.player, world, 0.4f, false);
    assert(!app.combat.projectiles[0].active && app.combat.enemies[0].health == 60);

    Setup(&app);
    app.player.weapon = WEAPON_STAFF;
    app.combat.enemyCount = 0;
    app.player.staffFacing = (Vector2){ 0.70710678f, 0.70710678f };
    StartPlayerAttack(&app.combat, &app.player, world);
    Vector2 origin = app.combat.projectiles[0].position;
    UpdateCombat(&app.combat, &app.player, world, 2.0f, false);
    Vector2 end = app.combat.projectiles[0].position;
    float length = sqrtf((end.x - origin.x) * (end.x - origin.x) + (end.y - origin.y) * (end.y - origin.y));
    assert(fabsf(length - 256) < 0.01f && !app.combat.projectiles[0].active);

    Setup(&app);
    app.player.position = world->spawn;
    app.combat.projectiles[0].active = true;
    app.combat.enemies[0].windup = 0.1f;
    assert(!StartPlayerAttack(&app.combat, &app.player, world));
    UpdateCombat(&app.combat, &app.player, world, 0.2f, true);
    assert(app.player.health == 100 && !app.combat.projectiles[0].active && app.combat.enemies[0].windup == 0);

    Setup(&app);
    app.combat.enemies[0].cooldown = 0;
    UpdateCombat(&app.combat, &app.player, world, 0.01f, false);
    assert(app.combat.enemies[0].windup > 0 && app.player.health == 100);
    app.player.position.y += 80;
    UpdateCombat(&app.combat, &app.player, world, 0.6f, false);
    assert(app.player.health == 100); /* warning can be dodged */
    Setup(&app);
    app.combat.enemies[0].cooldown = 0;
    UpdateCombat(&app.combat, &app.player, world, 0.01f, false);
    UpdateCombat(&app.combat, &app.player, world, 0.6f, false);
    assert(app.player.health == 82);
    assert(!DamagePlayer(&app.player, 18)); /* invulnerability */
    app.player.hurtTimer = 0;
    assert(DamagePlayer(&app.player, 999) && app.player.health == 0);
    RespawnAppPlayer(&app, false);
    assert(app.currentMap == MAP_FIELD && app.player.health == 100 && app.combat.enemies[0].health == 60);
    assert(IsBodyInSafeArea(world, GetPlayerBody(app.player.position)));
    app.player.health = 0;
    app.player.weapon = WEAPON_STAFF;
    RespawnAppPlayer(&app, true);
    assert(app.currentMap == MAP_TOWN && app.player.health == 100 && app.player.weapon == WEAPON_STAFF);
    assert(app.combat.enemyCount == 0 && !app.attackInputArmed);

    Setup(&app);
    app.player.health = 20;
    app.player.position = world->spawn;
    app.nearbyInteraction = 2;
    ActivateNearbyInteraction(&app);
    assert(app.player.health == 100 && app.interactionText == TEXT_HEALED);
    app.player.health = 20;
    app.player.position = (Vector2){ 200, 688 };
    ActivateNearbyInteraction(&app);
    assert(app.player.health == 20); /* nearby is not sufficient: must be inside safe area */
    bool armed = true;
    assert(!GetAttackIntent(&armed, true, false));
    assert(!GetAttackIntent(&armed, true, true));
    assert(!GetAttackIntent(&armed, false, true));
    assert(GetAttackIntent(&armed, true, true));
    puts("PASS: bilingual enemy names, complete translations, single-hit swing, shared cooldown, projectiles, walls, safe area, dodge, invulnerability, revival, healing and UI input release");
    return 0;
}
