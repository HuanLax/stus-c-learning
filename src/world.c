/* 地图与场景：地图布局、碰撞、安全区、传送、镜头和场景绘制。 */
#include "world.h"

#include <math.h>

/* ===== 地图数据与场景初始化 ===== */
static void AddScenery(World *world, SceneryKind kind, Rectangle collision, Color color)
{
    if (world->sceneryCount >= SCENERY_CAPACITY) return;
    SpriteId sprite = kind == SCENERY_HOUSE ? (world->sceneryCount % 2 == 0 ? SPRITE_HOUSE_LEADER : SPRITE_HOUSE_SELLER) : SPRITE_TREE;
    if (kind == SCENERY_ROCK) sprite = world->id != MAP_BOSS ? SPRITE_ROCK :
        world->sceneryCount == 0 ? SPRITE_STALAGMITES : SPRITE_RUBBLE;
    if (kind == SCENERY_LAMP) sprite = SPRITE_LAMP;
    if (kind == SCENERY_FENCE || kind == SCENERY_WELL || kind == SCENERY_FLOWERBED) sprite = SPRITE_COUNT;
    world->scenery[world->sceneryCount++] = (Scenery){ kind, collision, color, sprite };
}

static void AddInteraction(World *world, Interaction object)
{
    if (world->interactionCount >= INTERACTION_CAPACITY) return;
    world->interactions[world->interactionCount++] = object;
}

/* Measurements use the unrotated 1254x1254 town artwork. */
static void AddTownScenery(World *world, SceneryKind kind, Rectangle pixels, Color color)
{
    float scale = (float)(world->columns * TILE_SIZE) / 1254.0f;
    Rectangle collision = { pixels.x * scale, pixels.y * scale, pixels.width * scale, pixels.height * scale };
    AddScenery(world, kind, collision, color);
}

static void InitializeTown(World *world)
{
    /* These objects are already painted into map_main1.png. Keep collision
       records, but draw their fallbacks only when that background is missing.
       Whole house silhouettes are solid: a flat image has no separate roofs. */
    AddTownScenery(world, SCENERY_HOUSE, (Rectangle){ 60, 8, 420, 352 }, (Color){ 124, 69, 58, 255 });
    AddTownScenery(world, SCENERY_HOUSE, (Rectangle){ 784, 12, 392, 352 }, (Color){ 74, 90, 125, 255 });
    AddTownScenery(world, SCENERY_HOUSE, (Rectangle){ 45, 786, 425, 351 }, (Color){ 136, 99, 55, 255 });
    AddTownScenery(world, SCENERY_HOUSE, (Rectangle){ 780, 795, 396, 344 }, (Color){ 97, 66, 105, 255 });
    const Vector2 lamps[] = {
        {489,416}, {768,416}, {357,750}, {899,754}
    };
    for (int i = 0; i < 4; i++) {
        float x = lamps[i].x * (TOWN_COLUMNS * TILE_SIZE / 1254.0f);
        float y = lamps[i].y * (TOWN_ROWS * TILE_SIZE / 1254.0f);
        AddScenery(world, SCENERY_LAMP, (Rectangle){ x - 6, y - 6, 12, 12 }, GOLD);
    }
    const Rectangle fences[] = {
        {352,440,146,34}, {768,440,134,34}, {350,782,148,38}, {768,790,134,34},
        {42,477,166,15}, {1054,477,166,15}, {42,744,166,15}, {1054,744,166,15},
        {540,165,15,138}, {701,165,15,138}, {535,900,17,141}, {701,900,17,141}
    };
    for (int i = 0; i < 12; i++) AddTownScenery(world, SCENERY_FENCE, fences[i], BROWN);
    AddTownScenery(world, SCENERY_WELL, (Rectangle){ 575,500,104,162 }, GRAY);
    const Rectangle flowerbeds[] = {
        {382,480,82,61}, {788,480,86,61}, {391,726,83,52}, {784,728,88,55},
        {148,376,88,47}, {1141,342,85,58}, {366,1098,112,55}, {1052,1106,92,50}
    };
    for (int i = 0; i < 8; i++) AddTownScenery(world, SCENERY_FLOWERBED, flowerbeds[i], DARKGREEN);

    AddInteraction(world, (Interaction){ .kind = INTERACTION_QUEST, .position = { 400, 464 } });
    AddInteraction(world, (Interaction){ .kind = INTERACTION_SHOP, .position = { 592, 464 } });
    AddInteraction(world, (Interaction){ .kind = INTERACTION_HEAL, .position = { 480, 624 } });
    AddInteraction(world, (Interaction){ INTERACTION_EXIT, { 896, 480 }, MAP_FIELD, { 112, 656 } });
}

