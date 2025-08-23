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

// General
#define VGA_MISC_WRITE          0x3C2
#define VGA_MISC_READ           0x3CC
#define VGA_INPUT_STATUS        0x3DA
#define VGA_FEATURE_WRITE       0x3DA
#define VGA_FEATURE_READ        0x3CA

// Sequencer
#define VGA_SEQ_INDEX           0x3C4
#define VGA_SEQ_DATA            0x3C5

// CRTC (CRT Controller)
#define VGA_CRTC_INDEX          0x3D4
#define VGA_CRTC_DATA           0x3D5

// Graphics Controller
#define VGA_GRAPHICS_INDEX      0x3CE
#define VGA_GRAPHICS_DATA       0x3CF

// Attribute Controller
#define VGA_ATTR_INDEX          0x3C0
#define VGA_ATTR_DATA_WRITE     0x3C0
#define VGA_ATTR_DATA_READ      0x3C1

// Color definitions for text mode
#define VGA_COLOR_BLACK         0x0
#define VGA_COLOR_BLUE          0x1
#define VGA_COLOR_GREEN         0x2
#define VGA_COLOR_CYAN          0x3
#define VGA_COLOR_RED           0x4
#define VGA_COLOR_MAGENTA       0x5
#define VGA_COLOR_BROWN         0x6
#define VGA_COLOR_LIGHT_GREY    0x7
#define VGA_COLOR_DARK_GREY     0x8
#define VGA_COLOR_LIGHT_BLUE    0x9
#define VGA_COLOR_LIGHT_GREEN   0xA
#define VGA_COLOR_LIGHT_CYAN    0xB
#define VGA_COLOR_LIGHT_RED     0xC
#define VGA_COLOR_LIGHT_MAGENTA 0xD
#define VGA_COLOR_LIGHT_BROWN   0xE
#define VGA_COLOR_WHITE         0xF

void vga_init();

#endif