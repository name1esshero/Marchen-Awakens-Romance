/*
 * GeneratedMapBuildCellRecords, 0x08071A20 (796 bytes). Still assembly as
 * sub_08071A20; this candidate is not linked into the matching build.
 *
 * Converts the carving byte grid into the map's 32-bit per-cell records at
 * GeneratedFieldMap +0x650 in three passes: rooms and walls, corridors, then
 * a random connection pattern for every room cell.
 *
 * Why this is nonmatching (see docs/decompilation-notes.md, "Cell-record
 * builder", for the full search): this shape reproduces pass 1, both corridor
 * branches, and the ROM's register lifetimes for the shared locals, but three
 * independent differences remain, 155 bytes in total:
 *  1. Horizontal-continuation low byte. The ROM keeps `value` in r5 and
 *     computes `value & 0xFF` and then `& 7` into fresh r0/r1; agbcc reuses r5
 *     in place. `(value & 0xFF) & 7` folds to one AND; `(u8)value` emits
 *     lsl/lsr; `& 0xFF` in the else branch as well changes the block layout.
 *  2. Pass 3's outer loop. The ROM enters through a jump to the bottom exit
 *     test (no header copy); every for/while/continue spelling here gets the
 *     test duplicated at the top. A goto retry loop avoids the copy (120 bytes
 *     off) but loses the ROM's hoisting of the pattern table into sl.
 *  3. Small r6/r7 and r0/r1/r2 scratch swaps in pass 1 and the loop tails.
 * No forced registers, fences, or pointer/integer steering are used.
 */
#include "gba/types.h"
#include "map_generation.h"

/* Byte states in the carving grid built by the generator. */
#define CARVE_CELL_WALL     0
#define CARVE_CELL_CORRIDOR 1
#define CARVE_CELL_ROOM     2

/* Cell-record bits. Low byte: variant index, +8 on odd rows/columns. */
#define CELL_RECORD_PARITY_COLUMN 0x001
#define CELL_RECORD_PARITY_ROW    0x002
#define CELL_RECORD_ROOM          0x100
#define CELL_RECORD_CORRIDOR      0x200
#define CELL_RECORD_WALL          0x400
#define CELL_RECORD_ODD_VARIANT   8
#define CELL_RECORD_CONNECTION_BIT 0x10000
#define CELL_RECORD_CONNECTIONS   0xFFFF0000
#define CELL_RECORD_VERTICAL_LINK 0x000F0000
#define CELL_RECORD_HORIZONTAL_LINK 0xF0000000
#define CELL_RECORD_VARIANT       0xFF

#define CORRIDOR_VARIANT_MASK 3
#define ROOM_PATTERN_COUNT 13

extern const u32 gBattleArenaVisibilityMasks[ROOM_PATTERN_COUNT];

/**
 * @brief Convert the carving grid into per-cell generation records.
 * @param map Generated map whose +0x650 records are rewritten.
 * @param cells Carving grid: wall, corridor, or room byte per cell.
 */
void GeneratedMapBuildCellRecords(struct GeneratedFieldMap *map, const u8 *cells)
{
    u32 i;
    u8 cell;
    u32 shape;
    u32 record;
    u32 value;

    for (i = 0; i < map->height * map->width; i++)
    {
        if (cells[i] == CARVE_CELL_ROOM)
        {
            map->cellRecords[i] = CELL_RECORD_ROOM;
        }
        else if (cells[i] == CARVE_CELL_WALL)
        {
            map->cellRecords[i] = CELL_RECORD_WALL;
            if ((i / map->width) % 2 == 1 && (i % map->width) % 2 == 0)
                map->cellRecords[i] |= CELL_RECORD_PARITY_ROW;
            else if ((i / map->width) % 2 == 0 && (i % map->width) % 2 == 1)
                map->cellRecords[i] |= CELL_RECORD_PARITY_COLUMN;
        }
    }

    for (i = 0; i < map->height * map->width; i++)
    {
        cell = cells[i];
        if (cell == CARVE_CELL_CORRIDOR)
        {
            shape = GeneratedMapGetPathNeighborShape(map, cells, i, FALSE);
            if (shape == GENERATED_MAP_PATH_VERTICAL)
            {
                record = map->cellRecords[i];
                if (record == 0)
                {
                    value = MapGenerationRandom() & CORRIDOR_VARIANT_MASK;
                    map->cellRecords[i - map->width] |= CELL_RECORD_CONNECTION_BIT << (value + 8);
                    map->cellRecords[i + map->width] |= CELL_RECORD_CONNECTION_BIT << (3 - value);
                    map->cellRecords[i] = value | CELL_RECORD_CORRIDOR;
                    map->cellRecords[i] |= (CELL_RECORD_CONNECTION_BIT << (value + 8))
                                          | (CELL_RECORD_CONNECTION_BIT << (3 - value));
                    if ((i / map->width) % 2 == 1)
                        map->cellRecords[i] += CELL_RECORD_ODD_VARIANT;
                }
                else
                {
                    value = map->cellRecords[i - map->width];
                    map->cellRecords[i + map->width] |= record & CELL_RECORD_VERTICAL_LINK;
                    map->cellRecords[i] = value;
                    if ((i / map->width) % 2 == 1)
                        map->cellRecords[i] += CELL_RECORD_ODD_VARIANT;
                }
            }
            else if (shape == GENERATED_MAP_PATH_HORIZONTAL)
            {
                record = map->cellRecords[i];
                if (record == 0)
                {
                    value = MapGenerationRandom() & CORRIDOR_VARIANT_MASK;
                    map->cellRecords[i - 1] |= CELL_RECORD_CONNECTION_BIT << (value + 4);
                    map->cellRecords[i + 1] |= CELL_RECORD_CONNECTION_BIT << (15 - value);
                    map->cellRecords[i] = (value + 4) | CELL_RECORD_CORRIDOR;
                    map->cellRecords[i] |= (CELL_RECORD_CONNECTION_BIT << (value + 4))
                                          | (CELL_RECORD_CONNECTION_BIT << (15 - value));
                    if ((i % map->width) % 2 == 1)
                        map->cellRecords[i] += CELL_RECORD_ODD_VARIANT;
                }
                else
                {
                    value = map->cellRecords[i - 1];
                    map->cellRecords[i + 1] |= record & CELL_RECORD_HORIZONTAL_LINK;
                    map->cellRecords[i] = value & ~CELL_RECORD_VARIANT;
                    if (i % 2 != 0)
                        map->cellRecords[i] += (value & CELL_RECORD_VARIANT) % 8;
                    else
                        map->cellRecords[i] = (u8)value + CELL_RECORD_ODD_VARIANT;
                }
            }
        }
    }

    for (i = 0; i < map->height * map->width; i++)
    {
        if (map->cellRecords[i] & CELL_RECORD_ROOM)
        {
            u32 required;
            u32 pattern;

            do
            {
                required = map->cellRecords[i] & CELL_RECORD_CONNECTIONS;
                pattern = MapGenerationRandom() % ROOM_PATTERN_COUNT;
            } while ((gBattleArenaVisibilityMasks[pattern] & required) != required);
            map->cellRecords[i] |= pattern;
        }
    }
}
