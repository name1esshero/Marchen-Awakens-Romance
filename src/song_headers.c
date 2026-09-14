#include "gba/types.h"

#include "rom_section.h"
#define SONG_HEADER_TYPE(count) \
    struct { \
        u8 trackCount, blockCount, priority, reverb; \
        const void *voices; \
        const u8 *parts[count]; \
    }

/* Variable-sized MusicPlayer2000 song headers.  Each comment lists every
 * game-visible song-table ID that selects the header. */
/* A zero-track entry consists only of the four-byte prefix.  The bytes that
 * follow at 0808B89C are the first PCM wave header, not a voices pointer. */
AT("0008B898") const u32 gSongHeader_000 = 0;
/* song IDs: 0, 5, 6, 17, 25, 29, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43,
 * 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61,
 * 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
 * 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97,
 * 98, 99, 100, 108, 114, 136, 137, 138, 139, 144, 145, 146, 147, 148, 149,
 * 153, 154, 155, 156, 157, 158, 159, 164, 165, 166, 167, 168, 169, 173,
 * 174, 175, 176, 177, 178, 179, 183, 184, 185, 186, 187, 188, 189, 193,
 * 194, 195, 196, 197, 198, 199 */

AT("001A2BD4") const SONG_HEADER_TYPE(7) gSongHeader_001 = {
    7, 0, 10, 0, (const void *)0x08089810,
    {(const u8 *)0x081A2564, (const u8 *)0x081A262B, (const u8 *)0x081A283B, (const u8 *)0x081A2914, (const u8 *)0x081A2980, (const u8 *)0x081A29E5, (const u8 *)0x081A2B43}
}; /* song IDs: 1 */

AT("001A3104") const SONG_HEADER_TYPE(7) gSongHeader_002 = {
    7, 0, 10, 0, (const void *)0x08089810,
    {(const u8 *)0x081A2BF8, (const u8 *)0x081A2CF5, (const u8 *)0x081A2D84, (const u8 *)0x081A2DFE, (const u8 *)0x081A2EC5, (const u8 *)0x081A2F23, (const u8 *)0x081A308E}
}; /* song IDs: 2 */

AT("001A33C0") const SONG_HEADER_TYPE(6) gSongHeader_003 = {
    6, 0, 10, 0, (const void *)0x08089810,
    {(const u8 *)0x081A3128, (const u8 *)0x081A3193, (const u8 *)0x081A31EE, (const u8 *)0x081A3237, (const u8 *)0x081A32D1, (const u8 *)0x081A3379}
}; /* song IDs: 3 */

AT("001A3978") const SONG_HEADER_TYPE(8) gSongHeader_004 = {
    8, 0, 10, 0, (const void *)0x08089810,
    {(const u8 *)0x081A33E0, (const u8 *)0x081A3499, (const u8 *)0x081A3514, (const u8 *)0x081A357D, (const u8 *)0x081A365C, (const u8 *)0x081A36B6, (const u8 *)0x081A387A, (const u8 *)0x081A38D3}
}; /* song IDs: 4 */

AT("001A3C24") const SONG_HEADER_TYPE(7) gSongHeader_007 = {
    7, 0, 10, 0, (const void *)0x08089810,
    {(const u8 *)0x081A39A0, (const u8 *)0x081A39F8, (const u8 *)0x081A3A5A, (const u8 *)0x081A3A92, (const u8 *)0x081A3B3E, (const u8 *)0x081A3BA4, (const u8 *)0x081A3BF3}
}; /* song IDs: 7 */

AT("001A41AC") const SONG_HEADER_TYPE(5) gSongHeader_008 = {
    5, 0, 10, 0, (const void *)0x08089810,
    {(const u8 *)0x081A3C48, (const u8 *)0x081A3D69, (const u8 *)0x081A3F31, (const u8 *)0x081A3FFF, (const u8 *)0x081A408D}
}; /* song IDs: 8 */

