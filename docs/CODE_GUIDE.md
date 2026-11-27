# 项目代码分类与修改索引

本索引根据项目代码整理，2026-11-18 增补背景音乐。当前 `src/` 有 14 个 C 实现文件、13 个头文件。源码中的中文分区注释可直接搜索定位；下文使用函数名定位，避免增删代码后行号失效。

## 1. 目录分类

| 目录或文件 | 分类 | 用途 |
|---|---|---|
| `src/` | 游戏源码 | 游戏运行逻辑、画面、文案和资源加载 |
| `pic/` | 图片资源 | 25 张 PNG：地图、角色、场景、武器与背包 |
| `assets/fonts/` | 字体资源 | 中文字体与授权说明 |
| `assets/audio/` | 音乐资源 | 主城、草地与巢穴的三首 MP3 |
| `scripts/build.ps1` | 构建工具 | Debug / Release 编译 |
| `.vscode/` | 编辑器配置 | 构建任务、调试器、代码提示路径 |
| `docs/` | 当前说明 | 本索引与地图布局说明 |
| `bin/` | 生成文件 | 游戏程序 |
| `saves/` | 玩家进度 | 正式游戏的个人存档 |
| `project-lax.exe - Shortcut.lnk` | 启动快捷方式 | 本机使用，不是游戏源码 |

`bin/`、`saves/` 和根目录快捷方式已在 `.gitignore` 中忽略。旧版地图副本和历史归档已清理；`save.c` 的旧档迁移仍用于读取既有进度。构建脚本目前只收集 `src/` 直接下的 `.c`；以后若把源码移入子目录，必须同时修改构建脚本和相关路径。

## 2. 源码按功能分类

每个 `.h` 主要声明数据结构与可供其他模块调用的函数；对应 `.c` 实现具体行为。`main.c` 是程序入口，没有对应头文件。

| 分类 | 文件 | 主要内容 | 定位入口 |
|---|---|---|---|
| 程序启动 | `src/main.c` | 创建窗口、固定画布缩放、运行主循环、退出清理、开发预览 | `main`、`GetGameViewport` |
| 游戏流程 | `src/app.c` / `.h` | 汇总游戏状态，协调各系统，新游戏、继续、切图、交互、存档入口 | `App`、`InitializeApp`、`UpdateApp` |
| 界面与输入 | `src/ui.c` / `.h` | 菜单、按钮、状态栏、背包、商店、对话、死亡与确认弹窗 | `UpdateUi`、`DrawUi` |
| 地图与场景 | `src/world.c` / `.h` | 三张地图、场景碰撞、NPC 位置、安全区、出口、镜头、绘制顺序 | `InitializeWorld`、`CanStandInWorld`、`DrawWorld` |
| 角色与成长 | `src/player.c` / `.h` | 移动、朝向、生命、等级、经验、基础攻击、受伤与复活 | `InitializePlayer`、`MovePlayerWithObstacles`、`GrantPlayerRewards` |
| 通用战斗 | `src/combat.c` / `.h` | 敌人出生、攻击冷却、剑范围、投射物、击杀奖励、战斗效果 | `InitializeCombat`、`UpdateCombat`、`ClaimEnemyRewards` |
| 普通敌人 AI | `src/enemy_ai.c` / `.h` | 网格寻路、角色避障、巡逻、发现玩家、追击、远程攻击、脱战 | `FindEnemyWaypoint`、`MoveEnemyTowards`、`UpdateEnemyAI` |
| 头目技能 | `src/boss.c` / `.h` | 重斩、三发散射、攻击切换和预警 | `UpdateBoss`、`BossSlashHits`、`DrawBossTelegraph` |
| 背包与交易 | `src/inventory.c` / `.h` | 物品定义、12 格背包、装备槽、购买检查与扣款 | `GetItemDefinition`、`EquipInventoryItem`、`BuyInventoryItem` |
| 主线任务 | `src/quest.c` / `.h` | 任务阶段、近远程击杀计数、巢穴解锁、任务奖励、对话页选择 | `AdvanceQuest`、`UpdateQuestProgress`、`QuestDialogueText` |
| 存档 | `src/save.c` / `.h` | 存档格式、数值与状态验证、旧档迁移、临时文件替换 | `SaveData`、`ReadSave`、`WriteSave`、`ValidateSave` |
| 图片与字体 | `src/assets.c` / `.h` | 文件名与图片编号映射、贴图加载与绘制、中文字符收集、字体回退 | `SpriteId`、`LoadSprites`、`LoadUiAssets` |
| 背景音乐 | `src/music.c` / `.h` | 声卡初始化、地图曲目映射、循环串流、音量、静音与清理 | `BackgroundMusic`、`InitializeBackgroundMusic`、`UpdateMapMusic` |
| 中英文文案 | `src/text.c` / `.h` | 139 个文案编号及中英文内容 | `TextId`、`GetUiText` |

