#include "enemy_ai.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>

static float Distance(Vector2 a, Vector2 b)
{
    return sqrtf((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}

static void Setup(World *world, Combat *combat, Player *player)
{
    InitializeWorld(world, MAP_FIELD);
    InitializeCombat(combat, MAP_FIELD);
    combat->enemyCount = 1;
    InitializePlayer(player, world->spawn);
}

int main(void)
{
    World world;
    Combat combat;
    Player player;
    Setup(&world, &combat, &player);
    InitializeCombat(&combat, MAP_FIELD);
    assert(combat.enemyCount == 6);
    for (int i = 0; i < combat.enemyCount; i++) assert(CanEnemyStand(&world, combat.enemies[i].spawn));
    /* Traverse each real tree/rock from left to right, without crossing it. */
    for (int i = 0; i < world.sceneryCount; i++) {
        Setup(&world, &combat, &player);
        Rectangle r = world.scenery[i].collision;
        Enemy *walker = &combat.enemies[0];
        walker->position = (Vector2){ r.x - 48, r.y + r.height / 2 + 12 };
        Vector2 goal = { r.x + r.width + 48, walker->position.y };
        assert(CanEnemyStand(&world, walker->position) && CanEnemyStand(&world, goal));
        assert(!CanEnemyStand(&world, (Vector2){ r.x + r.width / 2, walker->position.y }));
        assert(!HasCombatLineOfSight(&world, (Vector2){ walker->position.x, walker->position.y - 12 },
            (Vector2){ goal.x, goal.y - 12 }));
        for (int frame = 0; frame < 150 && Distance(walker->position, goal) > 1; frame++) {
            MoveEnemyTowards(walker, &world, goal, 64, 0.05f, &combat, &player);
            assert(CanEnemyStand(&world, walker->position));
        }
        assert(Distance(walker->position, goal) <= 1);
    }
    Setup(&world, &combat, &player);
    for (int frame = 0; frame < 30; frame++) UpdateCombat(&combat, &player, &world, 0.05f, false);
    assert(Distance(combat.enemies[0].position, combat.enemies[0].spawn) > 1);
    assert(Distance(combat.enemies[0].position, combat.enemies[0].spawn) <= 48);
    assert(!combat.inCombat && player.health == 100);

    player.position = (Vector2){ 280, 656 };
    UpdateCombat(&combat, &player, &world, 0.05f, false);
    assert(combat.inCombat && combat.enemies[0].engaged);
    player.position = (Vector2){ 1100, 880 };
    for (int frame = 0; frame < 50; frame++) UpdateCombat(&combat, &player, &world, 0.05f, false);
    assert(combat.inCombat && combat.lostTimer > 2.4f && combat.lostTimer < 2.6f);
    player.position = (Vector2){ combat.enemies[0].position.x + 80, combat.enemies[0].position.y };
    UpdateCombat(&combat, &player, &world, 0.05f, false);
    assert(combat.inCombat && combat.lostTimer == 0);
    player.position = (Vector2){ 1100, 880 };
    for (int frame = 0; frame < 110; frame++) UpdateCombat(&combat, &player, &world, 0.05f, false);
    assert(!combat.inCombat && !combat.enemies[0].engaged);

    Setup(&world, &combat, &player);
    combat.enemyCount = 2;
    combat.enemies[1] = combat.enemies[0];
    combat.enemies[1].position = combat.enemies[1].spawn = (Vector2){ 800, 656 };
    player.position = (Vector2){ 280, 656 };
    UpdateCombat(&combat, &player, &world, 0.01f, false);
    player.position = (Vector2){ 1100, 880 };
    for (int frame = 0; frame < 50; frame++) UpdateCombat(&combat, &player, &world, 0.05f, false);
    assert(combat.lostTimer > 2);
    player.position = (Vector2){ 880, 656 };
    UpdateCombat(&combat, &player, &world, 0.05f, false);
    assert(combat.inCombat && combat.lostTimer == 0 && combat.enemies[1].engaged);

    World corner = { .columns = 20, .rows = 20, .id = MAP_FIELD };
    corner.tiles[10][11] = TILE_WALL;
    corner.tiles[11][10] = TILE_WALL;
    Vector2 next = FindEnemyWaypoint(&corner, (Vector2){ 336, 344 }, (Vector2){ 400, 408 });
    assert(CanEnemyStand(&corner, next));
    assert(!(next.x > 336 && next.y > 344));

    Setup(&world, &combat, &player);
    combat.enemies[0].position = (Vector2){ 480, 656 };
    combat.enemies[0].state = ENEMY_CHASE;
    combat.enemies[0].engaged = true;
    combat.enemies[0].health = 45;
    combat.inCombat = true;
    combat.wasSafe = false;
    combat.projectiles[0].active = true;
    combat.projectiles[0].hostile = true;
    UpdateCombat(&combat, &player, &world, 0.05f, false);
    assert(!combat.inCombat && combat.enemies[0].state == ENEMY_RETURN);
    assert(!combat.projectiles[0].active && player.health == 100);
    for (int frame = 0; frame < 200 && combat.enemies[0].state == ENEMY_RETURN; frame++) {
        player.position = (Vector2){ combat.enemies[0].position.x + 20, combat.enemies[0].position.y };
        UpdateCombat(&combat, &player, &world, 0.05f, false);
        assert(player.health == 100);
        assert(CanEnemyStand(&world, combat.enemies[0].position));
    }
    assert(combat.enemies[0].state == ENEMY_PATROL);
    assert(Distance(combat.enemies[0].position, combat.enemies[0].spawn) < 0.1f);
    assert(combat.enemies[0].health == 45);

    /* Pursue through the opening in a wall, even outside the spawn neighbourhood. */
    Setup(&world, &combat, &player);
    /* Synthetic obstacle: the actual forest no longer contains this wall. */
    for (int row = 7; row <= 32; row++) if (row < 18 || row > 22) world.tiles[row][19] = TILE_WALL;
    Enemy *enemy = &combat.enemies[0];
    enemy->position = (Vector2){ 496, 560 };
    enemy->spawn = enemy->position;
    enemy->state = ENEMY_CHASE;
    enemy->engaged = true;
    enemy->lastSeen = (Vector2){ 784, 560 };
    player.position = enemy->lastSeen;
    player.health = player.maxHealth = 100000;
    combat.rules.disengageDelay = 100; /* isolate route traversal from the 5s test above */
    for (int frame = 0; frame < 300; frame++) {
        UpdateCombat(&combat, &player, &world, 0.05f, false);
        assert(CanEnemyStand(&world, enemy->position));
    }
    assert(enemy->position.x > 640 && Distance(enemy->position, player.position) <= 50);

    Setup(&world, &combat, &player);
    enemy = &combat.enemies[0];
    enemy->kind = ENEMY_RANGED;
    enemy->cooldown = 0;
    player.position = (Vector2){ 432, 656 };
    UpdateCombat(&combat, &player, &world, 0.01f, false);
    assert(enemy->state == ENEMY_WINDUP);
    Vector2 locked = enemy->attackDirection;
    player.position.y += 80;
    UpdateCombat(&combat, &player, &world, 0.6f, false);
    assert(combat.projectiles[0].active && combat.projectiles[0].hostile);
    assert(combat.projectiles[0].direction.x == locked.x && combat.projectiles[0].direction.y == locked.y);
    enemy->health = 0;
    for (int frame = 0; frame < 12; frame++) UpdateCombat(&combat, &player, &world, 0.05f, false);
    assert(player.health == 100); /* locked shot can be dodged */

    Setup(&world, &combat, &player);
    enemy = &combat.enemies[0];
    enemy->kind = ENEMY_RANGED;
    enemy->attackDirection = (Vector2){ 1, 0 };
    player.position = (Vector2){ 400, 656 };
    FireEnemyProjectile(&combat, enemy);
    enemy->health = 0;
    for (int frame = 0; frame < 12; frame++) UpdateCombat(&combat, &player, &world, 0.05f, false);
    assert(player.health == 88);
    player.position = world.spawn;
    UpdateCombat(&combat, &player, &world, 0.05f, false);
    assert(enemy->health == 0 && enemy->state == ENEMY_DEAD);

    Setup(&world, &combat, &player);
    enemy = &combat.enemies[0];
    enemy->kind = ENEMY_RANGED;
    player.position = (Vector2){ 280, 656 };
    float previousDistance = Distance(enemy->position, player.position);
    for (int frame = 0; frame < 10; frame++) UpdateCombat(&combat, &player, &world, 0.05f, false);
    assert(Distance(enemy->position, player.position) > previousDistance);
    puts("PASS: patrol, detection, shared 5s reset, safe return, no return attacks, obstacle routing, ranged retreat/shots and dead-enemy persistence");
    return 0;
}
