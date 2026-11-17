#include "ui.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

/* C99 color literals keep the existing palette without file-scope variables. */
#define UI_BG ((Color){ 16, 20, 28, 255 })
#define UI_PANEL ((Color){ 28, 34, 46, 245 })
#define UI_BORDER ((Color){ 72, 84, 102, 255 })
#define UI_TEXT ((Color){ 232, 226, 210, 255 })
#define UI_MUTED ((Color){ 150, 156, 168, 255 })
#define UI_ACCENT ((Color){ 214, 164, 74, 255 })
#define UI_DANGER ((Color){ 190, 76, 62, 255 })
#define INTERACTION_BUTTON ((Rectangle){ 364, 692, 296, 52 })
#define CLOSE_BUTTON ((Rectangle){ 392, 452, 240, 52 })
#define ITEM_CLOSE_BUTTON ((Rectangle){ 664, 628, 240, 44 })
#define HUD_PANEL ((Rectangle){ 12, 12, 1000, 108 })
#define BAG_BUTTON ((Rectangle){ 880, 602, 132, 150 })

static void DrawItemIcon(const UiAssets *assets, ItemId item, Vector2 centre, float size);

/* A click is queried, not consumed: each screen still decides its priority. */
static bool Clicked(Rectangle bounds)
{
    return IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), bounds);
}

static bool Activated(int key, Rectangle bounds)
{
    return IsKeyPressed(key) || Clicked(bounds);
}

bool IsPointerOverHud(const App *app)
{
    Vector2 mouse = GetMousePosition();
    return !CheckCollisionPointRec(mouse, (Rectangle){ 0, 0, GAME_WIDTH, GAME_HEIGHT }) ||
        CheckCollisionPointRec(mouse, HUD_PANEL) ||
        CheckCollisionPointRec(mouse, (Rectangle){ 24, 128, 620, 48 }) ||
        CheckCollisionPointRec(mouse, (Rectangle){ 660, 128, 352, 48 }) ||
        CheckCollisionPointRec(mouse, BAG_BUTTON) ||
        CheckCollisionPointRec(mouse, (Rectangle){ 24, 736, 560, 28 }) ||
        (app->nearbyInteraction >= 0 && CheckCollisionPointRec(mouse, INTERACTION_BUTTON));
}

/* Like printf: the optional arguments fill %d / %.1f in the translated text. */
static void DrawLabel(const App *app, TextId id, float x, float y, float size, Color color, ...)
{
    char text[512];
    va_list values;
    va_start(values, color);
    vsnprintf(text, sizeof(text), GetUiText(app->language, id), values);
    va_end(values);
    DrawTextEx(app->assets.font, text, (Vector2){ x, y }, size, 1, color);
}

static void DrawTextInBox(const App *app, Rectangle bounds, const char *text, float size, Color color)
{
    Vector2 measured = MeasureTextEx(app->assets.font, text, size, 1);
    DrawTextEx(app->assets.font, text, (Vector2){ bounds.x + (bounds.width - measured.x) / 2,
        bounds.y + (bounds.height - measured.y) / 2 }, size, 1, color);
}

static void DrawOverlay(float top, float height, unsigned char shade)
{
    DrawRectangle(0, HUD_HEIGHT, 1024, 640, (Color){ 0, 0, 0, shade });
    DrawRectangle(96, (int)top, 832, (int)height, (Color){ 23, 32, 43, 255 });
}

static void DrawUiPanel(Rectangle bounds, float roundness, Color fill, Color border)
{
    DrawRectangleRounded(bounds, roundness, 8, fill);
    DrawRectangleRoundedLinesEx(bounds, roundness, 8, 1.5f, border);
}

static void DrawInsetBar(Rectangle bounds, float ratio, Color fill)
{
    if (ratio < 0.0f) ratio = 0.0f;
    if (ratio > 1.0f) ratio = 1.0f;
    DrawRectangleRounded(bounds, 0.5f, 8, (Color){ 58, 42, 28, 255 });
    Rectangle value = { bounds.x + 1, bounds.y + 1, (bounds.width - 2) * ratio, bounds.height - 2 };
    if (value.width > 0.0f) DrawRectangleRounded(value, 0.5f, 8, fill);
    DrawRectangleRoundedLinesEx(bounds, 0.5f, 8, 1.0f, (Color){ 30, 20, 14, 255 });
}

static void DrawMedievalPanel(Rectangle bounds)
{
    DrawRectangleRounded((Rectangle){ bounds.x + 3, bounds.y + 4, bounds.width, bounds.height },
                         0.04f, 8, (Color){ 0, 0, 0, 90 });
    DrawRectangleRounded(bounds, 0.04f, 8, (Color){ 96, 57, 24, 255 });
    DrawRectangleRoundedLinesEx(bounds, 0.04f, 8, 2.0f, (Color){ 28, 18, 12, 255 });
    DrawRectangleRoundedLinesEx((Rectangle){ bounds.x + 2, bounds.y + 2, bounds.width - 4, bounds.height - 4 },
        0.04f, 8, 2, (Color){ 205, 155, 67, 255 });
    Rectangle paper = { bounds.x + 6, bounds.y + 6, bounds.width - 12, bounds.height - 12 };
    DrawRectangleGradientV((int)paper.x, (int)paper.y, (int)paper.width, (int)paper.height,
        (Color){ 243, 222, 179, 255 }, (Color){ 211, 176, 120, 255 });
    DrawRectangleRoundedLinesEx(paper, 0.03f, 8, 1.0f, (Color){ 128, 102, 68, 255 });
    Vector2 rivets[4] = {
        { bounds.x + 4, bounds.y + 4 }, { bounds.x + bounds.width - 4, bounds.y + 4 },
        { bounds.x + 4, bounds.y + bounds.height - 4 }, { bounds.x + bounds.width - 4, bounds.y + bounds.height - 4 }
    };
    for (int i = 0; i < 4; i++) {
        DrawCircleV(rivets[i], 3.5f, (Color){ 52, 31, 16, 255 });
        DrawCircleV(rivets[i], 2.5f, (Color){ 223, 171, 69, 255 });
        DrawCircleV((Vector2){ rivets[i].x - 0.7f, rivets[i].y - 0.8f }, 0.9f, (Color){ 255, 232, 163, 255 });
    }
}

