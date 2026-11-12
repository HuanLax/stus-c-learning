#include "enemy_ai.h"
#include "boss.h"

#include <math.h>

static float Distance(Vector2 a, Vector2 b)
{
    return sqrtf((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}

static bool CanStandSized(const World *world, Vector2 position, bool large)
{
    Rectangle body = GetEnemyBodyForKind(position, large ? ENEMY_BOSS : ENEMY_MELEE);
    return CanStandInWorld(world, body) && !CheckCollisionRecs(body, world->safeArea);
}

bool CanEnemyStand(const World *world, Vector2 position)
{
    return CanStandSized(world, position, false);
}

static Vector2 CellPosition(int cell)
{
    return (Vector2){ (float)(cell % MAP_MAX_COLUMNS) * TILE_SIZE + 16,
                      (float)(cell / MAP_MAX_COLUMNS) * TILE_SIZE + 16 };
}

static Vector2 FindWaypointSized(const World *world, Vector2 start, Vector2 goal, bool large)
{
    int firstCol = (int)(start.x / TILE_SIZE);
    int firstRow = (int)(start.y / TILE_SIZE);
    if (firstCol < 0 || firstRow < 0 || firstCol >= world->columns || firstRow >= world->rows) return start;
    /* Same-cell targets need no queue or parent-array initialization. */
    if (firstCol == (int)(goal.x / TILE_SIZE) && firstRow == (int)(goal.y / TILE_SIZE))
        return CanStandSized(world, goal, large) && Distance(start, goal) < TILE_SIZE ? goal : start;
    int first = firstRow * MAP_MAX_COLUMNS + firstCol;
    int parents[MAP_MAX_COLUMNS * MAP_MAX_ROWS];
    int queue[MAP_MAX_COLUMNS * MAP_MAX_ROWS];
    for (int i = 0; i < MAP_MAX_COLUMNS * MAP_MAX_ROWS; i++) parents[i] = -1;
    int head = 0;
    int tail = 0;
    parents[first] = first;
    queue[tail++] = first;
    int best = first;
    float bestDistance = Distance(CellPosition(first), goal);
    const int directions[8][2] = { {1,0}, {-1,0}, {0,1}, {0,-1}, {1,1}, {1,-1}, {-1,1}, {-1,-1} };
    while (head < tail) {
        int cell = queue[head++];
        int col = cell % MAP_MAX_COLUMNS;
        int row = cell / MAP_MAX_COLUMNS;
        float distance = Distance(CellPosition(cell), goal);
        if (distance < bestDistance) { best = cell; bestDistance = distance; }
        if (col == (int)(goal.x / TILE_SIZE) && row == (int)(goal.y / TILE_SIZE)) { best = cell; break; }
        for (int direction = 0; direction < 8; direction++) {
            int dx = directions[direction][0];
            int dy = directions[direction][1];
            int nextCol = col + dx;
            int nextRow = row + dy;
            if (nextCol < 0 || nextRow < 0 || nextCol >= world->columns || nextRow >= world->rows) continue;
            int next = nextRow * MAP_MAX_COLUMNS + nextCol;
            if (parents[next] != -1 || !CanStandSized(world, CellPosition(next), large)) continue;
            /* Both adjacent cardinal cells must be clear before a diagonal step. */
            if (dx != 0 && dy != 0 &&
                (!CanStandSized(world, CellPosition(row * MAP_MAX_COLUMNS + nextCol), large) ||
                 !CanStandSized(world, CellPosition(nextRow * MAP_MAX_COLUMNS + col), large))) continue;
            parents[next] = cell;
            queue[tail++] = next;
        }
    }
    if (best == first) return CanStandSized(world, goal, large) && Distance(start, goal) < TILE_SIZE ? goal : start;
    while (parents[best] != first) best = parents[best];
    Vector2 next = CellPosition(best);
    int samples = (int)ceilf(Distance(start, next) / 4);
    for (int i = 1; i <= samples; i++) {
        float portion = (float)i / (float)samples;
        Vector2 point = { start.x + (next.x - start.x) * portion, start.y + (next.y - start.y) * portion };
        if (!CanStandSized(world, point, large)) return CellPosition(first);
    }
    return next;
}

Vector2 FindEnemyWaypoint(const World *world, Vector2 start, Vector2 goal)
{
    return FindWaypointSized(world, start, goal, false);
}

void MoveEnemyTowards(Enemy *enemy, const World *world, Vector2 goal, float speed, float seconds,
                      const Combat *combat, const Player *player)
{
    Rectangle obstacles[ENEMY_CAPACITY + 1];
    int count = 0;
    if (player->health > 0) obstacles[count++] = GetPlayerBody(player->position);
    for (int i = 0; i < combat->enemyCount; i++)
        if (&combat->enemies[i] != enemy && combat->enemies[i].health > 0)
            obstacles[count++] = GetEnemyCollision(&combat->enemies[i]);
    enemy->pathTimer -= seconds;
    int col = (int)(goal.x / TILE_SIZE);
    int row = (int)(goal.y / TILE_SIZE);
    if (enemy->pathTimer <= 0 || col != enemy->goalCol || row != enemy->goalRow || Distance(enemy->position, enemy->waypoint) < 1) {
        enemy->waypoint = FindWaypointSized(world, enemy->position, goal, enemy->kind == ENEMY_BOSS);
        enemy->pathTimer = 0.35f;
        enemy->goalCol = col;
        enemy->goalRow = row;
    }
    float distance = Distance(enemy->position, enemy->waypoint);
    if (distance <= 0.01f || speed <= 0) return;
    float travel = fminf(speed * seconds, distance);
    int steps = (int)ceilf(travel / 4);
    if (steps < 1) steps = 1;
    Vector2 direction = { (enemy->waypoint.x - enemy->position.x) / distance,
                          (enemy->waypoint.y - enemy->position.y) / distance };
    for (int i = 0; i < steps; i++) {
        Vector2 next = { enemy->position.x + direction.x * travel / (float)steps,
                         enemy->position.y + direction.y * travel / (float)steps };
        if (!CanStandSized(world, next, enemy->kind == ENEMY_BOSS) ||
            !CanMoveWithObstacles(world, GetEnemyCollision(enemy), GetEnemyBodyForKind(next, enemy->kind), obstacles, count)) {
            enemy->pathTimer = 0;
            break;
        }
        enemy->position = next;
        enemy->facing = direction;
    }
}

static void EndEncounter(Combat *combat)
{
    combat->inCombat = false;
    combat->lostTimer = 0;
    for (int i = 0; i < combat->enemyCount; i++) {
        Enemy *enemy = &combat->enemies[i];
        if (enemy->health <= 0) continue;
        if (enemy->engaged || enemy->windup > 0 || enemy->state != ENEMY_PATROL) {
            enemy->state = ENEMY_RETURN;
            enemy->pathTimer = 0;
        }
        enemy->engaged = false;
        enemy->windup = 0;
    }
    for (int i = 0; i < PROJECTILE_CAPACITY; i++) {
        if (combat->projectiles[i].hostile) combat->projectiles[i].active = false;
    }
}

void UpdateEnemyAI(Combat *combat, Player *player, const World *world, float seconds)
{
    bool safe = IsBodyInSafeArea(world, GetPlayerBody(player->position));
    bool detected = false;
    bool engaged = false;
    bool hostileShot = false;
    for (int i = 0; i < PROJECTILE_CAPACITY; i++) {
        hostileShot = hostileShot || (combat->projectiles[i].active && combat->projectiles[i].hostile);
    }
    for (int i = 0; i < combat->enemyCount; i++) {
        Enemy *enemy = &combat->enemies[i];
        enemy->flashTimer = fmaxf(0, enemy->flashTimer - seconds);
        if (enemy->health <= 0) { enemy->state = ENEMY_DEAD; enemy->engaged = false; continue; }
        bool sees = !safe && enemy->state != ENEMY_RETURN &&
            Distance(enemy->position, player->position) <= (enemy->kind == ENEMY_BOSS ? 240 : combat->rules.detectionRange) &&
            HasCombatLineOfSight(world, GetEnemyAttackOrigin(enemy),
                                  (Vector2){ player->position.x, player->position.y - 12 });
        if (sees) {
            detected = true;
            enemy->engaged = true;
            enemy->lastSeen = player->position;
            if (enemy->state == ENEMY_PATROL) { enemy->state = ENEMY_CHASE; enemy->pathTimer = 0; }
        }
        engaged = engaged || enemy->engaged;
    }
    if (safe) {
        if (!combat->wasSafe || combat->inCombat) {
            /* All survivors return, including currently patrolling enemies. */
            for (int i = 0; i < combat->enemyCount; i++) if (combat->enemies[i].health > 0) combat->enemies[i].engaged = true;
            EndEncounter(combat);
        } else {
            for (int i = 0; i < combat->enemyCount; i++) {
                if (combat->enemies[i].windup > 0) { combat->enemies[i].engaged = true; EndEncounter(combat); break; }
            }
        }
    } else if (detected) {
        combat->inCombat = true;
        combat->lostTimer = 0;
    } else if (!engaged && !hostileShot) {
        combat->inCombat = false;
        combat->lostTimer = 0;
    } else {
        combat->inCombat = true;
        combat->lostTimer += seconds;
        if (combat->lostTimer >= combat->rules.disengageDelay) EndEncounter(combat);
    }
    combat->wasSafe = safe;

    for (int i = 0; i < combat->enemyCount; i++) {
        Enemy *enemy = &combat->enemies[i];
        if (enemy->health <= 0) continue;
        enemy->cooldown = fmaxf(0, enemy->cooldown - seconds);
        if (enemy->state == ENEMY_RETURN) {
            MoveEnemyTowards(enemy, world, enemy->spawn, enemy->kind == ENEMY_BOSS ? 48 : combat->rules.enemySpeed, seconds, combat, player);
            if (Distance(enemy->position, enemy->spawn) < 1) {
                enemy->position = enemy->spawn;
                enemy->state = ENEMY_PATROL;
                enemy->patrolPause = 0.8f;
                enemy->cooldown = combat->rules.enemyCooldown;
            }
            continue;
        }
        if (enemy->kind == ENEMY_BOSS) { UpdateBoss(combat, enemy, player, world, seconds); continue; }
        if (enemy->state == ENEMY_PATROL) {
            if (enemy->patrolPause > 0) { enemy->patrolPause -= seconds; continue; }
            const Vector2 offsets[] = { {32,0}, {0,32}, {-32,0}, {0,-32} };
            Vector2 target = { enemy->spawn.x + offsets[enemy->patrolPoint].x,
                               enemy->spawn.y + offsets[enemy->patrolPoint].y };
            MoveEnemyTowards(enemy, world, target, combat->rules.enemySpeed * 0.5f, seconds, combat, player);
            if (Distance(enemy->position, target) < 1 ||
                (!CanEnemyStand(world, target) && Distance(enemy->position, enemy->waypoint) < 0.01f)) {
                enemy->patrolPoint = (enemy->patrolPoint + 1) % 4;
                enemy->patrolPause = 0.5f;
            }
            continue;
        }
        if (enemy->state == ENEMY_WINDUP) {
            enemy->windup -= seconds;
            if (enemy->windup <= 0) {
                enemy->windup = 0;
                if (enemy->kind == ENEMY_RANGED) FireEnemyProjectile(combat, enemy);
                else if (CheckCollisionRecs(enemy->attackArea, GetPlayerBody(player->position)) &&
                    HasCombatLineOfSight(world, (Vector2){ enemy->position.x, enemy->position.y - 12 },
                                          (Vector2){ player->position.x, player->position.y - 12 })) DamagePlayer(player, combat->rules.enemyDamage);
                enemy->cooldown = combat->rules.enemyCooldown;
                enemy->state = ENEMY_CHASE;
            }
            continue;
        }
        float distance = Distance(enemy->position, player->position);
        bool visible = !safe && HasCombatLineOfSight(world,
            (Vector2){ enemy->position.x, enemy->position.y - 12 },
            (Vector2){ player->position.x, player->position.y - 12 });
        float reach = enemy->kind == ENEMY_MELEE ? combat->rules.enemyReach : 160;
        if (enemy->kind == ENEMY_RANGED && visible && distance < 64) {
            Vector2 away = distance > 0.01f ? (Vector2){ (enemy->position.x - player->position.x) / distance,
                (enemy->position.y - player->position.y) / distance } : (Vector2){ 1, 0 };
            Vector2 target = { player->position.x + away.x * 96, player->position.y + away.y * 96 };
            Vector2 before = enemy->position;
            MoveEnemyTowards(enemy, world, target, combat->rules.enemySpeed, seconds, combat, player);
            if (Distance(before, enemy->position) > 0.01f) continue;
            /* If blocked, fall through to a stationary shot instead of freezing. */
        }
        if (visible && distance <= reach && enemy->cooldown <= 0) {
            enemy->state = ENEMY_WINDUP;
            enemy->windup = combat->rules.enemyWindup;
            enemy->attackArea = (Rectangle){ player->position.x - 24, player->position.y - 36, 48, 48 };
            if (distance > 0.01f) enemy->attackDirection = (Vector2){
                (player->position.x - enemy->position.x) / distance,
                (player->position.y - enemy->position.y) / distance };
            else enemy->attackDirection = (Vector2){ 1, 0 };
        } else if (!visible || distance > reach) {
            MoveEnemyTowards(enemy, world, enemy->lastSeen, combat->rules.enemySpeed, seconds, combat, player);
        }
    }
}
