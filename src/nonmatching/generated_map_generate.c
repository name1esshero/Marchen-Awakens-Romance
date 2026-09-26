/*
 * GeneratedMapGenerate, 0x08070238 (952 bytes). Still assembly as
 * sub_08070238; this candidate is not linked into the matching build.
 *
 * Builds a procedural field: carves a maze with a recursive backtracker over
 * a temporary byte grid (two cells per step, explicit u16 stack of cells),
 * remembers the cell reached at the greatest depth, marks the odd-row/even-
 * column lattice as room cells, converts the grid to cell records
 * (sub_08071A20), gives every room/corridor cell a random matching room
 * template, and places the first runtime room at a tile with attribute 360
 * inside the farthest cell's KMP.
 *
 * Why this is nonmatching: this source reaches the ROM's exact 952-byte size
 * and nearly all of its control flow, but agbcc's global allocator gives
 * `cells` register r9 and spills `depth`, where the ROM keeps `depth` in r9
 * and `cells` in a stack slot (sp+288). That one swap renames registers
 * through the whole routine (612 bytes differ) and leaves the frame 4 bytes
 * smaller. See docs/decompilation-notes.md, "Maze generator", for every
 * spelling tried. No forced registers or compiler hints are used.
 */
#include "gba/types.h"
#include "map_generation.h"
#include "kmp.h"
#include "heap.h"
#include "nfp.h"
#include "hit_region.h"

extern char *strcpy(char *destination, const char *source);
extern char *strcat(char *destination, const char *source);
extern void *GetBattleDefinition(u32 index);
extern void sub_08071A20(struct GeneratedFieldMap *map, u8 *cells);
extern struct GeneratedMapRuntimeRoom *sub_08070EA0(s32 selector);
/* ".KMP" and "MAR.NFP" at 0x08089460 and 0x08089468. */
extern const char gGeneratedMapKmpExtension[];
extern const char gGeneratedMapArchiveName[];

#define CARVE_CELL_WALL     0
#define CARVE_CELL_CORRIDOR 1
#define CARVE_CELL_ROOM     2
#define NO_ROOM 0xFFFF
#define CELL_RECORD_ROOM_LATTICE 2
#define CELL_RECORD_KIND_MASK 0xF00
#define CELL_RECORD_KIND_ROOM 0x100
#define CELL_RECORD_KIND_CORRIDOR 0x200
#define CELL_RECORD_CONNECTION_MASK 0xFFF
#define SPAWN_ATTRIBUTE 360
#define SPAWN_UNKNOWN10 64
#define TILE_SIZE 8

/**
 * @brief Generate a procedural field map from a start cell and seed.
 * @param map Map to fill; width, height, and arenaIndex must be set.
 * @param start Requested start cell; invalid cells fall back to width + 1.
 * @param seed Seed for MapGenerationRandom.
 */
