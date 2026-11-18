/* 资源管理：图片映射、贴图绘制、中文字符收集、字体加载与资源释放。 */
#include "assets.h"
#include "text.h"

#include <stdio.h>

/* ===== 图片文件映射与贴图绘制 ===== */
static void LoadSprites(UiAssets *assets)
{
    const char *const names[SPRITE_COUNT] = { "map_main1.png", "house_leader.png", "house_seller.png",
        "player_down.png", "player_left.png", "player_right.png", "player_up.png",
        "gobin_down.png", "gobin_left.png", "gobin_right.png", "gobin_up.png", "boss.png", "leader.png", "seller.png",
        "map_two.png", "medical_stone.png", "stone.png", "Transport.png", "tree.png",
        "boss_arena.png", "cave_stalagmites.png", "cave_rubble.png",
        "iron_sword.png", "wood_staff.png", "weapon_sword.png", "weapon_staff.png", "backpack.png", "street_lamp.png" };
    for (int i = 0; i < SPRITE_COUNT; i++) {
        assets->sprites[i] = (Sprite){ 0 };
        char path[1024];
        int length = snprintf(path, sizeof(path), "%s../pic/%s", GetApplicationDirectory(), names[i]);
        if (length < 0 || (size_t)length >= sizeof(path) || !FileExists(path)) continue;
        Image image = LoadImage(path);
        if (!IsImageValid(image)) continue;
        Rectangle source = GetImageAlphaBorder(image, 0.05f);
        if (source.width > 0 && source.height > 0) {
            Texture2D texture = LoadTextureFromImage(image);
            if (IsTextureValid(texture)) {
                SetTextureFilter(texture, TEXTURE_FILTER_POINT);
                assets->sprites[i] = (Sprite){ texture, source, image.format == PIXELFORMAT_UNCOMPRESSED_R8G8B8 };
            }
        }
        UnloadImage(image);
    }
}

SpriteId GetFacingSprite(SpriteId down, Vector2 direction)
{
    if (direction.y < 0 && direction.y * direction.y >= direction.x * direction.x) return (SpriteId)(down + 3);
    if (direction.y > 0 && direction.y * direction.y >= direction.x * direction.x) return down;
    if (direction.x < 0) return (SpriteId)(down + 1);
    if (direction.x > 0) return (SpriteId)(down + 2);
    return down;
}

bool DrawSprite(const UiAssets *assets, SpriteId id, Vector2 feet, Vector2 maximumSize, Color tint)
{
    if (!assets || id < 0 || id >= SPRITE_COUNT) return false;
    Sprite sprite = assets->sprites[id];
    if (!IsTextureValid(sprite.texture) || sprite.source.width <= 0 || sprite.source.height <= 0) return false;
    float scale = maximumSize.x / sprite.source.width;
    float heightScale = maximumSize.y / sprite.source.height;
    if (heightScale < scale) scale = heightScale;
    float width = sprite.source.width * scale, height = sprite.source.height * scale;
    DrawTexturePro(sprite.texture, sprite.source, (Rectangle){ feet.x - width / 2, feet.y - height, width, height },
                   (Vector2){ 0, 0 }, 0, tint);
    return true;
}

/* Only requested characters enter the GPU atlas, not the whole CJK font. */
/* ===== 字符收集与字体加载 ===== */
static bool AddTextCodepoints(const char *text, int *codepoints,
                              int *count, int capacity)
{
    int offset = 0;

    while (text[offset] != '\0') {
        int byteCount = 0;
        int codepoint = GetCodepointNext(text + offset, &byteCount);
        bool found = false;

        if (byteCount <= 0) return false;
        for (int i = 0; i < *count; i++) {
            if (codepoints[i] == codepoint) {
                found = true;
                break;
            }
        }
        if (!found) {
            if (*count >= capacity) return false;
            codepoints[*count] = codepoint;
            (*count)++;
        }
        offset += byteCount;
    }
    return true;
}

void LoadUiAssets(UiAssets *assets)
{
    LoadSprites(assets); /* Independent of font fallback and its early returns. */
    int codepoints[2048];
    int count = 0;
    char fontPath[1024];
    bool valid = true;

    assets->font = GetFontDefault();
    assets->ownsFont = false;
    assets->supportsChinese = false;
    assets->usesSystemFont = false;

    /* The executable lives in bin/. Resolve from its location, not terminal cwd. */
    int length = snprintf(fontPath, sizeof(fontPath), "%s../assets/fonts/NotoSansCJKsc-Regular.otf",
                          GetApplicationDirectory());
    if (length < 0 || (size_t)length >= sizeof(fontPath)) return;
    if (!FileExists(fontPath)) {
        /* Development fallback only: read the installed font without copying it.
           A portable submission needs a redistributable Simplified Chinese font. */
        const char *systemFont = "C:/Windows/Fonts/simhei.ttf";
        if (!FileExists(systemFont)) {
            TraceLog(LOG_WARNING, "APP: No Chinese font available; English fallback enabled");
            return;
        }
        snprintf(fontPath, sizeof(fontPath), "%s", systemFont);
        assets->usesSystemFont = true;
        TraceLog(LOG_WARNING, "APP: Using installed SimHei for development; not a portable asset");
    }

    if (!FileExists(fontPath)) {
        return;
    }

    /* Include ASCII for the project title and English error messages as well. */
    for (int cp = 32; cp <= 126; cp++) codepoints[count++] = cp;
    for (int language = 0; language < LANGUAGE_COUNT && valid; language++) {
        for (int id = 0; id < TEXT_COUNT && valid; id++) {
            valid = AddTextCodepoints(GetUiText((Language)language, (TextId)id),
                                      codepoints, &count, 2048);
        }
    }
    if (!valid) {
        TraceLog(LOG_WARNING, "APP: Font character list exceeded its capacity");
        return;
    }

    Font loadedFont = LoadFontEx(fontPath, 32, codepoints, count);
    if (!IsFontValid(loadedFont) || loadedFont.texture.id == GetFontDefault().texture.id) {
        TraceLog(LOG_WARNING, "APP: Custom font loading failed");
        return;
    }

    for (int i = 0; i < count; i++) {
        int glyphIndex = GetGlyphIndex(loadedFont, codepoints[i]);
        if (loadedFont.glyphs[glyphIndex].value != codepoints[i]) {
            valid = false;
            TraceLog(LOG_WARNING, "APP: Missing font character U+%04X", codepoints[i]);
        }
    }
    if (!valid) {
        UnloadFont(loadedFont);
        return;
    }

    SetTextureFilter(loadedFont.texture, TEXTURE_FILTER_BILINEAR);
    assets->font = loadedFont;
    assets->ownsFont = true;
    assets->supportsChinese = true;
    TraceLog(LOG_INFO, "APP: All %i requested font characters are available", count);
}

/* ===== 资源释放 ===== */
void UnloadUiAssets(UiAssets *assets)
{
    for (int i = 0; i < SPRITE_COUNT; i++) {
        if (IsTextureValid(assets->sprites[i].texture)) UnloadTexture(assets->sprites[i].texture);
        assets->sprites[i] = (Sprite){ 0 };
    }
    /* The default font belongs to raylib; unload only our own resource. */
    if (assets->ownsFont) UnloadFont(assets->font);
    assets->ownsFont = false;
    assets->supportsChinese = false;
    assets->usesSystemFont = false;
}
