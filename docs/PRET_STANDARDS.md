# pret Decompilation Standards & Best Practices Guide

Related load-bearing docs: [SCORE_METRIC_CRITERIA.md](SCORE_METRIC_CRITERIA.md)
explains how compliant work is judged, [AGBCC_CODEGEN.md](AGBCC_CODEGEN.md)
and [COMPILER_HINT_CLEANUP.md](COMPILER_HINT_CLEANUP.md) explain how to reach
matches without compiler steering, [PRET_AUDIT.md](PRET_AUDIT.md) tracks
mechanical findings, [decompilation-notes.md](decompilation-notes.md) records
per-function evidence, and [AGENT_ENVIRONMENT.md](AGENT_ENVIRONMENT.md)
describes how the documentation funnel fits together.

## Core Philosophy: The Golden Rule
In `pret` projects, there is one absolute, unbreakable rule that supersedes all others: **The compiled ROM must match the original ROM byte-for-byte (SHA1 hash).** 
Human readability is the second priority. If you cannot match a function in natural C without using compiler hacks, you must leave it in assembly using `INCLUDE_ASM`. Fake matches, forced registers (`asm("rX")`), naked functions, and compiler flag modifications are strictly forbidden.

---

## 1. Naming Conventions
Naming is critical. If a function or variable is named poorly, the entire codebase suffers.
*   **Global Variables:** Prefixed with `g` in camelCase. (e.g., `gPlayerParty`, `gMain`).
*   **Static/File-Local Variables:** Prefixed with `s` in camelCase. (e.g., `sSpriteTileAllocator`, `sWorkBuffer`).
*   **Functions:** PascalCase (UpperCamelCase). Should be verb-noun. (e.g., `CalculateDamage`, `GetMonData`, `AllocateSpriteTile`).
*   **Macros & Enums:** ALL_CAPS with underscores. (e.g., `MAX_PARTY_SIZE`, `BATTLE_ACTION_NONE`).
*   **Structs & Unions:** PascalCase, often prefixed with the module name. (e.g., `struct SpriteTileAllocator`, `struct ScriptNativeCommand`).
*   **Constants:** `const` variables or `#define` macros. Do not use magic numbers.
*   **Placeholders:** If a function hasn't been identified yet, name it `sub_080XXXXX` (matching the ROM address). Do not leave raw hex pointers in tables.

## 2. Types and Variables
*   **Use exact-width types:** Always use the types defined in `gba/types.h`. Never use `int`, `char`, `short`, or `long`.
    *   `u8`, `u16`, `u32` (unsigned)
    *   `s8`, `s16`, `s32` (signed)
    *   `bool8` (boolean, often just `u8`)
*   **Pointers:** Use `*` attached to the variable name (e.g., `u8 *ptr`, not `u8* ptr`).
*   **Pointer arithmetic:** Pointer arithmetic is ordinary C. Use a pointer to
    the real element type when the ROM advances through an array or structure;
    `ptr + n` then advances by `n * sizeof(*ptr)`. Use `u8 *` when the recovered
    operation is an arbitrary byte offset or the layout is not understood well
    enough to assign a stronger type. Prefer structure members once their
    offsets and meanings are proved. These forms do not require an audit
    exception.
*   **Pointer/integer authenticity:** Do not cast a pointer to `u32`, or pass it
    through a pointer/integer union, merely to
    influence register allocation or instruction scheduling. That is compiler
    steering even if it byte-matches. A cast through an integer is acceptable
    when the recovered operation actually treats the address as a raw 32-bit
    value: masking, shifting, tagging, alignment tests, signed address-sentinel
    comparisons, serialization into a word-oriented ABI or file structure, or
    explicitly setting the THUMB bit. Document that evidence beside the code.
    Integer casts may be temporary matching scaffolds while a structure remains
    unknown, but must be revisited when that layout is decoded. The PRET
    audit reports pointer/integer conversions as review warnings because syntax
    alone cannot prove intent. It does not report the required
    `(void *)((u32)Function + 1)` THUMB encoding, address alignment and low-bit
    tests, assembly, BIOS wrappers, or the documented `gIwramBase +
    (u32)gMapGenerationRootOffset` linker-symbol recovery. Resolve every other
    warning by rewriting it as natural pointer C, adding a nearby
    `PRET_PTR_INT_OK:` explanation for a verified deliberate recovery, or
    restoring the exact implementation to assembly. A documented recovery is
    reported and counted as an exception rather than silently ignored. The
    note format is `PRET_PTR_INT_OK: operation=...; evidence=...; typed=...`.
    `operation` says what raw-word operation the cast models; `evidence` names
    the caller, ABI, or ROM instruction that proves it; `typed` explains why
    ordinary typed or `u8 *` arithmetic does not express that operation. An
    incomplete note remains a warning. The generated report must show errors,
    warnings, and exceptions together on its summary line and enumerate every
    exception below it. Complete notes on complex integer-to-pointer expressions
    are enumerated even when the conservative cast scanner cannot classify the
    expression itself; this keeps accepted recoveries visible to reviewers.