static void InitializeTiles(World *world)
{
    int middleRow = world->rows / 2;
    for (int row = 0; row < world->rows; row++) {
        for (int col = 0; col < world->columns; col++) {
            bool border = row == 0 || col == 0 || row == world->rows - 1 || col == world->columns - 1;
            bool road = world->id == MAP_TOWN ?
                (col >= 13 && col <= 16) || (row >= 12 && row <= 17) ||
                (col >= 11 && col <= 20 && row >= 10 && row <= 19) :
                row >= middleRow - 1 && row <= middleRow + 1;
            if (world->id == MAP_FIELD) {
                /* Approximate the imported north branch and two south branches.
                   Grass and road share walkability, so these are fallback art. */
                road = road || (col >= 18 && col <= 20 && row <= 20) ||
                    (col >= 6 && col <= 8 && row >= 20) ||
                    (col >= 29 && col <= 31 && row >= 20);
            }
            world->tiles[row][col] = border ? TILE_WALL : road ? TILE_ROAD : TILE_GRASS;
        }
    }
}

static void InitializeAdventureObjects(World *world)
{
    if (world->id == MAP_FIELD) {
        /* Independent sprites over the unchanged grass artwork. Tree collision
           covers roots only; keep roads, junctions and spawn patrols open. */
        const Vector2 trees[] = {
            {144,240}, {240,288}, {432,176}, {896,224}, {1104,272},
            {1168,352}, {112,1008}, {464,1136}, {784,1088}, {1120,1136}
        };
        for (int i = 0; i < (int)(sizeof(trees) / sizeof(trees[0])); i++)
            AddScenery(world, SCENERY_TREE,
                (Rectangle){ trees[i].x - 12, trees[i].y - 16, 24, 16 }, DARKGREEN);
        const Rectangle rocks[] = {
            {320,416,48,32}, {784,384,56,40}, {1088,464,40,28},
            {352,960,56,36}, {688,896,48,32}, {1056,1008,56,40}
        };
        for (int i = 0; i < (int)(sizeof(rocks) / sizeof(rocks[0])); i++)
            AddScenery(world, SCENERY_ROCK, rocks[i], GRAY);
        AddInteraction(world, (Interaction){ INTERACTION_EXIT, { 80, 624 }, MAP_TOWN, { 848, 480 } });
        /* ===== 草地右侧传送点：以后修改此入口，搜索这条注释 =====
           下方 Interaction 的四项依次为：
           1. INTERACTION_EXIT：传送交互类型，靠近后按 E / 点击交互按钮触发。
           2. {1184, 656}：传送点在草地地图内的位置（世界坐标，非屏幕坐标）。
              草地为 1280×1280，左上角为 (0,0)，x 向右、y 向下。
           3. MAP_BOSS：目标地图；改为其他 MapId 即改变传送目的地。
           4. {112, 400}：进入目标地图后的玩家落点，必须在该地图可站立区域内。
           功能修改索引：
           - 任务解锁限制：本文件 TryMapExit() 中 destination == MAP_BOSS 的判断；
             解锁条件定义在 quest.c 的 IsBossUnlocked()。
           - 按 E 后的行为、未解锁提示：app.c 的 ActivateNearbyInteraction()。
             如只修改这个入口的行为，请同时判断 currentMap == MAP_FIELD 和 index == 1。
           - 图标：本文件 DrawGround() 的 SPRITE_PORTAL，图片映射在 assets.c。
           - 入口名称：ui.c 的 GetInteractionName()，目前按目标地图选择名称。
           此点是草地 interactions[1]，测试也引用此索引；调整添加顺序时须同步检查。
        */
        AddInteraction(world, (Interaction){ INTERACTION_EXIT, { 1184, 656 }, MAP_BOSS, { 112, 400 } });
        AddInteraction(world, (Interaction){ .kind = INTERACTION_HEAL, .position = { 144, 688 } });
    } else {
        /* Approximate the arena floor, keeping players and attacks out of the
           black void and perimeter walls baked into the new cave background. */
        const Vector2 boundary[] = {
            { 0, 570 }, { 220, 550 }, { 310, 370 }, { 308, 250 },
            { 430, 196 }, { 570, 210 }, { 706, 245 }, { 864, 190 },
            { 1068, 248 }, { 1136, 364 }, { 1148, 528 }, { 1180, 610 },
            { 1130, 790 }, { 1040, 960 }, { 910, 1070 }, { 750, 1100 },
            { 612, 1136 }, { 440, 1110 }, { 296, 1000 }, { 262, 874 },
            { 260, 766 }, { 204, 710 }, { 0, 720 }
        };
        float scale = (float)(world->columns * TILE_SIZE) / 1254.0f;
        world->floorBoundaryCount = (int)(sizeof(boundary) / sizeof(boundary[0]));
        for (int i = 0; i < world->floorBoundaryCount; i++) world->floorBoundary[i] =
            (Vector2){ boundary[i].x * scale, boundary[i].y * scale };
        AddScenery(world, SCENERY_ROCK, (Rectangle){ 288, 192, 64, 48 }, GRAY);
        AddScenery(world, SCENERY_ROCK, (Rectangle){ 512, 544, 64, 48 }, GRAY);
        AddInteraction(world, (Interaction){ INTERACTION_EXIT, { 80, 368 }, MAP_FIELD, { 112, 656 } });
        AddInteraction(world, (Interaction){ .kind = INTERACTION_HEAL, .position = { 144, 432 } });
    }
}

