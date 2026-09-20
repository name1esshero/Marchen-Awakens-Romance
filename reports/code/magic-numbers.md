# C numeric-literal audit

This report ranks numeric literals that may benefit from named constants,
enums, flags, or typed fields. It intentionally includes legitimate
arithmetic constants; each result still needs call-site evidence before
renaming.

Total review candidates: **2909**

Generated and table-heavy sources are excluded by default so the
ranking reflects executable code. Pass `--include-data` to audit
their initializer values too.

Excluded data sources: **39**

## Categories

| Category | Occurrences |
| --- | ---: |
| `power_of_two_or_flag` | 1300 |
| `small_value_or_id` | 1059 |
| `mask_or_size` | 442 |
| `io_register` | 48 |
| `numeric_constant` | 30 |
| `iwram_address` | 19 |
| `video_memory_address` | 11 |

## Highest-density files

| File | Occurrences |
| --- | ---: |
| `src/sound_m4a.c` | 261 |
| `src/task_constructors.c` | 154 |
| `src/battle_family_tasks.c` | 132 |
| `src/mapping.c` | 131 |
| `src/save.c` | 128 |
| `src/runtime_buffers.c` | 118 |
| `src/battle_task_adapters.c` | 102 |
| `src/battle_task_create.c` | 98 |
| `src/runtime_objects.c` | 88 |
| `src/font.c` | 79 |
| `src/script_effect_native.c` | 78 |
| `src/task_adapters.c` | 71 |
| `src/sprite_engine_state.c` | 60 |
| `src/runtime_accessors.c` | 59 |
| `src/script_sprite.c` | 54 |
| `src/script_bytecode.c` | 52 |
| `src/battle_object_adapters.c` | 51 |
| `src/battle_mode_adapters.c` | 50 |
| `src/sprite_transform.c` | 50 |
| `src/sound_cgb_update.c` | 48 |
| `src/runtime_misc.c` | 43 |
| `src/english/dialogue_runtime.c` | 41 |
| `src/game_state.c` | 39 |
| `src/ncd_sprite.c` | 39 |
| `src/scene_native.c` | 38 |
| `src/script_resources.c` | 36 |
| `src/script_tasks.c` | 31 |
| `src/sprite_math.c` | 31 |
| `src/sound_tasks.c` | 30 |
| `src/map_field.c` | 26 |
| `src/map_native.c` | 26 |
| `src/runtime_core.c` | 23 |
| `src/script_native.c` | 23 |
| `src/sprite_affine_matrix.c` | 22 |
| `src/sprite_tile_allocator.c` | 21 |
| `src/object.c` | 20 |
| `include/sound.h` | 20 |
| `src/dialogue_commands.c` | 19 |
| `src/heap.c` | 19 |
| `src/resource_native.c` | 19 |

## Repeated literals

