/* 程序入口：窗口、缩放画布、主循环、音乐，以及截图检查和巢穴预览入口。 */
#include "app.h"
#include "ui.h"
#include "music.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

/* Keep the game/UI in one coordinate system; only the final image is resized. */
/* ===== 窗口适配 ===== */
static Rectangle GetGameViewport(void)
{
    float scale = fminf((float)GetScreenWidth() / GAME_WIDTH, (float)GetScreenHeight() / GAME_HEIGHT);
    if (scale <= 0) scale = 1;
    return (Rectangle){ (GetScreenWidth() - GAME_WIDTH * scale) / 2,
        (GetScreenHeight() - GAME_HEIGHT * scale) / 2, GAME_WIDTH * scale, GAME_HEIGHT * scale };
}

/* Deterministic visual checks only. Normal play never calls this function. */
/* ===== 开发检查场景（正常游戏不执行） ===== */
static void PrepareSmokeFrame(App *app, int scene)
{
    InitializePlayer(&app->player, app->worlds[MAP_TOWN].spawn);
    InitializeQuest(&app->quest);
    EnterMap(app, MAP_TOWN, app->worlds[MAP_TOWN].spawn);
    SetAppLanguage(app, scene == 1 || scene == 5 ? LANGUAGE_ENGLISH : LANGUAGE_CHINESE);
    app->rewardNoticeTimer = 0;
    app->screen = scene <= 2 ? SCREEN_MENU : scene == 3 ? SCREEN_NEW_CONFIRM : SCREEN_PLAY;
    app->saveAvailable = scene == 1;
    app->saveExists = scene != 0;
    app->saveMessage = scene == 0 ? TEXT_SAVE_NONE : scene == 1 ? TEXT_SAVE_FOUND : scene == 2 ? TEXT_SAVE_BAD : TEXT_SAVE_OK;
    if (scene == 4) {
        GrantPlayerRewards(&app->player, 200, 240);
        app->player.health = 120;
        app->player.weapon = WEAPON_STAFF;
    }
    if (scene == 5) {
        EnterMap(app, MAP_FIELD, (Vector2){ 320, 600 });
        app->combat.inCombat = true;
        app->saveMessage = TEXT_SAVE_BUSY;
    }
    if (scene == 6) { app->screen = SCREEN_SAVE_ERROR; app->saveMessage = TEXT_SAVE_FAILED; }
    if (scene == 7) {
        app->quest.stage = QUEST_COMPLETE;
        app->quest.bossDefeated = true;
        app->saveMessage = TEXT_SAVE_LOADED;
        EnterMap(app, MAP_BOSS, app->worlds[MAP_BOSS].spawn);
    }
    if (scene == 8) EnterMap(app, MAP_TOWN, (Vector2){ 480, 300 });
    if (scene >= 9 && scene <= 11) {
        const Vector2 facings[] = { { -1, 0 }, { 1, 0 }, { 0, -1 } };
        app->player.swordFacing = facings[scene - 9];
        app->player.staffFacing = app->player.swordFacing;
    }
    if (scene == 12) EnterMap(app, MAP_BOSS, (Vector2){ 468, 400 });
    if (scene == 13) EnterMap(app, MAP_FIELD, (Vector2){ 560, 720 });
    if (scene == 14) EnterMap(app, MAP_BOSS, (Vector2){ 384, 256 });
    if (scene == 15) EnterMap(app, MAP_BOSS, (Vector2){ 448, 576 });
    if (scene >= 16 && scene <= 19) {
        app->player.gold = 200;
        BuyInventoryItem(&app->player.inventory, &app->player.gold, ITEM_STEEL_SWORD);
        BuyInventoryItem(&app->player.inventory, &app->player.gold, ITEM_RUNE_STAFF);
        SetAppLanguage(app, scene % 2 ? LANGUAGE_ENGLISH : LANGUAGE_CHINESE);
        app->screen = scene <= 17 ? SCREEN_INVENTORY : SCREEN_SHOP;
        app->selectedSlot = 2;
        app->selectedProduct = 1;
        app->inventoryMessage = scene <= 17 ? TEXT_BAG_HELP : TEXT_SHOP_HELP;
    }
    if (scene == 20) {
        EnterMap(app, MAP_BOSS, (Vector2){ 468, 400 });
        SetAppLanguage(app, LANGUAGE_ENGLISH);
    }
    if (scene == 21) {
        EnterMap(app, MAP_FIELD, (Vector2){ 320, 600 });
        SetAppLanguage(app, LANGUAGE_CHINESE);
    }
    if (scene == 22) GrantPlayerRewards(&app->player, 0, 1000);
    if (scene == 23) { app->screen = SCREEN_QUEST; app->dialoguePage = 1; }
    if (scene == 24) app->screen = SCREEN_PAUSE;
    if (scene == 25 || scene == 29) {
        EnterMap(app, MAP_FIELD, app->worlds[MAP_FIELD].spawn);
        app->player.health = 0;
        app->deathReturnToTown = scene == 25;
        app->screen = SCREEN_DEATH;
    }
    if (scene == 26) {
        app->nearbyInteraction = 2;
        app->interactionText = TEXT_HEALED;
        app->screen = SCREEN_INTERACTION;
    }
    if (scene == 27 || scene == 28) {
        app->quest.bossDefeated = true;
        app->quest.stage = scene == 27 ? QUEST_REPORT_BOSS : QUEST_COMPLETE;
        app->screen = scene == 27 ? SCREEN_BOSS_VICTORY : SCREEN_COMPLETE;
    }
    if (scene >= 30 && scene <= 34) {
        const Vector2 views[] = { {256,320}, {1008,384}, {256,1088}, {1008,1088}, {144,208} };
        EnterMap(app, MAP_FIELD, views[scene - 30]);
    }
    if (scene == 35) EnterMap(app, MAP_FIELD, (Vector2){ 1104, 656 });
    if (scene == 36) EnterMap(app, MAP_BOSS, app->worlds[MAP_BOSS].spawn);
}

