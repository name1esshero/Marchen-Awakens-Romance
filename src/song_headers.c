#include "gba/types.h"

#include "rom_section.h"
#include "sound.h"

extern const u8 gSong001Track1[];
extern const u8 gSong001Track2[];
extern const u8 gSong001Track3[];
extern const u8 gSong001Track4[];
extern const u8 gSong001Track5[];
extern const u8 gSong001Track6[];
extern const u8 gSong001Track7[];
extern const u8 gSong002Track1[];
extern const u8 gSong002Track2[];
extern const u8 gSong002Track3[];
extern const u8 gSong002Track4[];
extern const u8 gSong002Track5[];
extern const u8 gSong002Track6[];
extern const u8 gSong002Track7[];
extern const u8 gSong003Track1[];
extern const u8 gSong003Track2[];
extern const u8 gSong003Track3[];
extern const u8 gSong003Track4[];
extern const u8 gSong003Track5[];
extern const u8 gSong003Track6[];
extern const u8 gSong004Track1[];
extern const u8 gSong004Track2[];
extern const u8 gSong004Track3[];
extern const u8 gSong004Track4[];
extern const u8 gSong004Track5[];
extern const u8 gSong004Track6[];
extern const u8 gSong004Track7[];
extern const u8 gSong004Track8[];
extern const u8 gSong007Track1[];
extern const u8 gSong007Track2[];
extern const u8 gSong007Track3[];
extern const u8 gSong007Track4[];
extern const u8 gSong007Track5[];
extern const u8 gSong007Track6[];
extern const u8 gSong007Track7[];
extern const u8 gSong008Track1[];
extern const u8 gSong008Track2[];
extern const u8 gSong008Track3[];
extern const u8 gSong008Track4[];
extern const u8 gSong008Track5[];
extern const u8 gSong009Track1[];
extern const u8 gSong009Track2[];
extern const u8 gSong009Track3[];
extern const u8 gSong009Track4[];
extern const u8 gSong009Track5[];
extern const u8 gSong009Track6[];
extern const u8 gSong009Track7[];
extern const u8 gSong010Track1[];
extern const u8 gSong010Track2[];
extern const u8 gSong010Track3[];
extern const u8 gSong010Track4[];
extern const u8 gSong010Track5[];
extern const u8 gSong010Track6[];
extern const u8 gSong010Track7[];
extern const u8 gSong010Track8[];
extern const u8 gSong011Track1[];
extern const u8 gSong011Track2[];
extern const u8 gSong011Track3[];
extern const u8 gSong011Track4[];
extern const u8 gSong011Track5[];
extern const u8 gSong012Track1[];
extern const u8 gSong012Track2[];
extern const u8 gSong012Track3[];
extern const u8 gSong012Track4[];
extern const u8 gSong012Track5[];
extern const u8 gSong012Track6[];
extern const u8 gSong013Track1[];
extern const u8 gSong013Track2[];
extern const u8 gSong013Track3[];
extern const u8 gSong013Track4[];
extern const u8 gSong013Track5[];
extern const u8 gSong014Track1[];
extern const u8 gSong014Track2[];
extern const u8 gSong014Track3[];
extern const u8 gSong014Track4[];
extern const u8 gSong014Track5[];
extern const u8 gSong014Track6[];
extern const u8 gSong014Track7[];
extern const u8 gSong015Track1[];
extern const u8 gSong015Track2[];
extern const u8 gSong015Track3[];
extern const u8 gSong015Track4[];
extern const u8 gSong015Track5[];
extern const u8 gSong015Track6[];
extern const u8 gSong015Track7[];
extern const u8 gSong015Track8[];
extern const u8 gSong015Track9[];
extern const u8 gSong016Track1[];
extern const u8 gSong016Track2[];
extern const u8 gSong016Track3[];
extern const u8 gSong016Track4[];