## 3. 游戏运行顺序

```text
main.c：创建窗口与画布
  → app.c / InitializeApp：初始化地图、角色、战斗、任务、资源，检查存档
  → music.c / InitializeBackgroundMusic：初始化声卡并加载三首音乐
  → 每帧循环
      → UpdateApp
          → UpdateUi：读取按钮与界面按键，切换状态
          → 游玩状态：角色移动 → 战斗 / AI / 头目更新 → 任务进度更新
          → 更新附近交互与镜头
      → UpdateMapMusic：切换地图曲目并补充音频缓冲（面板打开时仍更新）
      → DrawUi
          → 菜单，或游戏世界 + 状态栏 + 当前面板
      → 将固定画布缩放到实际窗口
  → 请求退出：保存进度；失败则显示选择弹窗
  → 释放音乐并关闭声卡，释放其他资源并关闭窗口
```

`App` 是当前游戏状态的汇总；`SaveData` 是可写入文件的进度与现场快照。界面颜色、贴图和字体资源不作为 GPU 对象写入存档。

## 4. 想改内容时去哪里

| 想改的内容 | 文件与位置 | 说明 |
|---|---|---|
| 主菜单大标题 `project-lax` | `src/ui.c` → `DrawMenu` | 目前直接写在 `DrawCenteredText` 中 |
| 窗口标题 `project-lax` | `src/main.c` → `InitWindow` | 与主菜单大标题是两个位置 |
| 副标题“哥布林危机” | `src/text.c` → `TEXT_SUBTITLE` | 中英文分别修改 |
| 菜单标语“守护灰溪村” | `src/text.c` → `TEXT_STAGE` | 编号沿用旧名，内容已改为游戏标语 |
| 按钮、操作提示、错误提示 | `src/text.c` | 按文件中的中文分类寻找 |
| 村长姓名、对话、任务目标、通关文字 | `src/text.c` → `TEXT_ELDER`、`TEXT_QUEST_*`、`TEXT_OBJECTIVE_*` | 内容在这里，出现顺序由任务模块决定 |
| 对话页数、哪个阶段说什么 | `src/quest.c` → `QuestDialoguePages`、`QuestDialogueText` | 与文案内容分开管理 |
| 任务要求与任务奖励 | `src/quest.c` → 进度函数、`AdvanceQuest` | 改数量或奖励时也要同步对话与目标文字 |
| 武器名称 | `src/text.c` → `TEXT_IRON_SWORD` 等 | 图片、价格和加成另在物品定义中 |
| 武器价格、攻击加成、图标 | `src/inventory.c` → `GetItemDefinition` | 钢剑 75 金币、符文法杖 100 金币；均加 12 攻击 |
| 初始生命、移速、攻击、升级规则 | `src/player.c` | 初始生命 100、基础攻击 20；当前最高 5 级 |
| 普通敌人属性、出生位置、攻击冷却 | `src/combat.c` → `InitializeCombat` | 常用战斗参数集中在 `CombatRules` |
| 普通哥布林击杀奖励 | `src/combat.h` → `GOBLIN_GOLD`、`GOBLIN_EXPERIENCE` | 当前为 25 金币、30 经验 |
| 头目生命与出生位置 | `src/combat.c` → `InitializeCombat` | 头目技能行为另在 `boss.c` |
| 头目伤害、攻击范围、预警时间 | `src/boss.c`；散射投射物伤害见 `src/combat.c` | 重斩和投射物的伤害入口不同 |
| 村长、商人、治疗石、入口位置 | `src/world.c` → `InitializeTown`、`InitializeAdventureObjects` | 详细坐标见 `MAP_LAYOUT.md` |
| 地图障碍、安全区、地图尺寸 | `src/world.c` / `.h` | 底图画面与碰撞记录分别维护 |
| 图片文件名对应关系 | `src/assets.c` → `LoadSprites` | 原有 `gobin_*.png`、`Transport.png` 拼写需与磁盘文件一致 |
| 音乐文件、地图对应与默认音量 | `src/music.c` → `InitializeBackgroundMusic` | 默认 50%；音乐资源不写入存档 |
| 音乐更新与快捷键 | `src/main.c` → 主循环 | 每帧更新串流，F6 静音、F7/F8 调音量；菜单提示见 `src/ui.c` |
| 中文字体路径与回退方式 | `src/assets.c` → `LoadUiAssets` | 自动收集文案表中的中英文字符 |
| 字号、颜色、按钮大小、界面坐标 | `src/ui.c` | 顶部常量及各 `Draw...` 函数 |
| 按键与鼠标操作 | `src/ui.c` → `UpdateUi`；`src/app.c` → `UpdateApp` | 界面操作与角色移动、攻击分属两处 |
| 画布尺寸、最小窗口、缩放 | `src/world.h`；`src/main.c` | 虚拟画布 1024×768，最小窗口 640×480 |
| 存档路径 | `src/app.c` → `InitializeApp` | 相对可执行文件定位 `../saves/save.txt` |
| 编译器、raylib 路径与编译参数 | `scripts/build.ps1` | VS Code 调试器路径另见 `.vscode/launch.json` |

