/* 存档读写：文本格式、数据验证、旧档迁移及临时文件替换。 */
#include "save.h"

#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Keep Windows declarations from colliding with raylib names. */
#define WIN32_LEAN_AND_MEAN
#define CloseWindow WinCloseWindow
#define ShowCursor WinShowCursor
#define DrawText WinDrawText
#define Rectangle WinRectangle
#include <windows.h>
#undef CloseWindow
#undef ShowCursor
#undef DrawText
#undef Rectangle

/* ===== 基础字段解析与现场序列化 ===== */
static bool ReadLabel(FILE *file, const char *expected)
{
    char token[128];
    return fscanf(file, "%127s", token) == 1 && strcmp(token, expected) == 0;
}

static bool ReadInteger(FILE *file, int *output)
{
    char token[128], *end;
    if (fscanf(file, "%127s", token) != 1) return false;
    errno = 0;
    long value = strtol(token, &end, 10);
    if (errno || end == token || *end || value < INT_MIN || value > INT_MAX) return false;
    *output = (int)value;
    return true;
}

static bool ReadNumber(FILE *file, float *output)
{
    char token[128], *end;
    if (fscanf(file, "%127s", token) != 1) return false;
    errno = 0;
    double value = strtod(token, &end);
    if (errno || end == token || *end || !isfinite(value) || value < -1000000 || value > 1000000) return false;
    *output = (float)value;
    return true;
}

static bool ReadCoordinate(FILE *file, float *output)
{
    return ReadNumber(file, output) && *output >= 0 && *output <= 4096;
}

/* One field order for reading and writing avoids two diverging snapshot formats. */
typedef struct SaveIo { FILE *file; bool reading; bool good; } SaveIo;

static void Label(SaveIo *io, const char *name)
{
    if (!io->good) return;
    io->good = io->reading ? ReadLabel(io->file, name) : fprintf(io->file, "\n%s", name) > 0;
}

static int Integer(SaveIo *io, int value)
{
    if (io->good) io->good = io->reading ? ReadInteger(io->file, &value) : fprintf(io->file, " %d", value) > 0;
    return value;
}

static float Number(SaveIo *io, float value)
{
    if (io->good) io->good = io->reading ? ReadNumber(io->file, &value) : fprintf(io->file, " %.9g", (double)value) > 0;
    return value;
}

static bool Flag(SaveIo *io, bool value)
{
    int flag = Integer(io, value);
    if (flag != 0 && flag != 1) io->good = false;
    return flag != 0;
}

static void Vector(SaveIo *io, Vector2 *value)
{
    value->x = Number(io, value->x);
    value->y = Number(io, value->y);
}

static void Box(SaveIo *io, Rectangle *value)
{
    value->x = Number(io, value->x); value->y = Number(io, value->y);
    value->width = Number(io, value->width); value->height = Number(io, value->height);
}

static void EnemyFields(SaveIo *io, Enemy *enemy)
{
    enemy->kind = (EnemyKind)Integer(io, enemy->kind);
    enemy->state = (EnemyState)Integer(io, enemy->state);
    Vector(io, &enemy->spawn); Vector(io, &enemy->position);
    enemy->health = Integer(io, enemy->health); enemy->maxHealth = Integer(io, enemy->maxHealth);
    enemy->cooldown = Number(io, enemy->cooldown); enemy->windup = Number(io, enemy->windup);
    enemy->flashTimer = Number(io, enemy->flashTimer); Box(io, &enemy->attackArea);
    Vector(io, &enemy->lastSeen); Vector(io, &enemy->attackDirection); Vector(io, &enemy->facing);
    Vector(io, &enemy->waypoint); enemy->pathTimer = Number(io, enemy->pathTimer);
    enemy->goalCol = Integer(io, enemy->goalCol); enemy->goalRow = Integer(io, enemy->goalRow);
    enemy->patrolPoint = Integer(io, enemy->patrolPoint); enemy->patrolPause = Number(io, enemy->patrolPause);
    enemy->engaged = Flag(io, enemy->engaged); enemy->rewardGranted = Flag(io, enemy->rewardGranted);
    enemy->attackVariant = Integer(io, enemy->attackVariant);
}