typedef enum PreviewButton {
    BUTTON_ENGLISH,
    BUTTON_CHINESE,
    BUTTON_START,
    BUTTON_EXIT,
    BUTTON_CONTINUE,
    BUTTON_COUNT
} PreviewButton;

static Rectangle GetButtonBounds(PreviewButton button)
{
    if (button == BUTTON_CONTINUE) return (Rectangle){ 392, 556, 240, 48 };
    return (Rectangle){ 128.0f + (float)button * 196.0f, 490.0f, 184.0f, 52.0f };
}

static Rectangle GetDeathButton(bool town)
{
    return (Rectangle){ town ? 232 : 528, 452, 264, 52 };
}

static Rectangle GetBagSlot(int slot)
{
    return (Rectangle){ 128.0f + (float)(slot % 4) * 116, 260.0f + (float)(slot / 4) * 100, 108, 88 };
}

static Rectangle GetProductCard(int product)
{
    return (Rectangle){ 128, 284.0f + (float)product * 124, 768, 100 };
}

static Rectangle GetItemAction(bool shop)
{
    return (Rectangle){ shop ? 392 : 128, 628, 224, 44 };
}

static void OpenFromMenu(App *app, bool preferSave)
{
    if (app->screen == SCREEN_PAUSE) app->screen = SCREEN_PLAY;
    else if (preferSave && app->saveAvailable) ContinueGame(app);
    else if (app->saveExists) app->screen = SCREEN_NEW_CONFIRM;
    else StartNewGame(app);
}

void UpdateUi(App *app)
{
    if (app->screen == SCREEN_NEW_CONFIRM || app->screen == SCREEN_SAVE_ERROR) {
        bool newGame = app->screen == SCREEN_NEW_CONFIRM;
        if (Activated(KEY_ESCAPE, ITEM_CLOSE_BUTTON)) {
            app->screen = newGame ? SCREEN_MENU : app->beforeSaveError;
        } else if (Activated(KEY_ENTER, GetItemAction(true))) {
            if (newGame) StartNewGame(app); else RequestAppExit(app);
        } else if (!newGame && Clicked(CLOSE_BUTTON)) {
            app->exitRequested = true; /* Explicit choice after an unsuccessful save. */
        }
        return;
    }

    if (app->screen == SCREEN_QUEST) {
        if (Activated(KEY_ESCAPE, ITEM_CLOSE_BUTTON)) app->screen = SCREEN_PLAY;
        else if (Activated(KEY_ENTER, GetItemAction(true))) ConfirmQuestDialogue(app);
        return;
    }
    if (app->screen == SCREEN_BOSS_VICTORY || app->screen == SCREEN_COMPLETE) {
        if (IsKeyPressed(KEY_ESCAPE) || Activated(KEY_ENTER, CLOSE_BUTTON)) app->screen = SCREEN_PLAY;
        return;
    }

    if (app->screen == SCREEN_INVENTORY || app->screen == SCREEN_SHOP) {
        bool shop = app->screen == SCREEN_SHOP;
        if (Activated(KEY_ESCAPE, ITEM_CLOSE_BUTTON) || (!shop && IsKeyPressed(KEY_B))) { app->screen = SCREEN_PLAY; return; }
        int *selected = shop ? &app->selectedProduct : &app->selectedSlot;
        TextId help = shop ? TEXT_SHOP_HELP : TEXT_BAG_HELP;
        int movement = IsKeyPressed(KEY_RIGHT) ? 1 : IsKeyPressed(KEY_LEFT) ? -1 :
                       IsKeyPressed(KEY_DOWN) ? 4 : IsKeyPressed(KEY_UP) ? -4 : 0;
        if (movement != 0) {
            *selected = shop ? 1 - *selected : (*selected + movement + INVENTORY_CAPACITY) % INVENTORY_CAPACITY;
            app->inventoryMessage = help;
        }
        for (int i = 0; i < (shop ? 2 : INVENTORY_CAPACITY); i++)
            if (Clicked(shop ? GetProductCard(i) : GetBagSlot(i))) { *selected = i; app->inventoryMessage = help; }
        if (Activated(KEY_ENTER, GetItemAction(shop))) {
            if (shop) BuySelectedProduct(app); else EquipSelectedItem(app);
        }
        return;
    }

    if (app->screen == SCREEN_DEATH) {
        if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) app->deathReturnToTown = true;
        if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) app->deathReturnToTown = false;
        if (IsKeyPressed(KEY_ENTER)) RespawnAppPlayer(app, app->deathReturnToTown);
        else if (Clicked(GetDeathButton(true))) RespawnAppPlayer(app, true);
        else if (Clicked(GetDeathButton(false))) RespawnAppPlayer(app, false);
        return;
    }

    if (app->screen == SCREEN_INTERACTION) {
        if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_E) || Activated(KEY_ENTER, CLOSE_BUTTON)) app->screen = SCREEN_PLAY;
        return;
    }
    if (app->screen == SCREEN_PLAY) {
        if (IsKeyPressed(KEY_ESCAPE)) {
            app->screen = SCREEN_PAUSE;
        } else if (Activated(KEY_B, BAG_BUTTON)) {
            app->screen = SCREEN_INVENTORY;
            app->inventoryMessage = TEXT_BAG_HELP;
        } else if (app->nearbyInteraction >= 0 &&
                   Activated(KEY_E, INTERACTION_BUTTON)) {
            ActivateNearbyInteraction(app);
        }
        return;
    }
    if (app->screen == SCREEN_MENU && IsKeyPressed(KEY_C)) { ContinueGame(app); return; }
    if (app->screen == SCREEN_MENU && IsKeyPressed(KEY_N)) {
        OpenFromMenu(app, false);
        return;
    }
    if (IsKeyPressed(KEY_ENTER) || (app->screen == SCREEN_PAUSE && IsKeyPressed(KEY_ESCAPE))) {
        OpenFromMenu(app, true);
        return;
    }
    if (Clicked(GetButtonBounds(BUTTON_ENGLISH))) {
        SetAppLanguage(app, LANGUAGE_ENGLISH);
    } else if (Clicked(GetButtonBounds(BUTTON_CHINESE))) {
        SetAppLanguage(app, LANGUAGE_CHINESE);
    } else if (Clicked(GetButtonBounds(BUTTON_START))) {
        OpenFromMenu(app, false);
    } else if (app->screen == SCREEN_MENU && app->saveAvailable && Clicked(GetButtonBounds(BUTTON_CONTINUE))) {
        ContinueGame(app);
    } else if (Clicked(GetButtonBounds(BUTTON_EXIT))) {
        RequestAppExit(app);
    }
}

