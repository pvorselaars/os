#include "platform/pc/audio.h"

static void play_sound(uint32_t frequency)
{
    uint32_t divisor;
    uint8_t tmp;

    divisor = 1193180 / frequency;
    outb(PIT_COMMAND, 0xb6);
    outb(PIT_CHANNEL_2, (uint8_t) (divisor));
    outb(PIT_CHANNEL_2, (uint8_t) (divisor >> 8));

    tmp = inb(0x61);
    if (tmp != (tmp | 3)) {
        outb(0x61, tmp | 3);
    }
}

static void stop_sound()
{
    uint8_t tmp = inb(0x61) & 0xFC;
    outb(0x61, tmp);
}

void beep()
{
    play_sound(256);
    sleep(50);
    stop_sound();
}