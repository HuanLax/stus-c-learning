#include "inventory.h"

ItemDefinition GetItemDefinition(ItemId item)
{
    const ItemDefinition definitions[ITEM_COUNT] = {
        [ITEM_NONE] = { WEAPON_SWORD, 0, 0, TEXT_EMPTY_SLOT, SPRITE_COUNT },
        [ITEM_IRON_SWORD] = { WEAPON_SWORD, 0, 0, TEXT_IRON_SWORD, SPRITE_IRON_SWORD },
        [ITEM_WOOD_STAFF] = { WEAPON_STAFF, 0, 0, TEXT_WOOD_STAFF, SPRITE_WOOD_STAFF },
        [ITEM_STEEL_SWORD] = { WEAPON_SWORD, 12, 75, TEXT_STEEL_SWORD, SPRITE_STEEL_SWORD },
        [ITEM_RUNE_STAFF] = { WEAPON_STAFF, 12, 100, TEXT_RUNE_STAFF, SPRITE_RUNE_STAFF }
    };
    if (item < ITEM_NONE || item >= ITEM_COUNT) return definitions[ITEM_NONE];
    return definitions[item];
}

void InitializeInventory(Inventory *inventory)
{
    *inventory = (Inventory){ 0 };
    inventory->slots[0] = ITEM_IRON_SWORD;
    inventory->slots[1] = ITEM_WOOD_STAFF;
    inventory->equippedSword = 0;
    inventory->equippedStaff = 1;
}

ItemId GetEquippedItem(const Inventory *inventory, Weapon weapon)
{
    int slot = weapon == WEAPON_SWORD ? inventory->equippedSword : inventory->equippedStaff;
    if (slot < 0 || slot >= INVENTORY_CAPACITY) return ITEM_NONE;
    ItemId item = inventory->slots[slot];
    if (item <= ITEM_NONE || item >= ITEM_COUNT || GetItemDefinition(item).weapon != weapon) return ITEM_NONE;
    return item;
}

InventoryResult EquipInventoryItem(Inventory *inventory, int slot, bool inCombat)
{
    if (inCombat) return INVENTORY_BUSY;
    if (slot < 0 || slot >= INVENTORY_CAPACITY) return INVENTORY_INVALID;
    ItemId item = inventory->slots[slot];
    if (item == ITEM_NONE) return INVENTORY_EMPTY;
    if (item < ITEM_NONE || item >= ITEM_COUNT) return INVENTORY_INVALID;
    int *equipped = GetItemDefinition(item).weapon == WEAPON_SWORD ?
        &inventory->equippedSword : &inventory->equippedStaff;
    if (*equipped == slot) return INVENTORY_EQUIPPED;
    *equipped = slot; /* item stays in the bag; old equipment is not deleted */
    return INVENTORY_OK;
}

InventoryResult BuyInventoryItem(Inventory *inventory, int *gold, ItemId item)
{
    if (item != ITEM_STEEL_SWORD && item != ITEM_RUNE_STAFF) return INVENTORY_INVALID;
    bool *purchased = item == ITEM_STEEL_SWORD ? &inventory->purchasedSword : &inventory->purchasedStaff;
    if (*purchased) return INVENTORY_PURCHASED;
    int empty = -1;
    for (int slot = 0; slot < INVENTORY_CAPACITY; slot++) {
        if (inventory->slots[slot] == ITEM_NONE) { empty = slot; break; }
    }
    if (empty < 0) return INVENTORY_FULL;
    int price = GetItemDefinition(item).price;
    if (*gold < price) return INVENTORY_POOR;
    /* Commit only after every check passed. No failed purchase changes progress. */
    *gold -= price;
    inventory->slots[empty] = item;
    *purchased = true;
    return INVENTORY_OK;
}