*   **Audit scope:** A clean pointer/integer report proves only that this one
    class has been reviewed. Unions, extra locals, declaration order, narrower
    or wider integer types, and control-flow spelling can all steer code
    generation without a pointer-to-integer cast. Review those manually; zero
    warnings must never be described as proof that all compiler steering is
    absent.
*   **Volatile:** Use `volatile` only when strictly necessary (e.g., memory-mapped hardware registers or variables modified by interrupts).
*   **Const Correctness:** Use `const` for any data that should not be modified. Data tables and string literals must be `const`.

## 3. Functions and Headers
*   **Prototypes:** Functions should be prototyped in a header file (e.g., `sprite.h`) and defined in the corresponding C file (e.g., `sprite.c`).
*   **Static Functions:** If a function is only used in one file, declare it `static`.
*   **Arguments:** Pass structs by pointer if they are large. Pass primitives by value.
*   **Return Values:** Use `s32` or `u32` for general returns. Use `void` if nothing is returned.
*   **Include Guards:** Every header file must have an include guard or `#pragma once`.
*   **Header Includes:** Include what you use. Avoid circular dependencies. `global.h` is usually the first include.

## 4. Constants, Macros, and Magic Numbers
This is where `pret` code shines. Raw numbers are unacceptable in logic.
*   **Bitmasks and Flags:** Use `#define` or `enum` with bit shifts.
    *   *Bad:* `if (flags & 0x2000)`
    *   *Good:* `if (flags & FLAG_IS_FINAL_BLOCK)`
*   **Array Sizes:** Use `ARRAY_COUNT(array)` macro instead of hardcoded numbers.
*   **Sentinel Values:** Define them explicitly.
    *   *Bad:* `if (index == 444)`
    *   *Good:* `if (index == ARM_INVALID)`
*   **Hex vs Decimal:** Use hexadecimal (`0x...`) for memory addresses, bitmasks, and hardware registers. Use decimal for counts, sizes, and normal math.

## 5. Data Tables and Structs
Raw hex dumps are the hallmark of amateur decompilations. `pret` standards demand structured, readable tables.
*   **Function Pointer Tables:** Never use raw hex. Use the actual function names.
    *   *Bad:* `(void *)0x08015269, (void *)0x0803DE59`
    *   *Good:* `BattleAction_Flee, BattleAction_UseMove`
*   **String Tables:** Use named string variables or inline string literals. Never use raw addresses.
    *   *Bad:* `(const char *)0x08086C30`
    *   *Good:* `sText_MswStr`
*   **Designated Initializers:** When defining arrays of structs, use designated initializers to prevent misalignment.
    ```c
    const struct ScriptNativeCommand gScriptNativeCommands[] = {
        [0] = { .name = sText_MswStr, .func = MswStr },
        [1] = { .name = sText_MswHit, .func = MswHit },
    };
    ```
