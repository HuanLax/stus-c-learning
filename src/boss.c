/* 头目行为：重斩判定、散射法术、攻击切换及预警绘制。 */
#include "boss.h"
#include "enemy_ai.h"

#include <math.h>

/* ===== 重斩范围与方向计算 ===== */
bool BossSlashHits(const Enemy *boss, Rectangle target)
{
    Vector2 origin = GetEnemyAttackOrigin(boss);
    float dx = target.x + target.width / 2 - origin.x;
    float dy = target.y + target.height / 2 - origin.y;
    float distance = sqrtf(dx * dx + dy * dy);
    if (distance > 90) return false;
    if (distance < 0.01f) return true;
    return (dx * boss->attackDirection.x + dy * boss->attackDirection.y) / distance >= 0.55f;
}

static Vector2 RotateDirection(Vector2 direction, float radians)
{
    return (Vector2){ direction.x * cosf(radians) - direction.y * sinf(radians),
                      direction.x * sinf(radians) + direction.y * cosf(radians) };
}

/* ===== 头目行为更新 ===== */
void UpdateBoss(Combat *combat, Enemy *boss, Player *player, const World *world, float seconds)
{
    if (boss->state == ENEMY_PATROL) return;
    if (boss->state == ENEMY_WINDUP) {
        boss->windup -= seconds;
        if (boss->windup <= 0) {
            boss->windup = 0;
            if (boss->attackVariant == 0) {
                if (BossSlashHits(boss, GetPlayerBody(player->position)) &&
                    HasCombatLineOfSight(world, GetEnemyAttackOrigin(boss), player->position)) DamagePlayer(player, 30);
            } else {
                Enemy shot = *boss;
                for (int i = -1; i <= 1; i++) {
                    shot.attackDirection = RotateDirection(boss->attackDirection, (float)i * 0.26f);
                    FireEnemyProjectile(combat, &shot);
                }
            }
            boss->cooldown = 1.6f;
            boss->state = ENEMY_CHASE;
        }
        return;
    }
    Vector2 centre = GetEnemyAttackOrigin(boss);
    Vector2 target = { player->position.x, player->position.y - 7 };
    float dx = target.x - centre.x;
    float dy = target.y - centre.y;
    float distance = sqrtf(dx * dx + dy * dy);
    bool visible = !IsBodyInSafeArea(world, GetPlayerBody(player->position)) && HasCombatLineOfSight(world, centre, target);
    if (visible && distance <= 224 && boss->cooldown <= 0) {
        boss->attackVariant = distance <= 90 && boss->attackVariant != 0 ? 0 : 1;
        boss->attackDirection = distance > 0.01f ? (Vector2){ dx / distance, dy / distance } : (Vector2){ 1, 0 };
        boss->windup = boss->attackVariant == 0 ? 0.85f : 1.0f;
        boss->state = ENEMY_WINDUP;
    } else if (!visible || distance > 70) {
        MoveEnemyTowards(boss, world, boss->lastSeen, 48, seconds, combat, player);
    }
}

/* ===== 攻击预警绘制 ===== */
void DrawBossTelegraph(const Enemy *boss)
{
    Vector2 centre = GetEnemyAttackOrigin(boss);
    if (boss->attackVariant == 0) {
        float angle = atan2f(boss->attackDirection.y, boss->attackDirection.x) * RAD2DEG;
        DrawCircleSector(centre, 90, angle - 56, angle + 56, 24, (Color){ 240, 78, 41, 100 });
        DrawCircleSectorLines(centre, 90, angle - 56, angle + 56, 24, ORANGE);
    } else {
        for (int i = -1; i <= 1; i++) {
            Vector2 direction = RotateDirection(boss->attackDirection, (float)i * 0.26f);
            DrawLineEx(centre, (Vector2){ centre.x + direction.x * 160, centre.y + direction.y * 160 }, 2, ORANGE);
        }
    }
}