static bool SnapshotFields(SaveIo *io, SaveData *data)
{
    Combat *c = &data->combat;
    Label(io, "snapshot");
    data->screen = (AppScreen)Integer(io, data->screen);
    data->dialoguePage = Integer(io, data->dialoguePage);
    data->interactionText = (TextId)Integer(io, data->interactionText);
    data->deathReturnToTown = Flag(io, data->deathReturnToTown);
    data->nearbyInteraction = Integer(io, data->nearbyInteraction);
    data->selectedSlot = Integer(io, data->selectedSlot); data->selectedProduct = Integer(io, data->selectedProduct);
    data->inventoryMessage = (TextId)Integer(io, data->inventoryMessage);
    data->rewardNoticeTimer = Number(io, data->rewardNoticeTimer);
    data->rewardNoticeKills = Integer(io, data->rewardNoticeKills);
    Label(io, "motion");
    Vector(io, &data->player.swordFacing); Vector(io, &data->player.staffFacing);
    data->player.hurtTimer = Number(io, data->player.hurtTimer);
    Label(io, "combat");
    c->enemyCount = Integer(io, c->enemyCount);
    if (c->enemyCount < 0 || c->enemyCount > ENEMY_CAPACITY) { io->good = false; return false; }
    c->inCombat = Flag(io, c->inCombat); c->wasSafe = Flag(io, c->wasSafe);
    c->lostTimer = Number(io, c->lostTimer); c->cooldown = Number(io, c->cooldown);
    c->swingTimer = Number(io, c->swingTimer); c->swingDamage = Integer(io, c->swingDamage);
    Vector(io, &c->swingFacing);
    Label(io, "hits");
    for (int i = 0; i < ENEMY_CAPACITY; i++) c->swingHit[i] = Flag(io, c->swingHit[i]);
    for (int i = 0; i < c->enemyCount; i++) {
        Label(io, "enemy");
        if (Integer(io, i) != i) io->good = false;
        EnemyFields(io, &c->enemies[i]);
    }
    for (int i = 0; i < PROJECTILE_CAPACITY; i++) {
        Projectile *shot = &c->projectiles[i];
        Label(io, "shot");
        if (Integer(io, i) != i) io->good = false;
        shot->active = Flag(io, shot->active);
        Vector(io, &shot->position); Vector(io, &shot->direction);
        shot->remainingDistance = Number(io, shot->remainingDistance);
        shot->hostile = Flag(io, shot->hostile); shot->damage = Integer(io, shot->damage);
    }
    Label(io, "end_snapshot");
    return io->good;
}

/* ===== 存档数据与状态关系验证 ===== */
static bool Between(float value, float minimum, float maximum)
{
    return isfinite(value) && value >= minimum && value <= maximum;
}

static bool ValidVector(Vector2 value)
{
    return Between(value.x, -4096, 4096) && Between(value.y, -4096, 4096);
}

static bool Direction(Vector2 value, bool allowZero)
{
    if (!ValidVector(value)) return false;
    float length = value.x * value.x + value.y * value.y;
    return (allowZero && length == 0) || fabsf(length - 1) < 0.001f;
}

