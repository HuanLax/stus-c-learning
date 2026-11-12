#include "player.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>

static float Distance(Vector2 a, Vector2 b)
{
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return sqrtf(dx * dx + dy * dy);
}

static void CheckInteractionRoutes(const World *world)
{
    /* Flood-fill walkable tile centres to catch isolated entrances or NPCs. */
    bool visited[MAP_MAX_ROWS][MAP_MAX_COLUMNS] = { 0 };
    int queue[MAP_MAX_ROWS * MAP_MAX_COLUMNS];
    int head = 0;
    int tail = 0;
    int startCol = (int)(world->spawn.x / TILE_SIZE);
    int startRow = (int)(world->spawn.y / TILE_SIZE);
    queue[tail++] = startRow * MAP_MAX_COLUMNS + startCol;
    visited[startRow][startCol] = true;
    const int offsets[4][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
    while (head < tail) {
        int cell = queue[head++];
        int row = cell / MAP_MAX_COLUMNS;
        int col = cell % MAP_MAX_COLUMNS;
        for (int direction = 0; direction < 4; direction++) {
            int nextCol = col + offsets[direction][0];
            int nextRow = row + offsets[direction][1];
            if (nextCol < 0 || nextRow < 0 || nextCol >= world->columns || nextRow >= world->rows ||
                visited[nextRow][nextCol]) continue;
            Vector2 point = { (float)nextCol * TILE_SIZE + 16, (float)nextRow * TILE_SIZE + 16 };
            if (!CanStandInWorld(world, GetPlayerBody(point))) continue;
            visited[nextRow][nextCol] = true;
            queue[tail++] = nextRow * MAP_MAX_COLUMNS + nextCol;
        }
    }
    for (int object = 0; object < world->interactionCount; object++) {
        bool reachable = false;
        for (int row = 0; row < world->rows; row++) {
            for (int col = 0; col < world->columns; col++) {
                Vector2 point = { (float)col * TILE_SIZE + 16, (float)row * TILE_SIZE + 16 };
                if (visited[row][col] && Distance(point, world->interactions[object].position) <= 64) reachable = true;
            }
        }
        assert(reachable);
    }
}

int main(void)
{
    World world;
    Player straight;
    Player diagonal;
    InitializeWorld(&world, MAP_TOWN);
    InitializePlayer(&straight, world.spawn);
    InitializePlayer(&diagonal, world.spawn);
    assert(CanStandInWorld(&world, GetPlayerBody(world.spawn)));
    assert(FindWorldInteraction(&world, world.spawn) == -1);
    /* Town artwork remains visible, but its former obstacle blocks are gone. */
    for (int i = 0; i < world.sceneryCount; i++) {
        Rectangle r = world.scenery[i].collision;
        assert(CanStandInWorld(&world, GetPlayerBody((Vector2){ r.x + r.width / 2, r.y + r.height / 2 })));
    }
    assert(CanStandInWorld(&world, GetPlayerBody((Vector2){ 200, 350 })));

    /* Measure movement speed independently of interactions and map boundaries. */
    World openArea = { .columns = TOWN_COLUMNS, .rows = TOWN_ROWS };
    MovePlayer(&straight, &openArea, (Vector2){ 1, 0 }, 1.0f);
    MovePlayer(&diagonal, &openArea, (Vector2){ 1, 1 }, 1.0f);
    assert(fabsf(Distance(straight.position, world.spawn) - 96.0f) < 0.001f);
    assert(fabsf(Distance(diagonal.position, world.spawn) - 96.0f) < 0.001f);
    assert(diagonal.swordFacing.x == 0 && diagonal.swordFacing.y == 1);
    assert(fabsf(diagonal.staffFacing.x - diagonal.staffFacing.y) < 0.0001f);

    Player fineSteps;
    InitializePlayer(&fineSteps, world.spawn);
    for (int i = 0; i < 60; i++) MovePlayer(&fineSteps, &openArea, (Vector2){ 1, 0 }, 1.0f / 60.0f);
    assert(Distance(fineSteps.position, straight.position) < 0.01f);
    Vector2 resting = fineSteps.position;
    MovePlayer(&fineSteps, &world, (Vector2){ 0, 0 }, 1.0f);
    assert(Distance(resting, fineSteps.position) == 0);

    Player border;
    InitializePlayer(&border, (Vector2){ 48, 400 });
    MovePlayer(&border, &world, (Vector2){ -1, 0 }, 1.0f);
    assert(GetPlayerBody(border.position).x >= TILE_SIZE);
    assert(border.position.x < 48);
    assert(CanStandInWorld(&world, GetPlayerBody(border.position)));

    Player house;
    InitializePlayer(&house, (Vector2){ 104, 220 });
    MovePlayer(&house, &world, (Vector2){ 1, 0 }, 2.0f);
    Rectangle body = GetPlayerBody(house.position);
    assert(house.position.x > 256);
    assert(CanStandInWorld(&world, body));

    Player sliding;
    InitializePlayer(&sliding, (Vector2){ 110, 200 });
    MovePlayer(&sliding, &world, (Vector2){ 1, 1 }, 1.0f);
    assert(sliding.position.y > 260);
    assert(sliding.position.x > 128);
    assert(CanStandInWorld(&world, GetPlayerBody(sliding.position)));

    Player npc;
    InitializePlayer(&npc, (Vector2){ 400, 496 });
    MovePlayer(&npc, &world, (Vector2){ 0, -1 }, 0.5f);
    assert(GetPlayerBody(npc.position).y >= 464);
    assert(FindWorldInteraction(&world, npc.position) == 0);
    assert(CanStandInWorld(&world, GetPlayerBody(npc.position)));

    /* Two touching obstacles must not allow diagonal corner cutting. */
    World corner = { 0 };
    corner.columns = TOWN_COLUMNS;
    corner.rows = TOWN_ROWS;
    corner.tiles[10][11] = TILE_WALL;
    corner.tiles[11][10] = TILE_WALL;
    Player trapped;
    InitializePlayer(&trapped, (Vector2){ 336, 336 });
    MovePlayer(&trapped, &corner, (Vector2){ 1, 1 }, 2.0f);
    assert(trapped.position.x < 352 && trapped.position.y <= 352);
    assert(CanStandInWorld(&corner, GetPlayerBody(trapped.position)));

    Camera2D top = GetWorldCamera(&world, (Vector2){ 48, 48 }, 1024, 768);
    Camera2D bottom = GetWorldCamera(&world, (Vector2){ 912, 912 }, 1024, 768);
    assert(top.zoom == 2.0f);
    assert(fabsf(top.target.x - 1024 / (2 * top.zoom)) < 0.001f);
    assert(fabsf(bottom.target.x - (960 - top.target.x)) < 0.001f);
    assert(fabsf(top.target.y - 640 / (2 * top.zoom)) < 0.001f);
    assert(fabsf(bottom.target.y - (960 - top.target.y)) < 0.001f);
    assert(top.offset.y == 448);
    Camera2D largeWindow = GetWorldCamera(&world, (Vector2){ 48, 48 }, 2200, 2200);
    assert(largeWindow.target.x == 480 && largeWindow.target.y == 480);
    World maps[MAP_COUNT];
    for (int map = 0; map < MAP_COUNT; map++) {
        InitializeWorld(&maps[map], (MapId)map);
        assert(CanStandInWorld(&maps[map], GetPlayerBody(maps[map].spawn)));
        assert(IsBodyInSafeArea(&maps[map], GetPlayerBody(maps[map].spawn)));
        CheckInteractionRoutes(&maps[map]);
        for (int i = 0; i < maps[map].interactionCount; i++) {
            Interaction object = maps[map].interactions[i];
            if (object.kind != INTERACTION_HEAL) continue;
            assert(!CanStandInWorld(&maps[map], GetPlayerBody(object.position)));
            Player approach;
            InitializePlayer(&approach, (Vector2){ object.position.x, object.position.y - 24 });
            assert(CanStandInWorld(&maps[map], GetPlayerBody(approach.position)));
            assert(IsBodyInSafeArea(&maps[map], GetPlayerBody(approach.position)));
            assert(FindWorldInteraction(&maps[map], approach.position) == i);
            MovePlayer(&approach, &maps[map], (Vector2){ 0, 1 }, 1);
            assert(!CheckCollisionRecs(GetPlayerBody(approach.position), GetInteractionBody(object)));
        }
        if (map == MAP_FIELD) {
            assert(maps[map].sceneryCount == 0);
            for (int row = 1; row < maps[map].rows - 1; row++)
                for (int col = 1; col < maps[map].columns - 1; col++) assert(maps[map].tiles[row][col] != TILE_WALL);
        }
        if (map == MAP_BOSS) {
            assert(!CanStandInWorld(&maps[map], GetPlayerBody((Vector2){ 64, 64 })));
            assert(CanStandInWorld(&maps[map], GetPlayerBody((Vector2){ 448, 448 })));
        }
        if (map != MAP_TOWN) {
            assert(maps[map].safeArea.width == 3 * TILE_SIZE && maps[map].safeArea.height == 3 * TILE_SIZE);
            Vector2 partial = { maps[map].safeArea.x + 8, maps[map].spawn.y };
            assert(!IsBodyInSafeArea(&maps[map], GetPlayerBody(partial)));
        }
    }
    MapId destination = MAP_TOWN;
    Vector2 arrival = { 1, 2 };
    assert(TryMapExit(maps, MAP_TOWN, 3, false, &destination, &arrival));
    assert(destination == MAP_FIELD && IsBodyInSafeArea(&maps[destination], GetPlayerBody(arrival)));
    assert(TryMapExit(maps, MAP_FIELD, 0, false, &destination, &arrival));
    assert(destination == MAP_TOWN);
    Vector2 unchanged = arrival;
    assert(!TryMapExit(maps, MAP_FIELD, 1, false, &destination, &arrival));
    assert(destination == MAP_TOWN && Distance(arrival, unchanged) == 0);
    assert(TryMapExit(maps, MAP_FIELD, 1, true, &destination, &arrival));
    assert(destination == MAP_BOSS && IsBodyInSafeArea(&maps[destination], GetPlayerBody(arrival)));
    assert(TryMapExit(maps, MAP_BOSS, 0, false, &destination, &arrival));
    assert(destination == MAP_FIELD && IsBodyInSafeArea(&maps[destination], GetPlayerBody(arrival)));
    assert(!TryMapExit(maps, MAP_TOWN, -1, true, &destination, &arrival));
    assert(!TryMapExit(maps, MAP_TOWN, 0, true, &destination, &arrival));
    maps[MAP_FIELD].interactions[1].arrival.x = NAN;
    assert(!TryMapExit(maps, MAP_FIELD, 1, true, &destination, &arrival));
    Camera2D fieldCorner = GetWorldCamera(&maps[MAP_FIELD], (Vector2){ 1200, 1200 }, 1024, 768);
    assert(fabsf(fieldCorner.target.x - (1280 - 1024 / (2 * fieldCorner.zoom))) < 0.001f);
    assert(fabsf(fieldCorner.target.y - (1280 - 640 / (2 * fieldCorner.zoom))) < 0.001f);
    Camera2D bossCorner = GetWorldCamera(&maps[MAP_BOSS], (Vector2){ 700, 700 }, 1024, 768);
    assert(fabsf(bossCorner.target.x - (768 - 1024 / (2 * bossCorner.zoom))) < 0.001f);
    assert(fabsf(bossCorner.target.y - (768 - 640 / (2 * bossCorner.zoom))) < 0.001f);
    puts("PASS: movement, collisions, routes on all maps, safe areas, transitions, locked gate and camera bounds");
    return 0;
}
