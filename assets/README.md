# 美术资源约定

2026-10-15：已从项目根目录 `pic/` 接入 14 张用户提供的 PNG，包括主城底图、两类建筑、玩家四向、哥布林四向、Boss、村长和商人。其余地图、树石、图标和效果仍由代码绘制占位。

2026-10-18：新增 8 张素材接入，当前加载 22 张图片。野外和巢穴底图、树木、岩石、治疗点、传送入口与洞穴障碍已替换；装备图标与攻击效果仍为代码占位。镜头继续保持 2 倍。

2026-11-02：新增四种武器图标，当前共加载 26 张图片。背包、商店和 HUD 当前装备使用同一份 ItemDefinition.icon 映射；图标加载失败仍回退原几何占位。没有把背包图直接用于人物手持，手持握点和遮挡仍需另行适配。

后续调整：新增透明 `pic/backpack.png`，现共加载 27 张图片，替换右下角代码绘制的背包。背包与终结技按钮均为 132×150 屏幕像素，同在 Y=602，图标、名称对齐。普通哥布林 24×24、Boss 48×48 脚底矩形用于实体阻挡；活怪之间、活怪与玩家之间不能互相穿过，死怪不挡路。静态寻路仍按地图计算，移动时逐小步检查动态身体；旧档重叠允许向外走。Boss 切图/复活可刷新，主线完成记录不重置。

背包图由内置 imagegen 生成，透明 alpha 原样保留，游戏按可见边界缩小显示。生成提示词：

> Use case: stylized-concept. Asset type: a single inventory backpack icon PNG for a medieval fantasy 2D pixel-art RPG called project-lax. Primary request: a sturdy ordinary adventurer's warm brown leather backpack with a rounded folded flap, small side pouches, carrying handle and a clear central brass buckle. Front-facing with very slight three-quarter depth, centered isolated object, readable bold dark outlines, warm ochre highlights and restrained pixel-art shading, matching beige parchment and antique gold game UI. Square canvas, object occupies about 85 percent with clear padding. No text, letters, border, button, background, character, weapons or extra objects. Real transparent alpha background. Designed to read well reduced to 80x80 pixels.

| 目录 | 后续用途 |
|---|---|
| `fonts/` | 允许分发的中文字体及授权 |
| `maps/` | 地面底图或图块 |
| `scenery/` | 建筑、树木、岩石等独立场景对象 |
| `characters/` | 玩家与 NPC |
| `enemies/` | 哥布林与 Boss |
| `items/` | 装备图标 |
| `effects/` | 挥剑、法术、预警效果 |

以上是后续可用的分类方案。当前字体在 assets/fonts，图片直接使用 pic，不为这次接入复制图片或创建一套空目录。

## 当前图片如何进入游戏

| pic 文件 | 用途 |
|---|---|
| map_main1.png | 主城 30×30 格，完整图片缩放到 960×960 世界像素 |
| house_leader.png / house_seller.png | 两类建筑，按文件命名分配，复用于现有四栋房屋 |
| player_down / left / right / up.png | 玩家四向，最大 32×32 显示范围 |
| gobin_down / left / right / up.png | 两类普通哥布林共用四向；远程增加暖色调，名称区分类别；保留原 gobin 拼写 |
| boss.png | 哥布林王，固定正面，最大 64×64 显示范围 |
| leader.png / seller.png | 村长、商人，最大 32×32 显示范围 |
| map_two.png | 野外底图，完整缩放到 1280×1280 世界像素 |
| medical_stone.png | 所有地图的治疗石，最大 40×56；交互规则和治疗范围保持原设定 |
| Transport.png | 所有地图的入口传送阵，48×48，在角色之前绘制为地面层 |
| tree.png / stone.png | 树木和普通岩石；身体碰撞保持原矩形，外观独立缩放 |
| boss_arena.png | 洞穴底图，完整缩放到 768×768 世界像素 |
| cave_stalagmites.png / cave_rubble.png | 巢穴原有两处障碍分别显示为石笋和塌方岩堆 |
| iron_sword.png / wood_staff.png | 初始铁剑、木法杖的背包/商店/HUD 图标 |
| weapon_sword.png / weapon_staff.png | 升级钢剑、符文法杖的背包/商店/HUD 图标 |
| backpack.png | 透明棕色皮革背包，右下角背包按钮图标 |

