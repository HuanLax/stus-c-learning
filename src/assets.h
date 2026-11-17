#ifndef PROJECT_LAX_ASSETS_H
#define PROJECT_LAX_ASSETS_H

#include "raylib.h"

typedef enum SpriteId {
    SPRITE_TOWN, SPRITE_HOUSE_LEADER, SPRITE_HOUSE_SELLER,
    SPRITE_PLAYER_DOWN, SPRITE_PLAYER_LEFT, SPRITE_PLAYER_RIGHT, SPRITE_PLAYER_UP,
    SPRITE_GOBLIN_DOWN, SPRITE_GOBLIN_LEFT, SPRITE_GOBLIN_RIGHT, SPRITE_GOBLIN_UP,
    SPRITE_BOSS, SPRITE_LEADER, SPRITE_SELLER,
    SPRITE_FIELD, SPRITE_HEAL, SPRITE_ROCK, SPRITE_PORTAL, SPRITE_TREE,
    SPRITE_CAVE, SPRITE_STALAGMITES, SPRITE_RUBBLE,
    SPRITE_IRON_SWORD, SPRITE_WOOD_STAFF, SPRITE_STEEL_SWORD, SPRITE_RUNE_STAFF,
    SPRITE_BACKPACK, SPRITE_LAMP, SPRITE_COUNT
} SpriteId;

typedef struct Sprite {
    Texture2D texture;
    Rectangle source; /* Visible alpha bounds; original PNG is never modified. */
    bool opaque; /* RGB images can cover the map's fallback tiles completely. */
} Sprite;

typedef struct UiAssets {
    Font font;
    bool ownsFont;
    bool supportsChinese;
    bool usesSystemFont;
    Sprite sprites[SPRITE_COUNT];
} UiAssets;

void LoadUiAssets(UiAssets *assets);
void UnloadUiAssets(UiAssets *assets);
bool DrawSprite(const UiAssets *assets, SpriteId id, Vector2 feet, Vector2 maximumSize, Color tint);
SpriteId GetFacingSprite(SpriteId down, Vector2 direction);

#endif