AT("001A4928") const SONG_HEADER_TYPE(7) gSongHeader_009 = {
    7, 0, 10, 0, (const void *)0x08089810,
    {(const u8 *)0x081A41C8, (const u8 *)0x081A43F3, (const u8 *)0x081A44DD, (const u8 *)0x081A4609, (const u8 *)0x081A4671, (const u8 *)0x081A46C6, (const u8 *)0x081A475C}
}; /* song IDs: 9 */

AT("001A4E40") const SONG_HEADER_TYPE(8) gSongHeader_010 = {
    8, 0, 10, 0, (const void *)0x08089810,
    {(const u8 *)0x081A494C, (const u8 *)0x081A4974, (const u8 *)0x081A49AC, (const u8 *)0x081A4ABD, (const u8 *)0x081A4B77, (const u8 *)0x081A4BEB, (const u8 *)0x081A4C27, (const u8 *)0x081A4DED}
}; /* song IDs: 10 */

AT("001A514C") const SONG_HEADER_TYPE(5) gSongHeader_011 = {
    5, 0, 10, 0, (const void *)0x08089810,
    {(const u8 *)0x081A4E68, (const u8 *)0x081A4F10, (const u8 *)0x081A5035, (const u8 *)0x081A50B8, (const u8 *)0x081A50FC}
}; /* song IDs: 11 */

AT("001A5690") const SONG_HEADER_TYPE(6) gSongHeader_012 = {
    6, 0, 10, 0, (const void *)0x08089810,
    {(const u8 *)0x081A5168, (const u8 *)0x081A530E, (const u8 *)0x081A5389, (const u8 *)0x081A544C, (const u8 *)0x081A54DE, (const u8 *)0x081A5555}
}; /* song IDs: 12 */

AT("001A5BB0") const SONG_HEADER_TYPE(5) gSongHeader_013 = {
    5, 0, 10, 0, (const void *)0x08089810,
    {(const u8 *)0x081A56B0, (const u8 *)0x081A5859, (const u8 *)0x081A5923, (const u8 *)0x081A5A3B, (const u8 *)0x081A5AD8}
}; /* song IDs: 13 */

AT("001A60F4") const SONG_HEADER_TYPE(7) gSongHeader_014 = {
    7, 0, 10, 0, (const void *)0x08089810,
    {(const u8 *)0x081A5BCC, (const u8 *)0x081A5C6E, (const u8 *)0x081A5D61, (const u8 *)0x081A5E16, (const u8 *)0x081A5EC1, (const u8 *)0x081A5FF0, (const u8 *)0x081A6037}
}; /* song IDs: 14 */

AT("001A6AB4") const SONG_HEADER_TYPE(9) gSongHeader_015 = {
    9, 0, 10, 0, (const void *)0x08089810,
    {(const u8 *)0x081A6118, (const u8 *)0x081A63DD, (const u8 *)0x081A651F, (const u8 *)0x081A6617, (const u8 *)0x081A66C2, (const u8 *)0x081A6723, (const u8 *)0x081A67A8, (const u8 *)0x081A67DD, (const u8 *)0x081A68A4}
}; /* song IDs: 15 */

AT("001A738C") const SONG_HEADER_TYPE(6) gSongHeader_016 = {
    6, 0, 10, 0, (const void *)0x0808A6D4,
    {(const u8 *)0x081A6AE0, (const u8 *)0x081A6BBB, (const u8 *)0x081A6CDD, (const u8 *)0x081A6FE0, (const u8 *)0x081A704B, (const u8 *)0x081A70D2}
}; /* song IDs: 16 */

AT("001A77F4") const SONG_HEADER_TYPE(3) gSongHeader_018 = {
    3, 0, 10, 0, (const void *)0x08089810,
    {(const u8 *)0x081A73AC, (const u8 *)0x081A74F7, (const u8 *)0x081A7667}
}; /* song IDs: 18 */

