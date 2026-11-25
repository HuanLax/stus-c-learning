/* 主线任务：阶段推进、击杀计数、入口解锁、奖励和对话页选择。 */
#include "quest.h"

/* ===== 任务初始化与进度查询 ===== */
void InitializeQuest(Quest *quest)
{
    *quest = (Quest){ .stage = QUEST_OFFER };
}

int QuestMeleeProgress(const Quest *quest, const Player *player)
{
    int value = player->meleeDefeated - quest->startingMelee;
    return value < 0 ? 0 : value > 4 ? 4 : value;
}

int QuestRangedProgress(const Quest *quest, const Player *player)
{
    int value = player->rangedDefeated - quest->startingRanged;
    return value < 0 ? 0 : value > 2 ? 2 : value;
}

void UpdateQuestProgress(Quest *quest, const Player *player)
{
    if (quest->stage == QUEST_CLEAR_FIELD && QuestMeleeProgress(quest, player) == 4 && QuestRangedProgress(quest, player) == 2)
        quest->stage = QUEST_REPORT_FIELD;
    if (quest->stage == QUEST_HUNT_BOSS && quest->bossDefeated) quest->stage = QUEST_REPORT_BOSS;
}

bool IsBossUnlocked(const Quest *quest)
{
    return quest->stage >= QUEST_HUNT_BOSS;
}

/* ===== 阶段推进与一次性任务奖励 ===== */
bool AdvanceQuest(Quest *quest, Player *player)
{
    if (quest->stage == QUEST_OFFER) {
        quest->startingMelee = player->meleeDefeated;
        quest->startingRanged = player->rangedDefeated;
        quest->stage = QUEST_CLEAR_FIELD;
        return true;
    }
    if (quest->stage == QUEST_REPORT_FIELD) {
        quest->stage = QUEST_HUNT_BOSS; /* commit before paying: repeated input cannot repeat reward */
        GrantPlayerRewards(player, 50, 60);
        UpdateQuestProgress(quest, player);
        return true;
    }
    if (quest->stage == QUEST_REPORT_BOSS) {
        quest->stage = QUEST_COMPLETE;
        GrantPlayerRewards(player, 100, 60);
        return true;
    }
    return false;
}

/* ===== 各阶段的对话页数与文案选择 ===== */
int QuestDialoguePages(const Quest *quest)
{
    return quest->stage == QUEST_OFFER ? 3 : quest->stage == QUEST_REPORT_FIELD ||
        quest->stage == QUEST_HUNT_BOSS || quest->stage == QUEST_REPORT_BOSS ? 2 : 1;
}

TextId QuestDialogueText(const Quest *quest, int page)
{
    if (quest->stage == QUEST_OFFER) {
        const TextId texts[] = { TEXT_QUEST_GREETING, TEXT_QUEST_INTEL, TEXT_QUEST_REQUEST };
        return texts[page >= 0 && page < 3 ? page : 0];
    }
    if (quest->stage == QUEST_REPORT_FIELD) return page == 0 ? TEXT_QUEST_FIELD_THANKS : TEXT_QUEST_BOSS_INTEL;
    if (quest->stage == QUEST_HUNT_BOSS) return page == 0 ? TEXT_QUEST_BOSS_REMINDER : TEXT_QUEST_PREPARATION;
    if (quest->stage == QUEST_REPORT_BOSS) return page == 0 ? TEXT_QUEST_FINAL_THANKS : TEXT_QUEST_FINAL_REWARD;
    if (quest->stage == QUEST_COMPLETE) return TEXT_QUEST_AFTER;
    return TEXT_QUEST_FIELD_REMINDER;
}
