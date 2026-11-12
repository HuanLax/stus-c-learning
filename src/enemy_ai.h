#ifndef PROJECT_LAX_ENEMY_AI_H
#define PROJECT_LAX_ENEMY_AI_H

#include "combat.h"

bool CanEnemyStand(const World *world, Vector2 position);
Vector2 FindEnemyWaypoint(const World *world, Vector2 start, Vector2 goal);
void MoveEnemyTowards(Enemy *enemy, const World *world, Vector2 goal, float speed, float seconds,
                      const Combat *combat, const Player *player);
void UpdateEnemyAI(Combat *combat, Player *player, const World *world, float seconds);

#endif
