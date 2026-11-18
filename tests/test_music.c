#include "music.h"

#include <assert.h>
#include <stdio.h>

static void StreamFrames(BackgroundMusic *music, MapId map, int frames)
{
    for (int i = 0; i < frames; i++) {
        UpdateMapMusic(music, map);
        WaitTime(0.01);
    }
}

int main(void)
{
    SetTraceLogLevel(LOG_WARNING);
    SetConfigFlags(FLAG_WINDOW_HIDDEN);
    InitWindow(64, 64, "Music checks");
    assert(IsWindowReady());

    BackgroundMusic music;
    InitializeBackgroundMusic(&music);
    if (!music.deviceReady) {
        fprintf(stderr, "Music checks require an available audio device.\n");
        CloseWindow();
        return 1;
    }
    SetMasterVolume(0); /* Exercise the real decoder/output silently. */
    for (int i = 0; i < MAP_COUNT; i++) {
        assert(IsMusicValid(music.tracks[i]));
        assert(music.tracks[i].looping);
        assert(GetMusicTimeLength(music.tracks[i]) > 2);
    }

    /* Playback must advance rather than restart each frame. */
    for (int i = 0; i < MAP_COUNT; i++) {
        StreamFrames(&music, (MapId)i, 30);
        assert(music.activeTrack == i);
        assert(IsMusicStreamPlaying(music.tracks[i]));
        assert(GetMusicTimePlayed(music.tracks[i]) > 0.1f);
        for (int other = 0; other < MAP_COUNT; other++)
            if (other != i) assert(!IsMusicStreamPlaying(music.tracks[other]));
        float before = GetMusicTimePlayed(music.tracks[i]);
        StreamFrames(&music, (MapId)i, 20);
        assert(GetMusicTimePlayed(music.tracks[i]) > before);

        /* Seek near the real MP3's end and verify it wraps and keeps playing. */
        SeekMusicStream(music.tracks[i], GetMusicTimeLength(music.tracks[i]) - 0.1f);
        StreamFrames(&music, (MapId)i, 60);
        assert(IsMusicStreamPlaying(music.tracks[i]));
        assert(GetMusicTimePlayed(music.tracks[i]) < 2);
    }

    SetBackgroundMusicVolume(&music, 2);
    assert(music.volume == 1);
    SetBackgroundMusicVolume(&music, -1);
    assert(music.volume == 0);
    SetBackgroundMusicVolume(&music, 0.5f);
    SetBackgroundMusicMuted(&music, true);
    assert(music.muted && music.volume == 0.5f);
    StreamFrames(&music, MAP_TOWN, 15);
    SetBackgroundMusicMuted(&music, false);
    assert(!music.muted && IsMusicStreamPlaying(music.tracks[MAP_TOWN]));
    UpdateMapMusic(&music, MAP_COUNT);
    assert(music.activeTrack == MAP_TOWN);

    /* A missing track stops the old map's music without touching invalid streams. */
    Music field = music.tracks[MAP_FIELD];
    music.tracks[MAP_FIELD] = (Music){ 0 };
    UpdateMapMusic(&music, MAP_FIELD);
    assert(!IsMusicStreamPlaying(music.tracks[MAP_TOWN]));
    music.tracks[MAP_FIELD] = field;
    UpdateMapMusic(&music, MAP_BOSS);
    assert(IsMusicStreamPlaying(music.tracks[MAP_BOSS]));

    UnloadBackgroundMusic(&music);
    assert(!IsAudioDeviceReady() && !music.deviceReady && music.activeTrack == -1);
    /* Also covers the no-device path and repeated cleanup. */
    UpdateMapMusic(&music, MAP_TOWN);
    SetBackgroundMusicVolume(&music, 0.5f);
    SetBackgroundMusicMuted(&music, true);
    UnloadBackgroundMusic(&music);
    CloseWindow();
    puts("Music checks PASS: MP3 loading, streaming, map switching, looping, volume and cleanup.");
    return 0;
}