extern const u8 gSong016Track5[];
extern const u8 gSong016Track6[];
extern const u8 gSong018Track1[];
extern const u8 gSong018Track2[];
extern const u8 gSong018Track3[];
extern const u8 gSong019Track1[];
extern const u8 gSong019Track2[];
extern const u8 gSong019Track3[];
extern const u8 gSong019Track4[];
extern const u8 gSong019Track5[];
extern const u8 gSong019Track6[];
extern const u8 gSong020Track1[];
extern const u8 gSong020Track2[];
extern const u8 gSong020Track3[];
extern const u8 gSong020Track4[];
extern const u8 gSong020Track5[];
extern const u8 gSong020Track6[];
extern const u8 gSong020Track7[];
extern const u8 gSong020Track8[];
extern const u8 gSong020Track9[];
extern const u8 gSong021Track1[];
extern const u8 gSong021Track2[];
extern const u8 gSong021Track3[];
extern const u8 gSong021Track4[];
extern const u8 gSong021Track5[];
extern const u8 gSong021Track6[];
extern const u8 gSong021Track7[];
extern const u8 gSong021Track8[];
extern const u8 gSong021Track9[];
extern const u8 gSong022Track1[];
extern const u8 gSong022Track2[];
extern const u8 gSong022Track3[];
extern const u8 gSong022Track4[];
extern const u8 gSong022Track5[];
extern const u8 gSong023Track1[];
extern const u8 gSong023Track2[];
extern const u8 gSong023Track3[];
extern const u8 gSong023Track4[];
extern const u8 gSong023Track5[];
extern const u8 gSong024Track1[];
extern const u8 gSong024Track2[];
extern const u8 gSong024Track3[];
extern const u8 gSong024Track4[];
extern const u8 gSong026Track1[];
extern const u8 gSong026Track2[];
extern const u8 gSong026Track3[];
extern const u8 gSong026Track4[];
extern const u8 gSong026Track5[];
extern const u8 gSong026Track6[];
extern const u8 gSong026Track7[];
extern const u8 gSong026Track8[];
extern const u8 gSong027Track1[];
extern const u8 gSong027Track2[];
extern const u8 gSong027Track3[];
extern const u8 gSong027Track4[];
extern const u8 gSong027Track5[];
extern const u8 gSong027Track6[];
extern const u8 gSong027Track7[];
extern const u8 gSong027Track8[];
extern const u8 gSong027Track9[];
extern const u8 gSong027Track10[];
extern const u8 gSong028Track1[];
extern const u8 gSong028Track2[];
extern const u8 gSong028Track3[];
extern const u8 gSong028Track4[];
extern const u8 gSong028Track5[];
extern const u8 gSong028Track6[];
extern const u8 gSong030Track1[];
extern const u8 gSong030Track2[];
extern const u8 gSong030Track3[];
extern const u8 gSong030Track4[];
extern const u8 gSong030Track5[];
extern const u8 gSong030Track6[];
extern const u8 gSong031Track1[];
extern const u8 gSong031Track2[];
extern const u8 gSong031Track3[];
extern const u8 gSong031Track4[];
extern const u8 gSong031Track5[];
extern const u8 gSong031Track6[];
extern const u8 gSong031Track7[];
extern const u8 gSong031Track8[];
extern const u8 gSong032Track1[];
extern const u8 gSong032Track2[];
extern const u8 gSong032Track3[];
extern const u8 gSong032Track4[];
extern const u8 gSong032Track5[];
extern const u8 gSong032Track6[];
extern const u8 gSong101Track1[];
extern const u8 gSong102Track1[];
extern const u8 gSong103Track1[];
extern const u8 gSong104Track1[];
extern const u8 gSong105Track1[];
extern const u8 gSong106Track1[];
extern const u8 gSong107Track1[];
extern const u8 gSong109Track1[];
extern const u8 gSong109Track2[];
extern const u8 gSong109Track3[];
extern const u8 gSong109Track4[];
extern const u8 gSong109Track5[];
extern const u8 gSong109Track6[];
extern const u8 gSong110Track1[];
extern const u8 gSong110Track2[];
extern const u8 gSong110Track3[];
extern const u8 gSong110Track4[];
extern const u8 gSong111Track1[];
extern const u8 gSong112Track1[];
extern const u8 gSong113Track1[];
extern const u8 gSong115Track1[];
extern const u8 gSong116Track1[];
extern const u8 gSong117Track1[];
extern const u8 gSong118Track1[];
extern const u8 gSong119Track1[];
extern const u8 gSong120Track1[];
extern const u8 gSong120Track2[];
extern const u8 gSong120Track3[];
extern const u8 gSong121Track1[];
extern const u8 gSong122Track1[];
extern const u8 gSong123Track1[];
extern const u8 gSong124Track1[];
extern const u8 gSong125Track1[];
extern const u8 gSong125Track2[];
extern const u8 gSong125Track3[];
extern const u8 gSong125Track4[];
extern const u8 gSong125Track5[];
extern const u8 gSong125Track6[];
extern const u8 gSong125Track7[];
extern const u8 gSong126Track1[];
extern const u8 gSong126Track2[];
extern const u8 gSong126Track3[];
extern const u8 gSong127Track1[];
extern const u8 gSong127Track2[];
extern const u8 gSong128Track1[];
extern const u8 gSong128Track2[];
extern const u8 gSong128Track3[];
extern const u8 gSong128Track4[];
extern const u8 gSong128Track5[];
extern const u8 gSong129Track1[];
extern const u8 gSong129Track2[];
extern const u8 gSong129Track3[];
extern const u8 gSong129Track4[];
extern const u8 gSong129Track5[];
extern const u8 gSong130Track1[];
extern const u8 gSong131Track1[];
extern const u8 gSong132Track1[];
extern const u8 gSong133Track1[];
extern const u8 gSong134Track1[];
extern const u8 gSong135Track1[];
extern const u8 gSong140Track1[];
extern const u8 gSong141Track1[];
extern const u8 gSong142Track1[];
extern const u8 gSong143Track1[];
extern const u8 gSong143Track2[];
extern const u8 gSong150Track1[];
extern const u8 gSong151Track1[];
extern const u8 gSong152Track1[];
extern const u8 gSong160Track1[];
extern const u8 gSong161Track1[];
extern const u8 gSong162Track1[];
extern const u8 gSong163Track1[];
extern const u8 gSong170Track1[];
extern const u8 gSong171Track1[];
extern const u8 gSong172Track1[];
extern const u8 gSong180Track1[];
extern const u8 gSong181Track1[];
extern const u8 gSong182Track1[];
extern const u8 gSong190Track1[];
extern const u8 gSong191Track1[];
extern const u8 gSong192Track1[];
extern const u8 gSong200Track1[];
extern const u8 gSong201Track1[];
extern const u8 gSong202Track1[];
extern const u8 gSong203Track1[];
extern const u8 gSong204Track1[];
extern const u8 gSong205Track1[];
extern const u8 gSong206Track1[];
extern const u8 gSong207Track1[];
extern const u8 gSong208Track1[];
extern const u8 gSong209Track1[];
extern const u8 gSong210Track1[];
extern const u8 gSong211Track1[];
extern const u8 gSong212Track1[];
extern const u8 gSong213Track1[];
extern const u8 gSong214Track1[];
extern const u8 gSong215Track1[];
extern const u8 gSong216Track1[];
extern const u8 gSong217Track1[];
extern const u8 gSong218Track1[];
extern const u8 gSong219Track1[];
extern const u8 gSong220Track1[];

