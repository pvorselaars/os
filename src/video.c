#include "video.h"

void vga_init()
{
    // 2. Enable VGA with proper settings  
    outb(0x3C2, 0x67);  // Color mode, enable video, select clocks
    
    // 3. Reset sequencer
    outb(0x3C4, 0x00);
    outb(0x3C5, 0x01);  // Reset
    
    // 4. Sequencer setup for text mode
    outb(0x3C4, 0x01); outb(0x3C5, 0x00);  // Clocking mode
    outb(0x3C4, 0x02); outb(0x3C5, 0x03);  // Map mask (all planes)
    outb(0x3C4, 0x03); outb(0x3C5, 0x00);  // Char map select  
    outb(0x3C4, 0x04); outb(0x3C5, 0x02);  // Memory mode

    outb(0x3C4, 0x00);
    outb(0x3C5, 0x03);  // Normal operation
    
    // 5. CRTC unlock and basic setup
    outb(0x3D4, 0x11); outb(0x3D5, inb(0x3D5) & 0x7f);  // Unlock CRTC

    // Minimal CRTC timing for 80x25 text mode
    uint8_t crtc_minimal[] = {
        0x5F, 0x4F, 0x50, 0x82,  // Horizontal timing (registers 0-3)
        0x55, 0x81, 0xBF, 0x1F,  // More horizontal + vertical start (4-7)  
        0x00, 0x4F, 0x0E, 0x0F,  // Cursor and misc (8-11)
        0x00, 0x00, 0x00, 0x50,  // Start address (12-15)
        0x9C, 0x0E, 0x8F, 0x28,  // Vertical timing (16-19)
        0x1F, 0x96, 0xB9, 0xA3,  // More vertical timing (20-23) 
        0xFF                     // Line compare (24)
    };

    for(int i = 0; i < 25; i++) {
        outb(0x3D4, i);
        outb(0x3D5, crtc_minimal[i]);
    }
    
    // 6. Graphics Controller - CRITICAL for memory mapping
    uint8_t gc_regs[] = {
        0x00,  // 00: Set/reset
        0x00,  // 01: Enable set/reset
        0x00,  // 02: Color compare
        0x00,  // 03: Data rotate
        0x00,  // 04: Read map select
        0x10,  // 05: Graphics mode (FIXED: text mode, even/odd)
        0x0E,  // 06: Memory map select (0xB8000-0xBFFFF)
        0x00,  // 07: Color don't care
        0xFF   // 08: Bit mask
    };
    
    for(int i = 0; i < 9; i++) {
        outb(0x3CE, i);
        outb(0x3CF, gc_regs[i]);
    }

    // Enable access to plane 2 (font memory)
    outb(0x3C4, 0x02); outb(0x3C5, 0x04);  // Write to plane 2 only
    outb(0x3C4, 0x04); outb(0x3C5, 0x07);  // Sequential mode, extended memory
    outb(0x3CE, 0x05); outb(0x3CF, 0x00);  // Graphics mode: write mode 0
    outb(0x3CE, 0x06); outb(0x3CF, 0x05);  // Memory map: 0xA0000-0xBFFFF
    
    /*
    // Copy 8x16 font from VGA BIOS ROM to plane 2
    // VGA ROM font is typically at 0xC0000 + offset
    volatile uint8_t* font_dest = (volatile uint8_t*)0xA0000;
    volatile uint8_t* font_rom = (volatile uint8_t*)0xC0000;  // VGA ROM base
    
    // Simple built-in font data for basic ASCII chars (space through 'z')
    // This is a minimal 8x16 font - you can replace with ROM copy if available
    uint8_t basic_font[95][16] = {
        // Space (32)
        {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
        // ! (33)  
        {0x00,0x00,0x18,0x3C,0x3C,0x18,0x18,0x18,0x18,0x00,0x18,0x18,0x00,0x00,0x00,0x00},
        // " (34)
        {0x00,0x66,0x66,0x66,0x24,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
        // Skip detailed font data for brevity - using 'A' pattern for all
    };
    
    // Load a simple pattern for all printable ASCII characters
    uint8_t char_pattern[16] = {
        0x00, 0x00, 0x3C, 0x66, 0x66, 0x7E, 0x66, 0x66,
        0x66, 0x66, 0x66, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    
    // Load font for ASCII 32-126 (printable characters)
    for(int ch = 32; ch < 127; ch++) {
        for(int row = 0; row < 16; row++) {
            if(ch == 'A' || ch == 'B' || ch == 'C') {
                // Use defined pattern for letters
                font_dest[ch * 32 + row] = char_pattern[row];
            } else if(ch == ' ') {
                // Space character
                font_dest[ch * 32 + row] = 0x00;
            } else {
                // Generic pattern for other chars
                font_dest[ch * 32 + row] = (ch == row + 32) ? 0xFF : char_pattern[row % 16];
            }
        }
    }
    */
    
    // Restore normal memory access
    outb(0x3C4, 0x02); outb(0x3C5, 0x03);  // Write to planes 0,1
    outb(0x3C4, 0x04); outb(0x3C5, 0x02);  // Even/odd mode  
    outb(0x3CE, 0x05); outb(0x3CF, 0x10);  // Text mode
    outb(0x3CE, 0x06); outb(0x3CF, 0x0E);  // Memory map: 0xB8000-0xBFFFF

    // 7. Attribute controller - enable display and set basic colors
    inb(0x3DA);  // Reset flip-flop
    for(int i = 0; i < 16; i++) {
        outb(0x3C0, i);      // Select palette register i
        outb(0x3C0, i);      // Map to color i (identity mapping)
    }
    outb(0x3C0, 0x10); outb(0x3C0, 0x0C);  // Mode: text, enable line graphics
    outb(0x3C0, 0x11); outb(0x3C0, 0x00);  // Overscan: black
    outb(0x3C0, 0x12); outb(0x3C0, 0x0F);  // Color plane enable: all planes
    outb(0x3C0, 0x13); outb(0x3C0, 0x08);
    outb(0x3C0, 0x20); // CRITICAL: Enable palette AND video output 

    // 8. Set up DAC color palette (often the missing piece!)
    uint8_t colors[16][3] = {
        {0,0,0}, {0,0,42}, {0,42,0}, {0,42,42},
        {42,0,0}, {42,0,42}, {42,21,0}, {42,42,42},
        {21,21,21}, {21,21,63}, {21,63,21}, {21,63,63},
        {63,21,21}, {63,21,63}, {63,63,21}, {63,63,63}
    };
    for(int i = 0; i < 16; i++) {
        outb(0x3C8, i);           // Select DAC register
        outb(0x3C9, colors[i][0]); // Red
        outb(0x3C9, colors[i][1]); // Green  
        outb(0x3C9, colors[i][2]); // Blue
    }

    inb(0x3DA);  // Reset flip-flop
    outb(0x3C0, 0x20);  // Enable palette and video output

    volatile uint8_t* vga = (volatile uint8_t*)0xB8000;
    const char* msg = "VGA Test - Hello World!";
    for(int i = 0; msg[i] && i < 24; i++) {
        vga[i*2] = msg[i];
        vga[i*2 + 1] = 0x0F;  // Bright white on black
    }
}