/*
 * Readable model of the generated-map carving probes at 0x08071848 and
 * 0x080718A0. This file is intentionally not linked: the original functions
 * remain in assembly because this model is not instruction-matched against
 * the original routines yet, and must compile to the exact ROM bytes before
 * replacing them.
 */
#include "gba/types.h"

#define GENERATED_MAP_DIRECTION_EAST 0
#define GENERATED_MAP_DIRECTION_SOUTH 1
#define GENERATED_MAP_DIRECTION_WEST 2
#define GENERATED_MAP_DIRECTION_NORTH 3
#define GENERATED_MAP_CARVE_STEP 2
#define GENERATED_MAP_INTERIOR_MARGIN 3

struct GeneratedMapGridDimensions
{
    u16 width;
    u16 height;
};

/**
 * @brief Test whether a two-cell cardinal carve reaches an unused grid cell.
 * @param dimensions Grid width and height in cells.
 * @param cells Row-major byte-state grid; zero denotes an unused cell here.
 * @param index Row-major index of the current cell.
 * @param direction East, south, west, or north (0 through 3).
 * @return One if the two-step destination is inside the carved interior and
 *         has state zero; otherwise zero.
 *
 * The generator caller initializes this byte grid to zero, marks selected
 * destinations, and chooses a direction by retrying this test. This function
 * alone does not establish the meaning of every other grid state.
 */
s32 GeneratedMapCanCarveTwoCellStep(
    const struct GeneratedMapGridDimensions *dimensions,
    const u8 *cells,
    u32 index,
    s32 direction)
{
    u32 width = dimensions->width;
    u32 height = dimensions->height;
    u32 x;
    u32 y;
    u32 destination;

    if (direction == GENERATED_MAP_DIRECTION_EAST)
    {
        x = index % width;
        if (x >= width - GENERATED_MAP_INTERIOR_MARGIN)
            return 0;
        destination = index + GENERATED_MAP_CARVE_STEP;
    }
    else if (direction == GENERATED_MAP_DIRECTION_SOUTH)
    {
        y = index / width;
        if (y >= height - GENERATED_MAP_INTERIOR_MARGIN)
            return 0;
        destination = index + GENERATED_MAP_CARVE_STEP * width;
    }
    else if (direction == GENERATED_MAP_DIRECTION_WEST)
    {
        x = index % width;
        if (x <= GENERATED_MAP_CARVE_STEP)
            return 0;
        destination = index - GENERATED_MAP_CARVE_STEP;
    }
    else if (direction == GENERATED_MAP_DIRECTION_NORTH)
    {
        y = index / width;
        if (y <= GENERATED_MAP_CARVE_STEP)
            return 0;
        destination = index - GENERATED_MAP_CARVE_STEP * width;
    }
    else
    {
        return 0;
    }

    return cells[destination] == 0;
}

/**
 * @brief Test the four carve directions in their original order.
 * @return One as soon as an eligible two-cell destination is found.
 */
s32 GeneratedMapHasCarveDirection(
    const struct GeneratedMapGridDimensions *dimensions,
    const u8 *cells,
    u32 index)
{
    if (GeneratedMapCanCarveTwoCellStep(dimensions, cells, index,
                                        GENERATED_MAP_DIRECTION_EAST))
        return 1;
    if (GeneratedMapCanCarveTwoCellStep(dimensions, cells, index,
                                        GENERATED_MAP_DIRECTION_SOUTH))
        return 1;
    if (GeneratedMapCanCarveTwoCellStep(dimensions, cells, index,
                                        GENERATED_MAP_DIRECTION_WEST))
        return 1;
    if (GeneratedMapCanCarveTwoCellStep(dimensions, cells, index,
                                        GENERATED_MAP_DIRECTION_NORTH))
        return 1;
    return 0;
}