static bool ValidEnemy(const Enemy *e, const World *world)
{
    if (e->kind < ENEMY_MELEE || e->kind > ENEMY_BOSS || e->state < ENEMY_PATROL || e->state > ENEMY_DEAD ||
        e->maxHealth != (e->kind == ENEMY_BOSS ? 300 : 60) || e->health < 0 || e->health > e->maxHealth ||
        (e->state == ENEMY_DEAD) != (e->health == 0) || e->rewardGranted != (e->health == 0) ||
        !ValidVector(e->spawn) || !ValidVector(e->position) || !ValidVector(e->lastSeen) || !ValidVector(e->waypoint) ||
        !Direction(e->attackDirection, e->state != ENEMY_WINDUP) || !Direction(e->facing, true) ||
        !Between(e->cooldown, 0, 60) || !Between(e->windup, 0, 60) || !Between(e->flashTimer, 0, 60) ||
        !Between(e->pathTimer, -0.1f, 60) || !Between(e->patrolPause, -0.1f, 60) ||
        e->patrolPoint < 0 || e->patrolPoint >= 4 || e->goalCol < -4 || e->goalCol > world->columns + 4 ||
        e->goalRow < -4 || e->goalRow > world->rows + 4 || e->attackVariant < 0 || e->attackVariant > 1 ||
        !Between(e->attackArea.x, -512, 4096) || !Between(e->attackArea.y, -512, 4096) ||
        !Between(e->attackArea.width, 0, 512) || !Between(e->attackArea.height, 0, 512)) return false;
    Rectangle body = GetEnemyCollision(e);
    return CanStandInWorld(world, body) && !CheckCollisionRecs(body, world->safeArea);
}

static bool ValidSnapshot(const SaveData *data, const World *world)
{
    const Combat *c = &data->combat;
    if (data->screen < SCREEN_PLAY || data->screen > SCREEN_COMPLETE ||
        (data->screen == SCREEN_DEATH) != (data->player.health == 0) ||
        data->dialoguePage < 0 || data->dialoguePage >= QuestDialoguePages(&data->quest) ||
        data->selectedSlot < 0 || data->selectedSlot >= INVENTORY_CAPACITY || data->selectedProduct < 0 || data->selectedProduct > 1 ||
        data->nearbyInteraction < -1 || data->nearbyInteraction >= world->interactionCount ||
        data->interactionText < 0 || data->interactionText >= TEXT_COUNT ||
        data->inventoryMessage < 0 || data->inventoryMessage >= TEXT_COUNT ||
        strchr(GetUiText(LANGUAGE_ENGLISH, data->inventoryMessage), '%') ||
        strchr(GetUiText(LANGUAGE_CHINESE, data->inventoryMessage), '%') ||
        !Between(data->rewardNoticeTimer, 0, 60) || data->rewardNoticeKills < 0 || data->rewardNoticeKills > ENEMY_CAPACITY ||
        !Direction(data->player.swordFacing, false) || !Direction(data->player.staffFacing, false) ||
        (data->player.swordFacing.x != 0 && data->player.swordFacing.y != 0) || !Between(data->player.hurtTimer, 0, 10) ||
        c->enemyCount < 0 || c->enemyCount > ENEMY_CAPACITY || !Between(c->cooldown, 0, 60) ||
        !Between(c->swingTimer, 0, 60) || !Between(c->lostTimer, 0, c->rules.disengageDelay) ||
        c->swingDamage < 0 || c->swingDamage > 1000 || !Direction(c->swingFacing, c->swingTimer == 0)) return false;
    if ((data->screen == SCREEN_SHOP || data->screen == SCREEN_QUEST) && data->map != MAP_TOWN) return false;
    if (data->screen == SCREEN_INTERACTION && data->nearbyInteraction < 0) return false;
    if (data->screen == SCREEN_COMPLETE && data->quest.stage != QUEST_COMPLETE) return false;
    if (data->screen == SCREEN_BOSS_VICTORY && !data->quest.bossDefeated) return false;
    Combat template;
    InitializeCombat(&template, data->map);
    int expected = data->map == MAP_BOSS && data->quest.bossDefeated ? c->enemyCount : template.enemyCount;
    if (c->enemyCount != expected || c->enemyCount > template.enemyCount || (data->map == MAP_TOWN && c->inCombat)) return false;
    for (int i = 0; i < c->enemyCount; i++) {
        const Enemy *e = &c->enemies[i], *original = &template.enemies[i];
        if (!ValidEnemy(e, world) || e->kind != original->kind || e->spawn.x != original->spawn.x || e->spawn.y != original->spawn.y ||
            (e->kind == ENEMY_BOSS && e->health == 0 && !data->quest.bossDefeated)) return false;
    }
    for (int i = 0; i < PROJECTILE_CAPACITY; i++) {
        const Projectile *shot = &c->projectiles[i];
        if (!ValidVector(shot->position) || !Direction(shot->direction, !shot->active) ||
            !Between(shot->remainingDistance, -0.1f, 384) || shot->damage < 0 || shot->damage > 1000) return false;
        if (shot->active && (shot->damage == 0 || shot->remainingDistance <= 0 ||
            !CanStandInWorld(world, (Rectangle){ shot->position.x - 4, shot->position.y - 4, 8, 8 }))) return false;
    }
    return true;
}