static void DrawCenteredText(Font font, const char *text, float y,
                              float fontSize, Color color)
{
    Vector2 size = MeasureTextEx(font, text, fontSize, 1.0f);
    DrawTextEx(font, text, (Vector2){ (1024.0f - size.x) / 2.0f, y },
               fontSize, 1.0f, color);
}

static void DrawCenteredLabel(const App *app, TextId id, float y, float size, Color color)
{
    const char *text = GetUiText(app->language, id);
    DrawCenteredText(app->assets.font, text, y, size, color);
}

static void DrawWrappedParagraph(Font font, const char *text, Vector2 origin, float width, float size)
{
    float x = origin.x;
    float y = origin.y;
    int offset = 0;
    while (text[offset] != '\0') {
        int bytes = 0;
        int codepoint = GetCodepointNext(text + offset, &bytes);
        if (bytes <= 0 || bytes > 4) break;
        char glyph[5] = { 0 };
        memcpy(glyph, text + offset, (size_t)bytes);
        float advance = MeasureTextEx(font, glyph, size, 0).x + 1;
        if (codepoint == ' ') {
            char word[256] = { 0 };
            int count = 0;
            int next = offset + bytes;
            while (text[next] && text[next] != ' ' && count < 255) word[count++] = text[next++];
            float wordWidth = MeasureTextEx(font, word, size, 1).x;
            if (x + advance + wordWidth > origin.x + width) { x = origin.x; y += size + 10; offset += bytes; continue; }
        }
        if (x + advance > origin.x + width) { x = origin.x; y += size + 10; }
        DrawTextCodepoint(font, codepoint, (Vector2){ x, y }, size, WHITE);
        x += advance;
        offset += bytes;
    }
}

static void DrawButton(const App *app, PreviewButton button, TextId label,
                       bool selected, bool enabled)
{
    Rectangle bounds = GetButtonBounds(button);
    bool hovered = enabled && CheckCollisionPointRec(GetMousePosition(), bounds);
    Color fill = !enabled ? (Color){ 40, 44, 52, 255 } :
                 selected ? (Color){ 48, 86, 72, 255 } :
                 hovered ? (Color){ 44, 56, 72, 255 } : (Color){ 30, 38, 50, 255 };
    Color ink = enabled ? UI_TEXT : (Color){ 112, 118, 128, 255 };
    const char *text = GetUiText(app->language, label);
    if (button == BUTTON_CHINESE && !app->assets.supportsChinese) text = "Chinese";
    DrawUiPanel(bounds, 0.16f, fill, selected ? UI_ACCENT : UI_BORDER);
    DrawTextInBox(app, bounds, text, 24, ink);
}

static void DrawMenu(const App *app)
{
    Color ink = UI_TEXT;
    Color muted = UI_MUTED;

    ClearBackground(UI_BG);
    /* Menu decorations are placeholders independent of playable map data. */
    DrawRectangle(0, 630, 1024, 138, (Color){ 22, 40, 35, 255 });
    for (int i = 0; i < 16; i++) {
        int height = 32 + (i % 4) * 16;
        DrawRectangle(i * 64, 630 - height, 48, height, (Color){ 27, 38, 44, 255 });
    }
    DrawUiPanel((Rectangle){ 76, 64, 872, 568 }, 0.02f, UI_PANEL, UI_BORDER);
    DrawCenteredText(app->assets.font, "project-lax", 124.0f, 52.0f, UI_ACCENT);
    DrawCenteredLabel(app, TEXT_SUBTITLE, 206.0f, 24.0f, ink);
    DrawCenteredLabel(app, TEXT_STAGE, 300.0f, 22.0f, UI_ACCENT);
    DrawCenteredLabel(app, TEXT_DESCRIPTION, 350.0f, 20.0f, muted);
    DrawCenteredLabel(app, TEXT_LANGUAGE, 426.0f, 22.0f, ink);

    DrawButton(app, BUTTON_ENGLISH, TEXT_ENGLISH,
                app->language == LANGUAGE_ENGLISH, true);
    DrawButton(app, BUTTON_CHINESE, TEXT_CHINESE,
                app->language == LANGUAGE_CHINESE, app->assets.supportsChinese);
    DrawButton(app, BUTTON_START, app->screen == SCREEN_PAUSE ? TEXT_RESUME : TEXT_START, false, true);
    DrawButton(app, BUTTON_EXIT, TEXT_EXIT, false, true);
    DrawButton(app, BUTTON_CONTINUE, TEXT_LOAD_GAME, false, app->saveAvailable);
    DrawCenteredLabel(app, app->saveMessage, 644.0f, 18.0f, app->saveAvailable ? SKYBLUE : ORANGE);
    DrawCenteredLabel(app, TEXT_FOOTER, 700.0f, 18.0f, muted);
    if (!app->assets.supportsChinese) {
        DrawCenteredText(app->assets.font, GetUiText(LANGUAGE_ENGLISH, TEXT_FONT_ERROR),
                          674.0f, 16.0f, ORANGE);
    } else if (app->assets.usesSystemFont) {
        DrawCenteredLabel(app, TEXT_SYSTEM_FONT, 674.0f, 16.0f, ORANGE);
    }
}