后三张洞穴图片已直接重命名为 `boss_arena.png`、`cave_stalagmites.png`、`cave_rubble.png`，分别对应原来的中央竞技场、三柱石笋和塌方岩堆。每张只保留一个英文名文件，不创建副本。后续替换时直接更新对应英文名 PNG 即可，代码加载路径保持一致。

这些是高分辨率原图，不是实际 32px 画布。程序使用 GetImageAlphaBorder 找到可见区域，只在 DrawTexturePro 绘制时选择该区域并等比例缩放，不改写原图；过滤方式为最近邻。缩小时细节会减少，是否放大显示需试玩后再决定。
当前镜头以 2 倍拉近预览：32 世界像素的人物在屏幕上最高约 64 像素，Boss 最大约 128 像素。HUD 保持屏幕大小；地图边界按缩放后的可见世界范围计算，世界坐标、移速与攻击距离仍使用原数值。调整入口为 world.c 的 GetWorldCamera 中 camera.zoom。
人物以裁去透明留白后的脚底中心对齐，因此不会因为 NPC 图片留白较大而显示成小点。当前均为单帧静态图，没有行走帧动画。

加载和卸载集中在 assets.c，SpriteId 与 Sprite 定义在 assets.h。Texture2D 是 GPU 资源，启动时加载一次，退出时逐一卸载，不能每帧重新读取 PNG。
world.c 接收 UiAssets 指针并绘制，ui.c 传入各怪物的朝向与着色。贴图只负责外观，不把图片宽高当作身体或攻击碰撞框。
GetFacingSprite 将四向映射到图片 ID。玩家跟随原剑四向朝向；怪物行走使用独立的 facing，攻击预警使用已锁定的 attackDirection，避免换朝向改变攻击判定。Boss 仅有一张正面，出招方向仍用预警表示。

2026-11-02 按最新规则移除主城矮墙、花坛与建筑的碰撞；图片、脚底 Y 排序及遮挡淡化继续保留，屋顶拆层仍待对应资源。NPC 挡路。旧的 walls 表与 mapWalls 数组已删除。

野外移除独立城墙、树木和石头，底图中直接画入的装饰可行走；地图外围保留边界。所有地图的治疗石绑定脚底碰撞；GetInteractionBody 统一定义治疗石与 NPC 的阻挡矩形，入口不挡路。
洞穴底图的外围墙与黑色区域用 floorBoundary 多边形近似限制；CanStandInWorld 检查身体四角，怪物寻路和攻击视线也复用该检查。入口 3×3 安全区保留可站立能力，确保复活和治疗可用；不按画面每个像素生成碰撞，边缘仍需亲自试玩微调。
传送阵放在地面层，治疗石与角色/场景按脚底排序；资源加载本身不负责治疗或切图，实际行为仍在 app.c 的交互逻辑中。

### 这次接入的验收

1. 启动进入主城，确认背景无黑底，四栋房屋使用两类图片，村长和商人显示为对应人物。
2. WASD 四向移动后停下，确认玩家正面/背面/左右图正确，站立不在不同方向间跳动。
3. 主城建筑、矮墙和花坛可穿过，村长与商人不能穿过；三张地图治疗石均挡路，但可以靠近治疗和使用入口。
4. 切到野外，观察哥布林巡逻与追击时换方向，远程暖色调；战斗判定仍按原碰撞框。
5. 正常任务解锁巢穴后观察 Boss；单方向图不会随攻击旋转，预警照常显示真实攻击方向。
6. 切图、退出重启，确认加载和卸载正常。提交目录要包含 pic；截图模式不写实际游戏存档。

已通过全部六组逻辑检查，覆盖主城装饰可穿过、NPC/治疗石挡路、野外内部无墙、洞穴黑色区域不可站立、地图交互路线与 Boss 战斗。截图模式输出 30 张 bin/stage8-*.png，并要求 26 张纹理均有效；画面覆盖中英文背包、商店、三类敌人名称、满级经验条、任务对话、暂停、死亡选择、治疗提示与通关。没有新增源码文件，本次逻辑全部在现有模块中扩展。