AT("001A7A8C") const SONG_HEADER_TYPE(6) gSongHeader_019 = {
    6, 0, 10, 0, (const void *)0x08089810,
    {(const u8 *)0x081A7808, (const u8 *)0x081A7893, (const u8 *)0x081A7914, (const u8 *)0x081A7989, (const u8 *)0x081A7A41, (const u8 *)0x081A7A64}
}; /* song IDs: 19 */

AT("001A8164") const SONG_HEADER_TYPE(9) gSongHeader_020 = {
    9, 0, 10, 0, (const void *)0x08089810,
    {(const u8 *)0x081A7AAC, (const u8 *)0x081A7B0A, (const u8 *)0x081A7C93, (const u8 *)0x081A7D7D, (const u8 *)0x081A7DE2, (const u8 *)0x081A7E5A, (const u8 *)0x081A7FD3, (const u8 *)0x081A8021, (const u8 *)0x081A809D}
}; /* song IDs: 20 */

AT("001A8838") const SONG_HEADER_TYPE(9) gSongHeader_021 = {
    9, 0, 10, 0, (const void *)0x08089810,
    {(const u8 *)0x081A8190, (const u8 *)0x081A81E9, (const u8 *)0x081A8328, (const u8 *)0x081A83D1, (const u8 *)0x081A841F, (const u8 *)0x081A8539, (const u8 *)0x081A8565, (const u8 *)0x081A8620, (const u8 *)0x081A8674}
}; /* song IDs: 21 */

AT("001A8C14") const SONG_HEADER_TYPE(5) gSongHeader_022 = {
    5, 0, 10, 0, (const void *)0x08089810,
    {(const u8 *)0x081A8864, (const u8 *)0x081A8A6F, (const u8 *)0x081A8B05, (const u8 *)0x081A8B5A, (const u8 *)0x081A8BEA}
}; /* song IDs: 22 */

AT("001A8E58") const SONG_HEADER_TYPE(5) gSongHeader_023 = {
    5, 0, 10, 0, (const void *)0x08089810,
    {(const u8 *)0x081A8C30, (const u8 *)0x081A8CFA, (const u8 *)0x081A8D46, (const u8 *)0x081A8DC1, (const u8 *)0x081A8DEC}
}; /* song IDs: 23 */

AT("001A9210") const SONG_HEADER_TYPE(4) gSongHeader_024 = {
    4, 0, 10, 0, (const void *)0x08089810,
    {(const u8 *)0x081A8E74, (const u8 *)0x081A8F65, (const u8 *)0x081A8F9C, (const u8 *)0x081A916D}
}; /* song IDs: 24 */

AT("001A9920") const SONG_HEADER_TYPE(8) gSongHeader_026 = {
    8, 0, 10, 0, (const void *)0x08089810,
    {(const u8 *)0x081A9228, (const u8 *)0x081A9432, (const u8 *)0x081A9503, (const u8 *)0x081A9550, (const u8 *)0x081A967E, (const u8 *)0x081A96C7, (const u8 *)0x081A9722, (const u8 *)0x081A98A4}
}; /* song IDs: 26 */

AT("001A9EB8") const SONG_HEADER_TYPE(10) gSongHeader_027 = {
    10, 0, 10, 0, (const void *)0x08089810,
    {(const u8 *)0x081A9948, (const u8 *)0x081A9A7C, (const u8 *)0x081A9B30, (const u8 *)0x081A9B80, (const u8 *)0x081A9BCC, (const u8 *)0x081A9C27, (const u8 *)0x081A9C76, (const u8 *)0x081A9D1F, (const u8 *)0x081A9DBC, (const u8 *)0x081A9E6C}
}; /* song IDs: 27 */

AT("001AA140") const SONG_HEADER_TYPE(6) gSongHeader_028 = {
    6, 0, 10, 0, (const void *)0x08089810,
    {(const u8 *)0x081A9EE8, (const u8 *)0x081A9F4B, (const u8 *)0x081A9F93, (const u8 *)0x081A9FD0, (const u8 *)0x081A9FF6, (const u8 *)0x081AA04B}
}; /* song IDs: 28 */