static TextId GetInteractionName(Interaction object)
{
    if (object.kind == INTERACTION_EXIT) {
        return object.destination == MAP_TOWN ? TEXT_TOWN_EXIT :
               object.destination == MAP_BOSS ? TEXT_BOSS_EXIT : TEXT_FIELD_EXIT;
    }
    const TextId names[] = { TEXT_ELDER, TEXT_MERCHANT, TEXT_HEAL_POINT, TEXT_FIELD_EXIT };
    return names[object.kind];
}

/* Small HUD ornaments are drawn by raylib; values and labels remain live text. */
typedef enum HudSymbol { HUD_HEART, HUD_LEAF, HUD_COIN } HudSymbol;

static void DrawHudSymbol(HudSymbol symbol, Vector2 centre, float size)
{
    Color edge = (Color){ 59, 35, 18, 255 };
    float x = centre.x, y = centre.y, r = size / 2;
    if (symbol == HUD_HEART) {
        DrawCircle((int)(x - r * 0.4f), (int)(y - r * 0.3f), r * 0.65f, edge);
        DrawCircle((int)(x + r * 0.4f), (int)(y - r * 0.3f), r * 0.65f, edge);
        DrawTriangle((Vector2){ x - r, y }, (Vector2){ x, y + r }, (Vector2){ x + r, y }, edge);
        DrawCircle((int)(x - r * 0.38f), (int)(y - r * 0.3f), r * 0.5f, RED);
        DrawCircle((int)(x + r * 0.38f), (int)(y - r * 0.3f), r * 0.5f, RED);
        DrawTriangle((Vector2){ x - r * 0.83f, y }, (Vector2){ x, y + r * 0.8f }, (Vector2){ x + r * 0.83f, y }, MAROON);
        DrawCircle((int)(x - r * 0.48f), (int)(y - r * 0.5f), r * 0.17f, (Color){ 255, 189, 153, 255 });
    } else if (symbol == HUD_LEAF) {
        DrawEllipse((int)x, (int)y, r * 0.7f + 2, r + 2, edge);
        DrawEllipse((int)x, (int)y, r * 0.7f, r, (Color){ 99, 159, 38, 255 });
        DrawLineEx((Vector2){ x, y + r }, (Vector2){ x, y - r * 0.7f }, 2, (Color){ 211, 223, 111, 255 });
        for (int i = -1; i <= 1; i++) {
            DrawLineEx((Vector2){ x, y + i * r * 0.4f }, (Vector2){ x + r * 0.45f, y + (i - 1) * r * 0.4f }, 1, DARKGREEN);
        }
    } else {
        DrawCircleV(centre, r + 2, edge);
        DrawCircleV(centre, r, (Color){ 232, 176, 43, 255 });
        DrawCircleLines((int)x, (int)y, r - 3, (Color){ 255, 228, 142, 255 });
        DrawLineEx((Vector2){ x - r * 0.5f, y + r * 0.25f }, (Vector2){ x + r * 0.5f, y + r * 0.25f }, 3, edge);
        for (int i = -1; i <= 1; i++) {
            Vector2 tip = { x + i * r * 0.45f, y - r * (i == 0 ? 0.5f : 0.25f) };
            DrawLineEx((Vector2){ x + i * r * 0.4f, y + r * 0.25f }, tip, 3, edge);
            DrawCircleV(tip, 2, (Color){ 255, 235, 156, 255 });
        }
    }
}

static void DrawHudMeter(const App *app, Rectangle bounds, float ratio, Color color,
                         HudSymbol symbol, const char *label)
{
    DrawInsetBar(bounds, ratio, color);
    if (ratio > 0) {
        float fillWidth = (bounds.width - 6) * (ratio > 1 ? 1 : ratio);
        DrawRectangleGradientV((int)bounds.x + 3, (int)bounds.y + 3, (int)fillWidth, (int)bounds.height - 6,
            ColorBrightness(color, 0.3f), color);
    }
    DrawRectangleLinesEx((Rectangle){ bounds.x - 2, bounds.y - 2, bounds.width + 4, bounds.height + 4 },
        1, (Color){ 157, 112, 47, 255 });
    DrawHudSymbol(symbol, (Vector2){ bounds.x + 2, bounds.y + bounds.height / 2 }, 25);
    DrawTextEx(app->assets.font, label, (Vector2){ bounds.x + 26, bounds.y + 4 }, 18, 1, (Color){ 30, 19, 14, 255 });
    DrawTextEx(app->assets.font, label, (Vector2){ bounds.x + 25, bounds.y + 3 }, 18, 1, RAYWHITE);
}