bool ValidateSave(const SaveData *data, const World worlds[MAP_COUNT])
{
    const Player *p = &data->player;
    const Quest *q = &data->quest;
    const Inventory *bag = &p->inventory;
    if (data->map < MAP_TOWN || data->map >= MAP_COUNT || data->safeMap < MAP_TOWN || data->safeMap >= MAP_COUNT ||
        data->language < LANGUAGE_ENGLISH || data->language >= LANGUAGE_COUNT ||
        q->stage < QUEST_OFFER || q->stage > QUEST_COMPLETE) return false;
    if (p->level < 1 || p->level > 5 || p->maxHealth != 100 + 20 * (p->level - 1) ||
        p->baseAttack != 20 + 4 * (p->level - 1) || p->health < (data->hasSnapshot ? 0 : 1) || p->health > p->maxHealth ||
        p->experience < 0 || (p->level == 5 ? p->experience != 0 : p->experience >= ExperienceForNextLevel(p->level)) ||
        p->gold < 0 || p->gold > PLAYER_GOLD_LIMIT || p->weapon < WEAPON_SWORD || p->weapon > WEAPON_STAFF) return false;
    if (p->meleeDefeated < 0 || p->rangedDefeated < 0 ||
        (long long)p->defeatedGoblins != (long long)p->meleeDefeated + p->rangedDefeated ||
        q->startingMelee < 0 || q->startingMelee > p->meleeDefeated ||
        q->startingRanged < 0 || q->startingRanged > p->rangedDefeated) return false;
    bool cleared = QuestMeleeProgress(q, p) == 4 && QuestRangedProgress(q, p) == 2;
    if ((q->stage == QUEST_OFFER && (q->startingMelee || q->startingRanged)) ||
        (q->stage == QUEST_CLEAR_FIELD && cleared) || (q->stage >= QUEST_REPORT_FIELD && !cleared) ||
        q->bossDefeated != (q->stage >= QUEST_REPORT_BOSS) ||
        ((data->map == MAP_BOSS || data->safeMap == MAP_BOSS) && !IsBossUnlocked(q))) return false;
    if (!isfinite(p->position.x) || !isfinite(p->position.y) ||
        !CanStandInWorld(&worlds[data->map], GetPlayerBody(p->position)) ||
        !isfinite(data->safePosition.x) || !isfinite(data->safePosition.y) ||
        data->safePosition.x != worlds[data->safeMap].spawn.x || data->safePosition.y != worlds[data->safeMap].spawn.y ||
        !IsBodyInSafeArea(&worlds[data->safeMap], GetPlayerBody(data->safePosition))) return false;
    int counts[ITEM_COUNT] = { 0 };
    for (int i = 0; i < INVENTORY_CAPACITY; i++) {
        if (bag->slots[i] < ITEM_NONE || bag->slots[i] >= ITEM_COUNT) return false;
        counts[bag->slots[i]]++;
    }
    if (counts[ITEM_IRON_SWORD] != 1 || counts[ITEM_WOOD_STAFF] != 1 ||
        counts[ITEM_STEEL_SWORD] != (int)bag->purchasedSword || counts[ITEM_RUNE_STAFF] != (int)bag->purchasedStaff ||
        bag->equippedSword < 0 || bag->equippedSword >= INVENTORY_CAPACITY ||
        bag->equippedStaff < 0 || bag->equippedStaff >= INVENTORY_CAPACITY) return false;
    ItemId sword = bag->slots[bag->equippedSword], staff = bag->slots[bag->equippedStaff];
    return (sword == ITEM_IRON_SWORD || sword == ITEM_STEEL_SWORD) &&
           (staff == ITEM_WOOD_STAFF || staff == ITEM_RUNE_STAFF) &&
           (!data->hasSnapshot || ValidSnapshot(data, &worlds[data->map]));
}

