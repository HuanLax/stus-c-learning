/* 背景音乐：地图对应曲目、循环串流、音量与音频资源清理。 */
#include "music.h"

#include <stdio.h>

void SetBackgroundMusicVolume(BackgroundMusic *music, float volume)
{
    music->volume = volume < 0 ? 0 : volume > 1 ? 1 : volume;
    if (!music->deviceReady) return;
    for (int i = 0; i < MAP_COUNT; i++) {
        if (IsMusicValid(music->tracks[i]))
            SetMusicVolume(music->tracks[i], music->muted ? 0 : music->volume);
    }
}

void SetBackgroundMusicMuted(BackgroundMusic *music, bool muted)
{
    music->muted = muted;
    SetBackgroundMusicVolume(music, music->volume);
}

void InitializeBackgroundMusic(BackgroundMusic *music)
{
    *music = (BackgroundMusic){ .activeTrack = -1, .volume = 0.5f };
    InitAudioDevice();
    music->deviceReady = IsAudioDeviceReady();
    if (!music->deviceReady) {
        TraceLog(LOG_WARNING, "APP: Audio device unavailable; continuing without music");
        return;
    }

    const char *const files[MAP_COUNT] = {
        [MAP_TOWN] = "Morning_at_the_Sovereign_Gate.mp3",
        [MAP_FIELD] = "Steps_in_the_Green.mp3",
        [MAP_BOSS] = "Titan_s_Heavy_Step.mp3"
    };
    for (int i = 0; i < MAP_COUNT; i++) {
        char path[1024];
        int length = snprintf(path, sizeof(path), "%s../assets/audio/%s", GetApplicationDirectory(), files[i]);
        if (length < 0 || (size_t)length >= sizeof(path) || !FileExists(path)) {
            TraceLog(LOG_WARNING, "APP: Music file missing: %s", files[i]);
            continue;
        }
        music->tracks[i] = LoadMusicStream(path);
        if (!IsMusicValid(music->tracks[i])) {
            TraceLog(LOG_WARNING, "APP: Could not load music: %s", files[i]);
            continue;
        }
        music->tracks[i].looping = true;
        TraceLog(LOG_INFO, "APP: Map %i music loaded: %s (%.1f seconds)", i, files[i], GetMusicTimeLength(music->tracks[i]));
    }
    SetBackgroundMusicVolume(music, music->volume);
}

void UpdateMapMusic(BackgroundMusic *music, MapId map)
{
    if (!music->deviceReady || map < MAP_TOWN || map >= MAP_COUNT) return;
    if (music->activeTrack != (int)map) {
        if (music->activeTrack >= 0 && IsMusicValid(music->tracks[music->activeTrack]))
            StopMusicStream(music->tracks[music->activeTrack]);
        music->activeTrack = (int)map;
        if (IsMusicValid(music->tracks[map])) {
            PlayMusicStream(music->tracks[map]);
            TraceLog(LOG_INFO, "APP: Playing music for map %i", (int)map);
        }
    }
    /* 每帧都补充音频缓冲；暂停、背包、对话等界面不会中断或重播音乐。 */
    if (IsMusicValid(music->tracks[map])) UpdateMusicStream(music->tracks[map]);
}

void UnloadBackgroundMusic(BackgroundMusic *music)
{
    if (music->deviceReady) {
        for (int i = 0; i < MAP_COUNT; i++) {
            if (IsMusicValid(music->tracks[i])) UnloadMusicStream(music->tracks[i]);
        }
        CloseAudioDevice();
    }
    *music = (BackgroundMusic){ .activeTrack = -1 };
}
