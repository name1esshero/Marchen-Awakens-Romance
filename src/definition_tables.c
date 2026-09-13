/*
 * Fixed-layout ÄRM and inventory definition tables.
 *
 * The editable metadata lives in data/definition_tables.json.  Japanese names
 * and descriptions remain in text/arm_definitions.txt and
 * text/item_definitions.txt, where the English mapping tools already consume
 * them.  tools/definition_tables.py combines those readable sources into the
 * fixed-width initializers included below.
 */
#include "item.h"

#define AT(x) __attribute__((section(".rom." x)))

AT("001B096C")
const struct ArmDefinition gArmDefinitions[ARM_COUNT] = {
#include "../build/generated/arm_definitions.inc"
};

AT("001BE82C")
const struct ItemDefinition gItemDefinitions[ITEM_COUNT] = {
#include "../build/generated/item_definitions.inc"
};