#define SONG_HEADER_TYPE(count) \
    struct { \
        u8 trackCount, blockCount, priority, reverb; \
        const void *voices; \
        const u8 *parts[count]; \
    }

/* Variable-sized MusicPlayer2000 song headers.  Each comment lists every
 * game-visible song-table ID that selects the header. */
/** A zero-track entry consists only of the four-byte prefix.  The bytes that
 * follow at 0808B89C are the first PCM wave header, not a voices pointer. */
AT("0008B898") const u32 gSongHeader_000 = 0;
/** song IDs: 0, 5, 6, 17, 25, 29, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43,
 * 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61,
 * 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
 * 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97,
 * 98, 99, 100, 108, 114, 136, 137, 138, 139, 144, 145, 146, 147, 148, 149,
 * 153, 154, 155, 156, 157, 158, 159, 164, 165, 166, 167, 168, 169, 173,
 * 174, 175, 176, 177, 178, 179, 183, 184, 185, 186, 187, 188, 189, 193,
 * 194, 195, 196, 197, 198, 199 */

AT("001A2BD4") const SONG_HEADER_TYPE(7) gSongHeader_001 = {
    7, 0, 10, 0, gVoiceGroupMain,
    {gSong001Track1, gSong001Track2, gSong001Track3, gSong001Track4, gSong001Track5, gSong001Track6, gSong001Track7}
}; /** song IDs: 1 */

AT("001A3104") const SONG_HEADER_TYPE(7) gSongHeader_002 = {
    7, 0, 10, 0, gVoiceGroupMain,
    {gSong002Track1, gSong002Track2, gSong002Track3, gSong002Track4, gSong002Track5, gSong002Track6, gSong002Track7}
}; /** song IDs: 2 */

AT("001A33C0") const SONG_HEADER_TYPE(6) gSongHeader_003 = {
    6, 0, 10, 0, gVoiceGroupMain,
    {gSong003Track1, gSong003Track2, gSong003Track3, gSong003Track4, gSong003Track5, gSong003Track6}
}; /** song IDs: 3 */

AT("001A3978") const SONG_HEADER_TYPE(8) gSongHeader_004 = {
    8, 0, 10, 0, gVoiceGroupMain,
    {gSong004Track1, gSong004Track2, gSong004Track3, gSong004Track4, gSong004Track5, gSong004Track6, gSong004Track7, gSong004Track8}
}; /** song IDs: 4 */

AT("001A3C24") const SONG_HEADER_TYPE(7) gSongHeader_007 = {
    7, 0, 10, 0, gVoiceGroupMain,
    {gSong007Track1, gSong007Track2, gSong007Track3, gSong007Track4, gSong007Track5, gSong007Track6, gSong007Track7}
}; /** song IDs: 7 */

AT("001A41AC") const SONG_HEADER_TYPE(5) gSongHeader_008 = {
    5, 0, 10, 0, gVoiceGroupMain,
    {gSong008Track1, gSong008Track2, gSong008Track3, gSong008Track4, gSong008Track5}
}; /** song IDs: 8 */

