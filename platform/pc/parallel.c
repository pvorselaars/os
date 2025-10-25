#include "platform/pc/parallel.h"

void parallel_write(parallel_port port, uint8_t data)
{
    // wait for receiver to not be busy
    while (!(inb(port + 1) & 0x80))
        sleep(10);
    
    // set data bits
    outb(port, data);

    // strobe
    uint8_t control = inb(port + 2);
    outb(port + 2, control | 1);
    sleep(10);
    outb(port + 2, control);

}