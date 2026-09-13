# MAR - Knockin' on Heaven's Door : disassembly / decompilation
#
# Usage:
#   make -j$(nproc)     build the ROM
#   make compare        build and verify it matches the original byte for byte
#   make extract        regenerate graphics/, text/ and the split from baserom
#   make clean          remove build output
#
# The build is fully parallel-safe: every object has an explicit rule and no
# two rules write the same file.

NAME        := mar
.DEFAULT_GOAL := all
BASEROM     := baserom.gba
BUILD       := build
TARGET      := $(NAME).gba
ELF         := $(BUILD)/$(NAME).elf

PREFIX      ?= arm-none-eabi-
AS          := $(PREFIX)as
CC          := $(PREFIX)gcc
# The ROM was built with a compiler of its era. agbcc is that compiler, as
# preserved by the pret projects, and it reproduces the original register
# allocation and instruction selection that modern GCC will not. Functions
# only byte-match when compiled with it.
CPP         := $(PREFIX)cpp
CC1         := tools/agbcc/bin/agbcc
LD          := $(PREFIX)ld
OBJCOPY     := $(PREFIX)objcopy
PYTHON      ?= python3

ASFLAGS     := -mcpu=arm7tdmi -mthumb-interwork -I.
CPPFLAGS    := -nostdinc -undef -Iinclude -Itools/agbcc/include
CC1FLAGS    := -mthumb-interwork -O2 -fhex-asm
CFLAGS      := -mcpu=arm7tdmi -mthumb -mthumb-interwork -Os \
               -fno-builtin -fno-strict-aliasing -nostdinc -Iinclude \
               -fomit-frame-pointer -mlong-calls -Wall
LDFLAGS     := -T ld_script.ld --no-warn-rwx-segments

