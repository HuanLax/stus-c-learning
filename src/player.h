#ifndef PROJECT_LAX_PLAYER_H
#define PROJECT_LAX_PLAYER_H

#include "world.h"
#include "inventory.h"

enum { PLAYER_GOLD_LIMIT = 1000000000 };

typedef struct Player {
    Vector2 position; /* feet centre, not the top-left of the picture */
    Vector2 swordFacing;
    Vector2 staffFacing;
    float speed;
    Weapon weapon;
    int health;
    int maxHealth;
    float hurtTimer;
    int level;
    int experience;
    int gold;
    int baseAttack;
    int defeatedGoblins;
    int meleeDefeated;
    int rangedDefeated;
    Inventory inventory;
} Player;

void InitializePlayer(Player *player, Vector2 spawn);
Rectangle GetPlayerBody(Vector2 position);
void MovePlayer(Player *player, const World *world, Vector2 input, float seconds);
void MovePlayerWithObstacles(Player *player, const World *world, Vector2 input, float seconds,
                             const Rectangle obstacles[], int count);
bool DamagePlayer(Player *player, int damage);
void RevivePlayer(Player *player);
int ExperienceForNextLevel(int level);
void GrantPlayerRewards(Player *player, int gold, int experience);
int GetPlayerAttack(const Player *player);

#endif
