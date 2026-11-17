#ifndef PROJECT_LAX_WORLD_H
#define PROJECT_LAX_WORLD_H

#include "raylib.h"
#include "assets.h"

/* enum constants are not global variables. */
enum { TILE_SIZE = 32, TOWN_COLUMNS = 30, TOWN_ROWS = 30,
       MAP_MAX_COLUMNS = 40, MAP_MAX_ROWS = 40,
       SCENERY_CAPACITY = 32, INTERACTION_CAPACITY = 8, WORLD_ACTOR_CAPACITY = 8,
       FLOOR_BOUNDARY_CAPACITY = 24, HUD_HEIGHT = 128, GAME_WIDTH = 1024, GAME_HEIGHT = 768 };

typedef enum MapId { MAP_TOWN, MAP_FIELD, MAP_BOSS, MAP_COUNT } MapId;

typedef enum TileType { TILE_GRASS, TILE_ROAD, TILE_WALL } TileType;
typedef enum SceneryKind { SCENERY_HOUSE, SCENERY_TREE, SCENERY_ROCK,
                           SCENERY_LAMP, SCENERY_FENCE, SCENERY_WELL,
                           SCENERY_FLOWERBED } SceneryKind;
typedef enum InteractionKind { INTERACTION_QUEST, INTERACTION_SHOP,
                               INTERACTION_HEAL, INTERACTION_EXIT } InteractionKind;

typedef struct Scenery {
    SceneryKind kind;
    Rectangle collision;
    Color color;
    SpriteId sprite; /* Artwork ID, not a GPU texture or collision size. */
} Scenery;

typedef struct WorldActor {
    Vector2 position;
    Color color;
    bool large;
    SpriteId sprite;
    Color spriteTint;
} WorldActor;

typedef struct Interaction {
    InteractionKind kind;
    Vector2 position; /* feet/base centre in world pixels */
    MapId destination;
    Vector2 arrival;
} Interaction;

typedef struct World {
    MapId id;
    int columns;
    int rows;
    TileType tiles[MAP_MAX_ROWS][MAP_MAX_COLUMNS];
    Scenery scenery[SCENERY_CAPACITY];
    int sceneryCount;
    Interaction interactions[INTERACTION_CAPACITY];
    int interactionCount;
    Vector2 spawn;
    Rectangle safeArea;
    Vector2 floorBoundary[FLOOR_BOUNDARY_CAPACITY];
    int floorBoundaryCount;
} World;

void InitializeWorld(World *world, MapId id);
bool CanStandInWorld(const World *world, Rectangle body);
bool CanMoveWithObstacles(const World *world, Rectangle from, Rectangle to,
                          const Rectangle obstacles[], int count);
Rectangle GetInteractionBody(Interaction object);
int FindWorldInteraction(const World *world, Vector2 position);
bool IsBodyInSafeArea(const World *world, Rectangle body);
bool TryMapExit(const World worlds[MAP_COUNT], MapId currentMap, int exitIndex,
                bool bossUnlocked, MapId *destination, Vector2 *arrival);
Camera2D GetWorldCamera(const World *world, Vector2 position, int screenWidth, int screenHeight);
void DrawWorld(const World *world, Vector2 playerPosition,
              Vector2 swordFacing, Vector2 staffFacing, bool playerVisible,
              const WorldActor actors[], int actorCount, const UiAssets *assets);

#endif