static void DrawWorldHud(const App *app)
{
    Color ink = (Color){ 48, 34, 24, 255 };
    DrawMedievalPanel(HUD_PANEL);
    Rectangle weapon = { 28, 25, 78, 68 };
    DrawRectangleGradientV(28, 25, 78, 68, (Color){ 18, 35, 42, 255 }, (Color){ 40, 53, 51, 255 });
    DrawRectangleLinesEx(weapon, 2, (Color){ 133, 85, 30, 255 });
    ItemId currentItem = GetEquippedItem(&app->player.inventory, app->player.weapon);
    DrawItemIcon(&app->assets, currentItem, (Vector2){ 67, 58 }, 60);
    DrawTextInBox(app, (Rectangle){ 24, 94, 86, 18 },
        GetUiText(app->language, app->player.weapon == WEAPON_SWORD ? TEXT_SWORD : TEXT_STAFF), 14, ink);

    DrawCircle(158, 66, 34, (Color){ 61, 34, 17, 255 });
    DrawCircle(158, 66, 31, (Color){ 221, 174, 73, 255 });
    DrawCircle(158, 66, 27, (Color){ 91, 55, 25, 255 });
    DrawCircleLines(158, 66, 25, (Color){ 245, 207, 124, 255 });
    char text[96];
    snprintf(text, sizeof(text), GetUiText(app->language, TEXT_HUD_LEVEL), app->player.level);
    DrawTextInBox(app, (Rectangle){ 126, 50, 64, 30 }, text, 22, RAYWHITE);
    DrawLine(211, 28, 211, 105, (Color){ 172, 129, 78, 255 });

    snprintf(text, sizeof(text), GetUiText(app->language, TEXT_HUD_HP), app->player.health, app->player.maxHealth);
    DrawHudMeter(app, (Rectangle){ 239, 34, 498, 28 }, (float)app->player.health / app->player.maxHealth,
        (Color){ 172, 35, 32, 255 }, HUD_HEART, text);
    int next = ExperienceForNextLevel(app->player.level);
    if (next > 0) snprintf(text, sizeof(text), GetUiText(app->language, TEXT_HUD_XP), app->player.experience, next);
    else snprintf(text, sizeof(text), "%s", GetUiText(app->language, TEXT_XP_MAX));
    DrawHudMeter(app, (Rectangle){ 239, 79, 498, 25 }, next > 0 ? (float)app->player.experience / next : 1,
        (Color){ 83, 133, 26, 255 }, HUD_LEAF, text);

    DrawLine(757, 28, 757, 105, (Color){ 172, 129, 78, 255 });
    DrawUiPanel((Rectangle){ 771, 33, 111, 68 }, 0.08f, (Color){ 207, 169, 110, 255 }, (Color){ 165, 116, 54, 255 });
    DrawHudSymbol(HUD_COIN, (Vector2){ 795, 67 }, 32);
    snprintf(text, sizeof(text), "%d", app->player.gold);
    DrawTextInBox(app, (Rectangle){ 818, 46, 60, 42 }, text, app->player.gold < 10000 ? 22 : 13, ink);

    Color stateColor = app->combat.inCombat ? (Color){ 110, 36, 27, 255 } : (Color){ 47, 83, 29, 255 };
    DrawUiPanel((Rectangle){ 896, 26, 99, 80 }, 0.06f, stateColor, (Color){ 183, 136, 57, 255 });
    if (app->combat.inCombat) DrawItemIcon(&app->assets, ITEM_IRON_SWORD, (Vector2){ 945, 52 }, 36);
    else DrawHudSymbol(HUD_LEAF, (Vector2){ 945, 52 }, 30);
    DrawTextInBox(app, (Rectangle){ 899, 77, 93, 22 },
        GetUiText(app->language, app->combat.inCombat ? TEXT_IN_COMBAT : TEXT_PEACE), 16, RAYWHITE);
    const TextId maps[] = { TEXT_TOWN, TEXT_FIELD, TEXT_BOSS_MAP };
    bool safe = IsBodyInSafeArea(&app->worlds[app->currentMap], GetPlayerBody(app->player.position));
    DrawMedievalPanel((Rectangle){ 660, 128, 352, 48 });
    DrawLabel(app, maps[app->currentMap], 672, 134, 16, ink);
    DrawLabel(app, safe ? TEXT_SAFE : TEXT_OUTSIDE_SAFE, 672, 155, 13,
        safe ? (Color){ 47, 83, 29, 255 } : (Color){ 141, 47, 23, 255 });
    if (app->combat.inCombat && app->combat.lostTimer > 0)
        DrawLabel(app, TEXT_DISENGAGE, 810, 155, 13, ink, (double)(app->combat.rules.disengageDelay - app->combat.lostTimer));
}

static void DrawHudActions(const App *app)
{
    Color ink = (Color){ 48, 34, 24, 255 };
    DrawMedievalPanel(BAG_BUTTON);
    if (CheckCollisionPointRec(GetMousePosition(), BAG_BUTTON))
        DrawRectangleRec((Rectangle){ 888, 610, 116, 134 }, (Color){ 255, 244, 201, 45 });
    if (!DrawSprite(&app->assets, SPRITE_BACKPACK, (Vector2){ 946, 690 }, (Vector2){ 82, 76 }, WHITE))
        DrawTextInBox(app, (Rectangle){ 906, 628, 80, 52 }, "B", 36, ink);
    DrawUiPanel((Rectangle){ 981, 612, 21, 24 }, 0.1f, (Color){ 89, 44, 24, 255 }, (Color){ 52, 30, 17, 255 });
    DrawTextInBox(app, (Rectangle){ 981, 612, 21, 24 }, "B", 17, RAYWHITE);
    DrawTextInBox(app, (Rectangle){ 889, 694, 114, 24 }, GetUiText(app->language, TEXT_HUD_BAG), 22, ink);
}

static void DrawInteractionPrompt(const App *app)
{
    if (app->nearbyInteraction < 0) return;
    Rectangle bounds = INTERACTION_BUTTON;
    Interaction object = app->worlds[app->currentMap].interactions[app->nearbyInteraction];
    bool hovered = CheckCollisionPointRec(GetMousePosition(), bounds);
    DrawRectangleRec(bounds, hovered ? (Color){ 53, 79, 75, 255 } : (Color){ 25, 43, 47, 245 });
    DrawRectangleLinesEx(bounds, 2, GOLD);
    DrawCenteredLabel(app, GetInteractionName(object), 657, 20, WHITE);
    DrawCenteredLabel(app, TEXT_INTERACT, 708, 20, WHITE);
}

static void GetQuestObjective(const App *app, char *text, size_t capacity)
{
    const TextId objectives[] = { TEXT_OBJECTIVE_OFFER, TEXT_OBJECTIVE_FIELD, TEXT_OBJECTIVE_REPORT,
        TEXT_OBJECTIVE_BOSS, TEXT_OBJECTIVE_FINAL, TEXT_OBJECTIVE_COMPLETE };
    if (app->quest.stage == QUEST_CLEAR_FIELD) snprintf(text, capacity, GetUiText(app->language, TEXT_OBJECTIVE_FIELD),
        QuestMeleeProgress(&app->quest, &app->player), QuestRangedProgress(&app->quest, &app->player));
    else snprintf(text, capacity, "%s", GetUiText(app->language, objectives[app->quest.stage]));
}

