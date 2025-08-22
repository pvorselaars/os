#ifndef VIDEO_H
#define VIDEO_H

#include "io.h"
#include "utils.h"

// I/O port     Usage
// 0x3C2        Miscellaneous Output Register (write)
// 0x3CC        Miscellaneous Output Register (read)

// Bits         Usage
// 7            V-sync polarity (1 = negative)
// 6            H-sync polarity (1 = negative)
// 5            High 64K video memory bank if in odd/even
// 4            Reserved
// 3 and 2      Clock select (01 = 28 Mhz, 00 = 25 Mhz)
// 1            CPU access to video memory
// 0            Color mode

// I/O port     Usage
// 0x3C4        Sequencer Registers (index)
// 0x3C5        Sequencer Registers (data)

// Index 0      Reset

// Bits         Usage
// 1            Synchronous reset
// 0            Asynchronous reset

// Index 1      Clocking

// Bits         Usage
// 5            Screen disable
// 4            Shift four enable
// 3            Dot clock rate
// 2            Shift/load rate
// 0            9/8 dot mode

#define VGA_MISC_WRITE      0x3C2
#define VGA_SEQUENCER_INDEX 0x3C4
#define VGA_SEQUENCER_DATA  0x3C5
#define VGA_CRTC_INDEX      0x3D4
#define VGA_CRTC_DATA       0x3D5
#define VGA_GRAPHICS_INDEX  0x3CE
#define VGA_GRAPHICS_DATA   0x3CF
#define VGA_ATTR_INDEX      0x3C0
#define VGA_ATTR_READ       0x3C1
#define VGA_INPUT_STATUS    0x3DA

void vga_init();

#endif