/* ===== 程序启动、主循环与退出 ===== */
int main(int argc, char **argv)
{
    bool resizeTest = argc == 2 && strcmp(argv[1], "--resize-test") == 0;
    bool smokeTest = resizeTest || (argc == 2 && strcmp(argv[1], "--smoke-test") == 0);
    bool previewBoss = argc == 2 && strcmp(argv[1], "--preview-boss") == 0;
    int frameCount = 0;
    bool screenshotsSaved = true;
    App app = { 0 };
    BackgroundMusic music = { .activeTrack = -1 };
    float musicNoticeTimer = 0;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | (smokeTest ? FLAG_WINDOW_HIDDEN : 0));
    InitWindow(GAME_WIDTH, GAME_HEIGHT, "project-lax");
    if (!IsWindowReady()) return 1;
    SetWindowMinSize(640, 480);
    RenderTexture2D canvas = LoadRenderTexture(GAME_WIDTH, GAME_HEIGHT);
    if (!IsRenderTextureValid(canvas)) { CloseWindow(); return 1; }
    SetTextureFilter(canvas.texture, TEXTURE_FILTER_BILINEAR);
    SetExitKey(KEY_NULL); /* Esc belongs to panels/pause, not immediate exit. */
    SetTargetFPS(60);
    InitializeApp(&app);
    /* 截图检查保持静音且不依赖本机声卡。 */
    if (!smokeTest) InitializeBackgroundMusic(&music);
    if (smokeTest || previewBoss) {
        app.persistenceEnabled = false;
        app.saveMessage = TEXT_SAVE_PREVIEW;
    }
    if (previewBoss) EnterMap(&app, MAP_BOSS, app.worlds[MAP_BOSS].spawn);

    while (!app.exitRequested) {
        if (WindowShouldClose()) RequestAppExit(&app);
        if (app.exitRequested) break;
        if (resizeTest && frameCount % 2 == 0) {
            const int sizes[][2] = { { 1024, 768 }, { 1280, 720 }, { 800, 600 }, { 640, 480 } };
            int sample = (frameCount / 2) % 4;
            SetWindowSize(sizes[sample][0], sizes[sample][1]);
        }
        Rectangle viewport = GetGameViewport();
        float scale = viewport.width / GAME_WIDTH;
        /* GetMousePosition now returns coordinates used by all existing UI buttons. */
        SetMouseOffset(-(int)roundf(viewport.x), -(int)roundf(viewport.y));
        SetMouseScale(1 / scale, 1 / scale);
        if (resizeTest) {
            SetMousePosition((int)roundf(viewport.x + 946 * scale), (int)roundf(viewport.y + 676 * scale));
            Vector2 mouse = GetMousePosition();
            screenshotsSaved = screenshotsSaved && fabsf(mouse.x - 946) < 2 && fabsf(mouse.y - 676) < 2;
        }
        if (smokeTest) PrepareSmokeFrame(&app, frameCount / 2);
        else UpdateApp(&app);
        if (!smokeTest) {
            musicNoticeTimer = fmaxf(0, musicNoticeTimer - GetFrameTime());
            if (IsKeyPressed(KEY_F6)) {
                SetBackgroundMusicMuted(&music, !music.muted);
                musicNoticeTimer = 2.5f;
            }
            if (IsKeyPressed(KEY_F7) || IsKeyPressed(KEY_F8)) {
                SetBackgroundMusicVolume(&music, music.volume + (IsKeyPressed(KEY_F8) ? 0.1f : -0.1f));
                SetBackgroundMusicMuted(&music, false);
                musicNoticeTimer = 2.5f;
            }
            UpdateMapMusic(&music, app.currentMap);
        }

        BeginDrawing();
        ClearBackground((Color){ 16, 20, 28, 255 });
        BeginTextureMode(canvas);
        DrawUi(&app);
        if (musicNoticeTimer > 0) {
            char notice[128];
            snprintf(notice, sizeof(notice), GetUiText(app.language, TEXT_MUSIC_VOLUME),
                (int)roundf((music.muted ? 0 : music.volume) * 100));
            Vector2 size = MeasureTextEx(app.assets.font, notice, 18, 1);
            DrawRectangle(312, GAME_HEIGHT - 44, 400, 32, (Color){ 23, 32, 43, 235 });
            DrawTextEx(app.assets.font, notice, (Vector2){ (GAME_WIDTH - size.x) / 2, GAME_HEIGHT - 37 }, 18, 1, WHITE);
        }
        EndTextureMode();
        DrawTexturePro(canvas.texture, (Rectangle){ 0, 0, GAME_WIDTH, -GAME_HEIGHT },
            viewport, (Vector2){ 0, 0 }, 0, WHITE);
        EndDrawing();

        if (smokeTest && frameCount % 2 == 1) {
            const char *const names[] = { "no-save", "continue-en", "bad-save", "new-confirm",
                "saved", "combat-block", "save-error", "loaded-complete", "town-art", "player-left",
                "player-right", "player-up", "boss-art", "field-art", "cave-stalagmites", "cave-rubble",
                "bag-icons", "bag-icons-en", "shop-icons", "shop-icons-en", "boss-en", "enemies-zh", "level-max",
                "quest-dialog", "pause", "death-town", "heal-message", "boss-victory", "quest-complete", "death-here",
                "field-northwest", "field-northeast", "field-southwest", "field-southeast", "field-canopy",
                "field-boss-portal", "boss-return-portal" };
            char path[1024];
            int length = snprintf(path, sizeof(path), "%s%s-%s.png",
                                  GetApplicationDirectory(), resizeTest ? "resize" : "stage8", names[frameCount / 2]);
            if (length >= 0 && (size_t)length < sizeof(path)) {
                Image screenshot = LoadImageFromScreen();
                screenshotsSaved = ExportImage(screenshot, path) && screenshotsSaved;
                UnloadImage(screenshot);
            } else screenshotsSaved = false;
        }
        if (smokeTest && ++frameCount >= 74) app.exitRequested = true;
    }

    bool verificationPassed = app.assets.supportsChinese && screenshotsSaved;
    if (smokeTest) {
        for (int i = 0; i < SPRITE_COUNT; i++) {
            /* These scenery slots have complete procedural fallbacks. */
            if (i == SPRITE_HOUSE_LEADER || i == SPRITE_HOUSE_SELLER || i == SPRITE_LAMP) continue;
            verificationPassed = IsTextureValid(app.assets.sprites[i].texture) && verificationPassed;
        }
        TraceLog(verificationPassed ? LOG_INFO : LOG_ERROR, "APP: Smoke verification %s (%i sprite slots; house and lamp fallbacks optional)",
                 verificationPassed ? "PASS" : "FAIL", SPRITE_COUNT);
    }
    UnloadBackgroundMusic(&music);
    UnloadApp(&app);
    UnloadRenderTexture(canvas);
    CloseWindow();
    return smokeTest && !verificationPassed ? 1 : 0;
}
