/* 角色与成长：初始属性、升级、伤害、复活、移动及武器朝向。 */
#include "player.h"

#include <math.h>
#include <stddef.h>

/* ===== 初始属性、升级与攻击力 ===== */
void InitializePlayer(Player *player, Vector2 spawn)
{
    player->position = spawn;
    player->swordFacing = (Vector2){ 0.0f, 1.0f };
    player->staffFacing = player->swordFacing;
    player->speed = 3.0f * TILE_SIZE; /* candidate value: 3 tiles/second */
    player->weapon = WEAPON_SWORD;
    player->health = 100;
    player->maxHealth = 100;
    player->hurtTimer = 0;
    player->level = 1;
    player->experience = 0;
    player->gold = 0;
    player->baseAttack = 20;
    player->defeatedGoblins = 0;
    player->meleeDefeated = 0;
    player->rangedDefeated = 0;
    InitializeInventory(&player->inventory);
}

int ExperienceForNextLevel(int level)
{
    if (level < 1 || level >= 5) return 0;
    return 60 + (level - 1) * 40;
}

void GrantPlayerRewards(Player *player, int gold, int experience)
{
    if (gold > 0) player->gold += gold;
    if (experience <= 0 || player->level >= 5) return;
    player->experience += experience;
    while (player->level < 5 && player->experience >= ExperienceForNextLevel(player->level)) {
        player->experience -= ExperienceForNextLevel(player->level);
        player->level++;
        player->maxHealth += 20;
        player->baseAttack += 4;
        if (player->health > 0) player->health += 20; /* no accidental revival on a lethal frame */
    }
    if (player->level == 5) player->experience = 0;
}

int GetPlayerAttack(const Player *player)
{
    ItemId item = GetEquippedItem(&player->inventory, player->weapon);
    return player->baseAttack + GetItemDefinition(item).attackBonus;
}

/* ===== 受伤、碰撞体与复活 ===== */
bool DamagePlayer(Player *player, int damage)
{
    if (player->health <= 0 || damage <= 0 || player->hurtTimer > 0) return false;
    player->health -= damage;
    if (player->health < 0) player->health = 0;
    player->hurtTimer = 0.8f;
    return true;
}

Rectangle GetPlayerBody(Vector2 position)
{
    /* A small footprint under a roughly 32px sprite. */
    return (Rectangle){ position.x - 10.0f, position.y - 14.0f, 20.0f, 14.0f };
}

void RevivePlayer(Player *player)
{
    player->health = player->maxHealth;
    player->hurtTimer = 0;
}

/* ===== 移动与武器朝向 ===== */
static void UpdateFacing(Player *player, Vector2 direction)
{
    player->staffFacing = direction;
    if (direction.x == 0.0f) {
        player->swordFacing = (Vector2){ 0.0f, direction.y > 0.0f ? 1.0f : -1.0f };
    } else if (direction.y == 0.0f) {
        player->swordFacing = (Vector2){ direction.x > 0.0f ? 1.0f : -1.0f, 0.0f };
    } else {
        /* Preserve the previous cardinal direction if it is part of this diagonal.
           Otherwise use the vertical component, a deterministic tie-break. */
        bool matches = player->swordFacing.x * direction.x > 0.0f ||
                       player->swordFacing.y * direction.y > 0.0f;
        if (!matches) {
            player->swordFacing = (Vector2){ 0.0f, direction.y > 0.0f ? 1.0f : -1.0f };
        }
    }
}

void MovePlayer(Player *player, const World *world, Vector2 input, float seconds)
{
    MovePlayerWithObstacles(player, world, input, seconds, NULL, 0);
}

void MovePlayerWithObstacles(Player *player, const World *world, Vector2 input, float seconds,
                             const Rectangle obstacles[], int count)
{
    float length = sqrtf(input.x * input.x + input.y * input.y);
    if (length == 0.0f || seconds <= 0.0f) return;

    Vector2 direction = { input.x / length, input.y / length };
    UpdateFacing(player, direction);
    Vector2 movement = { direction.x * player->speed * seconds,
                         direction.y * player->speed * seconds };
    float largest = fmaxf(fabsf(movement.x), fabsf(movement.y));
    int steps = (int)ceilf(largest / 4.0f);
    if (steps < 1) steps = 1;

    /* Small steps prevent skipping a thin obstacle after a long frame.
       Try each axis separately so the player can slide along a wall. */
    for (int i = 0; i < steps; i++) {
        Vector2 candidate = player->position;
        candidate.x += movement.x / (float)steps;
        if (CanMoveWithObstacles(world, GetPlayerBody(player->position), GetPlayerBody(candidate), obstacles, count))
            player->position.x = candidate.x;
        candidate = player->position;
        candidate.y += movement.y / (float)steps;
        if (CanMoveWithObstacles(world, GetPlayerBody(player->position), GetPlayerBody(candidate), obstacles, count))
            player->position.y = candidate.y;
    }
}