AT("001A4928") const SONG_HEADER_TYPE(7) gSongHeader_009 = {
    7, 0, 10, 0, gVoiceGroupMain,
    {gSong009Track1, gSong009Track2, gSong009Track3, gSong009Track4, gSong009Track5, gSong009Track6, gSong009Track7}
}; /** song IDs: 9 */

AT("001A4E40") const SONG_HEADER_TYPE(8) gSongHeader_010 = {
    8, 0, 10, 0, gVoiceGroupMain,
    {gSong010Track1, gSong010Track2, gSong010Track3, gSong010Track4, gSong010Track5, gSong010Track6, gSong010Track7, gSong010Track8}
}; /** song IDs: 10 */

AT("001A514C") const SONG_HEADER_TYPE(5) gSongHeader_011 = {
    5, 0, 10, 0, gVoiceGroupMain,
    {gSong011Track1, gSong011Track2, gSong011Track3, gSong011Track4, gSong011Track5}
}; /** song IDs: 11 */

AT("001A5690") const SONG_HEADER_TYPE(6) gSongHeader_012 = {
    6, 0, 10, 0, gVoiceGroupMain,
    {gSong012Track1, gSong012Track2, gSong012Track3, gSong012Track4, gSong012Track5, gSong012Track6}
}; /** song IDs: 12 */

AT("001A5BB0") const SONG_HEADER_TYPE(5) gSongHeader_013 = {
    5, 0, 10, 0, gVoiceGroupMain,
    {gSong013Track1, gSong013Track2, gSong013Track3, gSong013Track4, gSong013Track5}
}; /** song IDs: 13 */

AT("001A60F4") const SONG_HEADER_TYPE(7) gSongHeader_014 = {
    7, 0, 10, 0, gVoiceGroupMain,
    {gSong014Track1, gSong014Track2, gSong014Track3, gSong014Track4, gSong014Track5, gSong014Track6, gSong014Track7}
}; /** song IDs: 14 */

AT("001A6AB4") const SONG_HEADER_TYPE(9) gSongHeader_015 = {
    9, 0, 10, 0, gVoiceGroupMain,
    {gSong015Track1, gSong015Track2, gSong015Track3, gSong015Track4, gSong015Track5, gSong015Track6, gSong015Track7, gSong015Track8, gSong015Track9}
}; /** song IDs: 15 */

AT("001A738C") const SONG_HEADER_TYPE(6) gSongHeader_016 = {
    6, 0, 10, 0, gVoiceGroupEffects,
    {gSong016Track1, gSong016Track2, gSong016Track3, gSong016Track4, gSong016Track5, gSong016Track6}
}; /** song IDs: 16 */

AT("001A77F4") const SONG_HEADER_TYPE(3) gSongHeader_018 = {
    3, 0, 10, 0, gVoiceGroupMain,
    {gSong018Track1, gSong018Track2, gSong018Track3}
}; /** song IDs: 18 */

AT("001A7A8C") const SONG_HEADER_TYPE(6) gSongHeader_019 = {
    6, 0, 10, 0, gVoiceGroupMain,
    {gSong019Track1, gSong019Track2, gSong019Track3, gSong019Track4, gSong019Track5, gSong019Track6}
}; /** song IDs: 19 */

AT("001A8164") const SONG_HEADER_TYPE(9) gSongHeader_020 = {
    9, 0, 10, 0, gVoiceGroupMain,
    {gSong020Track1, gSong020Track2, gSong020Track3, gSong020Track4, gSong020Track5, gSong020Track6, gSong020Track7, gSong020Track8, gSong020Track9}
}; /** song IDs: 20 */

AT("001A8838") const SONG_HEADER_TYPE(9) gSongHeader_021 = {
    9, 0, 10, 0, gVoiceGroupMain,
    {gSong021Track1, gSong021Track2, gSong021Track3, gSong021Track4, gSong021Track5, gSong021Track6, gSong021Track7, gSong021Track8, gSong021Track9}
}; /** song IDs: 21 */

AT("001A8C14") const SONG_HEADER_TYPE(5) gSongHeader_022 = {
    5, 0, 10, 0, gVoiceGroupMain,
    {gSong022Track1, gSong022Track2, gSong022Track3, gSong022Track4, gSong022Track5}
}; /** song IDs: 22 */

AT("001A8E58") const SONG_HEADER_TYPE(5) gSongHeader_023 = {
    5, 0, 10, 0, gVoiceGroupMain,
    {gSong023Track1, gSong023Track2, gSong023Track3, gSong023Track4, gSong023Track5}
}; /** song IDs: 23 */

AT("001A9210") const SONG_HEADER_TYPE(4) gSongHeader_024 = {
    4, 0, 10, 0, gVoiceGroupMain,
    {gSong024Track1, gSong024Track2, gSong024Track3, gSong024Track4}
}; /** song IDs: 24 */