### UI 精简后的阅读顺序

2026-11-02 HUD 按参考图调整为整条米色金边状态栏：武器图片、等级圆章、HP/MP/XP、金币、和平/战斗徽记；右下角增加终结技预留位和可点击的背包按钮（仍支持 B）。MP 与终结技当前标注未开放，不增加虚构数值或战斗机制。面板与小图标由 raylib 绘制，武器复用 pic 内装备 PNG，不新增贴图依赖。`DrawWorldHud / DrawHudMeter / DrawHudSymbol / DrawHudActions` 分别负责布局、数值条、图标和操作按钮；`IsPointerOverHud` 防止鼠标点击 UI 时触发普攻，J 的键盘攻击照常保留。文字保持在 text.c 中双语切换，界面数字直接取 Player/Combat 状态，不写死在图片里。

1. `DrawUi`：绘制入口，只安排场景、HUD、任务提示、弹窗与存档提示的顺序。
2. `DrawGameWorld / DrawWorldHud / DrawScreenPanel`：分别负责世界画面、状态栏和当前界面；具体面板继续用命名函数表达。
3. `DrawLabel`：取当前语言文字，像 printf 一样用可选参数填入 %d / %.1f，再绘制。内部 va_list 表示这组可选参数，vsnprintf 将结果写入有长度限制的缓冲区；调用时参数数量和类型必须匹配文字格式。
4. `Clicked / Activated`：鼠标点击，以及“指定按键或按钮点击”的共用判断；查询不会消费输入，分支顺序继续决定操作优先级。
5. `UpdateUi`：根据当前屏幕处理输入；背包/商店共用选择逻辑，selected 指针分别指向 selectedSlot 或 selectedProduct。

本次仍只使用原 ui.c，没有新增 UI 源文件。总行数适度减少，主要改善入口和重复逻辑；不通过删掉功能或压成难读的长行缩短。六组检查通过，重构前后 30 张截图的 SHA-256 全部一致；截图不代替键鼠操作验收，请亲自检查背包选格/装备、商店购买、对话翻页、暂停和两种复活。

### World 整理后的阅读顺序

1. `InitializeWorld`：设置地图尺寸、出生点和安全区，再调用 InitializeTiles 与对应地图的对象初始化。三张地图共用地形生成循环，洞穴边界保留，主城旧墙和花坛碰撞已移除。
2. `AddScenery`：创建场景记录时选定 SpriteId，绘制不再根据建筑顺序和地图反复选择同一张贴图。Scenery.sprite 只是图片 ID，碰撞仍取 collision。
3. `DrawWorld`：只安排 DrawGround、DrawObjects 和朝向提示，入口由 105 行整理为 12 行。
4. `DrawGround / DrawTiles`：RGB 不透明底图存在时，省去下面完全不可见的地形格子绘制。当前野外每帧省去 40×40=1600 个地形矩形；传送阵和安全区仍绘制。透明格式或资源缺失时保留地形占位。
5. `BuildDrawList / DrawObjects`：只对绘制引用按脚底 Y 稳定排序，不改变地图对象数组的顺序；遮挡淡化与资源缺失时的原有占位图保留。

此前 World 整理保持画面和碰撞一致，六组检查及 30 张截图比对通过；2026-11-02 的障碍调整是在此基础上的玩法变更，当前规则以上文为准。

### 文本与图标的维护（2026-11-02）

- 文本在 text.c 的 GetUiText 中按 TextId 使用 switch 配对中英文；不再依赖两行数组的字符串顺序，也不每次调用构造整张翻译指针表。新增文字时同时补 text.h 的 ID 与 switch 中的一对文本。
- 三种敌人使用 combat.c 的 GetEnemyName，绘制时传入 App 当前语言，Enemy 不存储已翻译的名字。F2、菜单切换及读档恢复语言都使用同一个 App.language。
- 四种物品的名称、属性、价格和图标 ID 在 inventory.c 的 GetItemDefinition 中集中定义。保存仍只记录物品 ID，不把纹理写进存档，已有格式保持兼容。
- 5 级经验门槛为 0；HUD 现在显示满级和完整经验条，避免除以零。界面颜色改用 C99 颜色值宏，保留原配色并移除文件级 Color 变量。
- 验收：启动新版，野外按 F2 在两种语言间切换，近战/远程名称应同时变化；Boss 同样切换。B 查看四种图标，Q 切换后 HUD 显示对应已装备武器。
- 当前源码的英文敌人文本原本就存在，本次没有复现用户此前的旧窗口问题；已执行名称往返切换、全部文本 ID 完整性检查，以及中英文实际绘制截图检查。

