/* 通用战斗：敌人配置、近战判定、投射物、伤害、奖励与战斗效果。 */
#include "combat.h"
#include "enemy_ai.h"
#include "boss.h"

#include <math.h>

/* ===== 敌人名称与碰撞体 ===== */
const char *GetEnemyName(EnemyKind kind, Language language)
{
    TextId name;
    switch (kind) {
        case ENEMY_MELEE: name = TEXT_MELEE_GOBLIN; break;
        case ENEMY_RANGED: name = TEXT_RANGED_GOBLIN; break;
        case ENEMY_BOSS: name = TEXT_BOSS_NAME; break;
        default: name = TEXT_TEST_ENEMY; break;
    }
    return GetUiText(language, name); /* Resolve on every draw, never cache a language-specific name. */
}

Rectangle GetEnemyBody(Vector2 feet)
{
    return GetEnemyBodyForKind(feet, ENEMY_MELEE);
}

Rectangle GetEnemyBodyForKind(Vector2 feet, EnemyKind kind)
{
    float size = kind == ENEMY_BOSS ? 48 : 24;
    return (Rectangle){ feet.x - size / 2, feet.y - size, size, size };
}

Rectangle GetEnemyCollision(const Enemy *enemy)
{
    return GetEnemyBodyForKind(enemy->position, enemy->kind);
}

Vector2 GetEnemyAttackOrigin(const Enemy *enemy)
{
    return (Vector2){ enemy->position.x, enemy->position.y - (enemy->kind == ENEMY_BOSS ? 24 : 12) };
}

/* ===== 战斗规则与敌人出生配置 ===== */
void InitializeCombat(Combat *combat, MapId map)
{
    *combat = (Combat){ 0 };
    /* All first-playtest values live in this local record, not global variables. */
    combat->rules = (CombatRules){
        .enemyDamage = 18,
        .attackCooldown = 0.45f, .swingDuration = 0.14f,
        .swordReach = 40, .swordWidth = 64,
        .projectileSpeed = 240, .projectileRange = RANGED_PROJECTILE_RANGE,
        .enemyReach = 48, .enemyWindup = 0.55f, .enemyCooldown = 1.25f,
        .enemySpeed = 64, .detectionRange = 160, .disengageDelay = 5
    };
    if (map == MAP_FIELD) {
        const Vector2 spawns[] = { { 320, 656 }, { 496, 560 }, { 784, 656 },
                                   { 1072, 800 }, { 432, 784 }, { 1008, 528 } };
        combat->enemyCount = 6;
        for (int i = 0; i < combat->enemyCount; i++) {
            combat->enemies[i] = (Enemy){ .kind = i < 4 ? ENEMY_MELEE : ENEMY_RANGED,
                .state = ENEMY_PATROL, .spawn = spawns[i], .position = spawns[i],
                .health = 60, .maxHealth = 60, .cooldown = 0.6f, .goalCol = -1, .goalRow = -1 };
        }
    }
    if (map == MAP_BOSS) {
        combat->enemyCount = 1;
        combat->enemies[0] = (Enemy){ .kind = ENEMY_BOSS, .state = ENEMY_PATROL,
            .spawn = { 528, 400 }, .position = { 528, 400 }, .health = 300, .maxHealth = 300,
            .cooldown = 0.8f, .goalCol = -1, .goalRow = -1, .attackVariant = 1 };
    }
    combat->wasSafe = true;
}

/* ===== 攻击范围、视线与玩家攻击 ===== */
Rectangle GetSwordArea(const Combat *combat, const Player *player)
{
    Vector2 p = { player->position.x, player->position.y - 12 };
    Vector2 facing = combat->swingFacing;
    float reach = combat->rules.swordReach;
    float width = combat->rules.swordWidth;
    if (facing.x > 0) return (Rectangle){ p.x + 10, p.y - width / 2, reach, width };
    if (facing.x < 0) return (Rectangle){ p.x - 10 - reach, p.y - width / 2, reach, width };
    if (facing.y < 0) return (Rectangle){ p.x - width / 2, p.y - 10 - reach, width, reach };
    return (Rectangle){ p.x - width / 2, p.y + 10, width, reach };
}