void InitializeWorld(World *world, MapId id)
{
    *world = (World){ .id = id };
    bool town = id == MAP_TOWN;
    world->columns = town ? TOWN_COLUMNS : id == MAP_FIELD ? 40 : 24;
    world->rows = town ? TOWN_ROWS : world->columns;
    int middleRow = world->rows / 2;
    world->spawn = town ? (Vector2){ 480, 528 } : (Vector2){ 112, (float)middleRow * TILE_SIZE + 16 };
    world->safeArea = town ? (Rectangle){ 0, 0, TOWN_COLUMNS * TILE_SIZE, TOWN_ROWS * TILE_SIZE } :
        (Rectangle){ 64, (float)(middleRow - 1) * TILE_SIZE, 96, 96 };
    InitializeTiles(world);
    if (town) InitializeTown(world); else InitializeAdventureObjects(world);
}

/* ===== 碰撞、移动与交互范围 ===== */
Rectangle GetInteractionBody(Interaction object)
{
    Vector2 feet = object.position;
    return object.kind == INTERACTION_HEAL ? (Rectangle){ feet.x - 14, feet.y - 18, 28, 18 } :
        (Rectangle){ feet.x - 10, feet.y - 14, 20, 14 };
}

bool CanStandInWorld(const World *world, Rectangle body)
{
    if (!isfinite(body.x) || !isfinite(body.y) || !isfinite(body.width) || !isfinite(body.height) ||
        body.width <= 0 || body.height <= 0 || world->columns <= 0 || world->rows <= 0 ||
        world->columns > MAP_MAX_COLUMNS || world->rows > MAP_MAX_ROWS) return false;
    if (body.x < 0.0f || body.y < 0.0f || body.x + body.width > world->columns * TILE_SIZE ||
        body.y + body.height > world->rows * TILE_SIZE) return false;

    /* The entrance safety square remains usable for healing and respawning.
       Everywhere else, all four body corners must stay on the arena floor. */
    if (world->floorBoundaryCount > 0 && !IsBodyInSafeArea(world, body)) {
        const Vector2 corners[] = { { body.x, body.y }, { body.x + body.width, body.y },
            { body.x, body.y + body.height }, { body.x + body.width, body.y + body.height } };
        for (int i = 0; i < 4; i++) if (!CheckCollisionPointPoly(corners[i], world->floorBoundary, world->floorBoundaryCount)) return false;
    }

    int firstCol = (int)floorf(body.x / TILE_SIZE);
    int firstRow = (int)floorf(body.y / TILE_SIZE);
    int lastCol = (int)floorf((body.x + body.width - 0.001f) / TILE_SIZE);
    int lastRow = (int)floorf((body.y + body.height - 0.001f) / TILE_SIZE);
    for (int row = firstRow; row <= lastRow; row++) {
        for (int col = firstCol; col <= lastCol; col++) {
            if (world->tiles[row][col] == TILE_WALL) return false;
        }
    }
    for (int i = 0; i < world->sceneryCount; i++) {
        Scenery object = world->scenery[i];
        Rectangle r = object.collision;
        if (CheckCollisionRecs(body, r)) return false;
    }
    for (int i = 0; i < world->interactionCount; i++) {
        Interaction object = world->interactions[i];
        if (object.kind != INTERACTION_EXIT && CheckCollisionRecs(body, GetInteractionBody(object))) return false;
    }
    return true;
}

