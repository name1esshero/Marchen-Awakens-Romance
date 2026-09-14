# pret Decompilation Standards & Best Practices Guide

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

---

## 9. AI Prompting Instructions
When feeding this guide to an AI agent, use the following directive:

> "This is the `pret` standard. Apply it rigorously to [FILE_NAME]. Extract all magic numbers into `#define` or `enum` in the relevant header files. Replace all raw hex addresses for functions and strings with their named equivalents from `decompiled.json` or the symbol map. Use designated initializers for all struct arrays. Run `make compare` after every change. If a change breaks the byte-match, revert it immediately and document the failure in `docs/decompilation-notes.md`."