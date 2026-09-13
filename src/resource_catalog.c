/* NFP2.0 header and named archive registry built from readable source. */
#include "resource_catalog.h"
#include "nfp.h"

#define AT(x) __attribute__((section(".rom." x)))

AT("001C0920")
const struct NfpHeader gRomNfpArchiveHeader = {
    "NFP2.0 (c)NOBORI 1997-2002",
    RESOURCE_CATALOG_COUNT,
    sizeof(struct NfpHeader),
    0x3420,
};

AT("001C0960")
const struct ResourceCatalogEntry gResourceCatalog[RESOURCE_CATALOG_COUNT] = {
#include "../build/generated/resource_catalog.inc"
};