int FindWorldInteraction(const World *world, Vector2 position)
{
    float closestSquared = 64.0f * 64.0f;
    int closest = -1;
    for (int i = 0; i < world->interactionCount; i++) {
        Vector2 target = world->interactions[i].position;
        float dx = position.x - target.x;
        float dy = position.y - target.y;
        float distanceSquared = dx * dx + dy * dy;
        if (distanceSquared <= closestSquared) {
            closestSquared = distanceSquared;
            closest = i;
        }
    }
    return closest;
}

bool CanMoveWithObstacles(const World *world, Rectangle from, Rectangle to,
                          const Rectangle obstacles[], int count)
{
    if (!CanStandInWorld(world, to)) return false;
    for (int i = 0; i < count; i++) {
        if (!CheckCollisionRecs(to, obstacles[i])) continue;
        /* Old saves may contain overlapping actors: allow moving out, never deeper in. */
        Rectangle before = GetCollisionRec(from, obstacles[i]);
        Rectangle after = GetCollisionRec(to, obstacles[i]);
        float previousArea = before.width * before.height;
        float nextArea = after.width * after.height;
        if (nextArea > previousArea) return false;
        if (nextArea == previousArea) {
            float oldX = from.x + from.width / 2 - obstacles[i].x - obstacles[i].width / 2;
            float oldY = from.y + from.height / 2 - obstacles[i].y - obstacles[i].height / 2;
            float newX = to.x + to.width / 2 - obstacles[i].x - obstacles[i].width / 2;
            float newY = to.y + to.height / 2 - obstacles[i].y - obstacles[i].height / 2;
            if (newX * newX + newY * newY <= oldX * oldX + oldY * oldY) return false;
        }
    }
    return true;
}

/* ===== 安全区、地图出口与镜头 ===== */
static float GetCameraTarget(float position, float worldSize, float viewSize)
{
    if (worldSize <= viewSize) return worldSize / 2.0f;
    float half = viewSize / 2.0f;
    return fminf(fmaxf(position, half), worldSize - half);
}

bool IsBodyInSafeArea(const World *world, Rectangle body)
{
    Rectangle safe = world->safeArea;
    return body.x >= safe.x && body.y >= safe.y &&
           body.x + body.width <= safe.x + safe.width &&
           body.y + body.height <= safe.y + safe.height;
}

bool TryMapExit(const World worlds[MAP_COUNT], MapId currentMap, int exitIndex,
                bool bossUnlocked, MapId *destination, Vector2 *arrival)
{
    if (currentMap < MAP_TOWN || currentMap >= MAP_COUNT) return false;
    const World *world = &worlds[currentMap];
    if (exitIndex < 0 || exitIndex >= world->interactionCount) return false;
    Interaction exit = world->interactions[exitIndex];
    if (exit.kind != INTERACTION_EXIT || exit.destination < MAP_TOWN || exit.destination >= MAP_COUNT) return false;
    if (exit.destination == MAP_BOSS && !bossUnlocked) return false;
    /* Check the destination before changing any live state. */
    Rectangle body = { exit.arrival.x - 10, exit.arrival.y - 14, 20, 14 };
    if (!CanStandInWorld(&worlds[exit.destination], body)) return false;
    *destination = exit.destination;
    *arrival = exit.arrival;
    return true;
}

Camera2D GetWorldCamera(const World *world, Vector2 position, int screenWidth, int screenHeight)
{
    float viewHeight = (float)(screenHeight - HUD_HEIGHT);
    Camera2D camera = { 0 };
    camera.zoom = 2.0f; /* Preview a closer view without changing world distances. */
    camera.offset = (Vector2){ (float)screenWidth / 2.0f, HUD_HEIGHT + viewHeight / 2.0f };
    camera.target.x = GetCameraTarget(position.x, (float)(world->columns * TILE_SIZE), (float)screenWidth / camera.zoom);
    camera.target.y = GetCameraTarget(position.y, (float)(world->rows * TILE_SIZE), viewHeight / camera.zoom);
    return camera;
}

