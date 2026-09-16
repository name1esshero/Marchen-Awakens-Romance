/* English-build replacement for src/resource_catalog.c: identical except for
 * which generated .inc gResourceCatalog pulls in (see
 * tools/resource_catalog.py's build_english() and
 * tools/split_scripts_english.py). Kept as its own object, filtered out in
 * favor of this one only for mar_english.gba's link, so mar.gba's build
 * never sees anything but the static, extraction-time literals. */
#include "resource_catalog.h"
#include "nfp.h"

#include "rom_section.h"

/* Declares gMarscriptScript_<NAME> for every entry gResourceCatalog below
 * resolves as a linker symbol instead of a literal offset -- must come
 * before the array initializer that references them. Empty when nothing
 * has grown past its original archive slot. */
#include "../../build/generated/resource_catalog_english_externs.inc"

AT("001C0920")
const struct NfpHeader gRomNfpArchiveHeader = {
    "NFP2.0 (c)NOBORI 1997-2002",
    RESOURCE_CATALOG_COUNT,
    sizeof(struct NfpHeader),
    0x3420,
};

AT("001C0960")
const struct ResourceCatalogEntry gResourceCatalog[RESOURCE_CATALOG_COUNT] = {
#include "../../build/generated/resource_catalog_english.inc"
};