AT("001A9920") const SONG_HEADER_TYPE(8) gSongHeader_026 = {
    8, 0, 10, 0, gVoiceGroupMain,
    {gSong026Track1, gSong026Track2, gSong026Track3, gSong026Track4, gSong026Track5, gSong026Track6, gSong026Track7, gSong026Track8}
}; /** song IDs: 26 */

AT("001A9EB8") const SONG_HEADER_TYPE(10) gSongHeader_027 = {
    10, 0, 10, 0, gVoiceGroupMain,
    {gSong027Track1, gSong027Track2, gSong027Track3, gSong027Track4, gSong027Track5, gSong027Track6, gSong027Track7, gSong027Track8, gSong027Track9, gSong027Track10}
}; /** song IDs: 27 */

AT("001AA140") const SONG_HEADER_TYPE(6) gSongHeader_028 = {
    6, 0, 10, 0, gVoiceGroupMain,
    {gSong028Track1, gSong028Track2, gSong028Track3, gSong028Track4, gSong028Track5, gSong028Track6}
}; /** song IDs: 28 */

AT("001AA6C8") const SONG_HEADER_TYPE(6) gSongHeader_030 = {
    6, 0, 10, 0, gVoiceGroupMain,
    {gSong030Track1, gSong030Track2, gSong030Track3, gSong030Track4, gSong030Track5, gSong030Track6}
}; /** song IDs: 30 */

AT("001AB350") const SONG_HEADER_TYPE(8) gSongHeader_031 = {
    8, 0, 10, 0, gVoiceGroupMain,
    {gSong031Track1, gSong031Track2, gSong031Track3, gSong031Track4, gSong031Track5, gSong031Track6, gSong031Track7, gSong031Track8}
}; /** song IDs: 31 */

AT("001AB6C0") const SONG_HEADER_TYPE(6) gSongHeader_032 = {
    6, 0, 10, 0, gVoiceGroupMain,
    {gSong032Track1, gSong032Track2, gSong032Track3, gSong032Track4, gSong032Track5, gSong032Track6}
}; /** song IDs: 32 */

AT("001AB6F4") const SONG_HEADER_TYPE(1) gSongHeader_101 = {
    1, 0, 250, 0, gVoiceGroupSecondary,
    {gSong101Track1}
}; /** song IDs: 101 */

AT("001AB714") const SONG_HEADER_TYPE(1) gSongHeader_102 = {
    1, 0, 250, 0, gVoiceGroupSecondary,
    {gSong102Track1}
}; /** song IDs: 102 */

AT("001AB73C") const SONG_HEADER_TYPE(1) gSongHeader_103 = {
    1, 0, 250, 0, gVoiceGroupSecondary,
    {gSong103Track1}
}; /** song IDs: 103 */

AT("001AB764") const SONG_HEADER_TYPE(1) gSongHeader_104 = {
    1, 0, 250, 0, gVoiceGroupSecondary,
    {gSong104Track1}
}; /** song IDs: 104 */

AT("001AB780") const SONG_HEADER_TYPE(1) gSongHeader_105 = {
    1, 0, 250, 0, gVoiceGroupSecondary,
    {gSong105Track1}
}; /** song IDs: 105 */

AT("001AB79C") const SONG_HEADER_TYPE(1) gSongHeader_106 = {
    1, 0, 250, 0, gVoiceGroupSecondary,
    {gSong106Track1}
}; /** song IDs: 106 */

AT("001AB7B8") const SONG_HEADER_TYPE(1) gSongHeader_107 = {
    1, 0, 250, 0, gVoiceGroupSecondary,
    {gSong107Track1}
}; /** song IDs: 107 */

AT("001AB888") const SONG_HEADER_TYPE(6) gSongHeader_109 = {
    6, 0, 120, 0, gVoiceGroupMain,
    {gSong109Track1, gSong109Track2, gSong109Track3, gSong109Track4, gSong109Track5, gSong109Track6}
}; /** song IDs: 109 */

AT("001AB91C") const SONG_HEADER_TYPE(4) gSongHeader_110 = {
    4, 0, 120, 0, gVoiceGroupMain,
    {gSong110Track1, gSong110Track2, gSong110Track3, gSong110Track4}
}; /** song IDs: 110 */

AT("001AB944") const SONG_HEADER_TYPE(1) gSongHeader_111 = {
    1, 0, 180, 0, gVoiceGroupSecondary,
    {gSong111Track1}
}; /** song IDs: 111 */

AT("001AB960") const SONG_HEADER_TYPE(1) gSongHeader_112 = {
    1, 0, 180, 0, gVoiceGroupSecondary,
    {gSong112Track1}
}; /** song IDs: 112 */

