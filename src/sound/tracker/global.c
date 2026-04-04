#include "zgdk/sound/tracker.h"
#include "private.h"

uint8_t ticks              = 0;
uint8_t current_pattern     = 0;
uint8_t current_arrangement = 0;
uint8_t next_step           = 0;
uint8_t last_step           = 0;

note_t NOTES[] = {
    /* Octave 0 */
    SOUND_FREQ_TO_DIV(16), // C0
    SOUND_FREQ_TO_DIV(17), // C#0
    SOUND_FREQ_TO_DIV(18), // D0
    SOUND_FREQ_TO_DIV(19), // D#0
    SOUND_FREQ_TO_DIV(21), // E0
    SOUND_FREQ_TO_DIV(22), // F0
    SOUND_FREQ_TO_DIV(23), // F#0
    SOUND_FREQ_TO_DIV(24), // G0
    SOUND_FREQ_TO_DIV(26), // G#0
    SOUND_FREQ_TO_DIV(27), // A0
    SOUND_FREQ_TO_DIV(29), // A#0
    SOUND_FREQ_TO_DIV(31), // B0

    /* Octave 1 */
    SOUND_FREQ_TO_DIV(33), // C1
    SOUND_FREQ_TO_DIV(35), // C#1
    SOUND_FREQ_TO_DIV(37), // D1
    SOUND_FREQ_TO_DIV(39), // D#1
    SOUND_FREQ_TO_DIV(41), // E1
    SOUND_FREQ_TO_DIV(44), // F1
    SOUND_FREQ_TO_DIV(47), // F#1
    SOUND_FREQ_TO_DIV(49), // G1
    SOUND_FREQ_TO_DIV(52), // G#1
    SOUND_FREQ_TO_DIV(55), // A1
    SOUND_FREQ_TO_DIV(59), // A#1
    SOUND_FREQ_TO_DIV(62), // B1

    /* Octave 2 */
    SOUND_FREQ_TO_DIV(65),  // C2
    SOUND_FREQ_TO_DIV(69),  // C#2
    SOUND_FREQ_TO_DIV(73),  // D2
    SOUND_FREQ_TO_DIV(78),  // D#2
    SOUND_FREQ_TO_DIV(82),  // E2
    SOUND_FREQ_TO_DIV(87),  // F2
    SOUND_FREQ_TO_DIV(92),  // F#2
    SOUND_FREQ_TO_DIV(98),  // G2
    SOUND_FREQ_TO_DIV(104), // G#2
    SOUND_FREQ_TO_DIV(110), // A2
    SOUND_FREQ_TO_DIV(117), // A#2
    SOUND_FREQ_TO_DIV(123), // B2

    /* Octave 3 */
    SOUND_FREQ_TO_DIV(131), // C3
    SOUND_FREQ_TO_DIV(139), // C#3
    SOUND_FREQ_TO_DIV(147), // D3
    SOUND_FREQ_TO_DIV(156), // D#3
    SOUND_FREQ_TO_DIV(165), // E3
    SOUND_FREQ_TO_DIV(175), // F3
    SOUND_FREQ_TO_DIV(185), // F#3
    SOUND_FREQ_TO_DIV(196), // G3
    SOUND_FREQ_TO_DIV(208), // G#3
    SOUND_FREQ_TO_DIV(220), // A3
    SOUND_FREQ_TO_DIV(233), // A#3
    SOUND_FREQ_TO_DIV(247), // B3

    /* Octave 4 */
    SOUND_FREQ_TO_DIV(262), // C4
    SOUND_FREQ_TO_DIV(277), // C#4
    SOUND_FREQ_TO_DIV(294), // D4
    SOUND_FREQ_TO_DIV(311), // D#4
    SOUND_FREQ_TO_DIV(330), // E4
    SOUND_FREQ_TO_DIV(349), // F4
    SOUND_FREQ_TO_DIV(370), // F#4
    SOUND_FREQ_TO_DIV(392), // G4
    SOUND_FREQ_TO_DIV(415), // G#4
    SOUND_FREQ_TO_DIV(440), // A4
    SOUND_FREQ_TO_DIV(466), // A#4
    SOUND_FREQ_TO_DIV(494), // B4

    /* Octave 5 */
    SOUND_FREQ_TO_DIV(523), // C5
    SOUND_FREQ_TO_DIV(554), // C#5
    SOUND_FREQ_TO_DIV(587), // D5
    SOUND_FREQ_TO_DIV(622), // D#5
    SOUND_FREQ_TO_DIV(659), // E5
    SOUND_FREQ_TO_DIV(698), // F5
    SOUND_FREQ_TO_DIV(740), // F#5
    SOUND_FREQ_TO_DIV(784), // G5
    SOUND_FREQ_TO_DIV(830), // G#5
    SOUND_FREQ_TO_DIV(880), // A5
    SOUND_FREQ_TO_DIV(932), // A#5
    SOUND_FREQ_TO_DIV(988), // B5

    /* Octave 6 */
    SOUND_FREQ_TO_DIV(1046), // C6
    SOUND_FREQ_TO_DIV(1109), // C#6
    SOUND_FREQ_TO_DIV(1175), // D6
    SOUND_FREQ_TO_DIV(1244), // D#6
    SOUND_FREQ_TO_DIV(1319), // E6
    SOUND_FREQ_TO_DIV(1397), // F6
    SOUND_FREQ_TO_DIV(1480), // F#6
    SOUND_FREQ_TO_DIV(1568), // G6
    SOUND_FREQ_TO_DIV(1661), // G#6
    SOUND_FREQ_TO_DIV(1760), // A6
    SOUND_FREQ_TO_DIV(1865), // A#6
    SOUND_FREQ_TO_DIV(1975), // B6

    /* Octave 7 */
    SOUND_FREQ_TO_DIV(2093), // C7
    SOUND_FREQ_TO_DIV(2217), // C#7
    SOUND_FREQ_TO_DIV(2349), // D7
    SOUND_FREQ_TO_DIV(2489), // D#7
    SOUND_FREQ_TO_DIV(2637), // E7
    SOUND_FREQ_TO_DIV(2794), // F7
    SOUND_FREQ_TO_DIV(2960), // F#7
    SOUND_FREQ_TO_DIV(3136), // G7
    SOUND_FREQ_TO_DIV(3322), // G#7
    SOUND_FREQ_TO_DIV(3520), // A7
    SOUND_FREQ_TO_DIV(3729), // A#7
    SOUND_FREQ_TO_DIV(3951), // B7

    /* Octave 8 */
    SOUND_FREQ_TO_DIV(4186), // C8
    SOUND_FREQ_TO_DIV(4435), // C#8
    SOUND_FREQ_TO_DIV(4698), // D8
    SOUND_FREQ_TO_DIV(4978), // D#8
    SOUND_FREQ_TO_DIV(5274), // E8
    SOUND_FREQ_TO_DIV(5588), // F8
    SOUND_FREQ_TO_DIV(5920), // F#8
    SOUND_FREQ_TO_DIV(6272), // G8
    SOUND_FREQ_TO_DIV(6645), // G#8
    SOUND_FREQ_TO_DIV(7040), // A8
    SOUND_FREQ_TO_DIV(7459), // A#8
    SOUND_FREQ_TO_DIV(7902), // B8

    0x0000, // OFF
};

