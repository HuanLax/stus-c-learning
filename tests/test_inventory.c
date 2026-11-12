#include "app.h"

#include <assert.h>
#include <stdio.h>

int main(void)
{
    const SpriteId icons[] = { SPRITE_IRON_SWORD, SPRITE_WOOD_STAFF, SPRITE_STEEL_SWORD, SPRITE_RUNE_STAFF };
    for (int i = 0; i < 4; i++) assert(GetItemDefinition((ItemId)(ITEM_IRON_SWORD + i)).icon == icons[i]);
    assert(GetItemDefinition(ITEM_NONE).icon == SPRITE_COUNT);
    Player player;
    InitializePlayer(&player, (Vector2){ 480, 528 });
    assert(player.level == 1 && player.gold == 0 && GetPlayerAttack(&player) == 20);
    assert(player.inventory.slots[0] == ITEM_IRON_SWORD && player.inventory.slots[1] == ITEM_WOOD_STAFF);
    assert(BuyInventoryItem(&player.inventory, &player.gold, ITEM_STEEL_SWORD) == INVENTORY_POOR);
    assert(player.gold == 0 && player.inventory.slots[2] == ITEM_NONE && !player.inventory.purchasedSword);
    player.gold = 100;
    assert(BuyInventoryItem(&player.inventory, &player.gold, ITEM_STEEL_SWORD) == INVENTORY_OK);
    assert(player.gold == 25 && player.inventory.slots[2] == ITEM_STEEL_SWORD);
    assert(GetEquippedItem(&player.inventory, WEAPON_SWORD) == ITEM_IRON_SWORD && GetPlayerAttack(&player) == 20);
    assert(BuyInventoryItem(&player.inventory, &player.gold, ITEM_STEEL_SWORD) == INVENTORY_PURCHASED);
    assert(player.gold == 25 && player.inventory.slots[3] == ITEM_NONE);
    assert(EquipInventoryItem(&player.inventory, 2, true) == INVENTORY_BUSY);
    assert(player.inventory.equippedSword == 0);
    assert(EquipInventoryItem(&player.inventory, -1, false) == INVENTORY_INVALID);
    assert(EquipInventoryItem(&player.inventory, 3, false) == INVENTORY_EMPTY);
    assert(EquipInventoryItem(&player.inventory, 2, false) == INVENTORY_OK);
    assert(GetPlayerAttack(&player) == 32 && player.baseAttack == 20);
    assert(player.inventory.slots[0] == ITEM_IRON_SWORD);
    assert(EquipInventoryItem(&player.inventory, 2, false) == INVENTORY_EQUIPPED);
    player.gold = 100;
    assert(BuyInventoryItem(&player.inventory, &player.gold, ITEM_RUNE_STAFF) == INVENTORY_OK);
    assert(player.gold == 0 && player.inventory.equippedStaff == 1);
    assert(EquipInventoryItem(&player.inventory, 3, false) == INVENTORY_OK);
    player.weapon = WEAPON_STAFF;
    assert(GetPlayerAttack(&player) == 32 && player.inventory.slots[1] == ITEM_WOOD_STAFF);
    player.weapon = WEAPON_SWORD;
    EquipInventoryItem(&player.inventory, 0, false);
    assert(GetPlayerAttack(&player) == 20 && player.baseAttack == 20);

    Inventory full;
    InitializeInventory(&full);
    for (int i = 0; i < INVENTORY_CAPACITY; i++) full.slots[i] = ITEM_IRON_SWORD;
    int gold = 200;
    assert(BuyInventoryItem(&full, &gold, ITEM_STEEL_SWORD) == INVENTORY_FULL);
    assert(gold == 200 && !full.purchasedSword);
    for (int i = 0; i < INVENTORY_CAPACITY; i++) assert(full.slots[i] == ITEM_IRON_SWORD);

    InitializePlayer(&player, (Vector2){ 0, 0 });
    player.health = 40;
    GrantPlayerRewards(&player, 25, 60);
    assert(player.level == 2 && player.experience == 0 && player.gold == 25);
    assert(player.maxHealth == 120 && player.health == 60 && player.baseAttack == 24);
    GrantPlayerRewards(&player, 0, 240);
    assert(player.level == 4 && player.experience == 0 && player.maxHealth == 160 && player.health == 100);
    GrantPlayerRewards(&player, 0, 180);
    assert(player.level == 5 && player.experience == 0 && player.baseAttack == 36);
    GrantPlayerRewards(&player, 25, 9999);
    assert(player.level == 5 && player.experience == 0 && player.gold == 50 && player.maxHealth == 180);
    InitializePlayer(&player, (Vector2){ 0, 0 });
    player.health = 0;
    GrantPlayerRewards(&player, 25, 60);
    assert(player.level == 2 && player.maxHealth == 120 && player.health == 0);

    App app = { 0 };
    for (int map = 0; map < MAP_COUNT; map++) InitializeWorld(&app.worlds[map], (MapId)map);
    InitializePlayer(&app.player, app.worlds[MAP_FIELD].spawn);
    InitializeCombat(&app.combat, MAP_FIELD);
    app.currentMap = MAP_FIELD;
    for (int i = 0; i < app.combat.enemyCount; i++) app.combat.enemies[i].health = 0;
    assert(ClaimEnemyRewards(&app.combat, &app.player) == 6);
    assert(app.player.gold == 150 && app.player.level == 3 && app.player.experience == 20 && app.player.defeatedGoblins == 6);
    assert(ClaimEnemyRewards(&app.combat, &app.player) == 0 && app.player.gold == 150);
    app.player.gold = 300;
    app.selectedProduct = 0;
    BuySelectedProduct(&app);
    assert(app.player.gold == 225 && app.inventoryMessage == TEXT_BUY_SUCCESS);
    app.selectedSlot = 2;
    app.combat.inCombat = true;
    EquipSelectedItem(&app);
    assert(app.inventoryMessage == TEXT_EQUIP_BUSY && app.player.inventory.equippedSword == 0);
    app.combat.inCombat = false;
    EquipSelectedItem(&app);
    assert(app.player.inventory.equippedSword == 2 && GetPlayerAttack(&app.player) == 40);
    int previousGold = app.player.gold;
    int previousXP = app.player.experience;
    app.player.health = 0;
    RespawnAppPlayer(&app, true);
    assert(app.currentMap == MAP_TOWN && app.player.health == 140 && app.player.level == 3);
    assert(app.player.gold == previousGold && app.player.experience == previousXP && app.player.inventory.equippedSword == 2);
    app.nearbyInteraction = 1;
    ActivateNearbyInteraction(&app);
    assert(app.screen == SCREEN_SHOP);
    puts("PASS: atomic purchases, bag full, unique upgrades, manual equipment, combat lock, derived damage, level cap, one-time rewards and progress-preserving revival");
    return 0;
}