bool HasCombatLineOfSight(const World *world, Vector2 from, Vector2 to)
{
    float dx = to.x - from.x;
    float dy = to.y - from.y;
    int steps = (int)ceilf(sqrtf(dx * dx + dy * dy) / 4);
    if (steps < 1) steps = 1;
    for (int i = 0; i <= steps; i++) {
        float portion = (float)i / (float)steps;
        Rectangle sample = { from.x + dx * portion - 1, from.y + dy * portion - 1, 2, 2 };
        if (!CanStandInWorld(world, sample)) return false;
    }
    return true;
}

bool StartPlayerAttack(Combat *combat, const Player *player, const World *world)
{
    if (player->health <= 0 || combat->cooldown > 0 || combat->swingTimer > 0 ||
        IsBodyInSafeArea(world, GetPlayerBody(player->position))) return false;
    if (player->weapon == WEAPON_SWORD) {
        combat->swingTimer = combat->rules.swingDuration;
        combat->swingFacing = player->swordFacing;
        combat->swingDamage = GetPlayerAttack(player);
        for (int i = 0; i < ENEMY_CAPACITY; i++) combat->swingHit[i] = false;
    } else {
        int slot = -1;
        for (int i = 0; i < PROJECTILE_CAPACITY; i++) {
            if (!combat->projectiles[i].active) { slot = i; break; }
        }
        if (slot < 0) return false;
        combat->projectiles[slot] = (Projectile){ true,
            { player->position.x, player->position.y - 12 }, player->staffFacing,
            combat->rules.projectileRange, false, GetPlayerAttack(player) };
    }
    combat->cooldown = combat->rules.attackCooldown;
    return true;
}

bool GetAttackIntent(bool *armed, bool held, bool worldReady)
{
    if (!worldReady) { *armed = false; return false; }
    if (!held) *armed = true;
    return held && *armed;
}

/* ===== 伤害与投射物更新 ===== */
static void HitEnemy(Enemy *enemy, int damage)
{
    enemy->health -= damage;
    if (enemy->health < 0) enemy->health = 0;
    enemy->flashTimer = 0.16f;
    if (enemy->health == 0) enemy->windup = 0;
    if (enemy->health == 0) { enemy->state = ENEMY_DEAD; enemy->engaged = false; }
    else if (enemy->state != ENEMY_RETURN) {
        enemy->engaged = true;
        if (enemy->state == ENEMY_PATROL) enemy->state = ENEMY_CHASE;
    }
}

void FireEnemyProjectile(Combat *combat, const Enemy *enemy)
{
    for (int i = 0; i < PROJECTILE_CAPACITY; i++) {
        if (!combat->projectiles[i].active) {
            combat->projectiles[i] = (Projectile){ true,
            GetEnemyAttackOrigin(enemy), enemy->attackDirection,
                enemy->kind == ENEMY_BOSS ? 384 : RANGED_PROJECTILE_RANGE, true, enemy->kind == ENEMY_BOSS ? 14 : 12 };
            return;
        }
    }
}

static void UpdateProjectiles(Combat *combat, Player *player, const World *world, float seconds)
{
    for (int i = 0; i < PROJECTILE_CAPACITY; i++) {
        Projectile *shot = &combat->projectiles[i];
        if (!shot->active) continue;
        float speed = shot->hostile ? 160 : combat->rules.projectileSpeed;
        float distance = fminf(speed * seconds, shot->remainingDistance);
        int steps = (int)ceilf(distance / 4);
        if (steps < 1) steps = 1;
        float step = distance / (float)steps;
        for (int sample = 0; sample < steps && shot->active; sample++) {
            shot->position.x += shot->direction.x * step;
            shot->position.y += shot->direction.y * step;
            shot->remainingDistance -= step;
            Rectangle body = { shot->position.x - 4, shot->position.y - 4, 8, 8 };
            if (!CanStandInWorld(world, body) || CheckCollisionRecs(body, world->safeArea)) {
                shot->active = false;
                break;
            }
            if (shot->hostile && CheckCollisionRecs(body, GetPlayerBody(player->position))) {
                DamagePlayer(player, shot->damage);
                shot->active = false;
            }
            for (int target = 0; !shot->hostile && shot->active && target < combat->enemyCount; target++) {
                Enemy *enemy = &combat->enemies[target];
                if (enemy->health > 0 && CheckCollisionRecs(body, GetEnemyCollision(enemy))) {
                    HitEnemy(enemy, shot->damage);
                    shot->active = false;
                    break;
                }
            }
        }
        if (shot->remainingDistance <= 0.001f) shot->active = false;
    }
}