AT("001AB97C") const SONG_HEADER_TYPE(1) gSongHeader_113 = {
    1, 0, 180, 0, gVoiceGroupSecondary,
    {gSong113Track1}
}; /** song IDs: 113 */

AT("001AB998") const SONG_HEADER_TYPE(1) gSongHeader_115 = {
    1, 0, 180, 0, gVoiceGroupSecondary,
    {gSong115Track1}
}; /** song IDs: 115 */

AT("001AB9B4") const SONG_HEADER_TYPE(1) gSongHeader_116 = {
    1, 0, 180, 0, gVoiceGroupSecondary,
    {gSong116Track1}
}; /** song IDs: 116 */

AT("001AB9D0") const SONG_HEADER_TYPE(1) gSongHeader_117 = {
    1, 0, 180, 0, gVoiceGroupSecondary,
    {gSong117Track1}
}; /** song IDs: 117 */

AT("001AB9EC") const SONG_HEADER_TYPE(1) gSongHeader_118 = {
    1, 0, 160, 0, gVoiceGroupSecondary,
    {gSong118Track1}
}; /** song IDs: 118 */

AT("001ABA08") const SONG_HEADER_TYPE(1) gSongHeader_119 = {
    1, 0, 180, 0, gVoiceGroupSecondary,
    {gSong119Track1}
}; /** song IDs: 119 */

AT("001ABA74") const SONG_HEADER_TYPE(3) gSongHeader_120 = {
    3, 0, 180, 0, gVoiceGroupMain,
    {gSong120Track1, gSong120Track2, gSong120Track3}
}; /** song IDs: 120 */

AT("001ABA98") const SONG_HEADER_TYPE(1) gSongHeader_121 = {
    1, 0, 180, 0, gVoiceGroupSecondary,
    {gSong121Track1}
}; /** song IDs: 121 */

AT("001ABAB4") const SONG_HEADER_TYPE(1) gSongHeader_122 = {
    1, 0, 180, 0, gVoiceGroupSecondary,
    {gSong122Track1}
}; /** song IDs: 122 */

AT("001ABAD8") const SONG_HEADER_TYPE(1) gSongHeader_123 = {
    1, 0, 140, 0, gVoiceGroupSecondary,
    {gSong123Track1}
}; /** song IDs: 123 */

AT("001ABAFC") const SONG_HEADER_TYPE(1) gSongHeader_124 = {
    1, 0, 140, 0, gVoiceGroupSecondary,
    {gSong124Track1}
}; /** song IDs: 124 */

AT("001ABDA4") const SONG_HEADER_TYPE(7) gSongHeader_125 = {
    7, 0, 120, 0, gVoiceGroupMain,
    {gSong125Track1, gSong125Track2, gSong125Track3, gSong125Track4, gSong125Track5, gSong125Track6, gSong125Track7}
}; /** song IDs: 125 */

AT("001ABE1C") const SONG_HEADER_TYPE(3) gSongHeader_126 = {
    3, 0, 120, 0, gVoiceGroupMain,
    {gSong126Track1, gSong126Track2, gSong126Track3}
}; /** song IDs: 126 */

AT("001ABE7C") const SONG_HEADER_TYPE(2) gSongHeader_127 = {
    2, 0, 120, 0, gVoiceGroupMain,
    {gSong127Track1, gSong127Track2}
}; /** song IDs: 127 */

AT("001ABF40") const SONG_HEADER_TYPE(5) gSongHeader_128 = {
    5, 0, 120, 0, gVoiceGroupMain,
    {gSong128Track1, gSong128Track2, gSong128Track3, gSong128Track4, gSong128Track5}
}; /** song IDs: 128 */

AT("001AC028") const SONG_HEADER_TYPE(5) gSongHeader_129 = {
    5, 0, 120, 0, gVoiceGroupMain,
    {gSong129Track1, gSong129Track2, gSong129Track3, gSong129Track4, gSong129Track5}
}; /** song IDs: 129 */

AT("001AC054") const SONG_HEADER_TYPE(1) gSongHeader_130 = {
    1, 0, 120, 0, gVoiceGroupSecondary,
    {gSong130Track1}
}; /** song IDs: 130 */

AT("001AC070") const SONG_HEADER_TYPE(1) gSongHeader_131 = {
    1, 0, 120, 0, gVoiceGroupSecondary,
    {gSong131Track1}
}; /** song IDs: 131 */

AT("001AC08C") const SONG_HEADER_TYPE(1) gSongHeader_132 = {
    1, 0, 120, 0, gVoiceGroupSecondary,
    {gSong132Track1}
}; /** song IDs: 132 */

AT("001AC0A8") const SONG_HEADER_TYPE(1) gSongHeader_133 = {
    1, 0, 120, 0, gVoiceGroupSecondary,
    {gSong133Track1}
}; /** song IDs: 133 */

