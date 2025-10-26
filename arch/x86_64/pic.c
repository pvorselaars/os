#include "arch/x86_64/io.h"
#include "definitions.h"

// x86_64 PIC (Programmable Interrupt Controller) management

#define PIC1                0x20    // Master PIC
#define PIC2                0xA0    // Slave PIC  
#define PIC1_COMMAND        PIC1
#define PIC1_DATA          (PIC1 + 1)
#define PIC2_COMMAND        PIC2
#define PIC2_DATA          (PIC2 + 1)

#define PIC_EOI             0x20    // End-of-interrupt command

#define ICW1_ICW4           0x01    // ICW4 (not) needed
#define ICW1_SINGLE         0x02    // Single (cascade) mode
#define ICW1_INTERVAL4      0x04    // Call address interval 4 (8)
#define ICW1_LEVEL          0x08    // Level triggered (edge) mode
#define ICW1_INIT           0x10    // Initialization - required!

#define ICW4_8086           0x01    // 8086/88 (MCS-80/85) mode
#define ICW4_AUTO           0x02    // Auto (normal) EOI
#define ICW4_BUF_SLAVE      0x08    // Buffered mode/slave
#define ICW4_BUF_MASTER     0x0C    // Buffered mode/master
#define ICW4_SFNM           0x10    // Special fully nested (not)

void x86_64_pic_remap(void)
{
    
    // Start initialization sequence (in cascade mode)
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    
    // Set vector offsets (remap to 0x20-0x2F instead of 0x08-0x0F)
    outb(PIC1_DATA, 0x20);  // Master PIC vector offset
    outb(PIC2_DATA, 0x28);  // Slave PIC vector offset
    
    // Tell master PIC that there is a slave PIC at IRQ2
    outb(PIC1_DATA, 0x04);
    // Tell slave PIC its cascade identity
    outb(PIC2_DATA, 0x02);
    
    // Set to 8086/88 mode
    outb(PIC1_DATA, ICW4_8086);
    outb(PIC2_DATA, ICW4_8086);
    
    // Restore masks (or set conservative defaults)
    outb(PIC1_DATA, 0b11111000);  // Enable IRQ 0,1,2 (timer, keyboard, cascade)
    outb(PIC2_DATA, 0b11101111);  // Enable IRQ 12 (mouse) on slave PIC
}

void x86_64_pic_eoi(unsigned int irq)
{
    if (irq >= 8) {
        outb(PIC2_COMMAND, PIC_EOI);  // Send EOI to slave PIC
    }
    outb(PIC1_COMMAND, PIC_EOI);      // Send EOI to master PIC
}

void x86_64_pic_mask_irq(unsigned int irq)
{
    uint16_t port;
    
    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }
    
    uint8_t value = inb(port) | (1 << irq);
    outb(port, value);
}

void x86_64_pic_unmask_irq(unsigned int irq)
{
    uint16_t port;
    
    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;  
        irq -= 8;
    }
    
    uint8_t value = inb(port) & ~(1 << irq);
    outb(port, value);
}