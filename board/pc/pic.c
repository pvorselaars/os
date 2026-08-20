#include "arch/x86_64/io.h"
#include "definitions.h"

#define PIC1                0x20    // Master PIC
#define PIC2                0xA0    // Slave PIC  
#define PIC1_COMMAND        PIC1
#define PIC1_DATA          (PIC1 + 1)
#define PIC2_COMMAND        PIC2
#define PIC2_DATA          (PIC2 + 1)

#define ICW1_ICW4           0x01    // ICW4 (not) needed
#define ICW1_INIT           0x10    // Initialization - required!

#define ICW4_8086           0x01    // 8086/88 (MCS-80/85) mode

static void pc_pic_remap(void)
{
    
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    
    outb(PIC1_DATA, 0x20);  // Master PIC vector offset
    outb(PIC2_DATA, 0x28);  // Slave PIC vector offset
    
    outb(PIC1_DATA, 0x04);
    outb(PIC2_DATA, 0x02);
    
    outb(PIC1_DATA, ICW4_8086);
    outb(PIC2_DATA, ICW4_8086);
    
    outb(PIC1_DATA, 0xFC);
    outb(PIC2_DATA, 0xFF);
}

void pc_pic_init(void)
{
    pc_pic_remap();
}