/* ===== 场景物件与备用图形绘制 ===== */
static void DrawPerson(Vector2 feet, Color clothing)
{
    DrawEllipse((int)feet.x, (int)feet.y - 2, 12, 4, (Color){ 15, 24, 23, 100 });
    DrawRectangle((int)feet.x - 8, (int)feet.y - 16, 16, 12, clothing);
    DrawRectangle((int)feet.x - 7, (int)feet.y - 27, 14, 11, (Color){ 227, 188, 133, 255 });
    DrawRectangle((int)feet.x - 8, (int)feet.y - 29, 16, 5, (Color){ 64, 49, 43, 255 });
    DrawRectangle((int)feet.x - 7, (int)feet.y - 4, 5, 4, DARKBROWN);
    DrawRectangle((int)feet.x + 2, (int)feet.y - 4, 5, 4, DARKBROWN);
}

static void DrawScenery(Scenery object, const UiAssets *assets)
{
    Rectangle r = object.collision;
    /* Fit the fallback roof inside the same solid house silhouette. */
    if (object.kind == SCENERY_HOUSE) { r.y += 64; r.height -= 64; }
    Vector2 size = { r.width + 8, r.height + (object.sprite == SPRITE_STALAGMITES ? 48 : 16) };
    if (object.kind == SCENERY_HOUSE) size = (Vector2){ r.width, r.height + 64 };
    else if (object.kind == SCENERY_TREE) size = (Vector2){ 72, 96 };
    else if (object.kind == SCENERY_LAMP) size = (Vector2){ 24, 72 };
    if (DrawSprite(assets, object.sprite, (Vector2){ r.x + r.width / 2, r.y + r.height },
        size, (Color){ 255, 255, 255, object.color.a })) return;
    /* Missing artwork keeps its original code-drawn fallback. */
    if (object.kind == SCENERY_HOUSE) {
        Color wall = { 195, 178, 133, object.color.a };
        DrawRectangleRec(r, wall);
        DrawRectangle((int)r.x, (int)(r.y + r.height - 20), (int)r.width, 20, (Color){ 119, 119, 105, object.color.a });
        for (int x = 0; x < (int)r.width; x += 48) {
            DrawRectangle((int)r.x + x, (int)r.y, 6, (int)r.height - 20, DARKBROWN);
        }
        DrawRectangle((int)r.x, (int)r.y - 64, (int)r.width, 80, object.color);
        for (int y = -60; y < 16; y += 12) {
            DrawLine((int)r.x + 3, (int)r.y + y, (int)(r.x + r.width) - 3, (int)r.y + y, DARKBROWN);
        }
        DrawRectangleLinesEx((Rectangle){ r.x, r.y - 64, r.width, 80 }, 4, DARKBROWN);
        DrawRectangle((int)(r.x + r.width / 2 - 16), (int)(r.y + r.height - 48), 32, 48, DARKBROWN);
        DrawCircle((int)(r.x + r.width / 2 + 8), (int)(r.y + r.height - 28), 2, GOLD);
        for (int side = 0; side < 2; side++) {
            int x = (int)(r.x + (side ? r.width - 68 : 28));
            DrawRectangle(x - 4, (int)r.y + 42, 48, 42, DARKBROWN);
            DrawRectangle(x, (int)r.y + 46, 40, 34, (Color){ 111, 168, 175, object.color.a });
            DrawRectangle(x + 18, (int)r.y + 46, 4, 34, DARKBROWN);
            DrawRectangle(x, (int)r.y + 60, 40, 4, DARKBROWN);
            /* Small attached flower boxes stay inside the solid house footprint. */
            DrawRectangle(x - 4, (int)r.y + 86, 48, 8, BROWN);
            for (int flower = 0; flower < 5; flower++) {
                DrawRectangle(x + flower * 8, (int)r.y + 80, 4, 6, DARKGREEN);
                DrawRectangle(x + flower * 8, (int)r.y + 78, 4, 4, flower % 2 ? GOLD : MAROON);
            }
        }
    } else if (object.kind == SCENERY_LAMP) {
        int x = (int)(r.x + r.width / 2), bottom = (int)(r.y + r.height);
        DrawRectangleRec(r, (Color){ 82, 79, 68, 255 });
        DrawRectangle(x - 3, bottom - 58, 6, 52, DARKBROWN);
        DrawRectangle(x - 10, bottom - 64, 20, 19, DARKBROWN);
        DrawRectangle(x - 7, bottom - 61, 14, 13, GOLD);
        DrawRectangle(x - 1, bottom - 61, 2, 13, BROWN);
        DrawRectangle(x - 12, bottom - 68, 24, 4, DARKBROWN);
        DrawRectangle(x - 6, bottom - 72, 12, 4, DARKBROWN);
    } else if (object.kind == SCENERY_FENCE) {
        DrawRectangle((int)r.x, (int)r.y - 13, (int)r.width, 6, (Color){ 145, 110, 66, 255 });
        DrawRectangle((int)r.x, (int)r.y - 2, (int)r.width, 5, BROWN);
        for (int x = 0; x <= (int)r.width - 8; x += 24) {
            DrawRectangle((int)r.x + x, (int)r.y - 20, 8, (int)r.height + 20, DARKBROWN);
            DrawRectangle((int)r.x + x, (int)r.y - 20, 8, 3, (Color){ 168, 130, 80, 255 });
        }
    } else if (object.kind == SCENERY_WELL) {
        DrawRectangleRec(r, (Color){ 105, 108, 101, 255 });
        DrawRectangle((int)r.x - 4, (int)r.y - 4, (int)r.width + 8, 16, (Color){ 164, 159, 134, 255 });
        DrawRectangle((int)r.x + 6, (int)r.y, (int)r.width - 12, 7, (Color){ 34, 66, 76, 255 });
        DrawRectangle((int)r.x + 2, (int)r.y - 32, 4, 32, DARKBROWN);
        DrawRectangle((int)(r.x + r.width - 6), (int)r.y - 32, 4, 32, DARKBROWN);
        DrawRectangle((int)r.x - 6, (int)r.y - 36, (int)r.width + 12, 8, (Color){ 124, 69, 58, 255 });
        DrawLine((int)(r.x + r.width / 2), (int)r.y - 28, (int)(r.x + r.width / 2), (int)r.y + 2, BROWN);
    } else if (object.kind == SCENERY_FLOWERBED) {
        DrawRectangleRec(r, (Color){ 148, 140, 116, 255 });
        DrawRectangle((int)r.x + 3, (int)r.y + 3, (int)r.width - 6, (int)r.height - 6, DARKGREEN);
        for (int x = 6; x < (int)r.width - 4; x += 8)
            DrawRectangle((int)r.x + x, (int)r.y + 5, 4, 5, x % 3 ? PINK : GOLD);
    } else if (object.kind == SCENERY_TREE) {
        DrawRectangle((int)r.x + 8, (int)r.y - 12, 8, (int)r.height + 12, DARKBROWN);
        DrawRectangle((int)r.x - 12, (int)r.y - 48, 48, 28, object.color);
        DrawRectangle((int)r.x - 4, (int)r.y - 64, 32, 24, (Color){ 48, 123, 68, object.color.a });
    } else {
        DrawRectangleRec(r, (Color){ 108, 119, 121, 255 });
        DrawRectangle((int)r.x + 4, (int)r.y - 4, (int)r.width - 8, 8, (Color){ 144, 153, 147, 255 });
    }
}

