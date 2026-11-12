#ifndef PROJECT_LAX_INVENTORY_H
#define PROJECT_LAX_INVENTORY_H

#include <stdbool.h>
#include "text.h"
#include "assets.h"

enum { INVENTORY_CAPACITY = 12 };
typedef enum Weapon { WEAPON_SWORD, WEAPON_STAFF } Weapon;
typedef enum ItemId { ITEM_NONE, ITEM_IRON_SWORD, ITEM_WOOD_STAFF,
                      ITEM_STEEL_SWORD, ITEM_RUNE_STAFF, ITEM_COUNT } ItemId;
typedef enum InventoryResult { INVENTORY_OK, INVENTORY_BUSY, INVENTORY_FULL,
    INVENTORY_POOR, INVENTORY_PURCHASED, INVENTORY_EMPTY, INVENTORY_EQUIPPED, INVENTORY_INVALID } InventoryResult;

typedef struct ItemDefinition {
    Weapon weapon;
    int attackBonus;
    int price;
    TextId name;
    SpriteId icon;
} ItemDefinition;

typedef struct Inventory {
    ItemId slots[INVENTORY_CAPACITY];
    int equippedSword;
    int equippedStaff;
    bool purchasedSword;
    bool purchasedStaff;
} Inventory;

void InitializeInventory(Inventory *inventory);
ItemDefinition GetItemDefinition(ItemId item);
ItemId GetEquippedItem(const Inventory *inventory, Weapon weapon);
InventoryResult EquipInventoryItem(Inventory *inventory, int slot, bool inCombat);
InventoryResult BuyInventoryItem(Inventory *inventory, int *gold, ItemId item);

#endif
