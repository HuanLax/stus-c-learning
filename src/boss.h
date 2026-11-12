#ifndef PROJECT_LAX_BOSS_H
#define PROJECT_LAX_BOSS_H

#include "combat.h"

bool BossSlashHits(const Enemy *boss, Rectangle target);
void UpdateBoss(Combat *combat, Enemy *boss, Player *player, const World *world, float seconds);
void DrawBossTelegraph(const Enemy *boss);

#endif
