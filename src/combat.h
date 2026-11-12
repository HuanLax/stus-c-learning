#ifndef PROJECT_LAX_COMBAT_H
#define PROJECT_LAX_COMBAT_H

#include "player.h"

enum { ENEMY_CAPACITY = WORLD_ACTOR_CAPACITY, PROJECTILE_CAPACITY = 16 };
enum { GOBLIN_GOLD = 25, GOBLIN_EXPERIENCE = 30 };
enum { RANGED_PROJECTILE_RANGE = 8 * TILE_SIZE }; /* Staff and ordinary ranged goblins. */

typedef enum EnemyKind { ENEMY_MELEE, ENEMY_RANGED, ENEMY_BOSS } EnemyKind;
typedef enum EnemyState { ENEMY_PATROL, ENEMY_CHASE, ENEMY_WINDUP, ENEMY_RETURN, ENEMY_DEAD } EnemyState;

typedef struct CombatRules {
    int enemyDamage;
    float attackCooldown;
    float swingDuration;
    float swordReach;
    float swordWidth;
    float projectileSpeed;
    float projectileRange;
    float enemyReach;
    float enemyWindup;
    float enemyCooldown;
    float enemySpeed;
    float detectionRange;
    float disengageDelay;
} CombatRules;

typedef struct Enemy {
    EnemyKind kind;
    EnemyState state;
    Vector2 spawn;
    Vector2 position;
    int health;
    int maxHealth;
    float cooldown;
    float windup;
    float flashTimer;
    Rectangle attackArea;
    Vector2 lastSeen;
    Vector2 attackDirection;
    Vector2 facing; /* Drawing only; never changes a locked attack direction. */
    Vector2 waypoint;
    float pathTimer;
    int goalCol;
    int goalRow;
    int patrolPoint;
    float patrolPause;
    bool engaged;
    bool rewardGranted;
    int attackVariant; /* boss: 0 slash, 1 scatter */
} Enemy;

typedef struct Projectile {
    bool active;
    Vector2 position;
    Vector2 direction;
    float remainingDistance;
    bool hostile;
    int damage;
} Projectile;

typedef struct Combat {
    CombatRules rules;
    Enemy enemies[ENEMY_CAPACITY];
    int enemyCount;
    Projectile projectiles[PROJECTILE_CAPACITY];
    float cooldown;
    float swingTimer;
    int swingDamage;
    Vector2 swingFacing;
    bool swingHit[ENEMY_CAPACITY];
    bool inCombat;
    bool wasSafe;
    float lostTimer;
} Combat;

void InitializeCombat(Combat *combat, MapId map);
const char *GetEnemyName(EnemyKind kind, Language language);
Rectangle GetEnemyBody(Vector2 feet);
Rectangle GetEnemyBodyForKind(Vector2 feet, EnemyKind kind);
Rectangle GetEnemyCollision(const Enemy *enemy);
Vector2 GetEnemyAttackOrigin(const Enemy *enemy);
Rectangle GetSwordArea(const Combat *combat, const Player *player);
bool HasCombatLineOfSight(const World *world, Vector2 from, Vector2 to);
bool StartPlayerAttack(Combat *combat, const Player *player, const World *world);
bool GetAttackIntent(bool *armed, bool held, bool worldReady);
void UpdateCombat(Combat *combat, Player *player, const World *world, float seconds, bool attackHeld);
void DrawCombat(const Combat *combat, const Player *player);
void FireEnemyProjectile(Combat *combat, const Enemy *enemy);
int ClaimEnemyRewards(Combat *combat, Player *player);

#endif
