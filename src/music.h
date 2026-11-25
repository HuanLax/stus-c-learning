#ifndef PROJECT_LAX_MUSIC_H
#define PROJECT_LAX_MUSIC_H

#include "world.h"

/* 音频资源只属于运行中的程序，不写入游戏存档。 */
typedef struct BackgroundMusic {
    Music tracks[MAP_COUNT];
    int activeTrack;
    float volume;
    bool muted;
    bool deviceReady;
} BackgroundMusic;

void InitializeBackgroundMusic(BackgroundMusic *music);
void UpdateMapMusic(BackgroundMusic *music, MapId map);
void SetBackgroundMusicVolume(BackgroundMusic *music, float volume);
void SetBackgroundMusicMuted(BackgroundMusic *music, bool muted);
void UnloadBackgroundMusic(BackgroundMusic *music);

#endif
