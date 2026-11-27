/* 程序入口：窗口、缩放画布、主循环、音乐，以及巢穴预览入口。 */
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

/* ===== 程序启动、主循环与退出 ===== */
int main(int argc, char **argv)
{
    bool previewBoss = argc == 2 && strcmp(argv[1], "--preview-boss") == 0;
    App app = { 0 };
    BackgroundMusic music = { .activeTrack = -1 };
    float musicNoticeTimer = 0;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(GAME_WIDTH, GAME_HEIGHT, "project-lax");
    if (!IsWindowReady()) return 1;
    SetWindowMinSize(640, 480);
    RenderTexture2D canvas = LoadRenderTexture(GAME_WIDTH, GAME_HEIGHT);
    if (!IsRenderTextureValid(canvas)) { CloseWindow(); return 1; }
    SetTextureFilter(canvas.texture, TEXTURE_FILTER_BILINEAR);
    SetExitKey(KEY_NULL); /* Esc belongs to panels/pause, not immediate exit. */
    SetTargetFPS(60);
    InitializeApp(&app);
    InitializeBackgroundMusic(&music);
    if (previewBoss) {
        app.persistenceEnabled = false;
        app.saveMessage = TEXT_SAVE_PREVIEW;
    }
    if (previewBoss) EnterMap(&app, MAP_BOSS, app.worlds[MAP_BOSS].spawn);

    while (!app.exitRequested) {
        if (WindowShouldClose()) RequestAppExit(&app);
        if (app.exitRequested) break;
        Rectangle viewport = GetGameViewport();
        float scale = viewport.width / GAME_WIDTH;
        /* GetMousePosition now returns coordinates used by all existing UI buttons. */
        SetMouseOffset(-(int)roundf(viewport.x), -(int)roundf(viewport.y));
        SetMouseScale(1 / scale, 1 / scale);
        UpdateApp(&app);
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
    }

    UnloadBackgroundMusic(&music);
    UnloadApp(&app);
    UnloadRenderTexture(canvas);
    CloseWindow();
    return 0;
}
