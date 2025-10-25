#ifndef VGA_H
#define VGA_H

#include "arch/x86_64/io.h"
#include "arch/x86_64/memory.h"

// TODO: add bit descriptions for each register

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

#define VGA_MISC_WRITE          0x3C2
#define VGA_MISC_READ           0x3CC

// I/O port     Usage
// 0x3DA        Input Status Register 1 (read), Feature Control Register (write)
// Bits (Input Status Register 1):
// 7            Display enable
// 4            Vertical retrace
// 3            Light pen
// 0            Horizontal retrace

#define VGA_INPUT_STATUS        0x3DA

// I/O port     Usage
// 0x3C4        Sequencer Registers (index)
// 0x3C5        Sequencer Registers (data)

// Sequencer Register Indexes:
// 0            Reset
// 1            Clocking
// 2            Map Mask
// 3            Character Map Select
// 4            Memory Mode

#define VGA_SEQ_INDEX           0x3C4
#define VGA_SEQ_DATA            0x3C5

// I/O port     Usage
// 0x3D4        CRT Controller Registers (index)
// 0x3D5        CRT Controller Registers (data)
// CRTC Controller Register Indexes:
//  0           Horizontal Total             (number of character clocks per line)
//  1           Horizontal Display End       (last displayed character column)
//  2           Start Horizontal Blank       (column where blanking starts)
//  3           End Horizontal Blank         (column where blanking ends)
//  4           Start Horizontal Retrace     (column where H-sync pulse starts)
//  5           End Horizontal Retrace       (column where H-sync pulse ends + flags)
//  6           Vertical Total               (number of scan lines per frame)
//  7           Overflow                     (high bits for several vertical values)
//  8           Preset Row Scan              (row scan counter preset value)
//  9           Maximum Scan Line            (number of scan lines per character row)
// 10           Cursor Start                 (scan line where cursor starts)
// 11           Cursor End                   (scan line where cursor ends)
// 12           Start Address High           (high byte of display memory start address)
// 13           Start Address Low            (low byte of display memory start address)
// 14           Cursor Address High          (high byte of cursor position)
// 15           Cursor Address Low           (low byte of cursor position)
// 16           Vertical Retrace Start       (scan line where V-sync starts)
// 17           Vertical Retrace End         (scan line where V-sync ends + flags)
// 18           Vertical Display End         (last visible scan line)
// 19           Offset                       (bytes between rows in display memory)
// 20           Underline Location           (scan line for underline attribute)
// 21           Start Vertical Blank         (scan line where vertical blank starts)
// 22           End Vertical Blank           (scan line where vertical blank ends)
// 23           CRTC Mode Control            (various mode flags: text/graphics, etc.)
// 24           Line Compare                 (used for split-screen effects)
#define VGA_CRTC_INDEX          0x3D4
#define VGA_CRTC_DATA           0x3D5

// I/O port     Usage
// 0x3CE        Graphics Controller Registers (index)
// 0x3CF        Graphics Controller Registers (data)
// Graphics Controller Register Indexes:
// 0            Set/Reset
// 1            Enable Set/Reset
// 2            Color Compare
// 3            Data Rotate
// 4            Read Map Select
// 5            Graphics Mode
// 6            Miscellaneous
// 7            Color Don't Care
// 8            Bit Mask

#define VGA_GRAPHICS_INDEX      0x3CE
#define VGA_GRAPHICS_DATA       0x3CF

// I/O port     Usage
// 0x3C0        Attribute Controller Registers (index and data write)
// 0x3C1        Attribute Controller Registers (data read)
// Attribute Controller Register Indexes:
// 0-15         Color palette registers
// 16           Mode control
// 17           Overscan color
// 18           Color plane enable
// 19           Horizontal PEL panning
// 20           Color select
#define VGA_ATTR_INDEX          0x3C0
#define VGA_ATTR_DATA_WRITE     0x3C0
#define VGA_ATTR_DATA_READ      0x3C1

// I/O port     Usage
// 0x3C8        DAC Write Index (selects which color register to write)
// 0x3C9        DAC Data Register (write RGB values for selected color register)
//              Each color register is set by writing 3 consecutive bytes: Red, Green, Blue
#define VGA_DAC_INDEX           0x3C8
#define VGA_DAC_DATA            0x3C9

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