AT("001AA6C8") const SONG_HEADER_TYPE(6) gSongHeader_030 = {
    6, 0, 10, 0, (const void *)0x08089810,
    {(const u8 *)0x081AA160, (const u8 *)0x081AA221, (const u8 *)0x081AA2C8, (const u8 *)0x081AA42D, (const u8 *)0x081AA4C3, (const u8 *)0x081AA52C}
}; /* song IDs: 30 */

AT("001AB350") const SONG_HEADER_TYPE(8) gSongHeader_031 = {
    8, 0, 10, 0, (const void *)0x08089810,
    {(const u8 *)0x081AA6E8, (const u8 *)0x081AA803, (const u8 *)0x081AA8FE, (const u8 *)0x081AAA57, (const u8 *)0x081AAC23, (const u8 *)0x081AADB7, (const u8 *)0x081AAEDC, (const u8 *)0x081AB173}
}; /* song IDs: 31 */

AT("001AB6C0") const SONG_HEADER_TYPE(6) gSongHeader_032 = {
    6, 0, 10, 0, (const void *)0x08089810,
    {(const u8 *)0x081AB378, (const u8 *)0x081AB3F9, (const u8 *)0x081AB430, (const u8 *)0x081AB4FB, (const u8 *)0x081AB571, (const u8 *)0x081AB5C0}
}; /* song IDs: 32 */

AT("001AB6F4") const SONG_HEADER_TYPE(1) gSongHeader_101 = {
    1, 0, 250, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081AB6E0}
}; /* song IDs: 101 */

AT("001AB714") const SONG_HEADER_TYPE(1) gSongHeader_102 = {
    1, 0, 250, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081AB700}
}; /* song IDs: 102 */

AT("001AB73C") const SONG_HEADER_TYPE(1) gSongHeader_103 = {
    1, 0, 250, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081AB720}
}; /* song IDs: 103 */

AT("001AB764") const SONG_HEADER_TYPE(1) gSongHeader_104 = {
    1, 0, 250, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081AB748}
}; /* song IDs: 104 */

AT("001AB780") const SONG_HEADER_TYPE(1) gSongHeader_105 = {
    1, 0, 250, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081AB770}
}; /* song IDs: 105 */

AT("001AB79C") const SONG_HEADER_TYPE(1) gSongHeader_106 = {
    1, 0, 250, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081AB78C}
}; /* song IDs: 106 */

AT("001AB7B8") const SONG_HEADER_TYPE(1) gSongHeader_107 = {
    1, 0, 250, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081AB7A8}
}; /* song IDs: 107 */

AT("001AB888") const SONG_HEADER_TYPE(6) gSongHeader_109 = {
    6, 0, 120, 0, (const void *)0x08089810,
    {(const u8 *)0x081AB7C4, (const u8 *)0x081AB7F1, (const u8 *)0x081AB814, (const u8 *)0x081AB849, (const u8 *)0x081AB862, (const u8 *)0x081AB873}
}; /* song IDs: 109 */

AT("001AB91C") const SONG_HEADER_TYPE(4) gSongHeader_110 = {
    4, 0, 120, 0, (const void *)0x08089810,
    {(const u8 *)0x081AB8A8, (const u8 *)0x081AB8D1, (const u8 *)0x081AB8F0, (const u8 *)0x081AB903}
}; /* song IDs: 110 */

AT("001AB944") const SONG_HEADER_TYPE(1) gSongHeader_111 = {
    1, 0, 180, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081AB934}
}; /* song IDs: 111 */

AT("001AB960") const SONG_HEADER_TYPE(1) gSongHeader_112 = {
    1, 0, 180, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081AB950}
}; /* song IDs: 112 */

AT("001AB97C") const SONG_HEADER_TYPE(1) gSongHeader_113 = {
    1, 0, 180, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081AB96C}
}; /* song IDs: 113 */

AT("001AB998") const SONG_HEADER_TYPE(1) gSongHeader_115 = {
    1, 0, 180, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081AB988}
}; /* song IDs: 115 */

