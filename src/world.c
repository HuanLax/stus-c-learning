#include "world.h"

#include <math.h>

static void AddScenery(World *world, SceneryKind kind, Rectangle collision, Color color)
{
    if (world->sceneryCount >= SCENERY_CAPACITY) return;
    SpriteId sprite = kind == SCENERY_HOUSE ? (world->sceneryCount % 2 == 0 ? SPRITE_HOUSE_LEADER : SPRITE_HOUSE_SELLER) : SPRITE_TREE;
    if (kind == SCENERY_ROCK) sprite = world->id != MAP_BOSS ? SPRITE_ROCK :
        world->sceneryCount == 0 ? SPRITE_STALAGMITES : SPRITE_RUBBLE;
    world->scenery[world->sceneryCount++] = (Scenery){ kind, collision, color, sprite };
}

static void AddInteraction(World *world, Interaction object)
{
    if (world->interactionCount >= INTERACTION_CAPACITY) return;
    world->interactions[world->interactionCount++] = object;
}

static void InitializeTown(World *world)
{
    /* Town scenery is decorative; only NPCs and healing stones block movement. */
    AddScenery(world, SCENERY_HOUSE, (Rectangle){ 128, 160, 128, 96 }, (Color){ 124, 69, 58, 255 });
    AddScenery(world, SCENERY_HOUSE, (Rectangle){ 672, 160, 128, 96 }, (Color){ 74, 90, 125, 255 });
    AddScenery(world, SCENERY_HOUSE, (Rectangle){ 96, 608, 160, 96 }, (Color){ 136, 99, 55, 255 });
    AddScenery(world, SCENERY_HOUSE, (Rectangle){ 704, 640, 128, 96 }, (Color){ 97, 66, 105, 255 });
    AddScenery(world, SCENERY_TREE, (Rectangle){ 304, 256, 24, 20 }, DARKGREEN);
    AddScenery(world, SCENERY_TREE, (Rectangle){ 592, 288, 24, 20 }, DARKGREEN);
    AddScenery(world, SCENERY_TREE, (Rectangle){ 320, 688, 24, 20 }, DARKGREEN);
    AddScenery(world, SCENERY_TREE, (Rectangle){ 608, 800, 24, 20 }, DARKGREEN);
    AddScenery(world, SCENERY_ROCK, (Rectangle){ 352, 352, 32, 24 }, GRAY);
    AddScenery(world, SCENERY_ROCK, (Rectangle){ 576, 704, 32, 24 }, GRAY);

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
                (col >= 14 && col <= 15) || (row >= 14 && row <= 16) ||
                ((row == 8 || row == 23) && col >= 6 && col <= 23) ||
                ((col == 6 || col == 23) && row >= 8 && row <= 23) : row >= middleRow - 1 && row <= middleRow + 1;
            world->tiles[row][col] = border ? TILE_WALL : road ? TILE_ROAD : TILE_GRASS;
        }
    }
}

static void InitializeAdventureObjects(World *world)
{
    if (world->id == MAP_FIELD) {
        AddInteraction(world, (Interaction){ INTERACTION_EXIT, { 80, 624 }, MAP_TOWN, { 848, 480 } });
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
    for (int i = 0; world->id != MAP_TOWN && i < world->sceneryCount; i++) {
        if (CheckCollisionRecs(body, world->scenery[i].collision)) return false;
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
    Vector2 size = { r.width + 8, r.height + (object.sprite == SPRITE_STALAGMITES ? 48 : 16) };
    if (object.kind == SCENERY_HOUSE) size = (Vector2){ r.width + 16, r.height + 72 };
    else if (object.kind == SCENERY_TREE) size = (Vector2){ 72, 96 };
    if (DrawSprite(assets, object.sprite, (Vector2){ r.x + r.width / 2, r.y + r.height },
        size, (Color){ 255, 255, 255, object.color.a })) return;
    /* Missing artwork keeps its original code-drawn fallback. */
    if (object.kind == SCENERY_HOUSE) {
        DrawRectangleRec(r, (Color){ 177, 157, 114, 255 });
        DrawRectangle((int)r.x - 8, (int)r.y - 48, (int)r.width + 16, 56, object.color);
        DrawRectangleLinesEx((Rectangle){ r.x - 8, r.y - 48, r.width + 16, 56 }, 3, DARKBROWN);
        DrawRectangle((int)(r.x + r.width / 2 - 12), (int)(r.y + r.height - 40), 24, 40, DARKBROWN);
        DrawRectangle((int)r.x + 16, (int)r.y + 24, 22, 24, (Color){ 81, 130, 149, 255 });
        DrawRectangle((int)(r.x + r.width - 38), (int)r.y + 24, 22, 24, (Color){ 81, 130, 149, 255 });
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
