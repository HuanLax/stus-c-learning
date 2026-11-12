#ifndef PROJECT_LAX_QUEST_H
#define PROJECT_LAX_QUEST_H

#include "player.h"

typedef enum QuestStage { QUEST_OFFER, QUEST_CLEAR_FIELD, QUEST_REPORT_FIELD,
    QUEST_HUNT_BOSS, QUEST_REPORT_BOSS, QUEST_COMPLETE } QuestStage;

typedef struct Quest {
    QuestStage stage;
    int startingMelee;
    int startingRanged;
    bool bossDefeated;
} Quest;

void InitializeQuest(Quest *quest);
void UpdateQuestProgress(Quest *quest, const Player *player);
int QuestMeleeProgress(const Quest *quest, const Player *player);
int QuestRangedProgress(const Quest *quest, const Player *player);
bool IsBossUnlocked(const Quest *quest);
bool AdvanceQuest(Quest *quest, Player *player);
int QuestDialoguePages(const Quest *quest);
TextId QuestDialogueText(const Quest *quest, int page);

#endif