AT("001AB9B4") const SONG_HEADER_TYPE(1) gSongHeader_116 = {
    1, 0, 180, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081AB9A4}
}; /* song IDs: 116 */

AT("001AB9D0") const SONG_HEADER_TYPE(1) gSongHeader_117 = {
    1, 0, 180, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081AB9C0}
}; /* song IDs: 117 */

AT("001AB9EC") const SONG_HEADER_TYPE(1) gSongHeader_118 = {
    1, 0, 160, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081AB9DC}
}; /* song IDs: 118 */

AT("001ABA08") const SONG_HEADER_TYPE(1) gSongHeader_119 = {
    1, 0, 180, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081AB9F8}
}; /* song IDs: 119 */

AT("001ABA74") const SONG_HEADER_TYPE(3) gSongHeader_120 = {
    3, 0, 180, 0, (const void *)0x08089810,
    {(const u8 *)0x081ABA14, (const u8 *)0x081ABA30, (const u8 *)0x081ABA59}
}; /* song IDs: 120 */

AT("001ABA98") const SONG_HEADER_TYPE(1) gSongHeader_121 = {
    1, 0, 180, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081ABA88}
}; /* song IDs: 121 */

AT("001ABAB4") const SONG_HEADER_TYPE(1) gSongHeader_122 = {
    1, 0, 180, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081ABAA4}
}; /* song IDs: 122 */

AT("001ABAD8") const SONG_HEADER_TYPE(1) gSongHeader_123 = {
    1, 0, 140, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081ABAC0}
}; /* song IDs: 123 */

AT("001ABAFC") const SONG_HEADER_TYPE(1) gSongHeader_124 = {
    1, 0, 140, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081ABAE4}
}; /* song IDs: 124 */

AT("001ABDA4") const SONG_HEADER_TYPE(7) gSongHeader_125 = {
    7, 0, 120, 0, (const void *)0x08089810,
    {(const u8 *)0x081ABB08, (const u8 *)0x081ABB3C, (const u8 *)0x081ABB6C, (const u8 *)0x081ABBED, (const u8 *)0x081ABC3F, (const u8 *)0x081ABCDA, (const u8 *)0x081ABD2C}
}; /* song IDs: 125 */

AT("001ABE1C") const SONG_HEADER_TYPE(3) gSongHeader_126 = {
    3, 0, 120, 0, (const void *)0x08089810,
    {(const u8 *)0x081ABDC8, (const u8 *)0x081ABDFA, (const u8 *)0x081ABE0B}
}; /* song IDs: 126 */

AT("001ABE7C") const SONG_HEADER_TYPE(2) gSongHeader_127 = {
    2, 0, 120, 0, (const void *)0x08089810,
    {(const u8 *)0x081ABE30, (const u8 *)0x081ABE62}
}; /* song IDs: 127 */

AT("001ABF40") const SONG_HEADER_TYPE(5) gSongHeader_128 = {
    5, 0, 120, 0, (const void *)0x08089810,
    {(const u8 *)0x081ABE8C, (const u8 *)0x081ABECE, (const u8 *)0x081ABEE3, (const u8 *)0x081ABF0B, (const u8 *)0x081ABF26}
}; /* song IDs: 128 */

AT("001AC028") const SONG_HEADER_TYPE(5) gSongHeader_129 = {
    5, 0, 120, 0, (const void *)0x08089810,
    {(const u8 *)0x081ABF5C, (const u8 *)0x081ABFA0, (const u8 *)0x081ABFD1, (const u8 *)0x081ABFEC, (const u8 *)0x081ABFFF}
}; /* song IDs: 129 */

AT("001AC054") const SONG_HEADER_TYPE(1) gSongHeader_130 = {
    1, 0, 120, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081AC044}
}; /* song IDs: 130 */

AT("001AC070") const SONG_HEADER_TYPE(1) gSongHeader_131 = {
    1, 0, 120, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081AC060}
}; /* song IDs: 131 */

