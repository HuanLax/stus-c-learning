#include "text.h"

const char *GetUiText(Language language, TextId id)
{
    if (language < LANGUAGE_ENGLISH || language >= LANGUAGE_COUNT) return "[invalid text]";
    /* Pair translations by named ID: no positional drift or large per-call array. */
    switch (id) {
        case TEXT_SUBTITLE:
            return language == LANGUAGE_ENGLISH ? "Our Enemy: Goblins" : "打击哥布林";
        case TEXT_STAGE:
            return language == LANGUAGE_ENGLISH ? "STAGE 8 / ASHBROOK ART" : "第八阶段 / 灰溪村美术接入";
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
            return language == LANGUAGE_ENGLISH ? "N: new game / C: continue / F5: save in peace" : "N：新游戏 / C：继续游戏 / F5：和平时存档";
        case TEXT_FONT_ERROR:
            return language == LANGUAGE_ENGLISH ? "Font unavailable or incomplete. English fallback is active." : "字体缺失或不完整，已使用英文显示。";
        case TEXT_SYSTEM_FONT:
            return language == LANGUAGE_ENGLISH ? "Development font: installed SimHei / Portable font still needed" : "开发字体：系统黑体 / 提交前需补充可分发字体";
        case TEXT_START:
            return language == LANGUAGE_ENGLISH ? "New game" : "新游戏";
        case TEXT_RESUME:
            return language == LANGUAGE_ENGLISH ? "Resume" : "返回游戏";
        case TEXT_PAUSED:
            return language == LANGUAGE_ENGLISH ? "Paused" : "已暂停";
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
        case TEXT_QUEST_NPC:
            return language == LANGUAGE_ENGLISH ? "Quest NPC" : "任务 NPC";
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
            return language == LANGUAGE_ENGLISH ? "Quest interaction preview. The main quest will be added later." : "这里预留主线任务交互，任务系统将在后续阶段加入。";
        case TEXT_SHOP_PREVIEW:
            return language == LANGUAGE_ENGLISH ? "Shop interaction preview. Inventory and purchases come later." : "这里预留商店交互，背包和购买功能将在后续阶段加入。";
        case TEXT_HEAL_PREVIEW:
            return language == LANGUAGE_ENGLISH ? "Enter the safe area first to use this healing point." : "请先完整进入安全区，再使用治疗点。";
        case TEXT_EXIT_PREVIEW:
            return language == LANGUAGE_ENGLISH ? "Use the entrance to travel. The lair requires quest progression." : "使用入口可切换地图，巢穴仍需主线解锁。";
        case TEXT_COORDINATES:
            return language == LANGUAGE_ENGLISH ? "Position: (%.0f, %.0f) px" : "坐标：(%.0f, %.0f) 像素";
        case TEXT_FIELD:
            return language == LANGUAGE_ENGLISH ? "Goblin field" : "哥布林野外";
        case TEXT_BOSS_MAP:
            return language == LANGUAGE_ENGLISH ? "King's lair" : "哥布林王巢穴";
        case TEXT_OUTSIDE_SAFE:
            return language == LANGUAGE_ENGLISH ? "Outside safe area" : "安全区外";
        case TEXT_TOWN_EXIT:
            return language == LANGUAGE_ENGLISH ? "Return to town" : "返回主城";
        case TEXT_BOSS_EXIT:
            return language == LANGUAGE_ENGLISH ? "King's lair entrance" : "巢穴入口";
        case TEXT_GATE_LOCKED:
            return language == LANGUAGE_ENGLISH ? "Locked: clear the goblins and report back to Elder Edric first." : "入口锁定：请先清理野外哥布林，并回村长处汇报。";
        case TEXT_EXIT_ERROR:
            return language == LANGUAGE_ENGLISH ? "Cannot enter: destination data is invalid." : "无法进入：目标地图位置数据不合法。";
        case TEXT_SWORD:
            return language == LANGUAGE_ENGLISH ? "Sword" : "剑";
        case TEXT_STAFF:
            return language == LANGUAGE_ENGLISH ? "Staff" : "法杖";
        case TEXT_ATTACK_HELP:
            return language == LANGUAGE_ENGLISH ? "J / Left click: attack    Q: switch" : "J / 鼠标左键：攻击    Q：切换";
        case TEXT_HP:
            return language == LANGUAGE_ENGLISH ? "HP %d / %d" : "生命 %d / %d";
        case TEXT_DEAD:
            return language == LANGUAGE_ENGLISH ? "You have fallen" : "你已死亡";
        case TEXT_DEATH_HELP:
            return language == LANGUAGE_ENGLISH ? "Left / Right: select. Enter or click: return with full health." : "左右键选择，Enter 确认，或点击按钮。返回后满血。";
        case TEXT_RESPAWN_TOWN:
            return language == LANGUAGE_ENGLISH ? "Return to town" : "返回主城";
        case TEXT_RESPAWN_HERE:
            return language == LANGUAGE_ENGLISH ? "Map safe area" : "当前地图安全区";
        case TEXT_TEST_ENEMY:
            return language == LANGUAGE_ENGLISH ? "Test goblin" : "测试哥布林";
        case TEXT_HEALED:
            return language == LANGUAGE_ENGLISH ? "Health restored. Healing is available inside safe areas." : "生命值已恢复。治疗只在安全区内生效。";
        case TEXT_MELEE_GOBLIN:
            return language == LANGUAGE_ENGLISH ? "Melee goblin" : "近战哥布林";
        case TEXT_RANGED_GOBLIN:
            return language == LANGUAGE_ENGLISH ? "Ranged goblin" : "远程哥布林";
        case TEXT_PEACE:
            return language == LANGUAGE_ENGLISH ? "Peace" : "和平";
        case TEXT_IN_COMBAT:
            return language == LANGUAGE_ENGLISH ? "In combat" : "战斗中";
        case TEXT_DISENGAGE:
            return language == LANGUAGE_ENGLISH ? "Disengage %.1fs" : "脱战 %.1f 秒";
        case TEXT_EMPTY_SLOT:
            return language == LANGUAGE_ENGLISH ? "Empty" : "空格";
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
            return language == LANGUAGE_ENGLISH ? "Merchant" : "商店";
        case TEXT_EQUIP:
            return language == LANGUAGE_ENGLISH ? "Equip" : "装备";
        case TEXT_BUY:
            return language == LANGUAGE_ENGLISH ? "Buy" : "购买";
        case TEXT_EQUIPPED:
            return language == LANGUAGE_ENGLISH ? "Equipped" : "已装备";
        case TEXT_BAG_HELP:
            return language == LANGUAGE_ENGLISH ? "Select with arrows or click. Enter equips; B / Esc closes." : "方向键或点击选择；Enter 装备；B / Esc 关闭。";
        case TEXT_SHOP_HELP:
            return language == LANGUAGE_ENGLISH ? "Select a weapon; Enter or Buy purchases it into your bag." : "选择武器，按 Enter 或点击购买；物品进入背包。";
        case TEXT_EQUIP_SUCCESS:
            return language == LANGUAGE_ENGLISH ? "Equipped. Your old weapon remains in the bag." : "装备成功，旧武器仍保留在背包中。";
        case TEXT_EQUIP_BUSY:
            return language == LANGUAGE_ENGLISH ? "Cannot equip during combat. Viewing the bag does not end combat." : "战斗中不能换装；查看背包不会结束战斗。";
        case TEXT_BAG_FULL:
            return language == LANGUAGE_ENGLISH ? "Bag full. No gold deducted." : "背包已满，未扣除金币。";
        case TEXT_GOLD_SHORT:
            return language == LANGUAGE_ENGLISH ? "Not enough gold. No progress changed." : "金币不足，进度未改变。";
        case TEXT_ALREADY_BOUGHT:
            return language == LANGUAGE_ENGLISH ? "Already purchased; each upgrade is sold once." : "已经购买过，每种升级武器限购一次。";
        case TEXT_ALREADY_EQUIPPED:
            return language == LANGUAGE_ENGLISH ? "This weapon is already equipped." : "这件武器已经装备。";
        case TEXT_ITEM_INVALID:
            return language == LANGUAGE_ENGLISH ? "Invalid item or selection." : "物品或选择不合法。";
        case TEXT_BUY_SUCCESS:
            return language == LANGUAGE_ENGLISH ? "Purchased. Open the bag and equip it manually." : "购买成功，请在背包中手动装备。";
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
            return language == LANGUAGE_ENGLISH ? "Item ATK: %d" : "使用此武器攻击：%d";
        case TEXT_ELDER:
            return language == LANGUAGE_ENGLISH ? "Elder Edric" : "村长艾德";
        case TEXT_QUEST_TITLE:
            return language == LANGUAGE_ENGLISH ? "Ashbrook's Plea" : "灰溪村的求援";
        case TEXT_QUEST_GREETING:
            return language == LANGUAGE_ENGLISH ? "Hello, adventurer! Our village, Ashbrook, has suffered repeated goblin raids. Fields lie ruined, trade wagons cannot pass, and our families fear the next attack. Will you hear our plea?" : "你好，冒险者！我是灰溪村的村长艾德。最近我们村庄饱受哥布林的折磨：庄稼被毁，商队受阻，村民连出门采集都提心吊胆。可以请你听听我们的委托吗？";
        case TEXT_QUEST_INTEL:
            return language == LANGUAGE_ENGLISH ? "Our brave villagers scouted the wilderness: four sword-bearing goblins and two archers guard the road. Their chief, Goblin King Grukk, hides in the eastern lair and commands these raids." : "勇敢的村民冒险侦察，确认野外道路附近有四只近战哥布林、两只远程哥布林。它们的头目——哥布林王格鲁克，藏在野外东侧的巢穴里，袭击都是它指使的。";
        case TEXT_QUEST_REQUEST:
            return language == LANGUAGE_ENGLISH ? "Please defeat four melee goblins and two ranged goblins after accepting this commission, then return to me. We offer 50 gold and 60 experience for restoring the road. You may prepare at our merchant and healing point first." : "请在接取委托后击败四只近战哥布林和两只远程哥布林，然后回来汇报。道路清理完成后，我们会支付五十金币和六十经验。出发前可以拜访商人、使用治疗点，准备好了就接受委托吧！";
        case TEXT_QUEST_FIELD_REMINDER:
            return language == LANGUAGE_ENGLISH ? "The road is not yet secure. Defeat four melee goblins and two ranged goblins, then report back. Your progress survives death and travel; use the safe area to retreat when needed." : "道路还没有完全恢复安全。请继续击败四只近战、两只远程哥布林，完成后回来找我。死亡和切换地图不会清空任务进度；遇到危险就撤回安全区。";
        case TEXT_QUEST_FIELD_THANKS:
            return language == LANGUAGE_ENGLISH ? "You have restored the road! Our families can gather supplies again. Yet the raids will resume while Grukk lives. We must end the threat at its source." : "道路终于畅通了！村民能够重新采集物资，商队也看到了希望。但格鲁克还活着，它一定会召集新的哥布林。我们必须从根源上结束这场灾难。";
        case TEXT_QUEST_BOSS_INTEL:
            return language == LANGUAGE_ENGLISH ? "The villagers found the lair entrance at the eastern end of the wilderness. Grukk uses a heavy forward slash and three spreading magic bolts. Watch the orange warnings and move before the attacks land. Report now to receive 50 gold and 60 experience and unlock the lair." : "村民找到的巢穴入口就在野外最东侧。格鲁克会向前重斩，也会发射三道散射法术；看到橙色预警后及时移动。确认汇报后，你将获得五十金币、六十经验，并解锁巢穴入口。";
        case TEXT_QUEST_BOSS_REMINDER:
            return language == LANGUAGE_ENGLISH ? "Your road-clearing reward has been paid, and the lair is unlocked. Travel east through the wilderness and defeat Grukk. Killing him grants 150 gold and 120 experience; return here afterward for the village's final reward." : "清理道路的报酬已经发放，巢穴入口也已开放。请沿野外道路向东，击败格鲁克。击杀头目可以获得一百五十金币、一百二十经验；之后请回村领取我们的最终谢礼。";
        case TEXT_QUEST_PREPARATION:
            return language == LANGUAGE_ENGLISH ? "Buy better weapons, equip them from your bag, and heal before leaving. Grukk's slash can be dodged to the side or behind him; his bolts fly in fixed directions. If you fall, recover at town or the map's safe area and try again." : "出发前请购买并手动装备更好的武器，再恢复生命。重斩可以向侧面或头目身后躲避；散射法术不会追踪你。失败后可以回主城或地图安全区，整装再战。";
        case TEXT_QUEST_FINAL_THANKS:
            return language == LANGUAGE_ENGLISH ? "Grukk has fallen! The villagers watched the goblins scatter and heard the roads grow quiet. Because of you, Ashbrook can rebuild its fields and welcome travellers again." : "格鲁克真的倒下了！村民亲眼看见哥布林四散逃走，田野终于恢复安宁。多亏了你，灰溪村可以重建农田、迎接旅人，不必再为下一次袭击担惊受怕。";
        case TEXT_QUEST_FINAL_REWARD:
            return language == LANGUAGE_ENGLISH ? "As promised, the village offers 100 gold and 60 experience as its final thanks. Claim the reward to finish this adventure. You may continue exploring afterward, and Grukk will not return." : "按照约定，村庄将再支付一百金币和六十经验，作为最后的谢礼。领取后，这次冒险就完成了。你仍然可以继续探索，格鲁克不会再次出现。";
        case TEXT_QUEST_AFTER:
            return language == LANGUAGE_ENGLISH ? "Welcome back, friend of Ashbrook. The village remembers your courage. The road is open and Grukk is defeated. Our reward has already been paid; enjoy your travels." : "欢迎回来，灰溪村的朋友。道路已经开放，格鲁克也已被击败。谢礼之前已经领取，村民会一直记得你的帮助。祝你今后的旅途平安！";
        case TEXT_NEXT:
            return language == LANGUAGE_ENGLISH ? "Next" : "下一页";
        case TEXT_ACCEPT_QUEST:
            return language == LANGUAGE_ENGLISH ? "Accept commission" : "接受委托";
        case TEXT_REPORT_QUEST:
            return language == LANGUAGE_ENGLISH ? "Report / claim reward" : "汇报并领取奖励";
        case TEXT_CONTINUE_EXPLORING:
            return language == LANGUAGE_ENGLISH ? "Continue exploring" : "继续探索";
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
        case TEXT_SAVE_FOUND:
            return language == LANGUAGE_ENGLISH ? "Save found. Choose Continue to resume." : "发现存档，选择继续游戏可恢复进度。";
        case TEXT_SAVE_NONE:
            return language == LANGUAGE_ENGLISH ? "No save yet. Start a new game; F5 saves in peace." : "尚无存档。开始新游戏后，可在和平状态按 F5 保存。";
        case TEXT_SAVE_BAD:
            return language == LANGUAGE_ENGLISH ? "Save unreadable or invalid. Original file preserved; new game needs confirmation." : "存档无法读取或数据不合法，原文件已保留；新游戏需确认。";
        case TEXT_SAVE_NEW:
            return language == LANGUAGE_ENGLISH ? "New game. F5 saves in peace; normal exit also saves." : "新游戏：和平状态 F5 保存，正常退出也会保存。";
        case TEXT_SAVE_LOADED:
            return language == LANGUAGE_ENGLISH ? "Save loaded. F5 saves in peace." : "读档成功。和平状态可按 F5 保存。";
        case TEXT_SAVE_OK:
            return language == LANGUAGE_ENGLISH ? "Saved successfully. F5 saves in peace." : "存档成功。和平状态可按 F5 保存。";
        case TEXT_SAVE_BUSY:
            return language == LANGUAGE_ENGLISH ? "Cannot save during combat or before respawning." : "战斗中或尚未复活时不能手动存档。";
        case TEXT_SAVE_FAILED:
            return language == LANGUAGE_ENGLISH ? "Save failed. Your previous save has been preserved." : "保存失败，之前的存档已保留。";
        case TEXT_SAVE_PREVIEW:
            return language == LANGUAGE_ENGLISH ? "Developer preview: saving is disabled." : "开发预览：存档已禁用。";
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
            return language == LANGUAGE_ENGLISH ? "Progress could not be saved. Retry after fixing the problem, cancel to return, or explicitly exit without saving. Your previous save remains unchanged." : "未能保存当前进度。你可以解决问题后重试，取消并返回游戏，或主动选择不保存退出。之前的存档保持不变。";
        case TEXT_SAVE_HINT:
            return language == LANGUAGE_ENGLISH ? "F5: save in peace" : "F5：和平状态保存";
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
        case TEXT_ULTIMATE:
            return language == LANGUAGE_ENGLISH ? "Ultimate" : "终结技";
        case TEXT_NOT_AVAILABLE:
            return language == LANGUAGE_ENGLISH ? "Not available" : "暂未开放";
        case TEXT_HUD_BAG:
            return language == LANGUAGE_ENGLISH ? "Bag" : "背包";
        default: return "[invalid text]";
    }
}