AT("001AC0C4") const SONG_HEADER_TYPE(1) gSongHeader_134 = {
    1, 0, 120, 0, gVoiceGroupSecondary,
    {gSong134Track1}
}; /** song IDs: 134 */

AT("001AC0E0") const SONG_HEADER_TYPE(1) gSongHeader_135 = {
    1, 0, 120, 0, gVoiceGroupSecondary,
    {gSong135Track1}
}; /** song IDs: 135 */

AT("001AC0FC") const SONG_HEADER_TYPE(1) gSongHeader_140 = {
    1, 0, 120, 0, gVoiceGroupSecondary,
    {gSong140Track1}
}; /** song IDs: 140 */

AT("001AC118") const SONG_HEADER_TYPE(1) gSongHeader_141 = {
    1, 0, 120, 0, gVoiceGroupSecondary,
    {gSong141Track1}
}; /** song IDs: 141 */

AT("001AC134") const SONG_HEADER_TYPE(1) gSongHeader_142 = {
    1, 0, 160, 0, gVoiceGroupSecondary,
    {gSong142Track1}
}; /** song IDs: 142 */

AT("001AC188") const SONG_HEADER_TYPE(2) gSongHeader_143 = {
    2, 0, 120, 0, gVoiceGroupSecondary,
    {gSong143Track1, gSong143Track2}
}; /** song IDs: 143 */

AT("001AC1A8") const SONG_HEADER_TYPE(1) gSongHeader_150 = {
    1, 0, 120, 0, gVoiceGroupSecondary,
    {gSong150Track1}
}; /** song IDs: 150 */

AT("001AC1C4") const SONG_HEADER_TYPE(1) gSongHeader_151 = {
    1, 0, 120, 0, gVoiceGroupSecondary,
    {gSong151Track1}
}; /** song IDs: 151 */

AT("001AC208") const SONG_HEADER_TYPE(1) gSongHeader_152 = {
    1, 0, 120, 0, gVoiceGroupSecondary,
    {gSong152Track1}
}; /** song IDs: 152 */

AT("001AC224") const SONG_HEADER_TYPE(1) gSongHeader_160 = {
    1, 0, 120, 0, gVoiceGroupSecondary,
    {gSong160Track1}
}; /** song IDs: 160 */

AT("001AC240") const SONG_HEADER_TYPE(1) gSongHeader_161 = {
    1, 0, 120, 0, gVoiceGroupSecondary,
    {gSong161Track1}
}; /** song IDs: 161 */

AT("001AC25C") const SONG_HEADER_TYPE(1) gSongHeader_162 = {
    1, 0, 120, 0, gVoiceGroupSecondary,
    {gSong162Track1}
}; /** song IDs: 162 */

AT("001AC278") const SONG_HEADER_TYPE(1) gSongHeader_163 = {
    1, 0, 120, 0, gVoiceGroupSecondary,
    {gSong163Track1}
}; /** song IDs: 163 */

AT("001AC294") const SONG_HEADER_TYPE(1) gSongHeader_170 = {
    1, 0, 120, 0, gVoiceGroupSecondary,
    {gSong170Track1}
}; /** song IDs: 170 */

AT("001AC2B0") const SONG_HEADER_TYPE(1) gSongHeader_171 = {
    1, 0, 120, 0, gVoiceGroupSecondary,
    {gSong171Track1}
}; /** song IDs: 171 */

AT("001AC2CC") const SONG_HEADER_TYPE(1) gSongHeader_172 = {
    1, 0, 120, 0, gVoiceGroupSecondary,
    {gSong172Track1}
}; /** song IDs: 172 */

AT("001AC2E8") const SONG_HEADER_TYPE(1) gSongHeader_180 = {
    1, 0, 120, 0, gVoiceGroupSecondary,
    {gSong180Track1}
}; /** song IDs: 180 */

AT("001AC304") const SONG_HEADER_TYPE(1) gSongHeader_181 = {
    1, 0, 120, 0, gVoiceGroupSecondary,
    {gSong181Track1}
}; /** song IDs: 181 */

AT("001AC340") const SONG_HEADER_TYPE(1) gSongHeader_182 = {
    1, 0, 120, 0, gVoiceGroupSecondary,
    {gSong182Track1}
}; /** song IDs: 182 */

AT("001AC35C") const SONG_HEADER_TYPE(1) gSongHeader_190 = {
    1, 0, 120, 0, gVoiceGroupSecondary,
    {gSong190Track1}
}; /** song IDs: 190 */

AT("001AC378") const SONG_HEADER_TYPE(1) gSongHeader_191 = {
    1, 0, 160, 0, gVoiceGroupSecondary,
    {gSong191Track1}
}; /** song IDs: 191 */