static void DrawInteraction(Interaction object, const UiAssets *assets)
{
    Vector2 p = object.position;
    if (object.kind == INTERACTION_QUEST || object.kind == INTERACTION_SHOP) {
        if (!DrawSprite(assets, object.kind == INTERACTION_QUEST ? SPRITE_LEADER : SPRITE_SELLER,
            p, (Vector2){ 32, 32 }, WHITE))
            DrawPerson(p, object.kind == INTERACTION_QUEST ? (Color){ 178, 129, 54, 255 } : (Color){ 135, 75, 150, 255 });
        DrawCircle((int)p.x, (int)p.y - 40, 5, object.kind == INTERACTION_QUEST ? GOLD : VIOLET);
    } else if (object.kind == INTERACTION_HEAL) {
        if (DrawSprite(assets, SPRITE_HEAL, p, (Vector2){ 40, 56 }, WHITE)) return;
        DrawRectangle((int)p.x - 20, (int)p.y - 24, 40, 28, (Color){ 71, 123, 143, 255 });
        DrawRectangle((int)p.x - 14, (int)p.y - 20, 28, 18, SKYBLUE);
        DrawRectangle((int)p.x - 3, (int)p.y - 34, 6, 16, GREEN);
        DrawRectangle((int)p.x - 8, (int)p.y - 29, 16, 6, GREEN);
    } else {
        if (assets && IsTextureValid(assets->sprites[SPRITE_PORTAL].texture)) return; /* Floor layer draws it. */
        DrawRectangle((int)p.x - 16, (int)p.y - 24, 32, 48, (Color){ 62, 95, 91, 255 });
        DrawTriangle((Vector2){ p.x + 10, p.y }, (Vector2){ p.x - 6, p.y - 9 },
                     (Vector2){ p.x - 6, p.y + 9 }, GOLD);
    }
}