| Literal | Category | Occurrences | Example |
| --- | --- | ---: | --- |
| `2` | `power_of_two_or_flag` | 493 | `src/battle_family_tasks.c:150` |
| `4` | `power_of_two_or_flag` | 277 | `src/battle_family_tasks.c:71` |
| `3` | `small_value_or_id` | 194 | `src/battle_mode_adapters.c:56` |
| `8` | `power_of_two_or_flag` | 139 | `src/battle_family_tasks.c:71` |
| `16` | `power_of_two_or_flag` | 112 | `src/battle_family_tasks.c:39` |
| `32` | `power_of_two_or_flag` | 107 | `src/archive.c:104` |
| `12` | `small_value_or_id` | 87 | `src/battle_task_create.c:323` |
| `5` | `small_value_or_id` | 67 | `src/battle_mode_adapters.c:62` |
| `14` | `small_value_or_id` | 55 | `src/mapping.c:698` |
| `6` | `small_value_or_id` | 53 | `src/battle_mode_adapters.c:95` |
| `64` | `power_of_two_or_flag` | 37 | `src/archive.c:102` |
| `24` | `small_value_or_id` | 30 | `src/dialogue_commands.c:71` |
| `20` | `small_value_or_id` | 25 | `src/battle_mode_adapters.c:122` |
| `15` | `small_value_or_id` | 24 | `src/dialogue_start.c:26` |
| `7` | `small_value_or_id` | 23 | `src/bitset.c:14` |
| `72` | `small_value_or_id` | 19 | `src/battle_family_tasks.c:68` |
| `0x80` | `power_of_two_or_flag` | 19 | `src/english/dialogue_runtime.c:41` |
| `36` | `small_value_or_id` | 18 | `src/script_resources.c:209` |
| `10` | `small_value_or_id` | 16 | `src/battle_mode_adapters.c:121` |
| `68` | `small_value_or_id` | 15 | `src/battle_family_tasks.c:68` |
| `1672` | `mask_or_size` | 14 | `src/runtime_buffers.c:134` |
| `0xFFFF` | `mask_or_size` | 14 | `src/font.c:172` |
| `31` | `small_value_or_id` | 13 | `src/script_bytecode.c:340` |
| `128` | `power_of_two_or_flag` | 13 | `src/battle_family_tasks.c:79` |
| `0x20` | `power_of_two_or_flag` | 12 | `src/input.c:28` |
| `256` | `power_of_two_or_flag` | 11 | `src/ncd_sprite.c:85` |
| `9` | `small_value_or_id` | 10 | `src/dialogue.c:15` |
| `0xC0` | `small_value_or_id` | 10 | `src/input.c:29` |
| `0x400` | `power_of_two_or_flag` | 10 | `src/kmp_loader.c:78` |
| `18` | `small_value_or_id` | 9 | `src/map_native.c:78` |
| `0xFF` | `small_value_or_id` | 9 | `src/font.c:163` |
| `0x7FFF` | `mask_or_size` | 9 | `src/map_native.c:48` |
| `0x03007FF0` | `iwram_address` | 9 | `src/sound_cgb_update.c:106` |
| `0x1C` | `small_value_or_id` | 8 | `src/runtime_objects.c:118` |
| `40` | `small_value_or_id` | 8 | `src/game_state_records.c:176` |
| `0x40` | `power_of_two_or_flag` | 8 | `src/input.c:30` |
| `132` | `small_value_or_id` | 8 | `src/battle_family_tasks.c:74` |
| `0x2000` | `power_of_two_or_flag` | 8 | `src/save.c:412` |
| `28` | `small_value_or_id` | 7 | `src/encounter_task.c:43` |
| `30` | `small_value_or_id` | 7 | `src/battle_task_create.c:48` |
| `44` | `small_value_or_id` | 7 | `src/battle_family_tasks.c:71` |
| `52` | `small_value_or_id` | 7 | `src/battle_family_tasks.c:71` |
| `104` | `small_value_or_id` | 7 | `src/battle_family_tasks.c:74` |
| `224` | `small_value_or_id` | 7 | `src/battle_task_create.c:44` |
| `0xFFF` | `mask_or_size` | 7 | `src/sprite_affine_matrix.c:58` |
| `0x1000` | `power_of_two_or_flag` | 7 | `src/save.c:420` |
| `0x10` | `power_of_two_or_flag` | 6 | `src/mapping.c:463` |
| `0x18` | `small_value_or_id` | 6 | `src/runtime_objects.c:117` |
| `34` | `small_value_or_id` | 6 | `src/resource_native.c:198` |
| `0x24` | `small_value_or_id` | 6 | `src/runtime_objects.c:129` |
| `100` | `small_value_or_id` | 6 | `src/battle_family_tasks.c:89` |
| `136` | `small_value_or_id` | 6 | `src/battle_family_tasks.c:77` |
| `144` | `small_value_or_id` | 6 | `src/battle_family_tasks.c:77` |
| `168` | `small_value_or_id` | 6 | `src/battle_family_tasks.c:79` |
| `192` | `small_value_or_id` | 6 | `src/battle_task_create.c:190` |
| `512` | `power_of_two_or_flag` | 6 | `src/encounter_task.c:46` |
| `0x220` | `mask_or_size` | 6 | `src/runtime_misc.c:38` |
| `0x7fff` | `mask_or_size` | 6 | `src/mapping.c:963` |
| `19` | `small_value_or_id` | 5 | `src/kmp.c:29` |
| `26` | `small_value_or_id` | 5 | `src/runtime_accessors.c:38` |
| `33` | `small_value_or_id` | 5 | `src/script_resources.c:53` |
| `120` | `small_value_or_id` | 5 | `src/battle_family_tasks.c:68` |
| `156` | `small_value_or_id` | 5 | `src/battle_family_tasks.c:81` |
| `160` | `small_value_or_id` | 5 | `src/battle_family_tasks.c:100` |
| `255` | `small_value_or_id` | 5 | `src/mapping.c:1049` |
| `0x214` | `mask_or_size` | 5 | `src/dialogue_start.c:25` |
| `0x7A0` | `mask_or_size` | 5 | `src/runtime_buffers.c:135` |
| `0x31D0` | `mask_or_size` | 5 | `src/mapping.c:1039` |
| `0x04000208` | `io_register` | 5 | `src/sound.c:30` |
| `0x08` | `power_of_two_or_flag` | 4 | `src/sound_cgb_update.c:376` |
| `13` | `small_value_or_id` | 4 | `src/map_placements.c:18` |
| `17` | `small_value_or_id` | 4 | `src/mapping.c:21` |
| `22` | `small_value_or_id` | 4 | `src/mapping.c:817` |
| `35` | `small_value_or_id` | 4 | `src/script_resources.c:183` |
| `0x30` | `small_value_or_id` | 4 | `src/input.c:27` |
| `84` | `small_value_or_id` | 4 | `src/task_adapters.c:252` |
| `96` | `small_value_or_id` | 4 | `src/battle_family_tasks.c:74` |
| `108` | `small_value_or_id` | 4 | `src/battle_family_tasks.c:83` |
| `116` | `small_value_or_id` | 4 | `src/battle_family_tasks.c:83` |
| `127` | `small_value_or_id` | 4 | `src/sound_m4a.c:863` |
| `172` | `small_value_or_id` | 4 | `src/battle_family_tasks.c:77` |
| `180` | `small_value_or_id` | 4 | `src/battle_family_tasks.c:98` |
| `0x1FFF` | `mask_or_size` | 4 | `src/sprite_tile_allocator.c:82` |
| `0x38B8` | `mask_or_size` | 4 | `src/resource_native.c:114` |
| `0xC000` | `mask_or_size` | 4 | `src/sprite_tile_allocator.c:83` |
| `0x04000089` | `io_register` | 4 | `src/sound_m4a.c:351` |
| `0x040000C4` | `io_register` | 4 | `src/sound_m4a.c:346` |
| `11` | `small_value_or_id` | 3 | `src/script_sprite.c:118` |
| `0x0C` | `small_value_or_id` | 3 | `src/runtime_misc.c:38` |
| `0x14` | `small_value_or_id` | 3 | `src/mapping.c:420` |
| `0x2C` | `small_value_or_id` | 3 | `src/runtime_accessors.c:61` |
| `0x3C` | `small_value_or_id` | 3 | `src/runtime_objects.c:255` |
| `0x4C` | `small_value_or_id` | 3 | `src/item.c:24` |
| `76` | `small_value_or_id` | 3 | `src/runtime_buffers.c:164` |
| `112` | `small_value_or_id` | 3 | `src/battle_family_tasks.c:68` |
| `178` | `small_value_or_id` | 3 | `src/runtime_accessors.c:357` |
| `194` | `small_value_or_id` | 3 | `src/map_field.c:93` |
| `196` | `small_value_or_id` | 3 | `src/battle_family_tasks.c:100` |
| `204` | `small_value_or_id` | 3 | `src/battle_family_tasks.c:79` |
| `208` | `small_value_or_id` | 3 | `src/battle_family_tasks.c:98` |