AT("001AC394") const SONG_HEADER_TYPE(1) gSongHeader_192 = {
    1, 0, 120, 0, gVoiceGroupSecondary,
    {gSong192Track1}
}; /** song IDs: 192 */

AT("001AC3B0") const SONG_HEADER_TYPE(1) gSongHeader_200 = {
    1, 0, 120, 0, gVoiceGroupSecondary,
    {gSong200Track1}
}; /** song IDs: 200 */

AT("001AC3CC") const SONG_HEADER_TYPE(1) gSongHeader_201 = {
    1, 0, 120, 0, gVoiceGroupSecondary,
    {gSong201Track1}
}; /** song IDs: 201 */

AT("001AC3E8") const SONG_HEADER_TYPE(1) gSongHeader_202 = {
    1, 0, 120, 0, gVoiceGroupSecondary,
    {gSong202Track1}
}; /** song IDs: 202 */

AT("001AC404") const SONG_HEADER_TYPE(1) gSongHeader_203 = {
    1, 0, 120, 0, gVoiceGroupSecondary,
    {gSong203Track1}
}; /** song IDs: 203 */

AT("001AC420") const SONG_HEADER_TYPE(1) gSongHeader_204 = {
    1, 0, 120, 0, gVoiceGroupSecondary,
    {gSong204Track1}
}; /** song IDs: 204 */

AT("001AC43C") const SONG_HEADER_TYPE(1) gSongHeader_205 = {
    1, 0, 120, 0, gVoiceGroupSecondary,
    {gSong205Track1}
}; /** song IDs: 205 */

AT("001AC458") const SONG_HEADER_TYPE(1) gSongHeader_206 = {
    1, 0, 120, 0, gVoiceGroupSecondary,
    {gSong206Track1}
}; /** song IDs: 206 */

AT("001AC494") const SONG_HEADER_TYPE(1) gSongHeader_207 = {
    1, 0, 120, 0, gVoiceGroupSecondary,
    {gSong207Track1}
}; /** song IDs: 207 */

AT("001AC4B0") const SONG_HEADER_TYPE(1) gSongHeader_208 = {
    1, 0, 120, 0, gVoiceGroupSecondary,
    {gSong208Track1}
}; /** song IDs: 208 */

AT("001AC4CC") const SONG_HEADER_TYPE(1) gSongHeader_209 = {
    1, 0, 120, 0, gVoiceGroupSecondary,
    {gSong209Track1}
}; /** song IDs: 209 */

AT("001AC4E8") const SONG_HEADER_TYPE(1) gSongHeader_210 = {
    1, 0, 120, 0, gVoiceGroupSecondary,
    {gSong210Track1}
}; /** song IDs: 210 */

AT("001AC508") const SONG_HEADER_TYPE(1) gSongHeader_211 = {
    1, 0, 120, 0, gVoiceGroupSecondary,
    {gSong211Track1}
}; /** song IDs: 211 */

AT("001AC528") const SONG_HEADER_TYPE(1) gSongHeader_212 = {
    1, 0, 120, 0, gVoiceGroupSecondary,
    {gSong212Track1}
}; /** song IDs: 212 */

AT("001AC544") const SONG_HEADER_TYPE(1) gSongHeader_213 = {
    1, 0, 120, 0, gVoiceGroupSecondary,
    {gSong213Track1}
}; /** song IDs: 213 */

AT("001AC564") const SONG_HEADER_TYPE(1) gSongHeader_214 = {
    1, 0, 120, 0, gVoiceGroupSecondary,
    {gSong214Track1}
}; /** song IDs: 214 */

AT("001AC580") const SONG_HEADER_TYPE(1) gSongHeader_215 = {
    1, 0, 120, 0, gVoiceGroupSecondary,
    {gSong215Track1}
}; /** song IDs: 215 */

AT("001AC59C") const SONG_HEADER_TYPE(1) gSongHeader_216 = {
    1, 0, 120, 0, gVoiceGroupSecondary,
    {gSong216Track1}
}; /** song IDs: 216 */

AT("001AC5B8") const SONG_HEADER_TYPE(1) gSongHeader_217 = {
    1, 0, 120, 0, gVoiceGroupSecondary,
    {gSong217Track1}
}; /** song IDs: 217 */

AT("001AC5D4") const SONG_HEADER_TYPE(1) gSongHeader_218 = {
    1, 0, 120, 0, gVoiceGroupSecondary,
    {gSong218Track1}
}; /** song IDs: 218 */

AT("001AC5F0") const SONG_HEADER_TYPE(1) gSongHeader_219 = {
    1, 0, 120, 0, gVoiceGroupSecondary,
    {gSong219Track1}
}; /** song IDs: 219 */

AT("001AC60C") const SONG_HEADER_TYPE(1) gSongHeader_220 = {
    1, 0, 120, 0, gVoiceGroupSecondary,
    {gSong220Track1}
}; /* song IDs: 220 */