static void DrawItemIcon(const UiAssets *assets, ItemId item, Vector2 centre, float size)
{
    if (item <= ITEM_NONE || item >= ITEM_COUNT) return;
    ItemDefinition definition = GetItemDefinition(item);
    if (DrawSprite(assets, definition.icon, (Vector2){ centre.x, centre.y + size / 2 },
        (Vector2){ size, size }, WHITE)) return;
    if (definition.weapon == WEAPON_SWORD) {
        DrawRectangle((int)centre.x - 3, (int)centre.y - 18, 6, 28, LIGHTGRAY);
        DrawRectangle((int)centre.x - 10, (int)centre.y + 7, 20, 4, GOLD);
        DrawRectangle((int)centre.x - 2, (int)centre.y + 11, 4, 10, DARKBROWN);
    } else {
        DrawRectangle((int)centre.x - 2, (int)centre.y - 12, 4, 34, BROWN);
        DrawRectangle((int)centre.x - 7, (int)centre.y - 20, 14, 14, SKYBLUE);
    }
    if (definition.attackBonus > 0) DrawRectangle((int)centre.x + 12, (int)centre.y - 18, 5, 5, GOLD);
}

static TextId GetItemBlockReason(const App *app, bool shop)
{
    if (app->combat.inCombat) return TEXT_EQUIP_BUSY;
    if (shop) {
        bool bought = app->selectedProduct == 0 ? app->player.inventory.purchasedSword : app->player.inventory.purchasedStaff;
        if (bought) return TEXT_ALREADY_BOUGHT;
        bool empty = false;
        for (int slot = 0; slot < INVENTORY_CAPACITY; slot++) empty = empty || app->player.inventory.slots[slot] == ITEM_NONE;
        if (!empty) return TEXT_BAG_FULL;
        ItemId item = app->selectedProduct == 0 ? ITEM_STEEL_SWORD : ITEM_RUNE_STAFF;
        if (app->player.gold < GetItemDefinition(item).price) return TEXT_GOLD_SHORT;
    } else {
        ItemId item = app->player.inventory.slots[app->selectedSlot];
        if (item == ITEM_NONE) return TEXT_EMPTY_SLOT;
        if (app->selectedSlot == app->player.inventory.equippedSword || app->selectedSlot == app->player.inventory.equippedStaff) return TEXT_ALREADY_EQUIPPED;
    }
    return TEXT_COUNT; /* sentinel: action is available */
}

static void DrawPanelButton(const App *app, Rectangle bounds, TextId label, bool enabled)
{
    bool hovered = enabled && CheckCollisionPointRec(GetMousePosition(), bounds);
    Color fill = !enabled ? (Color){ 40, 44, 52, 255 } :
                 hovered ? (Color){ 44, 58, 72, 255 } : (Color){ 30, 40, 52, 255 };
    DrawUiPanel(bounds, 0.14f, fill, enabled && hovered ? UI_ACCENT : UI_BORDER);
    const char *text = GetUiText(app->language, label);
    DrawTextInBox(app, bounds, text, 20, enabled ? UI_TEXT : (Color){ 112, 118, 128, 255 });
}

static void DrawItemPanel(const App *app)
{
    bool shop = app->screen == SCREEN_SHOP;
    Font font = app->assets.font;
    DrawOverlay(180, 524, 155);
    DrawLabel(app, shop ? TEXT_SHOP : TEXT_BAG, 128, 212, 28, GOLD);
    DrawLabel(app, TEXT_GOLD, 660, 220, 21, WHITE, app->player.gold);
    if (shop) {
        const ItemId products[] = { ITEM_STEEL_SWORD, ITEM_RUNE_STAFF };
        for (int i = 0; i < 2; i++) {
            ItemDefinition definition = GetItemDefinition(products[i]);
            Rectangle card = GetProductCard(i);
            DrawRectangleRec(card, (Color){ 34, 46, 57, 255 });
            DrawRectangleLinesEx(card, 2, i == app->selectedProduct ? GOLD : GRAY);
            DrawItemIcon(&app->assets, products[i], (Vector2){ card.x + 44, card.y + 44 }, 56);
            DrawLabel(app, definition.name, card.x + 90, card.y + 16, 24, WHITE);
            DrawLabel(app, TEXT_BONUS, card.x + 90, card.y + 58, 19, SKYBLUE, definition.attackBonus);
            DrawLabel(app, TEXT_PRICE, card.x + 450, card.y + 20, 21, GOLD, definition.price);
            bool bought = i == 0 ? app->player.inventory.purchasedSword : app->player.inventory.purchasedStaff;
            if (bought) DrawLabel(app, TEXT_SOLD_OUT, card.x + 450, card.y + 60, 13, GRAY);
        }
    } else {
        for (int slot = 0; slot < INVENTORY_CAPACITY; slot++) {
            Rectangle bounds = GetBagSlot(slot);
            ItemId item = app->player.inventory.slots[slot];
            DrawRectangleRec(bounds, (Color){ 34, 46, 57, 255 });
            DrawRectangleLinesEx(bounds, 2, slot == app->selectedSlot ? GOLD : GRAY);
            DrawItemIcon(&app->assets, item, (Vector2){ bounds.x + 54, bounds.y + 36 }, 40);
            const char *name = GetUiText(app->language, GetItemDefinition(item).name);
            DrawTextEx(font, name, (Vector2){ bounds.x + 6, bounds.y + 66 }, 15, 0.5f, item == ITEM_NONE ? GRAY : WHITE);
            if (slot == app->player.inventory.equippedSword || slot == app->player.inventory.equippedStaff) {
                DrawLabel(app, TEXT_EQUIPPED, bounds.x + 5, bounds.y + 4, 12, GOLD);
            }
        }
        ItemId selected = app->player.inventory.slots[app->selectedSlot];
        ItemDefinition definition = GetItemDefinition(selected);
        DrawLabel(app, definition.name, 650, 276, 24, WHITE);
        DrawLabel(app, TEXT_BONUS, 650, 324, 19, SKYBLUE, definition.attackBonus);
        if (selected != ITEM_NONE) {
            DrawLabel(app, TEXT_ITEM_ATTACK, 650, 366, 19, WHITE, app->player.baseAttack + definition.attackBonus);
        }
        DrawLabel(app, TEXT_SWORD, 650, 432, 18, GOLD);
        DrawLabel(app, GetItemDefinition(GetEquippedItem(&app->player.inventory, WEAPON_SWORD)).name, 650, 460, 20, WHITE);
        DrawLabel(app, TEXT_STAFF, 650, 494, 18, SKYBLUE);
        DrawLabel(app, GetItemDefinition(GetEquippedItem(&app->player.inventory, WEAPON_STAFF)).name, 650, 522, 20, WHITE);
    }
    TextId block = GetItemBlockReason(app, shop);
    TextId message = app->inventoryMessage;
    if ((message == TEXT_BAG_HELP || message == TEXT_SHOP_HELP) && block != TEXT_COUNT) message = block;
    DrawLabel(app, message, 128, 582, 18, block == TEXT_COUNT ? WHITE : ORANGE);
    DrawPanelButton(app, GetItemAction(shop), shop ? TEXT_BUY : TEXT_EQUIP, block == TEXT_COUNT);
    DrawPanelButton(app, ITEM_CLOSE_BUTTON, TEXT_CLOSE, true);
    DrawLabel(app, shop ? TEXT_SHOP_HELP : TEXT_BAG_HELP, 128, 681, 15, GRAY);
}

