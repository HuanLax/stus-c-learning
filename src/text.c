/* 中英文文案：按 TextId 返回对应语言文字；类别与修改位置见 docs/CODE_GUIDE.md。 */
#include "text.h"

const char *GetUiText(Language language, TextId id)
{
    if (language < LANGUAGE_ENGLISH || language >= LANGUAGE_COUNT) return "[invalid text]";
    /* Pair translations by named ID: no positional drift or large per-call array. */
    switch (id) {
        /* 菜单、语言与字体提示 */
        case TEXT_SUBTITLE:
            return language == LANGUAGE_ENGLISH ? "The Goblin Threat" : "哥布林危机";
        case TEXT_STAGE:
            return language == LANGUAGE_ENGLISH ? "DEFEND ASHBROOK" : "守护灰溪村";
        case TEXT_DESCRIPTION:
            return language == LANGUAGE_ENGLISH ? "Meet Elder Edric, defend Ashbrook and defeat Goblin King Grukk." : "接受村长委托，守护灰溪村，击败哥布林王格鲁克。";
        case TEXT_LANGUAGE:
            return language == LANGUAGE_ENGLISH ? "Language: English" : "当前语言：中文";
        case TEXT_ENGLISH:
            return language == LANGUAGE_ENGLISH ? "English" : "English";
        case TEXT_CHINESE:
            return language == LANGUAGE_ENGLISH ? "中文" : "中文";
        case TEXT_EXIT:
            return language == LANGUAGE_ENGLISH ? "Exit" : "退出";
        case TEXT_HELP:
            return language == LANGUAGE_ENGLISH ? "Click a language button or press F2 to switch." : "点击语言按钮，或按 F2 切换语言。";
        case TEXT_FOOTER:
            return language == LANGUAGE_ENGLISH ? "N: new game / C: continue / F5: save out of combat" : "N：新游戏 / C：继续游戏 / F5：非战斗时保存";
        case TEXT_FONT_ERROR:
            return language == LANGUAGE_ENGLISH ? "Chinese font unavailable. Using English." : "中文字体无法使用，已切换为英文。";
        case TEXT_SYSTEM_FONT:
            return language == LANGUAGE_ENGLISH ? "Using a system font." : "当前使用系统字体。";
        case TEXT_START:
            return language == LANGUAGE_ENGLISH ? "New game" : "新游戏";
        case TEXT_RESUME:
            return language == LANGUAGE_ENGLISH ? "Resume" : "返回游戏";
        case TEXT_PAUSED:
            return language == LANGUAGE_ENGLISH ? "Paused" : "已暂停";
        /* 地图状态与基本操作 */
        case TEXT_TOWN:
            return language == LANGUAGE_ENGLISH ? "Town" : "主城";
        case TEXT_SAFE:
            return language == LANGUAGE_ENGLISH ? "Safe area" : "安全区域";
        case TEXT_MOVE_HELP:
            return language == LANGUAGE_ENGLISH ? "WASD / Arrows: move     E: interact" : "WASD / 方向键：移动    E：交互";
        case TEXT_UI_HELP:
            return language == LANGUAGE_ENGLISH ? "B: bag  Esc: menu  F2: language" : "B：背包  Esc：菜单  F2：语言";
        case TEXT_FACING_HELP:
            return language == LANGUAGE_ENGLISH ? "Gold: sword (4 directions)    Blue: staff (8 directions)" : "金色：剑的四向朝向    蓝色：法杖的八向朝向";
        /* 场景名称与交互提示 */
        case TEXT_QUEST_NPC:
            return language == LANGUAGE_ENGLISH ? "Village elder" : "村长";
        case TEXT_MERCHANT:
            return language == LANGUAGE_ENGLISH ? "Merchant" : "商人";
        case TEXT_HEAL_POINT:
            return language == LANGUAGE_ENGLISH ? "Healing point" : "治疗点";
        case TEXT_FIELD_EXIT:
            return language == LANGUAGE_ENGLISH ? "Field entrance" : "野外入口";
        case TEXT_INTERACT:
            return language == LANGUAGE_ENGLISH ? "E / Click: interact" : "E / 点击：交互";
        case TEXT_CLOSE:
            return language == LANGUAGE_ENGLISH ? "Close / Esc" : "关闭 / Esc";
        case TEXT_QUEST_PREVIEW:
            return language == LANGUAGE_ENGLISH ? "Speak to Elder Edric about the goblin threat." : "与村长艾德交谈，了解哥布林的威胁。";
        case TEXT_SHOP_PREVIEW:
            return language == LANGUAGE_ENGLISH ? "Browse weapons at the shop. Equip purchases from your bag." : "在商店选购武器，购买后可在背包中装备。";
        case TEXT_HEAL_PREVIEW:
            return language == LANGUAGE_ENGLISH ? "Step fully inside the safe area to restore your health." : "请走进安全区，再使用治疗点恢复生命。";
        case TEXT_EXIT_PREVIEW:
            return language == LANGUAGE_ENGLISH ? "Use the entrance to travel. Report the road cleared to unlock the lair." : "通过入口前往其他地图。清理道路并向村长汇报后，可进入巢穴。";
        case TEXT_COORDINATES:
            return language == LANGUAGE_ENGLISH ? "Position: (%.0f, %.0f) px" : "坐标：(%.0f, %.0f) 像素";
        /* 地图名称、入口与传送反馈 */
        case TEXT_FIELD:
            return language == LANGUAGE_ENGLISH ? "Goblin grasslands" : "哥布林草地";
        case TEXT_BOSS_MAP:
            return language == LANGUAGE_ENGLISH ? "King's lair" : "哥布林王巢穴";
        case TEXT_OUTSIDE_SAFE:
            return language == LANGUAGE_ENGLISH ? "Outside safe area" : "安全区外";
        case TEXT_TOWN_EXIT:
            return language == LANGUAGE_ENGLISH ? "Return to town" : "返回主城";
        case TEXT_BOSS_EXIT:
            return language == LANGUAGE_ENGLISH ? "King's lair entrance" : "巢穴入口";
        case TEXT_GATE_LOCKED:
            return language == LANGUAGE_ENGLISH ? "Lair sealed. Clear the goblins and report to Elder Edric first." : "巢穴尚未开放，请先清理野外哥布林，再向村长艾德汇报。";
        case TEXT_EXIT_ERROR:
            return language == LANGUAGE_ENGLISH ? "This entrance is unavailable. Move away and try again." : "暂时无法通过此入口，请离开后重试。";
        /* 武器、生命、复活与战斗状态 */
        case TEXT_SWORD:
            return language == LANGUAGE_ENGLISH ? "Sword" : "剑";
        case TEXT_STAFF:
            return language == LANGUAGE_ENGLISH ? "Staff" : "法杖";
        case TEXT_ATTACK_HELP:
            return language == LANGUAGE_ENGLISH ? "J / Left click: attack    Q: switch weapon" : "J / 鼠标左键：攻击    Q：切换武器";
        case TEXT_HP:
            return language == LANGUAGE_ENGLISH ? "HP %d / %d" : "生命 %d / %d";
        case TEXT_DEAD:
            return language == LANGUAGE_ENGLISH ? "You have fallen" : "你倒下了";
        case TEXT_DEATH_HELP:
            return language == LANGUAGE_ENGLISH ? "Left / Right: select. Enter or click: revive with full health." : "左右键选择复活地点，按 Enter 或点击确认，复活后恢复全部生命。";
        case TEXT_RESPAWN_TOWN:
            return language == LANGUAGE_ENGLISH ? "Return to town" : "返回主城";
        case TEXT_RESPAWN_HERE:
            return language == LANGUAGE_ENGLISH ? "Map safe area" : "当前地图安全区";
        case TEXT_GENERIC_ENEMY:
            return language == LANGUAGE_ENGLISH ? "Goblin" : "哥布林";
        case TEXT_HEALED:
            return language == LANGUAGE_ENGLISH ? "Health fully restored." : "生命值已全部恢复。";
        case TEXT_MELEE_GOBLIN:
            return language == LANGUAGE_ENGLISH ? "Melee goblin" : "近战哥布林";
        case TEXT_RANGED_GOBLIN:
            return language == LANGUAGE_ENGLISH ? "Ranged goblin" : "远程哥布林";
        case TEXT_PEACE:
            return language == LANGUAGE_ENGLISH ? "At peace" : "非战斗";
        case TEXT_IN_COMBAT:
            return language == LANGUAGE_ENGLISH ? "In combat" : "战斗中";
        case TEXT_DISENGAGE:
            return language == LANGUAGE_ENGLISH ? "Disengage %.1fs" : "脱离战斗 %.1f 秒";
        /* 背包、商店、交易与装备反馈 */
        case TEXT_EMPTY_SLOT:
            return language == LANGUAGE_ENGLISH ? "Empty slot" : "空位";
        case TEXT_IRON_SWORD:
            return language == LANGUAGE_ENGLISH ? "Iron sword" : "铁剑";
        case TEXT_WOOD_STAFF:
            return language == LANGUAGE_ENGLISH ? "Wood staff" : "木法杖";
        case TEXT_STEEL_SWORD:
            return language == LANGUAGE_ENGLISH ? "Steel sword" : "钢剑";
        case TEXT_RUNE_STAFF:
            return language == LANGUAGE_ENGLISH ? "Rune staff" : "符文法杖";
        case TEXT_BAG:
            return language == LANGUAGE_ENGLISH ? "Bag / 12 slots" : "背包 / 12 格";
        case TEXT_SHOP:
            return language == LANGUAGE_ENGLISH ? "Weapon shop" : "武器商店";
        case TEXT_EQUIP:
            return language == LANGUAGE_ENGLISH ? "Equip" : "装备";
        case TEXT_BUY:
            return language == LANGUAGE_ENGLISH ? "Buy" : "购买";
        case TEXT_EQUIPPED:
            return language == LANGUAGE_ENGLISH ? "Equipped" : "已装备";
        case TEXT_BAG_HELP:
            return language == LANGUAGE_ENGLISH ? "Select with arrows or click. Enter equips; B / Esc closes." : "方向键或点击选择；Enter 装备；B / Esc 关闭。";
        case TEXT_SHOP_HELP:
            return language == LANGUAGE_ENGLISH ? "Select a weapon, then press Enter or click Buy. Equip it from your bag." : "选择武器后按 Enter 或点击购买，再到背包中装备。";
        case TEXT_EQUIP_SUCCESS:
            return language == LANGUAGE_ENGLISH ? "Equipped. Your old weapon remains in the bag." : "装备成功，旧武器仍保留在背包中。";
        case TEXT_EQUIP_BUSY:
            return language == LANGUAGE_ENGLISH ? "Cannot equip during combat. Viewing the bag does not end combat." : "战斗中不能换装；查看背包不会结束战斗。";
        case TEXT_BAG_FULL:
            return language == LANGUAGE_ENGLISH ? "Bag full. No gold deducted." : "背包已满，未扣除金币。";
        case TEXT_GOLD_SHORT:
            return language == LANGUAGE_ENGLISH ? "Not enough gold to buy this weapon." : "金币不足，无法购买这件武器。";
        case TEXT_ALREADY_BOUGHT:
            return language == LANGUAGE_ENGLISH ? "You already own this upgrade. Each is sold once." : "你已购买这件升级武器，每种限购一件。";
        case TEXT_ALREADY_EQUIPPED:
            return language == LANGUAGE_ENGLISH ? "This weapon is already equipped." : "这件武器已经装备。";
        case TEXT_ITEM_INVALID:
            return language == LANGUAGE_ENGLISH ? "Select a weapon first." : "请先选择一件武器。";
        case TEXT_BUY_SUCCESS:
            return language == LANGUAGE_ENGLISH ? "Purchased. Open the bag and equip it manually." : "购买成功，请在背包中手动装备。";
        /* 属性、价格、成长与奖励格式 */
        case TEXT_BONUS:
            return language == LANGUAGE_ENGLISH ? "Attack bonus: +%d" : "攻击加成：+%d";
        case TEXT_PRICE:
            return language == LANGUAGE_ENGLISH ? "Price: %d gold" : "价格：%d 金币";
        case TEXT_GOLD:
            return language == LANGUAGE_ENGLISH ? "Gold: %d" : "金币：%d";
        case TEXT_PROGRESS:
            return language == LANGUAGE_ENGLISH ? "Lv %d  XP %d/%d  Gold %d" : "等级 %d  经验 %d/%d  金币 %d";
        case TEXT_PROGRESS_MAX:
            return language == LANGUAGE_ENGLISH ? "Lv 5 MAX  Gold %d" : "等级 5 满级  金币 %d";
        case TEXT_ATTACK_VALUE:
            return language == LANGUAGE_ENGLISH ? "ATK %d" : "攻击 %d";
        case TEXT_REWARD:
            return language == LANGUAGE_ENGLISH ? "+%d gold / +%d XP" : "+%d 金币 / +%d 经验";
        case TEXT_SOLD_OUT:
            return language == LANGUAGE_ENGLISH ? "Sold out" : "售罄";
        case TEXT_ITEM_ATTACK:
            return language == LANGUAGE_ENGLISH ? "ATK with weapon: %d" : "装备后攻击力：%d";
        /* 村长与主线对话 */
        case TEXT_ELDER:
            return language == LANGUAGE_ENGLISH ? "Elder Edric" : "村长艾德";
        case TEXT_QUEST_TITLE:
            return language == LANGUAGE_ENGLISH ? "Ashbrook's Plea" : "灰溪村的求援";
        case TEXT_QUEST_GREETING:
            return language == LANGUAGE_ENGLISH ? "Welcome, adventurer. I am Edric, elder of Ashbrook. Goblins have ruined our crops and blocked the trade road. Even gathering supplies has become dangerous. Will you help our village?" : "你好，冒险者。我是灰溪村的村长艾德。最近哥布林毁坏庄稼、拦截商队，村民连外出采集都不敢了。你愿意帮帮我们吗？";
        case TEXT_QUEST_INTEL:
            return language == LANGUAGE_ENGLISH ? "Our scouts found four melee goblins and two archers along the road. Their leader, Goblin King Grukk, commands the raids from a lair to the east." : "村民侦察发现，道路附近盘踞着四只近战哥布林和两只远程哥布林。它们听命于哥布林王格鲁克，那个家伙就藏在野外东侧的巢穴里。";
        case TEXT_QUEST_REQUEST:
            return language == LANGUAGE_ENGLISH ? "Accept the commission, then defeat four melee goblins and two ranged goblins. Return to me for 50 gold and 60 experience. Visit the merchant and healing point before you leave." : "接受委托后，请击败四只近战哥布林和两只远程哥布林，再回来找我。村庄会酬谢你五十金币和六十经验。出发前，记得去商店挑选武器，并在治疗点恢复生命。";
        case TEXT_QUEST_FIELD_REMINDER:
            return language == LANGUAGE_ENGLISH ? "The road is not yet secure. Defeat four melee goblins and two ranged goblins, then report back. Your progress survives death and travel; use the safe area to retreat when needed." : "道路还没有完全恢复安全。请继续击败四只近战、两只远程哥布林，完成后回来找我。死亡和切换地图不会清空任务进度；遇到危险就撤回安全区。";
        case TEXT_QUEST_FIELD_THANKS:
            return language == LANGUAGE_ENGLISH ? "You have restored the road! Our families can gather supplies again. Yet the raids will resume while Grukk lives. We must end the threat at its source." : "道路终于畅通了！村民能够重新采集物资，商队也看到了希望。但格鲁克还活着，它一定会召集新的哥布林。我们必须从根源上结束这场灾难。";
        case TEXT_QUEST_BOSS_INTEL:
            return language == LANGUAGE_ENGLISH ? "The lair lies at the eastern edge of the grasslands. Grukk strikes forward with a heavy slash and fires three spreading magic bolts. Move when you see an orange warning. Claim your 50 gold and 60 experience below to open the way to his lair." : "巢穴入口就在草地最东侧。格鲁克会向前重斩，也会发射三道散射法术，看到橙色预警就及时躲开。领取这次的五十金币和六十经验后，通往巢穴的路就会开放。";
        case TEXT_QUEST_BOSS_REMINDER:
            return language == LANGUAGE_ENGLISH ? "Your road-clearing reward has been paid, and the lair is unlocked. Travel east through the wilderness and defeat Grukk. Killing him grants 150 gold and 120 experience; return here afterward for the village's final reward." : "清理道路的报酬已经发放，巢穴入口也已开放。请沿野外道路向东，击败格鲁克。击杀头目可以获得一百五十金币、一百二十经验；之后请回村领取我们的最终谢礼。";
        case TEXT_QUEST_PREPARATION:
            return language == LANGUAGE_ENGLISH ? "Buy better weapons, equip them from your bag, and heal before leaving. Grukk's slash can be dodged to the side or behind him; his bolts fly in fixed directions. If you fall, recover at town or the map's safe area and try again." : "出发前请购买并手动装备更好的武器，再恢复生命。重斩可以向侧面或头目身后躲避；散射法术不会追踪你。失败后可以回主城或地图安全区，整装再战。";
        case TEXT_QUEST_FINAL_THANKS:
            return language == LANGUAGE_ENGLISH ? "Grukk has fallen! The villagers watched the goblins scatter and heard the roads grow quiet. Because of you, Ashbrook can rebuild its fields and welcome travellers again." : "格鲁克真的倒下了！村民亲眼看见哥布林四散逃走，田野终于恢复安宁。多亏了你，灰溪村可以重建农田、迎接旅人，不必再为下一次袭击担惊受怕。";
        case TEXT_QUEST_FINAL_REWARD:
            return language == LANGUAGE_ENGLISH ? "As promised, the village offers 100 gold and 60 experience as its final thanks. Claim the reward to finish this adventure. You may continue exploring afterward, and Grukk will not return." : "按照约定，村庄将再支付一百金币和六十经验，作为最后的谢礼。领取后，这次冒险就完成了。你仍然可以继续探索，格鲁克不会再次出现。";
        case TEXT_QUEST_AFTER:
            return language == LANGUAGE_ENGLISH ? "Welcome back, friend of Ashbrook. The road is clear and Grukk is defeated. Our village will always remember your courage. May your travels be safe!" : "欢迎回来，灰溪村的朋友。道路已经畅通，格鲁克也已被击败，村民会一直记得你的帮助。愿你今后的旅途平安！";
        /* 任务对话按钮 */
        case TEXT_NEXT:
            return language == LANGUAGE_ENGLISH ? "Next" : "下一页";
        case TEXT_ACCEPT_QUEST:
            return language == LANGUAGE_ENGLISH ? "Accept commission" : "接受委托";
        case TEXT_REPORT_QUEST:
            return language == LANGUAGE_ENGLISH ? "Report / claim reward" : "汇报并领取奖励";
        case TEXT_CONTINUE_EXPLORING:
            return language == LANGUAGE_ENGLISH ? "Continue exploring" : "继续探索";
        /* 任务目标 */
        case TEXT_OBJECTIVE_OFFER:
            return language == LANGUAGE_ENGLISH ? "Objective: speak to Elder Edric in town." : "任务：在主城与村长艾德交谈。";
        case TEXT_OBJECTIVE_FIELD:
            return language == LANGUAGE_ENGLISH ? "Objective: melee %d/4, ranged %d/2. Return when complete." : "任务：近战 %d/4，远程 %d/2；完成后回村汇报。";
        case TEXT_OBJECTIVE_REPORT:
            return language == LANGUAGE_ENGLISH ? "Objective: return to Elder Edric and unlock the lair." : "任务：回村长处汇报，领取奖励并解锁巢穴。";
        case TEXT_OBJECTIVE_BOSS:
            return language == LANGUAGE_ENGLISH ? "Objective: enter the eastern lair and defeat Goblin King Grukk." : "任务：进入野外东侧巢穴，击败哥布林王格鲁克。";
        case TEXT_OBJECTIVE_FINAL:
            return language == LANGUAGE_ENGLISH ? "Objective: report Grukk's defeat to Elder Edric." : "任务：回村长处汇报格鲁克已被击败。";
        case TEXT_OBJECTIVE_COMPLETE:
            return language == LANGUAGE_ENGLISH ? "Ashbrook is safe. Main quest complete." : "灰溪村恢复安宁，主线任务已完成。";
        /* 头目胜利与通关提示 */
        case TEXT_BOSS_NAME:
            return language == LANGUAGE_ENGLISH ? "Goblin King Grukk" : "哥布林王格鲁克";
        case TEXT_BOSS_VICTORY:
            return language == LANGUAGE_ENGLISH ? "Grukk defeated!" : "格鲁克已被击败！";
        case TEXT_BOSS_VICTORY_BODY:
            return language == LANGUAGE_ENGLISH ? "You gained 150 gold and 120 experience. Return to Elder Edric to finish the commission." : "你已获得一百五十金币和一百二十经验。请回村长处交付委托，完成主线。";
        case TEXT_GAME_COMPLETE:
            return language == LANGUAGE_ENGLISH ? "Adventure complete" : "冒险完成";
        case TEXT_GAME_COMPLETE_BODY:
            return language == LANGUAGE_ENGLISH ? "Ashbrook is safe again. Your final reward is 100 gold and 60 experience. Thank you, adventurer! Continue exploring, or leave through the pause menu." : "灰溪村重获安宁，你已领取一百金币和六十经验的最终谢礼。感谢你，冒险者！你可以继续探索，或通过暂停菜单退出。";
        /* 存档状态、确认与错误处理 */
        case TEXT_SAVE_FOUND:
            return language == LANGUAGE_ENGLISH ? "Choose Continue to return to your adventure." : "选择“继续游戏”，接着上次的冒险出发。";
        case TEXT_SAVE_NONE:
            return language == LANGUAGE_ENGLISH ? "No saved game. Start an adventure; F5 saves out of combat." : "尚无存档。开始冒险后，可在非战斗时按 F5 保存。";
        case TEXT_SAVE_BAD:
            return language == LANGUAGE_ENGLISH ? "Cannot read this save. It has been kept. You can start a new game." : "无法读取存档，原文件已保留。你可以选择开始新游戏。";
        case TEXT_SAVE_NEW:
            return language == LANGUAGE_ENGLISH ? "Your adventure begins. F5 saves out of combat; exiting also saves." : "冒险开始！非战斗时可按 F5 保存，正常退出时也会自动保存。";
        case TEXT_SAVE_LOADED:
            return language == LANGUAGE_ENGLISH ? "Game loaded. F5 saves out of combat." : "进度已恢复。非战斗时可按 F5 保存。";
        case TEXT_SAVE_OK:
            return language == LANGUAGE_ENGLISH ? "Progress saved." : "进度已保存。";
        case TEXT_SAVE_BUSY:
            return language == LANGUAGE_ENGLISH ? "Cannot save in combat or while defeated." : "请在存活且脱离战斗后保存进度。";
        case TEXT_SAVE_FAILED:
            return language == LANGUAGE_ENGLISH ? "Save failed. Your previous save has been preserved." : "保存失败，之前的存档已保留。";
        case TEXT_SAVE_PREVIEW:
            return language == LANGUAGE_ENGLISH ? "Preview mode: progress will not be saved." : "预览模式下不会保存游戏进度。";
        case TEXT_LOAD_GAME:
            return language == LANGUAGE_ENGLISH ? "Continue" : "继续游戏";
        case TEXT_NEW_CONFIRM:
            return language == LANGUAGE_ENGLISH ? "Start a new game?" : "开始新游戏？";
        case TEXT_NEW_CONFIRM_BODY:
            return language == LANGUAGE_ENGLISH ? "Your existing save will be replaced on the next successful save or normal exit. Cancel to keep your current save, or confirm to begin a fresh adventure." : "已有存档会在下一次成功保存或正常退出时被替换。取消可保留原存档，确认则开始一段新的冒险。";
        case TEXT_CONFIRM_NEW:
            return language == LANGUAGE_ENGLISH ? "Confirm new game" : "确认新游戏";
        case TEXT_CANCEL:
            return language == LANGUAGE_ENGLISH ? "Cancel / Esc" : "取消 / Esc";
        case TEXT_RETRY_SAVE:
            return language == LANGUAGE_ENGLISH ? "Retry / Enter" : "重试 / Enter";
        case TEXT_EXIT_UNSAVED:
            return language == LANGUAGE_ENGLISH ? "Exit without saving" : "不保存并退出";
        case TEXT_SAVE_ERROR_BODY:
            return language == LANGUAGE_ENGLISH ? "Could not save your progress. Retry, cancel to return to the game, or exit without saving. Your previous save has been kept." : "当前进度保存失败，之前的存档已保留。你可以重试、取消并返回游戏，或选择不保存并退出。";
        case TEXT_SAVE_HINT:
            return language == LANGUAGE_ENGLISH ? "F5: save out of combat" : "F5：非战斗时保存";
        /* 等级、经验与状态栏标签 */
        case TEXT_LEVEL:
            return language == LANGUAGE_ENGLISH ? "LV %d" : "等级 %d";
        case TEXT_XP:
            return language == LANGUAGE_ENGLISH ? "XP %d/%d" : "经验 %d/%d";
        case TEXT_XP_MAX:
            return language == LANGUAGE_ENGLISH ? "XP MAX" : "经验已满";
        case TEXT_HUD_HP: return "HP  %d / %d";
        case TEXT_HUD_MP:
            return language == LANGUAGE_ENGLISH ? "MP  Unavailable" : "MP  未开放";
        case TEXT_HUD_XP: return "XP  %d / %d";
        case TEXT_HUD_LEVEL:
            return language == LANGUAGE_ENGLISH ? "Lv.%d" : "%d 级";
        /* 当前界面未使用的技能标签（保留编号） */
        case TEXT_ULTIMATE:
            return language == LANGUAGE_ENGLISH ? "Ultimate" : "终结技";
        case TEXT_NOT_AVAILABLE:
            return language == LANGUAGE_ENGLISH ? "Not available" : "暂未开放";
        /* 背包快捷按钮 */
        case TEXT_HUD_BAG:
            return language == LANGUAGE_ENGLISH ? "Bag" : "背包";
        case TEXT_MUSIC_HELP:
            return language == LANGUAGE_ENGLISH ? "Music: F6 mute / F7 quieter / F8 louder" : "音乐：F6 静音 / F7 降低音量 / F8 提高音量";
        case TEXT_MUSIC_VOLUME:
            return language == LANGUAGE_ENGLISH ? "Music volume: %d%%" : "音乐音量：%d%%";
        default: return "[invalid text]";
    }
}