/* Called only after validation against the earlier, obstacle-free field. */
/* ===== 旧版地图存档迁移 ===== */
static void MoveOffFieldScenery(const World *world, Vector2 *position, Rectangle body, bool enemy)
{
    if (CanStandInWorld(world, body)) return;
    float closest = INFINITY;
    Vector2 nearest = *position;
    for (int row = 1; row < world->rows - 1; row++) {
        for (int col = 1; col < world->columns - 1; col++) {
            Vector2 candidate = { col * TILE_SIZE + 16, row * TILE_SIZE + 16 };
            float dx = candidate.x - position->x, dy = candidate.y - position->y;
            Rectangle moved = { body.x + dx, body.y + dy, body.width, body.height };
            float distance = dx * dx + dy * dy;
            if (distance < closest && CanStandInWorld(world, moved) &&
                (!enemy || !CheckCollisionRecs(moved, world->safeArea))) {
                closest = distance;
                nearest = candidate;
            }
        }
    }
    *position = nearest;
}

static void MigrateFieldScenery(SaveData *data, const World worlds[MAP_COUNT])
{
    if (data->map != MAP_FIELD || ValidateSave(data, worlds)) return;
    World previous[MAP_COUNT];
    memcpy(previous, worlds, sizeof(previous));
    previous[MAP_FIELD].sceneryCount = 0;
    /* Reject malformed saves even when they also overlap a new obstacle. */
    if (!ValidateSave(data, previous)) return;
    const World *field = &worlds[MAP_FIELD];
    MoveOffFieldScenery(field, &data->player.position, GetPlayerBody(data->player.position), false);
    if (!data->hasSnapshot) return;
    for (int i = 0; i < data->combat.enemyCount; i++) {
        Enemy *enemy = &data->combat.enemies[i];
        MoveOffFieldScenery(field, &enemy->position, GetEnemyCollision(enemy), true);
    }
    for (int i = 0; i < PROJECTILE_CAPACITY; i++) {
        Projectile *shot = &data->combat.projectiles[i];
        if (shot->active && !CanStandInWorld(field,
            (Rectangle){ shot->position.x - 4, shot->position.y - 4, 8, 8 })) shot->active = false;
    }
}

