#include "arch/x86_64/io.h"
#include "definitions.h"

// x86_64 PIT (Programmable Interval Timer) management
// Intel 8253/8254 compatible timer chip

#define PIT_CHANNEL_0   0x40    // Channel 0 data port
#define PIT_CHANNEL_1   0x41    // Channel 1 data port  
#define PIT_CHANNEL_2   0x42    // Channel 2 data port
#define PIT_COMMAND     0x43    // Mode/Command register

// PIT frequency: ~1.193182 MHz
#define PIT_FREQUENCY   1193182

// Command register bits
#define PIT_CHANNEL_0_SELECT    0x00    // Select channel 0
#define PIT_CHANNEL_1_SELECT    0x40    // Select channel 1
#define PIT_CHANNEL_2_SELECT    0x80    // Select channel 2

#define PIT_ACCESS_LATCH        0x00    // Latch count value
#define PIT_ACCESS_LOW          0x10    // Low byte only
#define PIT_ACCESS_HIGH         0x20    // High byte only  
#define PIT_ACCESS_BOTH         0x30    // Low byte then high byte

#define PIT_MODE_0              0x00    // Interrupt on terminal count
#define PIT_MODE_1              0x02    // Hardware re-triggerable one-shot
#define PIT_MODE_2              0x04    // Rate generator
#define PIT_MODE_3              0x06    // Square wave generator
#define PIT_MODE_4              0x08    // Software triggered strobe
#define PIT_MODE_5              0x0A    // Hardware triggered strobe

#define PIT_BINARY              0x00    // Binary mode
#define PIT_BCD                 0x01    // BCD mode

void x86_64_pit_init(unsigned int frequency_hz)
{
    // Calculate divisor for desired frequency
    uint32_t divisor = PIT_FREQUENCY / frequency_hz;
    
    // Limit divisor to 16-bit range
    if (divisor > 0xFFFF) {
        divisor = 0xFFFF;
    }
    if (divisor < 1) {
        divisor = 1;
    }
    
    // Configure PIT Channel 0:
    // - Channel 0 select
    // - Access both low and high byte
    // - Mode 2 (rate generator) - generates periodic interrupts
    // - Binary mode
    uint8_t command = PIT_CHANNEL_0_SELECT | PIT_ACCESS_BOTH | PIT_MODE_2 | PIT_BINARY;
    
    outb(PIT_COMMAND, command);
    
    // Send divisor (low byte first, then high byte)
    outb(PIT_CHANNEL_0, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL_0, (uint8_t)((divisor >> 8) & 0xFF));
}

void x86_64_pit_disable(void)
{
    // Set channel 0 to one-shot mode with maximum count (effectively disables periodic interrupts)
    outb(PIT_COMMAND, PIT_CHANNEL_0_SELECT | PIT_ACCESS_BOTH | PIT_MODE_1 | PIT_BINARY);
    outb(PIT_CHANNEL_0, 0xFF);
    outb(PIT_CHANNEL_0, 0xFF);
}

uint16_t x86_64_pit_read_count(void)
{
    // Latch current count
    outb(PIT_COMMAND, PIT_CHANNEL_0_SELECT | PIT_ACCESS_LATCH);
    
    // Read latched count (low byte first, then high byte)
    uint8_t low = inb(PIT_CHANNEL_0);
    uint8_t high = inb(PIT_CHANNEL_0);
    
    return ((uint16_t)high << 8) | low;
}