ASM_SRCS    := $(wildcard asm/*.s) $(wildcard asm/code/*.s) $(wildcard asm/data/*.s)
# src/nonmatching/ holds readable C that does not yet reproduce the original
# bytes; it is excluded so the default build stays byte-exact.
C_SRCS      := $(wildcard src/*.c)
ifdef NONMATCHING
C_SRCS      += $(wildcard src/nonmatching/*.c)
endif
TEXT_SRCS   := $(wildcard text/*.txt)
DATA_BINS   := $(wildcard data/*.bin)

ASM_OBJS    := $(patsubst %.s,$(BUILD)/%.o,$(ASM_SRCS))
C_OBJS      := $(patsubst %.c,$(BUILD)/%.o,$(C_SRCS))
OBJS        := $(ASM_OBJS) $(C_OBJS)

# Recompile matching C when a recovered structure or hardware definition changes.
-include $(C_OBJS:.o=.d)

.PHONY: all compare extract clean tidy stats test test-english ci
.SUFFIXES:

# Keep `all` first: it is the default goal.
all: $(TARGET) stats

include graphics.mk

# --- editable sources -----------------------------------------------------
# Scripts in text/ are re-encoded onto the extracted blobs before assembly.
# One rule regenerates all of build/data at once, guarded by a stamp.
$(BUILD)/.data.stamp: $(DATA_BINS) $(TEXT_SRCS) tools/build_text.py                       tools/text_codec.py
	@mkdir -p $(BUILD)/data
	$(PYTHON) tools/build_text.py
	@touch $@

# Sound sample headers and signed PCM bytes are compiled from editable WAV/JSON.
# List required inputs from the manifest, so a deleted WAV fails even on an incremental build.
SOUND_WAVS := $(shell $(PYTHON) -c 'import json; print(" ".join(e["wav"] for e in json.load(open("sound/samples/manifest.json"))))')
$(BUILD)/.sound.stamp: $(SOUND_WAVS) sound/samples/manifest.json tools/sound_assets.py
	$(PYTHON) tools/sound_assets.py build
	@touch $@

$(BUILD)/asm/code/%.o: asm/code/%.s $(BUILD)/.sound.stamp
	@mkdir -p $(dir $@)
	@echo "AS      $<"
	@$(AS) $(ASFLAGS) -o $@ $<

# --- assembly -------------------------------------------------------------
# Data fragments incbin from build/data and graphics/, so those are real
# prerequisites and -j stays correct.
# Uncompressed art is converted from PNG into raw tile data before assembly;
# RAW_GFX comes from graphics_rules.mk.
$(BUILD)/asm/data/%.o: asm/data/%.s $(BUILD)/.data.stamp \
                       $(BUILD)/.graphics.stamp $(RAW_GFX) $(BUILD)/graphics/fonts/font.nft \
                       $(BUILD)/.palettes.stamp $(BUILD)/.named-scripts.stamp \
                       $(BUILD)/.ncd.stamp $(BUILD)/.named-maps.stamp
	@mkdir -p $(dir $@)
	@echo "AS      $<"
	@$(AS) $(ASFLAGS) -o $@ $<

$(BUILD)/%.o: %.s
	@mkdir -p $(dir $@)
	@echo "AS      $<"
	@$(AS) $(ASFLAGS) -o $@ $<

# C is preprocessed with the modern toolchain, then compiled by agbcc, then
# assembled. agbcc is a cc1 only: it takes preprocessed input and emits
# assembly, so those three steps stay separate.
$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	@echo "CC1     $<"
	@$(CPP) $(CPPFLAGS) -MMD -MP -MF $(BUILD)/$*.d -MT $@ $< -o $(BUILD)/$*.i
	@$(CC1) $(CC1FLAGS) $(BUILD)/$*.i -o $(BUILD)/$*.s
	@$(AS) $(ASFLAGS) -o $@ $(BUILD)/$*.s

# --- link -----------------------------------------------------------------
# The object list is long, so pass it to the linker via a file and keep the
# terminal readable.
$(ELF): $(OBJS) ld_script.ld
	@mkdir -p $(dir $@)
	@printf '%s\n' $(OBJS) > $(BUILD)/objects.rsp
	@echo "LD      $@"
	@$(LD) $(LDFLAGS) -o $@ @$(BUILD)/objects.rsp -Map $(BUILD)/$(NAME).map

$(TARGET): $(ELF)
	@echo "OBJCOPY $@"
	@$(OBJCOPY) -O binary --pad-to 0x09000000 $< $@

stats: $(TARGET)
	@$(PYTHON) tools/rom_stats.py $(ELF) --rom $(TARGET) --objdump $(PREFIX)objdump

test:
	@$(PYTHON) -m unittest discover -s tests

test-english:
	@$(PYTHON) -m unittest discover -s tests -p 'test_english*.py'
	@$(PYTHON) -m unittest discover -s tests -p 'test_translations.py'

# Public CI deliberately has no baserom. Local compare remains the stronger,
# byte-for-byte verification when the legally obtained reference is present.
ci: all english test

# --- verification ---------------------------------------------------------
compare: $(TARGET)
	@$(PYTHON) tools/compare.py $(BASEROM) $(TARGET)

# --- modern-compiler check ----------------------------------------------
# agbcc is a 1998 compiler and says very little. Modern GCC will not replace
# it for building, because every C function here is pinned to an exact ROM
# offset and modern codegen is a different size, so it would not fit its hole.
# pokeemerald's "make modern" works only because that project builds the whole
# image from source, where a size change just shifts what follows.
#
# The diagnostics are still worth having, so this compiles the same sources
# with a modern toolchain purely to report on them. It produces no object the
# ROM uses and cannot affect `make compare`.
MODERN_CFLAGS := -mcpu=arm7tdmi -mthumb -mthumb-interwork -Os -fno-builtin \
                 -fno-strict-aliasing -nostdinc -Iinclude -Wall -Wextra \
                 -Wno-unused-parameter -fsyntax-only

.PHONY: snapshot
snapshot:
	@sh tools/snapshot.sh

.PHONY: check-modern
check-modern:
	@echo "checking $(words $(C_SRCS)) sources with $(CC)"
	@for f in $(C_SRCS) $(wildcard src/nonmatching/*.c); do \
		$(CC) $(MODERN_CFLAGS) $$f || exit 1; \
	done
	@echo "modern compiler reports no problems"

$(BUILD)/graphics/fonts/font.nft: graphics/fonts/font.png graphics/fonts/font.nft \
                               tools/font.py tools/gfx.py tools/nfp.py
	$(PYTHON) tools/font.py build --output $@

$(BUILD)/.palettes.stamp: $(wildcard graphics/palettes/*.pal) \
                         graphics/palettes/manifest.json tools/palettes.py tools/gfx.py
	$(PYTHON) tools/palettes.py build
	@touch $@

$(BUILD)/.named-scripts.stamp: $(wildcard scripts/nfp/*.bin) $(wildcard text/nfp/*.txt) \
                              maps/events $(wildcard maps/events/*.json) tools/script_events.py \
                              scripts/nfp/manifest.json tools/named_scripts.py \
                              tools/extract_scrp_text.py tools/text_codec.py tools/lz77.py
	$(PYTHON) tools/named_scripts.py build
	@touch $@

$(BUILD)/.named-maps.stamp: $(wildcard maps/nfp/*.bin) \
                           maps/editable $(wildcard maps/editable/*.json) tools/map_editor/model.py \
                           $(wildcard graphics/tilemaps/nfp/*.bin) \
                           maps/nfp/manifest.json assets.json $(MAPPED_LAYOUTS) tools/mapped_images.py tools/affine_images.py tools/regular_images.py tools/named_maps.py tools/nfp.py
	$(PYTHON) tools/named_maps.py build
	@touch $@

# Each container compiles from its own readable tables, palettes, cell images,
# and assembled-frame edits. No original NCD or whole-pool PNG is an input.
NCD_OUTPUTS := $(addprefix $(BUILD)/graphics/ncd/,CHR.ncd EFFECT.ncd SYSTEM.ncd)
NCD_TOOLS := tools/sprite_sources.py tools/ncd.py tools/scenes.py tools/icons.py tools/gfx.py
SYSTEM_VIEWS := $(wildcard graphics/icons/*.png) graphics/icons/manifest.json \
                $(wildcard graphics/portraits/*.png) graphics/portraits/manifest.json

define NCD_RULE
$(BUILD)/graphics/ncd/$(1).ncd: $(wildcard $(2)/source/*.json) \
                              $(wildcard $(2)/cells/*/*.png) $(wildcard $(2)/palettes/*.pal) \
                              $(wildcard $(2)/frames/*.png) $(2)/manifest.json \
                              $(3) $(NCD_TOOLS) graphics/sprite_containers.json
	$(PYTHON) tools/sprite_sources.py build --stem $(1)
endef
$(eval $(call NCD_RULE,CHR,graphics/battle/characters,))
$(eval $(call NCD_RULE,EFFECT,graphics/battle/effects,))
$(eval $(call NCD_RULE,SYSTEM,graphics/ui,$(SYSTEM_VIEWS)))

$(BUILD)/.ncd.stamp: $(NCD_OUTPUTS)
	@touch $@

.PHONY: sprite-previews
sprite-previews:
	$(PYTHON) tools/scenes.py extract

.PHONY: graphics-audit font-preview
graphics-audit:
	$(PYTHON) tools/audit_graphics.py

font-preview:
	$(PYTHON) tools/font.py preview --output build/font-preview.png

# --- regeneration from the base ROM --------------------------------------
extract:
	$(PYTHON) tools/analyze.py $(BASEROM)
	$(PYTHON) tools/extract_assets.py $(BASEROM)
	$(PYTHON) tools/find_free.py $(BASEROM)
	$(PYTHON) tools/classify_raw.py $(BASEROM) .analysis/free.json
	$(PYTHON) tools/extract_raw.py $(BASEROM) .analysis/runs.json
	$(PYTHON) tools/gen_rules.py
	$(PYTHON) tools/extract_text.py $(BASEROM)
	$(PYTHON) tools/font.py extract
	$(PYTHON) tools/palettes.py extract
	$(PYTHON) tools/named_scripts.py extract
	$(PYTHON) tools/ncd.py extract
	$(PYTHON) tools/icons.py
	$(PYTHON) tools/portraits.py
	$(PYTHON) tools/split.py $(BASEROM)

clean:
	rm -rf $(BUILD) $(TARGET) mar_english.gba

# note: .analysis/ holds data derived from the base ROM and is deliberately
# not removed by clean; use "make extract" to regenerate it.

tidy:
	rm -rf $(BUILD)/asm $(BUILD)/src $(ELF) $(TARGET)

# Documentation is published separately from the decomp source branch.
.PHONY: galleries site docs-fetch
.PHONY: map-editor
map-editor:
	$(PYTHON) tools/map_editor/server.py

galleries:
	$(PYTHON) tools/galleries.py

site: galleries
	$(PYTHON) tools/publish_docs.py

build/published-docs/.git:
	git clone --depth 1 --branch gh-pages https://github.com/name1esshero/Marchen-Awakens-Romance.git build/published-docs

docs-fetch: build/published-docs/.git
	git -C build/published-docs pull --ff-only
	$(PYTHON) tools/publish_docs.py --restore build/published-docs

# Optional English dialogue build. Reuse the verified Japanese objects except
# the constructor, replaced by a fixed-size bridge to readable localization C.
# The matching mar.gba is never patched or used as an English build input.
ENGLISH_DIR := build/english
ENGLISH_OBJS := $(ENGLISH_DIR)/dialogue_bridge.o $(ENGLISH_DIR)/dialogue_runtime.o \
                $(ENGLISH_DIR)/dialogue_original.o $(ENGLISH_DIR)/mappings.o $(ENGLISH_DIR)/system_graphics.o $(ENGLISH_DIR)/effect_graphics.o $(ENGLISH_DIR)/background_graphics.o
.PHONY: english english-stats
english: mar_english.gba english-stats

$(ENGLISH_DIR)/mappings.c: $(wildcard text/nfp/*.txt) text/arm_definitions.txt text/item_definitions.txt text/runtime_strings.txt text/translation/english_font.json \
                         graphics/fonts/font.png graphics/fonts/font.json tools/build_english.py tools/english_layout.py
	$(PYTHON) tools/build_english.py

$(ENGLISH_DIR)/dialogue_bridge.o: src/english/dialogue_bridge.s
	@mkdir -p $(ENGLISH_DIR)
	$(AS) $(ASFLAGS) -o $@ $<

$(ENGLISH_DIR)/dialogue_original.o: src/dialogue_start.c include/dialogue.h
	@mkdir -p $(ENGLISH_DIR)
	$(CPP) $(CPPFLAGS) -DENGLISH=1 $< -o $(ENGLISH_DIR)/dialogue_original.i
	$(CC1) $(CC1FLAGS) $(ENGLISH_DIR)/dialogue_original.i -o $(ENGLISH_DIR)/dialogue_original.s
	$(AS) $(ASFLAGS) -o $@ $(ENGLISH_DIR)/dialogue_original.s

$(ENGLISH_DIR)/dialogue_runtime.o: src/english/dialogue_runtime.c include/dialogue.h include/english.h include/input.h
	@mkdir -p $(ENGLISH_DIR)
	$(CPP) $(CPPFLAGS) -DENGLISH=1 $< -o $(ENGLISH_DIR)/dialogue_runtime.i
	$(CC1) $(CC1FLAGS) $(ENGLISH_DIR)/dialogue_runtime.i -o $(ENGLISH_DIR)/dialogue_runtime.s
	$(AS) $(ASFLAGS) -o $@ $(ENGLISH_DIR)/dialogue_runtime.s

$(ENGLISH_DIR)/mappings.o: $(ENGLISH_DIR)/mappings.c include/english.h
	$(CPP) $(CPPFLAGS) -DENGLISH=1 $< -o $(ENGLISH_DIR)/mappings.i
	$(CC1) $(CC1FLAGS) $(ENGLISH_DIR)/mappings.i -o $(ENGLISH_DIR)/mappings.s
	$(AS) $(ASFLAGS) -o $@ $(ENGLISH_DIR)/mappings.s

# English artwork is compiled into a separate archive object. The matching
# Japanese object and source PNGs are never overwritten. Directory prerequisites
# also detect removing the last override (which must restore Japanese pixels).
$(ENGLISH_DIR)/graphics/SYSTEM.ncd: $(BUILD)/graphics/ncd/SYSTEM.ncd $(NCD_TOOLS) \
        tools/english_credits.py graphics/ui/source/english/credits_layouts.json \
        $(wildcard graphics/ui/frames/*_en.png graphics/ui/cells/*/*_en.png) \
        graphics/ui/frames $(wildcard graphics/ui/cells/*)
	$(PYTHON) tools/sprite_sources.py build --stem SYSTEM --english

$(ENGLISH_DIR)/system_graphics.o: $(ENGLISH_DIR)/graphics/SYSTEM.ncd
	@printf '%s\n' '.section .rom.00DD69E0, "a"' '.incbin "$(ENGLISH_DIR)/graphics/SYSTEM.ncd"' > $(ENGLISH_DIR)/system_graphics.s
	$(AS) $(ASFLAGS) -o $@ $(ENGLISH_DIR)/system_graphics.s

$(ENGLISH_DIR)/graphics/EFFECT.ncd: $(BUILD)/graphics/ncd/EFFECT.ncd $(NCD_TOOLS) \
        $(wildcard graphics/battle/effects/frames/*_en.png graphics/battle/effects/cells/*/*_en.png) \
        graphics/battle/effects/frames $(wildcard graphics/battle/effects/cells/*)
	$(PYTHON) tools/sprite_sources.py build --stem EFFECT --english

$(ENGLISH_DIR)/effect_graphics.o: $(ENGLISH_DIR)/graphics/EFFECT.ncd
	@printf '%s\n' '.section .rom.005AEAF0, "a"' '.incbin "$(ENGLISH_DIR)/graphics/EFFECT.ncd"' > $(ENGLISH_DIR)/effect_graphics.s
	$(AS) $(ASFLAGS) -o $@ $(ENGLISH_DIR)/effect_graphics.s

# Startup/title KCG backgrounds have their own English-only mapped-image path.
$(ENGLISH_DIR)/background_graphics.s: tools/english_backgrounds.py tools/build_assets.py tools/mapped_images.py tools/gfx.py tools/lz77.py assets.json maps/nfp/manifest.json $(wildcard maps/nfp/T_C01.KMP.bin maps/nfp/T_TTL06.KMP.bin maps/nfp/SM_BG12.KMP.bin) $(MAPPED_PNGS) $(MAPPED_LAYOUTS) $(MAPPED_UNUSED) $(wildcard graphics/backgrounds/*_en.png) graphics/backgrounds
	$(PYTHON) tools/english_backgrounds.py

$(ENGLISH_DIR)/background_graphics.o: $(ENGLISH_DIR)/background_graphics.s
	$(AS) $(ASFLAGS) -o $@ $<

$(ENGLISH_DIR)/mar_english.elf: $(OBJS) $(ENGLISH_OBJS) ld_script.ld ld_english.ld
	@printf '%s\n' $(filter-out $(BUILD)/src/dialogue_start.o $(BUILD)/asm/data/data_DD69E0.o $(BUILD)/asm/data/data_5AEAF0.o $(BUILD)/asm/data/data_F19AA0.o $(BUILD)/asm/data/data_F1A320.o $(BUILD)/asm/data/data_F1EF50.o $(BUILD)/asm/data/data_F21420.o $(BUILD)/asm/data/data_DA8990.o $(BUILD)/asm/data/data_DA9BF0.o,$(OBJS)) $(ENGLISH_OBJS) > $(ENGLISH_DIR)/objects.rsp
	$(LD) -T ld_english.ld --no-warn-rwx-segments -o $@ @$(ENGLISH_DIR)/objects.rsp -Map $(ENGLISH_DIR)/mar_english.map

mar_english.gba: $(ENGLISH_DIR)/mar_english.elf
	$(OBJCOPY) -O binary --gap-fill 0xFF --pad-to 0x0A000000 $< $@

english-stats: mar_english.gba
	@$(PYTHON) tools/rom_stats.py $(ENGLISH_DIR)/mar_english.elf --rom mar_english.gba --objdump $(PREFIX)objdump