AT("001AC08C") const SONG_HEADER_TYPE(1) gSongHeader_132 = {
    1, 0, 120, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081AC07C}
}; /* song IDs: 132 */

AT("001AC0A8") const SONG_HEADER_TYPE(1) gSongHeader_133 = {
    1, 0, 120, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081AC098}
}; /* song IDs: 133 */

AT("001AC0C4") const SONG_HEADER_TYPE(1) gSongHeader_134 = {
    1, 0, 120, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081AC0B4}
}; /* song IDs: 134 */

AT("001AC0E0") const SONG_HEADER_TYPE(1) gSongHeader_135 = {
    1, 0, 120, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081AC0D0}
}; /* song IDs: 135 */

AT("001AC0FC") const SONG_HEADER_TYPE(1) gSongHeader_140 = {
    1, 0, 120, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081AC0EC}
}; /* song IDs: 140 */

AT("001AC118") const SONG_HEADER_TYPE(1) gSongHeader_141 = {
    1, 0, 120, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081AC108}
}; /* song IDs: 141 */

AT("001AC134") const SONG_HEADER_TYPE(1) gSongHeader_142 = {
    1, 0, 160, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081AC124}
}; /* song IDs: 142 */

AT("001AC188") const SONG_HEADER_TYPE(2) gSongHeader_143 = {
    2, 0, 120, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081AC140, (const u8 *)0x081AC15D}
}; /* song IDs: 143 */

AT("001AC1A8") const SONG_HEADER_TYPE(1) gSongHeader_150 = {
    1, 0, 120, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081AC198}
}; /* song IDs: 150 */

AT("001AC1C4") const SONG_HEADER_TYPE(1) gSongHeader_151 = {
    1, 0, 120, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081AC1B4}
}; /* song IDs: 151 */

AT("001AC208") const SONG_HEADER_TYPE(1) gSongHeader_152 = {
    1, 0, 120, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081AC1D0}
}; /* song IDs: 152 */

AT("001AC224") const SONG_HEADER_TYPE(1) gSongHeader_160 = {
    1, 0, 120, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081AC214}
}; /* song IDs: 160 */

AT("001AC240") const SONG_HEADER_TYPE(1) gSongHeader_161 = {
    1, 0, 120, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081AC230}
}; /* song IDs: 161 */

AT("001AC25C") const SONG_HEADER_TYPE(1) gSongHeader_162 = {
    1, 0, 120, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081AC24C}
}; /* song IDs: 162 */

AT("001AC278") const SONG_HEADER_TYPE(1) gSongHeader_163 = {
    1, 0, 120, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081AC268}
}; /* song IDs: 163 */

AT("001AC294") const SONG_HEADER_TYPE(1) gSongHeader_170 = {
    1, 0, 120, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081AC284}
}; /* song IDs: 170 */

AT("001AC2B0") const SONG_HEADER_TYPE(1) gSongHeader_171 = {
    1, 0, 120, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081AC2A0}
}; /* song IDs: 171 */

AT("001AC2CC") const SONG_HEADER_TYPE(1) gSongHeader_172 = {
    1, 0, 120, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081AC2BC}
}; /* song IDs: 172 */

AT("001AC2E8") const SONG_HEADER_TYPE(1) gSongHeader_180 = {
    1, 0, 120, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081AC2D8}
}; /* song IDs: 180 */

AT("001AC304") const SONG_HEADER_TYPE(1) gSongHeader_181 = {
    1, 0, 120, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081AC2F4}
}; /* song IDs: 181 */

AT("001AC340") const SONG_HEADER_TYPE(1) gSongHeader_182 = {
    1, 0, 120, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081AC310}
}; /* song IDs: 182 */

AT("001AC35C") const SONG_HEADER_TYPE(1) gSongHeader_190 = {
    1, 0, 120, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081AC34C}
}; /* song IDs: 190 */

AT("001AC378") const SONG_HEADER_TYPE(1) gSongHeader_191 = {
    1, 0, 160, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081AC368}
}; /* song IDs: 191 */

