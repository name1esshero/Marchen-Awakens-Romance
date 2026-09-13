#ifndef RESOURCE_CATALOG_H
#define RESOURCE_CATALOG_H

#include "gba/types.h"

#define RESOURCE_CATALOG_COUNT 830
#define RESOURCE_NAME_SIZE 12

/* Master NFP catalog at 081C0F60. Offsets are file-relative ROM offsets. */
struct ResourceCatalogEntry {
    char name[RESOURCE_NAME_SIZE];
    u32 romOffset;
};

extern const struct ResourceCatalogEntry
    gResourceCatalog[RESOURCE_CATALOG_COUNT];

#endif