*   **Static Data:** Mark all lookup tables as `static const` if they are not shared across files.
*   **The THUMB-bit possibility:** Before giving up on a function-pointer table entry as "not yet decompiled," check whether the stored address is odd. THUMB function pointers are conventionally stored with bit 0 set (real address + 1) so the CPU knows to stay in THUMB state when the caller `bx`s to it; a table of such pointers will have every entry one more than the function's real, even address. Subtract 1 before cross-referencing `decompiled.json` or the symbol map -- a table that looks 0% identified can turn out to be almost entirely already-named once this is accounted for (game_tables.c's gScriptNativeCommands went from 0/128 to 124/128 known this way).
    *   **Verify before applying it.** An odd value in a table is not automatically a THUMB pointer -- it could be a packed flag bit, a signed relative offset, or ordinary data that happens to be odd. Only treat it as one if, after subtracting 1, the result (a) falls inside a real code region (check the ROM range in the relevant `asm/code/*.s` header) and (b) a `sub_XXXXXXXX` label or a named function actually exists at that exact address. If neither holds, leave it as plain data.
    *   **Cast to `u32` before adding the bit, always.** Write `(void *)((u32)RealName + 1)`, never `(void *)RealName + 1`. The latter relies on GNU `void *` arithmetic (a GCC extension, not portable, and not how the original source would have expressed this) instead of plain integer arithmetic; the cast-first form is unambiguous and matches what the ROM's own codegen is doing. When writing the entry back, restore the bit explicitly and visibly -- never by hand-editing the literal.
    *   **Double indirection is a related trap.** Occasionally a table entry is a pointer to a pointer, not a pointer to the data itself. If the value at an address doesn't look like the expected type (not ASCII for a supposed string, no valid THUMB prologue for a supposed function), read the 4 bytes at that address as a little-endian word and check whether *that* looks right instead -- one more level of dereference before concluding the entry is unidentified.
    *   **Audit tool:** `tools/audit_thumb_ptrs.py` scans `src/*.c` for raw `0x08XXXXXX` table entries, checks `address - 1` against `decompiled.json`, and reports how many entries in each file could be renamed this way -- run it before manually inspecting a new table.

## 5a. agbcc Quirks That Cause "Near Misses"
Several agbcc-specific compiler behaviors produce functions that are logically correct but byte-mismatched. Recognizing which quirk is in play saves time chasing the wrong fix.

*   **The "Reverse" Register Allocation Order:** agbcc's Thumb compiler does not define `REG_ALLOC_ORDER`, so registers are allocated sequentially, but the community has observed that sometimes low registers (`r0`-`r3`) are effectively allocated in reverse for certain variables -- `r3` first, then `r2`, `r1`, finally `r0`. A "register allocation mismatch" where the logic is identical but the registers are swapped often traces back to this: declaring local variables in a different order (not adding register hints) can shift which variable lands in which register.
*   **The `ldr`/`lsls` Scheduling Mismatch:** agbcc has a fixed preference to schedule ALU operations (like shifts) before memory loads (like literal-pool references). The original compiler used for some GBA games instead interleaved them, loading data before shifting indices. This is a genuine instruction-scheduling limitation, not something reorderable from C -- if a function only mismatches by ldr/lsls order, the honest options are: use inline assembly to force the order, or accept the mismatch and leave the function in `src/nonmatching/` (never fake it with a forced register or naked function).
*   **The fakematch trap:** a fakematch is C code that happens to compile to byte-matching assembly by relying on fragile, coincidental compiler behavior (e.g. the compiler merging adjacent stores into one instruction) rather than expressing the actual logic robustly. These are technical debt even though `make compare` passes. When one is found, the fix is to rewrite the C so the match no longer depends on incidental optimizer behavior, not just to leave it because it currently passes.
*   **The "Shiftable" Decomp and xMAP files:** in more advanced decomp projects (e.g. pmd-sky), the whole codebase is "shiftable" -- every software pointer is a linker symbol rather than a hardcoded address, so code can be added or removed without invalidating C references. Compatibility with old ASM patches that assumed fixed addresses can be managed with xMAP files that calculate and apply resulting pointer shifts. The matching linker may still freeze the original section order and addresses; that is expected and is separate from whether C contains hardcoded references. A modern or expanded linker can relax placement later. GBA I/O registers and the fixed BIOS, EWRAM, IWRAM, palette, VRAM, OAM, ROM, and save-memory regions are physical platform addresses and are not shiftability violations when accessed through their named, correctly typed `#define` macros or linker regions. PRET projects commonly define a hardware base address, named register offsets, and typed volatile access macros; this use of `#define` is important and correct. A `#define` that aliases a real linker symbol or expresses a platform constant remains shiftable, while one that merely hides a fixed software-object address is transitional and must eventually become a real linker-defined object. `AT()` is matching-build placement metadata and is not itself a hardcoded reference in C logic. An absolute `.set` alias is likewise transitional naming, not full shiftability: replace it with a real relocatable definition when the underlying function or data is recovered. Run `make shiftability-audit` for the current inventory.
*   **The `-fprologue-bugfix` compiler flag:** a known agbcc patch that removes unnecessary push/pop instructions in leaf functions (functions that call nothing else). Some games need this flag to match certain functions with an unexpected stack frame. This project's compiler flags are fixed (per the Golden Rule, flag changes are forbidden as a matching shortcut) -- if a function only matches with a flag change, it stays in `src/nonmatching/` with that noted as the reason, it is not "fixed" by changing the build.
*   **The "pool" and alignment gotcha:** PC-relative `ldr` loads read constants from literal pools placed inside the code; agbcc may pad a pool to a 4-byte boundary even in Thumb code (which is otherwise 2-byte aligned), and that padding alone can cause a mismatch. Separately, VRAM/SRAM access requires `volatile` pointers, or the compiler may generate incompatible load/store instructions. Both are common, subtle, and easy to misdiagnose as logic bugs when the real cause is alignment or missing `volatile`.