## 5. 文案分类与旧内容

`text.c` 和 `text.h` 已加中文分区：菜单语言、地图操作、场景交互、传送、战斗复活、背包交易、属性奖励、村长对话、任务按钮、任务目标、胜利通关、存档、状态栏与技能标签。

以下 18 个编号在其他运行源码中没有直接引用。它们属于旧界面文字或尚未使用的标签，不等于当前界面上仍然显示：

| 类别 | 编号 |
|---|---|
| 操作帮助 | `TEXT_HELP`、`TEXT_MOVE_HELP`、`TEXT_UI_HELP`、`TEXT_FACING_HELP`、`TEXT_ATTACK_HELP` |
| 旧交互或调试标签 | `TEXT_QUEST_NPC`、`TEXT_EXIT_PREVIEW`、`TEXT_COORDINATES` |
| 旧属性展示格式 | `TEXT_HP`、`TEXT_PROGRESS`、`TEXT_PROGRESS_MAX`、`TEXT_ATTACK_VALUE`、`TEXT_LEVEL`、`TEXT_XP` |
| 重复的存档帮助 | `TEXT_SAVE_HINT` |
| 当前未使用的功能标签 | `TEXT_HUD_MP`、`TEXT_ULTIMATE`、`TEXT_NOT_AVAILABLE` |

保留这些编号是因为 `SaveData.interactionText`、`inventoryMessage` 会按整数编号写入存档。即使整理文案，也不要直接删除或重排枚举项；若要彻底移除，需要处理存档版本兼容。新增编号应追加到 `TEXT_COUNT` 前。

`TEXT_QUEST_PREVIEW`、`TEXT_SHOP_PREVIEW` 等旧名字仍有默认状态引用，内容已更新为实际交互说明。名字中的 `PREVIEW` 不表示主线和商店功能尚未完成。

修改文案里的 `%d`、`%.1f` 等格式时，要核对调用处参数；它们是用于显示数值的占位符。中文文本应通过文案表提供，字体加载会自动收集所需字符。

## 6. 资源分类

| 类别 | 图片文件 |
|---|---|
| 地图背景 | `map_main1.png`、`map_two.png`、`boss_arena.png` |
| 玩家四方向 | `player_down.png`、`player_left.png`、`player_right.png`、`player_up.png` |
| 哥布林四方向 | `gobin_down.png`、`gobin_left.png`、`gobin_right.png`、`gobin_up.png` |
| NPC 与头目 | `leader.png`、`seller.png`、`boss.png` |
| 场景与交互 | `tree.png`、`stone.png`、`medical_stone.png`、`Transport.png`、`cave_stalagmites.png`、`cave_rubble.png` |
| 武器与背包 | `iron_sword.png`、`wood_staff.png`、`weapon_sword.png`、`weapon_staff.png`、`backpack.png` |

资源编号共 28 个，其中 `house_leader.png`、`house_seller.png`、`street_lamp.png` 三个图片槽没有独立文件，代码提供备用绘制，主城底图也已包含房屋和路灯。这三项不是必需资源加载失败。

## 7. 巢穴预览

使用 `--preview-boss` 打开巢穴预览。

## 8. 后续适合整理的内容

| 内容 | 扫描发现 | 建议 |
|---|---|---|
| 游戏标题 | 菜单与窗口分别写有 `project-lax` | 若需要统一改名，可集中定义，再供两处读取 |
| 大文件职责 | `ui.c` 同时包含输入、布局和各面板；`world.c` 同时包含布局、碰撞与绘制 | 后续功能增加时再按职责拆文件，并同步构建配置 |
| 界面状态定义 | `AppScreen` 目前放在 `save.h`，流程和界面也依赖它 | 后续可独立为公共状态定义 |
| 规则与对白 | 任务数量、金币、经验同时出现在逻辑和文案中 | 调整规则时同步核对两边 |
| 头目复现说明 | 最终奖励对白称“格鲁克不会再次出现”，但切图会重新初始化头目 | 后续决定是否允许重复挑战，再统一对白、说明与实现 |

本次整理增加分类注释和索引，保留已有文件位置、函数行为、资源映射及存档编号。