static void DrawTiles(const World *world)
{
    for (int row = 0; row < world->rows; row++) {
        for (int col = 0; col < world->columns; col++) {
            TileType tile = world->tiles[row][col];
            Color color = tile == TILE_WALL ? (Color){ 73, 86, 85, 255 } :
                          tile == TILE_ROAD ? (Color){ 163, 145, 106, 255 } :
                          (row + col) % 2 == 0 ? (Color){ 72, 111, 67, 255 } :
                                                (Color){ 76, 116, 70, 255 };
            if (world->id == MAP_BOSS && tile == TILE_GRASS) color = (Color){ 66, 63, 72, 255 };
            DrawRectangle(col * TILE_SIZE, row * TILE_SIZE, TILE_SIZE, TILE_SIZE, color);
            if (world->id == MAP_BOSS || tile == TILE_WALL) continue;
            int x = col * TILE_SIZE, y = row * TILE_SIZE;
            unsigned int variation = (unsigned int)(row * 97 + col * 53);
            if (tile == TILE_ROAD && world->id == MAP_TOWN) {
                /* Uneven old pavers, shallow seams and occasional grass in cracks. */
                for (int brick = 0; brick < 4; brick++) {
                    int by = y + brick * 8;
                    Color stone = { (unsigned char)(151 + (variation + brick * 7) % 23),
                        (unsigned char)(143 + (variation + brick * 7) % 20), 116, 255 };
                    DrawRectangle(x + 1, by + 1, 30, 6, stone);
                    DrawRectangle(x + (brick % 2 ? 9 : 22), by + 1, 1, 6, (Color){ 116, 111, 85, 255 });
                }
                if (variation % 7 == 0) DrawRectangle(x + 6, y + 23, 3, 5, (Color){ 80, 110, 61, 255 });
            } else if (tile == TILE_GRASS) {
                int gx = x + 4 + (int)(variation % 22), gy = y + 5 + (int)((variation / 3) % 20);
                DrawRectangle(gx, gy, 2, 5, (Color){ 93, 134, 72, 255 });
                DrawRectangle(gx - 3, gy + 2, 2, 3, (Color){ 57, 96, 54, 255 });
                if (variation % 17 == 0) DrawRectangle(gx + 4, gy + 2, 2, 2, (Color){ 213, 194, 119, 255 });
            } else {
                DrawRectangle(x + 4 + (int)(variation % 20), y + 8, 4, 2, (Color){ 147, 128, 89, 255 });
                DrawRectangle(x + 9, y + 20, 3, 2, (Color){ 180, 158, 114, 255 });
            }
        }
    }
}

static void DrawGround(const World *world, const UiAssets *assets)
{
    float mapWidth = (float)(world->columns * TILE_SIZE), mapHeight = (float)(world->rows * TILE_SIZE);
    SpriteId background = world->id == MAP_TOWN ? SPRITE_TOWN : world->id == MAP_FIELD ? SPRITE_FIELD : SPRITE_CAVE;
    /* RGB backgrounds cover every underlying tile. Transparent or missing
       artwork still gets the original fallback ground before it is drawn. */
    bool covered = assets && assets->sprites[background].opaque && IsTextureValid(assets->sprites[background].texture);
    if (!covered) DrawTiles(world);
    DrawSprite(assets, background, (Vector2){ mapWidth / 2, mapHeight },
        (Vector2){ mapWidth, mapHeight }, WHITE);
    if (world->id != MAP_TOWN) {
        DrawRectangleRec(world->safeArea, (Color){ 55, 148, 123, 155 });
        DrawRectangleLinesEx(world->safeArea, 2, (Color){ 136, 239, 190, 255 });
    }
    /* Transport artwork is a floor tile: draw before feet-sorted objects so
       players can stand on it without the platform covering their legs. */
    for (int i = 0; i < world->interactionCount; i++) {
        Interaction object = world->interactions[i];
        if (object.kind == INTERACTION_EXIT)
            DrawSprite(assets, SPRITE_PORTAL, (Vector2){ object.position.x, object.position.y + 24 }, (Vector2){ 48, 48 }, WHITE);
    }
}

/* Sort references, never the actual map records. -1 is the player; other
   indices select scenery, interactions, then temporary actors in that order. */
/* ===== 按脚底位置排序并绘制角色与场景 ===== */
typedef struct DrawEntry { float feetY; int index; } DrawEntry;