note_name_t NOTE_NAMES[] = {
    {'C', '-', '0'},
    {'C', '#', '0'},
    {'D', '-', '0'},
    {'D', '#', '0'},
    {'E', '-', '0'},
    {'F', '-', '0'},
    {'F', '#', '0'},
    {'G', '-', '0'},
    {'G', '#', '0'},
    {'A', '-', '0'},
    {'A', '#', '0'},
    {'B', '-', '0'},

    {'C', '-', '1'},
    {'C', '#', '1'},
    {'D', '-', '1'},
    {'D', '#', '1'},
    {'E', '-', '1'},
    {'F', '-', '1'},
    {'F', '#', '1'},
    {'G', '-', '1'},
    {'G', '#', '1'},
    {'A', '-', '1'},
    {'A', '#', '1'},
    {'B', '-', '1'},

    {'C', '-', '2'},
    {'C', '#', '2'},
    {'D', '-', '2'},
    {'D', '#', '2'},
    {'E', '-', '2'},
    {'F', '-', '2'},
    {'F', '#', '2'},
    {'G', '-', '2'},
    {'G', '#', '2'},
    {'A', '-', '2'},
    {'A', '#', '2'},
    {'B', '-', '2'},

    {'C', '-', '3'},
    {'C', '#', '3'},
    {'D', '-', '3'},
    {'D', '#', '3'},
    {'E', '-', '3'},
    {'F', '-', '3'},
    {'F', '#', '3'},
    {'G', '-', '3'},
    {'G', '#', '3'},
    {'A', '-', '3'},
    {'A', '#', '3'},
    {'B', '-', '3'},

    {'C', '-', '4'},
    {'C', '#', '4'},
    {'D', '-', '4'},
    {'D', '#', '4'},
    {'E', '-', '4'},
    {'F', '-', '4'},
    {'F', '#', '4'},
    {'G', '-', '4'},
    {'G', '#', '4'},
    {'A', '-', '4'},
    {'A', '#', '4'},
    {'B', '-', '4'},

    {'C', '-', '5'},
    {'C', '#', '5'},
    {'D', '-', '5'},
    {'D', '#', '5'},
    {'E', '-', '5'},
    {'F', '-', '5'},
    {'F', '#', '5'},
    {'G', '-', '5'},
    {'G', '#', '5'},
    {'A', '-', '5'},
    {'A', '#', '5'},
    {'B', '-', '5'},

    {'C', '-', '6'},
    {'C', '#', '6'},
    {'D', '-', '6'},
    {'D', '#', '6'},
    {'E', '-', '6'},
    {'F', '-', '6'},
    {'F', '#', '6'},
    {'G', '-', '6'},
    {'G', '#', '6'},
    {'A', '-', '6'},
    {'A', '#', '6'},
    {'B', '-', '6'},

    {'C', '-', '7'},
    {'C', '#', '7'},
    {'D', '-', '7'},
    {'D', '#', '7'},
    {'E', '-', '7'},
    {'F', '-', '7'},
    {'F', '#', '7'},
    {'G', '-', '7'},
    {'G', '#', '7'},
    {'A', '-', '7'},
    {'A', '#', '7'},
    {'B', '-', '7'},

    {'C', '-', '8'},
    {'C', '#', '8'},
    {'D', '-', '8'},
    {'D', '#', '8'},
    {'E', '-', '8'},
    {'F', '-', '8'},
    {'F', '#', '8'},
    {'G', '-', '8'},
    {'G', '#', '8'},
    {'A', '-', '8'},
    {'A', '#', '8'},
    {'B', '-', '8'},

    {'O', 'F', 'F'}
};