/* ===== 读取、迁移并提交存档 ===== */
SaveResult ReadSave(const char *path, const World worlds[MAP_COUNT], SaveData *output)
{
    FILE *file = fopen(path, "rb");
    if (!file) return errno == ENOENT ? SAVE_MISSING : SAVE_IO_ERROR;
    if (fseek(file, 0, SEEK_END) != 0) { fclose(file); return SAVE_IO_ERROR; }
    long size = ftell(file);
    if (size < 0 || size > 65536 || fseek(file, 0, SEEK_SET) != 0) { fclose(file); return SAVE_INVALID; }
    SaveData data = { 0 };
    InitializePlayer(&data.player, worlds[MAP_TOWN].spawn);
    int version, map, language, weapon, stage, defeated, safeMap, swordBought, staffBought;
    bool valid = ReadLabel(file, "PROJECT_LAX") && ReadInteger(file, &version) && (version == 1 || version == 2) &&
        ReadLabel(file, "location") && ReadInteger(file, &map) &&
        ReadCoordinate(file, &data.player.position.x) && ReadCoordinate(file, &data.player.position.y) &&
        ReadLabel(file, "player") && ReadInteger(file, &data.player.health) && ReadInteger(file, &data.player.level) &&
        ReadInteger(file, &data.player.experience) && ReadInteger(file, &data.player.gold) && ReadInteger(file, &weapon) &&
        ReadLabel(file, "kills") && ReadInteger(file, &data.player.defeatedGoblins) &&
        ReadInteger(file, &data.player.meleeDefeated) && ReadInteger(file, &data.player.rangedDefeated) &&
        ReadLabel(file, "quest") && ReadInteger(file, &stage) && ReadInteger(file, &data.quest.startingMelee) &&
        ReadInteger(file, &data.quest.startingRanged) && ReadInteger(file, &defeated) &&
        ReadLabel(file, "equipment") && ReadInteger(file, &data.player.inventory.equippedSword) &&
        ReadInteger(file, &data.player.inventory.equippedStaff) && ReadInteger(file, &swordBought) && ReadInteger(file, &staffBought) &&
        ReadLabel(file, "bag");
    for (int i = 0; i < INVENTORY_CAPACITY && valid; i++) {
        int item;
        valid = ReadInteger(file, &item);
        if (valid) data.player.inventory.slots[i] = (ItemId)item;
    }
    valid = valid && ReadLabel(file, "language") && ReadInteger(file, &language) &&
        ReadLabel(file, "safe") && ReadInteger(file, &safeMap) &&
        ReadCoordinate(file, &data.safePosition.x) && ReadCoordinate(file, &data.safePosition.y);
    if (valid) valid = (defeated == 0 || defeated == 1) && (swordBought == 0 || swordBought == 1) &&
        (staffBought == 0 || staffBought == 1) && data.player.level >= 1 && data.player.level <= 5 && map >= MAP_TOWN && map < MAP_COUNT;
    if (valid) {
        data.map = (MapId)map;
        data.language = (Language)language;
        data.safeMap = (MapId)safeMap;
        data.player.weapon = (Weapon)weapon;
        data.quest.stage = (QuestStage)stage;
        data.quest.bossDefeated = defeated != 0;
        data.player.inventory.purchasedSword = swordBought != 0;
        data.player.inventory.purchasedStaff = staffBought != 0;
        data.player.maxHealth = 100 + 20 * (data.player.level - 1);
        data.player.baseAttack = 20 + 4 * (data.player.level - 1);
        data.hasSnapshot = version == 2;
        if (data.hasSnapshot) {
            InitializeCombat(&data.combat, data.map);
            SaveIo io = { file, true, true };
            valid = SnapshotFields(&io, &data);
        }
    }
    char extra[2];
    valid = valid && ReadLabel(file, "END") && fscanf(file, "%1s", extra) == EOF && !ferror(file);
    if (fclose(file) != 0) return SAVE_IO_ERROR;
    if (!valid) return SAVE_INVALID;
    MigrateFieldScenery(&data, worlds);
    /* Earlier town layouts let players stand inside decorative scenery.
       Move only those overlaps to the nearest walkable tile centre. Other
       invalid coordinates still fail validation, and the file is not rewritten. */
    if (data.map == MAP_TOWN && isfinite(data.player.position.x) && isfinite(data.player.position.y)) {
        const World *town = &worlds[MAP_TOWN];
        Rectangle body = GetPlayerBody(data.player.position);
        bool inScenery = false;
        for (int i = 0; i < town->sceneryCount; i++) {
            if (CheckCollisionRecs(body, town->scenery[i].collision)) inScenery = true;
        }
        if (inScenery && !CanStandInWorld(town, body) && body.x >= TILE_SIZE && body.y >= TILE_SIZE &&
            body.x + body.width <= (town->columns - 1) * TILE_SIZE &&
            body.y + body.height <= (town->rows - 1) * TILE_SIZE) {
            float closest = INFINITY;
            Vector2 nearest = data.player.position;
            for (int row = 1; row < town->rows - 1; row++) {
                for (int col = 1; col < town->columns - 1; col++) {
                    Vector2 candidate = { col * TILE_SIZE + 16, row * TILE_SIZE + 16 };
                    float dx = candidate.x - data.player.position.x, dy = candidate.y - data.player.position.y;
                    float distance = dx * dx + dy * dy;
                    if (distance < closest && CanStandInWorld(town, GetPlayerBody(candidate))) {
                        closest = distance;
                        nearest = candidate;
                    }
                }
            }
            data.player.position = nearest;
        }
    }
    /* A v1 player could stand on a healing stone before it became solid.
       Migrate only that known layout change, never arbitrary invalid positions. */
    if (!data.hasSnapshot && !CanStandInWorld(&worlds[data.map], GetPlayerBody(data.player.position))) {
        const World *world = &worlds[data.map];
        for (int i = 0; i < world->interactionCount; i++) {
            Interaction object = world->interactions[i];
            if (object.kind != INTERACTION_HEAL || !CheckCollisionRecs(GetPlayerBody(data.player.position), GetInteractionBody(object))) continue;
            const Vector2 offsets[] = { {0,32}, {32,0}, {0,-32}, {-32,0} };
            for (int j = 0; j < 4; j++) {
                Vector2 candidate = { data.player.position.x + offsets[j].x, data.player.position.y + offsets[j].y };
                if (CanStandInWorld(world, GetPlayerBody(candidate)) && IsBodyInSafeArea(world, GetPlayerBody(candidate))) {
                    data.player.position = candidate;
                    break;
                }
            }
            break;
        }
    }
    if (!ValidateSave(&data, worlds)) return SAVE_INVALID;
    *output = data; /* Commit only after parsing and every relationship check. */
    return SAVE_OK;
}