AT("001AC394") const SONG_HEADER_TYPE(1) gSongHeader_192 = {
    1, 0, 120, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081AC384}
}; /* song IDs: 192 */

AT("001AC3B0") const SONG_HEADER_TYPE(1) gSongHeader_200 = {
    1, 0, 120, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081AC3A0}
}; /* song IDs: 200 */

AT("001AC3CC") const SONG_HEADER_TYPE(1) gSongHeader_201 = {
    1, 0, 120, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081AC3BC}
}; /* song IDs: 201 */

AT("001AC3E8") const SONG_HEADER_TYPE(1) gSongHeader_202 = {
    1, 0, 120, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081AC3D8}
}; /* song IDs: 202 */

AT("001AC404") const SONG_HEADER_TYPE(1) gSongHeader_203 = {
    1, 0, 120, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081AC3F4}
}; /* song IDs: 203 */

AT("001AC420") const SONG_HEADER_TYPE(1) gSongHeader_204 = {
    1, 0, 120, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081AC410}
}; /* song IDs: 204 */

AT("001AC43C") const SONG_HEADER_TYPE(1) gSongHeader_205 = {
    1, 0, 120, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081AC42C}
}; /* song IDs: 205 */

AT("001AC458") const SONG_HEADER_TYPE(1) gSongHeader_206 = {
    1, 0, 120, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081AC448}
}; /* song IDs: 206 */

AT("001AC494") const SONG_HEADER_TYPE(1) gSongHeader_207 = {
    1, 0, 120, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081AC464}
}; /* song IDs: 207 */

AT("001AC4B0") const SONG_HEADER_TYPE(1) gSongHeader_208 = {
    1, 0, 120, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081AC4A0}
}; /* song IDs: 208 */

AT("001AC4CC") const SONG_HEADER_TYPE(1) gSongHeader_209 = {
    1, 0, 120, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081AC4BC}
}; /* song IDs: 209 */

AT("001AC4E8") const SONG_HEADER_TYPE(1) gSongHeader_210 = {
    1, 0, 120, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081AC4D8}
}; /* song IDs: 210 */

AT("001AC508") const SONG_HEADER_TYPE(1) gSongHeader_211 = {
    1, 0, 120, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081AC4F4}
}; /* song IDs: 211 */

AT("001AC528") const SONG_HEADER_TYPE(1) gSongHeader_212 = {
    1, 0, 120, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081AC514}
}; /* song IDs: 212 */

AT("001AC544") const SONG_HEADER_TYPE(1) gSongHeader_213 = {
    1, 0, 120, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081AC534}
}; /* song IDs: 213 */

AT("001AC564") const SONG_HEADER_TYPE(1) gSongHeader_214 = {
    1, 0, 120, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081AC550}
}; /* song IDs: 214 */

AT("001AC580") const SONG_HEADER_TYPE(1) gSongHeader_215 = {
    1, 0, 120, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081AC570}
}; /* song IDs: 215 */

AT("001AC59C") const SONG_HEADER_TYPE(1) gSongHeader_216 = {
    1, 0, 120, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081AC58C}
}; /* song IDs: 216 */

AT("001AC5B8") const SONG_HEADER_TYPE(1) gSongHeader_217 = {
    1, 0, 120, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081AC5A8}
}; /* song IDs: 217 */

AT("001AC5D4") const SONG_HEADER_TYPE(1) gSongHeader_218 = {
    1, 0, 120, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081AC5C4}
}; /* song IDs: 218 */

AT("001AC5F0") const SONG_HEADER_TYPE(1) gSongHeader_219 = {
    1, 0, 120, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081AC5E0}
}; /* song IDs: 219 */

AT("001AC60C") const SONG_HEADER_TYPE(1) gSongHeader_220 = {
    1, 0, 120, 0, (const void *)0x0808A0E0,
    {(const u8 *)0x081AC5FC}
}; /* song IDs: 220 */
