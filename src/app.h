#ifndef PROJECT_LAX_APP_H
#define PROJECT_LAX_APP_H

#include "assets.h"
#include "text.h"
#include "player.h"
#include "combat.h"
#include "quest.h"
#include "save.h"

typedef struct App {
    Language language;
    UiAssets assets;
    bool exitRequested;
    AppScreen screen;
    World worlds[MAP_COUNT];
    MapId currentMap;
    MapId lastSafeMap;
    Vector2 lastSafePosition;
    Quest quest;
    int dialoguePage;
    TextId interactionText;
    Player player;
    Combat combat;
    bool attackInputArmed;
    bool deathReturnToTown;
    Camera2D camera;
    int nearbyInteraction;
    int selectedSlot;
    int selectedProduct;
    TextId inventoryMessage;
    float rewardNoticeTimer;
    int rewardNoticeKills;
    char savePath[1024];
    bool saveAvailable;
    bool saveExists;
    bool sessionStarted;
    bool persistenceEnabled;
    TextId saveMessage;
    AppScreen beforeSaveError;
} App;

void InitializeApp(App *app);
void SetAppLanguage(App *app, Language language);
void UpdateApp(App *app);
void EnterMap(App *app, MapId map, Vector2 position);
void ActivateNearbyInteraction(App *app);
void RespawnAppPlayer(App *app, bool returnToTown);
void EquipSelectedItem(App *app);
void BuySelectedProduct(App *app);
void UpdateAppQuest(App *app);
void ConfirmQuestDialogue(App *app);
void StartNewGame(App *app);
void ContinueGame(App *app);
SaveData CaptureSave(const App *app);
void ManualSave(App *app);
void RequestAppExit(App *app);
void UnloadApp(App *app);

#endif