static int BuildDrawList(const World *world, Vector2 playerPosition,
                         const WorldActor actors[], int actorCount, DrawEntry entries[])
{
    int count = 0;
    for (int i = 0; i < world->sceneryCount; i++) {
        Rectangle r = world->scenery[i].collision;
        entries[count++] = (DrawEntry){ r.y + r.height, i };
    }
    for (int i = 0; i < world->interactionCount; i++) {
        entries[count++] = (DrawEntry){ world->interactions[i].position.y, SCENERY_CAPACITY + i };
    }
    entries[count++] = (DrawEntry){ playerPosition.y, -1 };
    if (actorCount > WORLD_ACTOR_CAPACITY) actorCount = WORLD_ACTOR_CAPACITY;
    for (int i = 0; i < actorCount; i++) entries[count++] =
        (DrawEntry){ actors[i].position.y, SCENERY_CAPACITY + INTERACTION_CAPACITY + i };
    for (int i = 1; i < count; i++) {
        DrawEntry current = entries[i];
        int previous = i - 1;
        while (previous >= 0 && entries[previous].feetY > current.feetY) {
            entries[previous + 1] = entries[previous];
            previous--;
        }
        entries[previous + 1] = current;
    }
    return count;
}

static void DrawObjects(const World *world, Vector2 playerPosition, Vector2 swordFacing,
                        bool playerVisible, const WorldActor actors[], int actorCount, const UiAssets *assets)
{
    DrawEntry entries[SCENERY_CAPACITY + INTERACTION_CAPACITY + WORLD_ACTOR_CAPACITY + 1];
    int count = BuildDrawList(world, playerPosition, actors, actorCount, entries);
    for (int i = 0; i < count; i++) {
        int index = entries[i].index;
        if (index == -1) {
            if (playerVisible && !DrawSprite(assets, GetFacingSprite(SPRITE_PLAYER_DOWN, swordFacing),
                playerPosition, (Vector2){ 32, 32 }, WHITE)) DrawPerson(playerPosition, (Color){ 51, 104, 177, 255 });
        } else if (index < SCENERY_CAPACITY) {
            /* The complete town artwork already includes all town scenery. */
            if (world->id == MAP_TOWN && assets && IsTextureValid(assets->sprites[SPRITE_TOWN].texture)) continue;
            Scenery object = world->scenery[index];
            Rectangle r = object.collision;
            bool behind = playerPosition.y < r.y + r.height &&
                          playerPosition.y > r.y - 80 && playerPosition.x > r.x - 16 &&
                          playerPosition.x < r.x + r.width + 16;
            /* Placeholder-only tint for occlusion; final roofs/canopies will be separate. */
            if (behind) object.color = Fade(object.color, 0.45f);
            DrawScenery(object, assets);
        } else if (index < SCENERY_CAPACITY + INTERACTION_CAPACITY) {
            DrawInteraction(world->interactions[index - SCENERY_CAPACITY], assets);
        } else {
            WorldActor actor = actors[index - SCENERY_CAPACITY - INTERACTION_CAPACITY];
            float size = actor.large ? 64 : 32;
            if (DrawSprite(assets, actor.sprite, actor.position, (Vector2){ size, size }, actor.spriteTint)) continue;
            if (actor.large) {
                DrawRectangle((int)actor.position.x - 28, (int)actor.position.y - 56, 56, 56, actor.color);
                DrawRectangle((int)actor.position.x - 16, (int)actor.position.y - 64, 32, 10, GOLD);
            } else DrawPerson(actor.position, actor.color);
        }
    }
}

void DrawWorld(const World *world, Vector2 playerPosition,
              Vector2 swordFacing, Vector2 staffFacing, bool playerVisible,
              const WorldActor actors[], int actorCount, const UiAssets *assets)
{
    DrawGround(world, assets);
    DrawObjects(world, playerPosition, swordFacing, playerVisible, actors, actorCount, assets);
    Vector2 origin = { playerPosition.x, playerPosition.y - 14 };
    DrawLineEx(origin, (Vector2){ origin.x + staffFacing.x * 28, origin.y + staffFacing.y * 28 }, 2, SKYBLUE);
    DrawLineEx(origin, (Vector2){ origin.x + swordFacing.x * 20, origin.y + swordFacing.y * 20 }, 3, GOLD);
    /* Tiny foot marker remains visible even while placeholder scenery covers the sprite. */
    DrawCircleLines((int)playerPosition.x, (int)playerPosition.y, 4, WHITE);
}