static void DrawQuestPanel(const App *app)
{
    DrawOverlay(220, 484, 155);
    DrawLabel(app, TEXT_QUEST_TITLE, 128, 250, 28, GOLD);
    DrawLabel(app, TEXT_ELDER, 680, 258, 20, WHITE);
    DrawWrappedParagraph(app->assets.font, GetUiText(app->language, QuestDialogueText(&app->quest, app->dialoguePage)),
                          (Vector2){ 128, 320 }, 768, 20);
    char objective[512];
    GetQuestObjective(app, objective, sizeof(objective));
    DrawTextEx(app->assets.font, objective, (Vector2){ 128, 582 }, 17, 1, SKYBLUE);
    bool morePages = app->dialoguePage + 1 < QuestDialoguePages(&app->quest);
    TextId action = morePages ? TEXT_NEXT : app->quest.stage == QUEST_OFFER ? TEXT_ACCEPT_QUEST :
        app->quest.stage == QUEST_REPORT_FIELD || app->quest.stage == QUEST_REPORT_BOSS ? TEXT_REPORT_QUEST : TEXT_CLOSE;
    DrawPanelButton(app, GetItemAction(true), action, true);
    DrawPanelButton(app, ITEM_CLOSE_BUTTON, TEXT_CLOSE, true);
    char page[32];
    snprintf(page, sizeof(page), "%d / %d", app->dialoguePage + 1, QuestDialoguePages(&app->quest));
    DrawTextEx(app->assets.font, page, (Vector2){ 128, 650 }, 18, 1, GRAY);
}

static void DrawGameWorld(const App *app)
{
    ClearBackground((Color){ 23, 34, 37, 255 });
    BeginScissorMode(0, 0, GAME_WIDTH, GAME_HEIGHT);
    BeginMode2D(app->camera);
    bool visible = app->player.health > 0 && (app->player.hurtTimer <= 0 || (int)(app->player.hurtTimer * 12) % 2 == 0);
    WorldActor actors[ENEMY_CAPACITY];
    int actorCount = 0;
    for (int i = 0; i < app->combat.enemyCount; i++) {
        const Enemy *enemy = &app->combat.enemies[i];
        if (enemy->health > 0) actors[actorCount++] = (WorldActor){ enemy->position,
            enemy->flashTimer > 0 ? WHITE : enemy->kind == ENEMY_MELEE ? LIME :
            enemy->kind == ENEMY_BOSS ? (Color){ 126, 49, 69, 255 } : (Color){ 190, 104, 53, 255 }, enemy->kind == ENEMY_BOSS,
            enemy->kind == ENEMY_BOSS ? SPRITE_BOSS : GetFacingSprite(SPRITE_GOBLIN_DOWN,
                enemy->state == ENEMY_WINDUP ? enemy->attackDirection : enemy->facing),
            enemy->flashTimer > 0 ? (Color){ 255, 160, 160, 255 } : enemy->kind == ENEMY_RANGED ? (Color){ 255, 205, 160, 255 } : WHITE };
    }
    DrawWorld(&app->worlds[app->currentMap], app->player.position, app->player.swordFacing,
               app->player.staffFacing, visible, actors, actorCount, &app->assets);
    DrawCombat(&app->combat, &app->player);
    for (int i = 0; i < app->combat.enemyCount; i++) {
        const Enemy *enemy = &app->combat.enemies[i];
        if (enemy->health > 0) {
            const char *name = GetEnemyName(enemy->kind, app->language);
            Vector2 size = MeasureTextEx(app->assets.font, name, 17, 1);
            DrawTextEx(app->assets.font, name, (Vector2){ enemy->position.x - size.x / 2,
                enemy->position.y - (enemy->kind == ENEMY_BOSS ? 100 : 62) }, 17, 1, WHITE);
        }
    }
    /* Names follow world objects; font resources still belong to App. */
    for (int i = 0; i < app->worlds[app->currentMap].interactionCount; i++) {
        Interaction object = app->worlds[app->currentMap].interactions[i];
        const char *name = GetUiText(app->language, GetInteractionName(object));
        Vector2 size = MeasureTextEx(app->assets.font, name, 17, 1);
        DrawTextEx(app->assets.font, name,
                   (Vector2){ object.position.x - size.x / 2, object.position.y - (object.kind == INTERACTION_HEAL ? 80 : 62) }, 17, 1, WHITE);
    }
    EndMode2D();
    EndScissorMode();
}

