# Project-Lax

COS10009 Introduction to Programming 课程项目：使用 C99 / raylib 开发的中英文俯视角单机探险游戏。

当前版本包含主城、哥布林草地、Boss 巢穴、剑与法杖、敌人寻路、成长、背包与商店、主线任务、存读档。主城使用完整村庄底图；野外有 10 棵树和 6 块石头作为障碍。

## 启动与构建

直接试玩：打开根目录的 `project-lax.exe - Shortcut.lnk`，或运行 `bin/project-lax.exe`。请保持 `bin`、`pic`、`assets` 的相对位置。

VS Code 中打开项目目录，按 `Ctrl+Shift+B` 构建 Debug，按 `F5` 调试运行。也可以从任务列表选择 `Run game (no debugger)`，先构建再启动。

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\build.ps1 -Configuration Debug
& .\bin\project-lax.exe
```

发布构建将 `Debug` 改为 `Release`，两种配置都输出 `bin/project-lax.exe`。

构建配置使用以下依赖；更换电脑或安装路径后，调整 `scripts/build.ps1` 的参数与 `.vscode` 配置：

- GCC：`C:\raylib\w64devkit\bin\gcc.exe`
- GDB：`C:\raylib\w64devkit\bin\gdb.exe`
- raylib：`C:\raylib\raylib\src` 内的 `raylib.h`、`libraylib.a`
- 编译选项：C99、UTF-8、`-Wall -Wextra -Wpedantic -Werror`

## 操作与通关

| 操作 | 按键 |
|---|---|
| 移动 | WASD / 方向键 |
| 普通攻击 | J / 鼠标左键，支持按住连续攻击 |
| 切换剑 / 法杖 | Q |
| 与 NPC、治疗石、入口交互 | E / 界面交互按钮 |
| 背包 | B，方向键选择，Enter 装备 |
| 对话翻页 / 商店购买 | Enter / 对应按钮 |
| 暂停 / 返回 | Esc |
| 切换中英文 | F2 |
| 手动保存 | 游戏窗口内 F5，需和平且存活 |

攻击朝向跟随移动方向；剑为四向、法杖为八向。主城和入口安全区不能攻击。战斗中可以查看背包，但不能换装。

通关流程：向主城村长接取任务 → 清剿四只近战与两只远程哥布林 → 回村交付，解锁巢穴 → 击败哥布林王 → 回村长处完成最终交付。

主城右侧入口通往野外，野外左侧入口回城，右侧入口通往巢穴。死亡后可以选择在主城或当前地图安全区满血复活。

窗口支持缩放和最大化，最小 640×480；以 1024×768 虚拟画布等比例适配，地图镜头为 2 倍。当前人物使用四向静态图片，攻击效果仍使用程序绘制；MP 和终结技是未开放的界面预留位。

## 存档

个人进度保存在 `saves/save.txt`。正常退出自动保存原地图、位置、战斗现场和界面；继续游戏恢复保存状态。主菜单直接退出不会覆盖已有存档。

手动保存要求和平且存活；自动退出保存允许战斗现场。坏存档不会自动覆盖，保存失败时可重试或取消退出。复制项目时，如需保留个人进度，一并复制 `saves`。

## 文件位置

| 路径 | 用途 |
|---|---|
| `src/` | 游戏 C 源码和头文件 |
| `pic/` | 当前游戏使用的 25 张 PNG |
| `assets/fonts/` | 中文字体与分发授权，需一起保留 |
| `scripts/build.ps1` | 构建与逻辑检查入口 |
| `tests/` | 六组逻辑检查源码，构建脚本仍使用 |
| `bin/project-lax.exe` | 当前可运行程序 |
| `saves/` | 个人存档 |
| `docs/MAP_LAYOUT.md` | 最新地图坐标、障碍与旧档迁移说明 |
| `_cleanup_review_2026-11-17/` | 已归档的旧文档、测试产物与临时文件，供检查后自行删除 |

地图详细坐标见 [地图布局](docs/MAP_LAYOUT.md)。中文字体使用 Noto Sans CJK SC，授权见 [LICENSE.txt](assets/fonts/LICENSE.txt)。

早期阶段讲解、原版 README、美术接入记录和未来功能备忘录，已集中到待清理文件夹的 `02_old_documents/`；其中的旧规则属于开发历史。需要准备代码讲解时，可先保留这些资料。

## 开发检查

构建时可按需增加 `-CheckMovement`、`-CheckCombat`、`-CheckAI`、`-CheckInventory`、`-CheckQuest`、`-CheckSave`，可同时启用。检查会在 `bin` 中重新生成测试程序和测试文件。

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\build.ps1 -CheckMovement -CheckCombat -CheckAI -CheckInventory -CheckQuest -CheckSave
& .\bin\project-lax.exe --smoke-test
```

`--smoke-test` 检查字体和必要贴图，并在 `bin` 中生成截图，不写实际存档。`--preview-boss` 可单独预览巢穴，也不写实际存档。