## 统一要求

### 文件格式与建议交付尺寸

游戏使用 PNG 位图；玩家、NPC、怪物、建筑、道具和特效需要真实 Alpha 透明背景，不要把棋盘格背景画进图片。不使用 JPG、GIF 或截图作为最终游戏资源。

| 类别 | 第一批建议 |
|---|---|
| 地面图块 | 每块 32×32 像素，可拼接，无文字、网格线或角色 |
| 整张地面底图（可选） | 主城 960×960，野外 1280×1280，巢穴 768×768；仅画地面，不能烘焙建筑和角色 |
| 玩家、NPC、普通哥布林 | 每帧 32×32 像素透明画布；脚底中心对齐，四个朝向保持同一比例 |
| Boss | 每帧 64×64 像素透明画布；脚底中心对齐 |
| 建筑与景观 | 单独透明 PNG，画布优先为 32 的整数倍；尺寸按所占格数确定，角色通路不要画进建筑图片 |
| 装备图标 | 每张 32×32 像素透明 PNG，剑与法杖分开 |
| 攻击效果 | 单独透明 PNG；画布与帧数在生成该效果前约定，命中范围由代码决定 |

第一批可先交静态单帧。玩家与普通怪物建议分别提供 down、left、right、up 四个朝向；八向移动可复用四向外观，法杖弹道依然是八向。
如需动画精灵表，先约定每帧尺寸、行列顺序、帧数与播放速度，再生成，不把不等大的图片直接拼在一起。
名称用英文小写和下划线，例如 player_down.png、goblin_melee_left.png、goblin_king_down.png、steel_sword.png。
像素风避免模糊边缘与平滑缩放；显示时使用最近邻与整数倍缩放。生成图若不是实际目标尺寸，保留原图并检查缩小后的可读性，不仅把文件名改成 32px。
请同时保留素材来源或授权说明。当前已接入上表 PNG，其余类型素材到位后逐类加载和验收。

- 像素风、统一俯视视角、需要叠加的资源使用透明背景。
- 格子暂定 32 像素；普通角色显示约一格，Boss 暂定两格。
- 角色以脚底中心定位；建筑记录地面锚点和独立碰撞范围。
- 不将阴影、碰撞、NPC 对话或地图出口逻辑烘焙进一张地图图片。
- 地面先绘制；建筑、景观、角色按脚底 Y 坐标排序；效果按用途叠加；HUD 最后绘制。
- 会遮挡角色的屋顶/树冠需要独立前景部分，以便后续淡化，而不是直接让整栋建筑消失。
- 贴图帧尺寸、帧数、缩放方式等在生成每类素材前确定；图片尺寸不自动成为攻击或身体碰撞尺寸。
- 资源路径集中在资源加载模块管理。未来使用资源 ID 和记录，保留占位显示；不把图片文件名散落到战斗逻辑里。
- 当前主城布局与底图碰撞在 `src/world.c` 的 InitializeTown 中修改；其他地图在 InitializeWorld 中按 MapId 初始化。DrawWorld / DrawScenery / DrawInteraction 使用图片并保留 DrawPerson 等占位回退。路径清单集中在 assets.c 的 LoadSprites。

## 字体

已提供：`fonts/NotoSansCJKsc-Regular.otf`，未经修改的 Noto Sans CJK SC Regular。
授权文件：`fonts/LICENSE.txt`（SIL Open Font License 1.1）。
官方来源：
https://github.com/notofonts/noto-cjk/tree/main/Sans/OTF/SimplifiedChinese

官方许可：
https://github.com/notofonts/noto-cjk/blob/main/Sans/LICENSE

字体文件内保留原始版权元数据，项目同时附带完整授权；提交时必须一起保留。
已验证项目字体覆盖第二阶段界面的全部所需字形，正常运行不使用系统字体。