/* ===== 战斗更新与击杀奖励 ===== */
void UpdateCombat(Combat *combat, Player *player, const World *world, float seconds, bool attackHeld)
{
    if (seconds <= 0 || player->health <= 0) return;
    combat->cooldown = fmaxf(0, combat->cooldown - seconds);
    player->hurtTimer = fmaxf(0, player->hurtTimer - seconds);
    bool safe = IsBodyInSafeArea(world, GetPlayerBody(player->position));
    if (safe) {
        combat->swingTimer = 0;
        for (int i = 0; i < PROJECTILE_CAPACITY; i++) combat->projectiles[i].active = false;
    }
    if (attackHeld) StartPlayerAttack(combat, player, world);
    if (combat->swingTimer > 0) {
        Rectangle attack = GetSwordArea(combat, player);
        Vector2 from = { player->position.x, player->position.y - 12 };
        for (int i = 0; i < combat->enemyCount; i++) {
            Enemy *enemy = &combat->enemies[i];
            Vector2 target = { enemy->position.x, enemy->position.y - 12 };
            if (!combat->swingHit[i] && enemy->health > 0 &&
                CheckCollisionRecs(attack, GetEnemyCollision(enemy)) &&
                HasCombatLineOfSight(world, from, target)) {
                HitEnemy(enemy, combat->swingDamage);
                combat->swingHit[i] = true;
            }
        }
        combat->swingTimer = fmaxf(0, combat->swingTimer - seconds);
    }
    UpdateProjectiles(combat, player, world, seconds);
    UpdateEnemyAI(combat, player, world, seconds);
    ClaimEnemyRewards(combat, player);
}

int ClaimEnemyRewards(Combat *combat, Player *player)
{
    int defeated = 0;
    for (int i = 0; i < combat->enemyCount; i++) {
        Enemy *enemy = &combat->enemies[i];
        if (enemy->health <= 0 && !enemy->rewardGranted) {
            enemy->rewardGranted = true;
            if (enemy->kind == ENEMY_BOSS) { GrantPlayerRewards(player, 150, 120); continue; }
            defeated++;
            player->defeatedGoblins++;
            if (enemy->kind == ENEMY_MELEE) player->meleeDefeated++;
            else player->rangedDefeated++;
            GrantPlayerRewards(player, GOBLIN_GOLD, GOBLIN_EXPERIENCE);
        }
    }
    return defeated;
}

/* ===== 战斗效果绘制 ===== */
void DrawCombat(const Combat *combat, const Player *player)
{
    for (int i = 0; i < combat->enemyCount; i++) {
        const Enemy *enemy = &combat->enemies[i];
        if (enemy->health <= 0) continue;
        if (enemy->windup > 0) {
            if (enemy->kind == ENEMY_BOSS) DrawBossTelegraph(enemy);
            else if (enemy->kind == ENEMY_RANGED) {
                Vector2 start = { enemy->position.x, enemy->position.y - 12 };
                DrawLineEx(start, (Vector2){ start.x + enemy->attackDirection.x * 160,
                                             start.y + enemy->attackDirection.y * 160 }, 3, ORANGE);
            } else {
            DrawRectangleRec(enemy->attackArea, (Color){ 235, 80, 40, 100 });
            DrawRectangleLinesEx(enemy->attackArea, 2, ORANGE);
            }
        }
        int width = enemy->kind == ENEMY_BOSS ? 56 : 24;
        int height = enemy->kind == ENEMY_BOSS ? 72 : 38;
        DrawRectangle((int)enemy->position.x - width / 2, (int)enemy->position.y - height, width, 4, DARKGRAY);
        DrawRectangle((int)enemy->position.x - width / 2, (int)enemy->position.y - height,
                       width * enemy->health / enemy->maxHealth, 4, RED);
    }
    if (combat->swingTimer > 0) {
        Rectangle area = GetSwordArea(combat, player);
        DrawRectangleRec(area, (Color){ 255, 203, 0, 80 });
        DrawRectangleLinesEx(area, 2, GOLD);
    }
    for (int i = 0; i < PROJECTILE_CAPACITY; i++) {
        if (combat->projectiles[i].active) DrawCircleV(combat->projectiles[i].position, 4,
            combat->projectiles[i].hostile ? ORANGE : SKYBLUE);
    }
}
