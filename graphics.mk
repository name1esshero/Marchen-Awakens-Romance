# Graphics build rules.
#
# Art lives in graphics/ as:
#     <asset>.png   the indexed editable image or tile sheet
#     backgrounds/*.json   recovered map layouts for assembled images
#     palettes/*.pal   named original palettes in JASC format
#
# PNG/layout sources compile to tile bytes, then to deterministic VRAM-safe LZ.
# All 104 original graphics streams are reproduced by this encoder without
# reading a baserom or retaining compressed source templates. Edited streams
# must fit their fixed ROM allocation. Every .lz output lives under build/.

ASSET_MANIFEST := assets.json
GFX_PNGS       := $(shell $(PYTHON) -c 'import json; print(" ".join(e["path"]+".png" for e in json.load(open("assets.json")) if e["kind"] not in ("script", "data", "rle_data")))')
MAPPED_PNGS := $(shell $(PYTHON) tools/mapped_images.py dependencies)
MAPPED_LAYOUTS := $(shell $(PYTHON) -c 'import json; print(" ".join(e["image_layout"] for e in json.load(open("assets.json")) if e["kind"] == "mapped_image"))')
MAPPED_UNUSED := $(shell $(PYTHON) -c 'import json; print(" ".join(e["unused_tiles_image"] for e in json.load(open("assets.json")) if "unused_tiles_image" in e))')

# One pass stages every asset; the stamp keeps it from re-running needlessly.
$(BUILD)/.graphics.stamp: $(MAPPED_PNGS) $(MAPPED_LAYOUTS) $(MAPPED_UNUSED) tools/mapped_images.py $(ASSET_MANIFEST) $(GFX_PNGS) \
                          tools/build_assets.py tools/gfx.py \
                          tools/lz77.py tools/rle.py
	@mkdir -p $(BUILD)/graphics $(BUILD)/scripts
	$(PYTHON) tools/build_assets.py
	@touch $@

.PHONY: graphics graphics-clean

# Force every asset to be staged again from whatever is currently in graphics/.
graphics:
	@rm -f $(BUILD)/.graphics.stamp
	@$(MAKE) --no-print-directory $(BUILD)/.graphics.stamp

graphics-clean:
	rm -rf $(BUILD)/graphics $(BUILD)/scripts $(BUILD)/.graphics.stamp

# Regenerate the per-asset rules after editing assets.json.
.PHONY: rules
rules:
	$(PYTHON) tools/gen_rules.py

include graphics_rules.mk