static void DrawConfirmation(const App *app)
{
    bool newGame = app->screen == SCREEN_NEW_CONFIRM;
    DrawRectangle(0, 0, 1024, 768, (Color){ 0, 0, 0, 175 });
    DrawRectangleRounded((Rectangle){ 108, 252, 832, 460 }, 0.025f, 8, (Color){ 0, 0, 0, 90 });
    DrawUiPanel((Rectangle){ 96, 244, 832, 460 }, 0.025f, UI_PANEL, UI_BORDER);
    DrawCenteredLabel(app, newGame ? TEXT_NEW_CONFIRM : TEXT_SAVE_FAILED, 284, 28, newGame ? UI_ACCENT : UI_DANGER);
    DrawRectangle(128, 326, 48, 2, newGame ? UI_ACCENT : UI_DANGER);
    DrawWrappedParagraph(app->assets.font, GetUiText(app->language, newGame ? TEXT_NEW_CONFIRM_BODY : TEXT_SAVE_ERROR_BODY),
        (Vector2){ 128, 352 }, 768, 20);
    if (!newGame) DrawPanelButton(app, CLOSE_BUTTON, TEXT_EXIT_UNSAVED, true);
    DrawPanelButton(app, GetItemAction(true), newGame ? TEXT_CONFIRM_NEW : TEXT_RETRY_SAVE, true);
    DrawPanelButton(app, ITEM_CLOSE_BUTTON, TEXT_CANCEL, true);
    return;

}

static void DrawScreenPanel(const App *app)
{
    if (app->screen == SCREEN_PLAY) DrawInteractionPrompt(app);
    if (app->screen == SCREEN_QUEST) {
        DrawQuestPanel(app);
    } else if (app->screen == SCREEN_BOSS_VICTORY || app->screen == SCREEN_COMPLETE) {
        bool complete = app->screen == SCREEN_COMPLETE;
        DrawOverlay(268, 288, 160);
        DrawCenteredLabel(app, complete ? TEXT_GAME_COMPLETE : TEXT_BOSS_VICTORY, 302, 30, GOLD);
        DrawWrappedParagraph(app->assets.font, GetUiText(app->language, complete ? TEXT_GAME_COMPLETE_BODY : TEXT_BOSS_VICTORY_BODY),
                              (Vector2){ 128, 360 }, 768, 20);
        DrawPanelButton(app, CLOSE_BUTTON, TEXT_CONTINUE_EXPLORING, true);
    } else if (app->screen == SCREEN_INVENTORY || app->screen == SCREEN_SHOP) {
        DrawItemPanel(app);
    } else if (app->screen == SCREEN_DEATH) {
        DrawOverlay(268, 288, 165);
        DrawCenteredLabel(app, TEXT_DEAD, 302, 32, RED);
        DrawCenteredLabel(app, TEXT_DEATH_HELP, 376, 20, WHITE);
        for (int i = 0; i < 2; i++) {
            bool town = i == 0;
            Rectangle bounds = GetDeathButton(town);
            DrawRectangleRec(bounds, (Color){ 45, 73, 65, 255 });
            DrawRectangleLinesEx(bounds, 2, app->deathReturnToTown == town ? GOLD : GRAY);
            const char *label = GetUiText(app->language, town ? TEXT_RESPAWN_TOWN : TEXT_RESPAWN_HERE);
            Vector2 size = MeasureTextEx(app->assets.font, label, 21, 1);
            DrawTextEx(app->assets.font, label, (Vector2){ bounds.x + (bounds.width - size.x) / 2, 466 }, 21, 1, WHITE);
        }
    } else if (app->screen == SCREEN_PAUSE) {
        DrawOverlay(260, 324, 150);
        DrawCenteredLabel(app, TEXT_PAUSED, 310, 36, GOLD);
        DrawButton(app, BUTTON_ENGLISH, TEXT_ENGLISH, app->language == LANGUAGE_ENGLISH, true);
        DrawButton(app, BUTTON_CHINESE, TEXT_CHINESE, app->language == LANGUAGE_CHINESE, app->assets.supportsChinese);
        DrawButton(app, BUTTON_START, TEXT_RESUME, false, true);
        DrawButton(app, BUTTON_EXIT, TEXT_EXIT, false, true);
    } else if (app->screen == SCREEN_INTERACTION && app->nearbyInteraction >= 0) {
        Interaction object = app->worlds[app->currentMap].interactions[app->nearbyInteraction];
        DrawOverlay(268, 288, 145);
        DrawCenteredLabel(app, GetInteractionName(object), 302, 28, GOLD);
        DrawCenteredLabel(app, app->interactionText, 376, 20, WHITE);
        Rectangle close = CLOSE_BUTTON;
        DrawRectangleRec(close, (Color){ 45, 73, 65, 255 });
        DrawRectangleLinesEx(close, 2, GOLD);
        DrawCenteredLabel(app, TEXT_CLOSE, 466, 22, WHITE);
    }
}

void DrawUi(const App *app)
{
    if (app->screen == SCREEN_MENU) {
        DrawMenu(app);
        return;
    }

    DrawGameWorld(app);
    DrawWorldHud(app);
    char objective[512];
    GetQuestObjective(app, objective, sizeof(objective));
    DrawMedievalPanel((Rectangle){ 24, 136, 620, 32 });
    DrawTextEx(app->assets.font, objective, (Vector2){ 36, 143 }, 15, 1, (Color){ 48, 34, 24, 255 });
    if (app->rewardNoticeTimer > 0) {
        char reward[128];
        snprintf(reward, sizeof(reward), GetUiText(app->language, TEXT_REWARD),
                 app->rewardNoticeKills * GOBLIN_GOLD, app->rewardNoticeKills * GOBLIN_EXPERIENCE);
        DrawUiPanel((Rectangle){ 320, 138, 384, 34 }, 0.18f, UI_PANEL, UI_ACCENT);
        DrawCenteredText(app->assets.font, reward, 145, 18, UI_ACCENT);
    }

    if (app->screen == SCREEN_NEW_CONFIRM || app->screen == SCREEN_SAVE_ERROR) {
        DrawConfirmation(app);
        return;
    }
    if (app->screen == SCREEN_PLAY) DrawHudActions(app);
    DrawScreenPanel(app);
    DrawMedievalPanel((Rectangle){ 24, 736, 560, 28 });
    DrawLabel(app, app->saveMessage, 36, 742, 13, (Color){ 48, 34, 24, 255 });
}
