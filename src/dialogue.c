/* 08011C1C: four hex digits used by dialogue C/T formatting commands.
 * Malformed digits retain their signed byte value, as in the original.
 * Cast before shifting to preserve ARM bit operations without signed UB. */
#include "dialogue.h"
#include "font.h"
__attribute__((section(".rom.00011C1C")))
const u8 *DialogueReadHex4(const u8 *text, u32 *result)
{
    u32 value = 0;
    s32 shift = 12;
    s32 i;
    for (i = 3; i >= 0; i--)
    {
        s32 digit = (s8)*text++;
        if ((u32)(digit - '0') <= 9)
            digit -= '0';
        else if ((u32)(digit - 'A') <= 5)
            digit -= 'A' - 10;
        else if ((u32)(digit - 'a') <= 5)
            digit -= 'a' - 10;
        value |= (u32)digit << shift;
        shift -= 4;
    }
    *result = value;
    return text;
}


__attribute__((section(".rom.00011C1C"))) const u8 DialogueReadHex4Tail[2]={0,0};

/* Consume Cxxxx/Txxxx controls and skip ASCII. The drawing task advances
 * double-byte glyphs; this reader returns them without moving the cursor.
 * Preserve the original signed cursor behavior for the matching build. */
__attribute__((section(".rom.00011AF4")))
u16 DialogueNextGlyph(struct DialogueState *state)
{
    u32 argument = 0;
    u32 done = 0;
    u16 character;
    const u8 *text = state->rows[state->row] + state->byteOffset;
    do
    {
        if (!IsEngineDoubleByte(text))
        {
            character = ReadEngineCharacter(text);
            switch (character)
            {
            case 'T': case 't':
                text = DialogueReadHex4(text + 1, &argument);
                state->byteOffset += 5;
                state->countdown = argument;
                state->delay = argument;
                break;
            case 'C': case 'c':
                text = DialogueReadHex4(text + 1, &argument);
                state->byteOffset += 5;
                state->ink = ((s32)argument & 0xFF00) >> 8;
                state->shadow = argument;
                break;
            case 0:
                state->drawnBytes = 0;
                state->byteOffset = 0;
                state->row++;
                if (state->row > 2)
                    goto finished;
                text = state->rows[state->row] + state->byteOffset;
                break;
            default:
                text++;
                state->byteOffset++;
                break;
            }
        }
        else
        {
            character = ReadEngineCharacter(text);
            done = 1;
        }
    } while (!done);
finished:
    return character;
}