/* ===== 写入临时文件并替换存档 ===== */
SaveResult WriteSave(const char *path, const World worlds[MAP_COUNT], const SaveData *data)
{
    if (!ValidateSave(data, worlds)) return SAVE_INVALID;
    char temporary[1200];
    int length = snprintf(temporary, sizeof(temporary), "%s.tmp", path);
    if (length < 0 || (size_t)length >= sizeof(temporary)) return SAVE_IO_ERROR;
    FILE *file = fopen(temporary, "wb");
    if (!file) return SAVE_IO_ERROR;
    const Player *p = &data->player;
    const Quest *q = &data->quest;
    const Inventory *bag = &p->inventory;
    bool good = fprintf(file, "PROJECT_LAX %d\nlocation %d %.9g %.9g\nplayer %d %d %d %d %d\n"
        "kills %d %d %d\nquest %d %d %d %d\nequipment %d %d %d %d\nbag",
        data->hasSnapshot ? 2 : 1, data->map, (double)p->position.x, (double)p->position.y, p->health, p->level, p->experience, p->gold, p->weapon,
        p->defeatedGoblins, p->meleeDefeated, p->rangedDefeated, q->stage, q->startingMelee, q->startingRanged,
        q->bossDefeated, bag->equippedSword, bag->equippedStaff, bag->purchasedSword, bag->purchasedStaff) > 0;
    for (int i = 0; i < INVENTORY_CAPACITY; i++) if (fprintf(file, " %d", bag->slots[i]) < 0) good = false;
    if (fprintf(file, "\nlanguage %d\nsafe %d %.9g %.9g", data->language, data->safeMap,
        (double)data->safePosition.x, (double)data->safePosition.y) < 0) good = false;
    if (data->hasSnapshot && good) {
        SaveData copy = *data;
        SaveIo io = { file, false, true };
        good = SnapshotFields(&io, &copy);
    }
    if (fputs("\nEND\n", file) == EOF) good = false;
    if (fflush(file) != 0) good = false;
    if (fclose(file) != 0) good = false;
    /* Replace only after complete output: a failed replacement leaves the old save intact. */
    if (!good || !MoveFileExA(temporary, path, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        remove(temporary);
        return SAVE_IO_ERROR;
    }
    return SAVE_OK;
}