## 6. Formatting and Style
*   **Indentation:** 4 spaces. No tabs.
*   **Braces:** Allman style (braces on their own new line) for functions, structs, and enums. K&R style is sometimes used for control flow, but Allman is the safest bet for matching `pret`.
    ```c
    void FunctionName(void)
    {
        if (condition)
        {
            DoSomething();
        }
    }
    ```
*   **Line Length:** Keep lines under 100 characters if possible.
*   **Spacing:** Space after keywords (`if`, `while`, `for`). No space after function names (`FunctionName()`).
*   **Pointers:** `u8 *ptr` (space before the asterisk, no space after).

## 7. Comments and Documentation
*   **Doxygen Comments:** Every function and struct should have a Doxygen-style comment block explaining its purpose, arguments, and return value.
    ```c
    /**
     * @brief Calculates the damage of a move.
     * @param attacker The attacking Pokémon.
     * @param defender The defending Pokémon.
     * @return The calculated damage.
     */
    ```
*   **Inline Comments:** Use `//` for single-line comments. Explain *why* the code does something, not *what* it does.
*   **Magic Addresses:** If a memory address is hardcoded (e.g., `0x0300611C`), add a comment explaining what hardware register or RAM offset it represents.

## 8. The "Non-Matching" Workflow
*   **Matching C:** Code that compiles to the exact original ASM goes in `src/`.
*   **Non-Matching C:** Code that compiles but does not match goes in `src/nonmatching/`. It is kept for reference and future refinement.
*   **Assembly:** Code that cannot be matched in C is left in `asm/` and included via `INCLUDE_ASM`.
*   **Documentation:** Every time a function is moved to `nonmatching`, document *why* it failed (e.g., "Compiler optimized register allocation incorrectly," "Operand order mismatch").

## 8a. Fast Iteration: Compile Against a Single .o, Not the Whole ROM
A full `make -j$(nproc) && make compare` is the required final confirmation for any change, but it is far too slow (~60-90s) to use for every micro-iteration while hunting a byte-exact register allocation or instruction order. Instead:
1.  Write the candidate function in a throwaway `.c` file and run the same `cpp` + `agbcc` + `as` invocation the Makefile uses on it directly (check `Makefile`/build logs for the exact flags), producing a single `.o` in about a second.
2.  `objdump -d` that `.o` and diff it instruction-by-instruction against the real ROM bytes at the target address (pulled from `baserom.gba`, not from the `.s` file's own text — the original disassembler sometimes hid real instructions behind raw `.2byte`/`.4byte` runs).
3.  Iterate the ordinary C shape (declaration order, expression grouping, and
    `s16` versus `s32`) against this single-`.o` loop until the disassembly
    matches exactly. Never use forced-register declarations or inline assembly.
4.  Only then drop the function into its real source file, remove the corresponding raw asm (see §8's rule: nothing decompiled to C should be left duplicated in `asm/`), and run the full `make && make compare` plus the test suite as the final, authoritative check.
This single-`.o` loop is roughly 60-90x faster per iteration than a full build and was the key unlock that made bulk small-function decompilation practical in one session.

---

## 9. AI Prompting Instructions
When feeding this guide to an AI agent, use the following directive:

> "This is the `pret` standard. Apply it rigorously to [FILE_NAME]. Extract all magic numbers into `#define` or `enum` in the relevant header files. Replace all raw hex addresses for functions and strings with their named equivalents from `decompiled.json` or the symbol map. Use designated initializers for all struct arrays. Run `make compare` after every change. If a change breaks the byte-match, revert it immediately and document the failure in `docs/decompilation-notes.md`."