void GeneratedMapGenerate(struct GeneratedFieldMap *map, u16 start, u32 seed)
{
    char name[32];
    struct KmpViewport view;
    s32 depth = 0;
    u8 *cells;
    u16 *stack;
    u16 maxDepth = 0;
    u32 farthest = 0;
    u32 i;
    u32 cell;
    u32 direction;
    u32 index;
    s32 tile;
    struct GeneratedMapRuntimeRoom *room;

    cells = HeapAlloc(NULL, map->width * map->height);
    stack = HeapAlloc(NULL, map->width * map->height * 2);
    map->seed = seed;
    MapGenerationSeedRandom(seed);
    map->rooms = GetBattleDefinition(map->arenaIndex);

    for (i = 0; i < map->height * map->width; i++)
    {
        cells[i] = CARVE_CELL_WALL;
        stack[i] = 0;
        map->cellRecords[i] = 0;
        map->cellRoomIndices[i] = NO_ROOM;
    }
    map->unknown624 = 0;
    map->unknown628 = 0;
    map->unknown626 = 0;
    for (i = 0; i < GENERATED_MAP_RUNTIME_ROOM_COUNT; i++)
    {
        map->runtimeRooms[i].roomIndex = -1;
        map->runtimeRooms[i].unknown04 = 0;
        map->runtimeRooms[i].unknown10 = 0;
        map->runtimeRooms[i].x = 0;
        map->runtimeRooms[i].y = 0;
        map->runtimeRooms[i].active = 0;
        map->runtimeRooms[i].scriptFlag = 0;
    }
    HitRegionDisableAll();

    if (GeneratedMapIsValidStartCell(map, start))
        cell = map->startCell = start;
    else
        cell = map->startCell = map->width + 1;
    map->currentCell = cell;
    cells[cell] = CARVE_CELL_CORRIDOR;

    goto carve;
backtrack:
    cell = stack[depth];
carve:
    {
        while (GeneratedMapHasCarveDirection(map, cells, cell))
        {
            do
                direction = MapGenerationRandom() & 3;
            while (!GeneratedMapCanCarveTwoCellStep(map, cells, cell, direction));
            stack[depth] = cell;
            depth = (s16)(depth + 1);
            switch (direction)
            {
            case GENERATED_MAP_DIRECTION_EAST:
                cells[++cell] = CARVE_CELL_CORRIDOR;
                cells[++cell] = CARVE_CELL_CORRIDOR;
                break;
            case GENERATED_MAP_DIRECTION_SOUTH:
                cells[cell + map->width] = CARVE_CELL_CORRIDOR;
                cells[cell + map->width * 2] = CARVE_CELL_CORRIDOR;
                cell += map->width * 2;
                break;
            case GENERATED_MAP_DIRECTION_WEST:
                cells[--cell] = CARVE_CELL_CORRIDOR;
                cells[--cell] = CARVE_CELL_CORRIDOR;
                break;
            case GENERATED_MAP_DIRECTION_NORTH:
                cells[cell - map->width] = CARVE_CELL_CORRIDOR;
                cells[cell - map->width * 2] = CARVE_CELL_CORRIDOR;
                cell -= map->width * 2;
                break;
            }
        }
        if (depth > maxDepth)
        {
            farthest = cell;
            maxDepth = depth;
        }
        depth--;
        if (depth >= 0)
            goto backtrack;
    }

    for (i = 0; i < map->height * map->width; i++)
    {
        if (i % 2 == 0 && (i / map->width) % 2 != 0)
        {
            cells[i] = CARVE_CELL_ROOM;
            map->cellRecords[i] = CELL_RECORD_ROOM_LATTICE;
        }
    }
    sub_08071A20(map, cells);

    for (i = 0; i < map->width * map->height; i++)
    {
        u32 kind = map->cellRecords[i] & CELL_RECORD_KIND_MASK;
        if (kind == CELL_RECORD_KIND_ROOM || kind == CELL_RECORD_KIND_CORRIDOR)
        {
            do
                index = MapGenerationRandom() % map->rooms[0].connections + 1;
            while (map->rooms[index].connections != (map->cellRecords[i] & CELL_RECORD_CONNECTION_MASK));
            map->cellRoomIndices[i] = index;
        }
    }

    room = sub_08070EA0(-1);
    room->roomIndex = farthest;
    room->unknown04 = 0;
    room->unknown10 = SPAWN_UNKNOWN10;
    room->active = 1;
    strcpy(name, map->rooms[map->cellRoomIndices[farthest]].name);
    strcat(name, gGeneratedMapKmpExtension);
    view.data = NfpOpenByName(gGeneratedMapArchiveName, name);
    tile = GeneratedMapChooseAttributeIndex(&view, SPAWN_ATTRIBUTE);
    room->x = tile % view.data->widthTiles * TILE_SIZE;
    room->y = tile / view.data->widthTiles * TILE_SIZE;
    if (cells != NULL)
        HeapFree(NULL, cells);
    if (stack != NULL)
        HeapFree(NULL, stack);
}
