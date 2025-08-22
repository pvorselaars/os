#include "serial.h"

void serial_init()
{
    outb(SERIAL_PORT_0 + 1, 0x00); // Disable interrupts

    outb(SERIAL_PORT_0 + 3, 0x80); // Set DLAB
    outb(SERIAL_PORT_0 + 0, 0x03); // Set divisor low byte (115200 / 3 = 38400 baud)
    outb(SERIAL_PORT_0 + 1, 0x00); // Set divisor high byte

    outb(SERIAL_PORT_0 + 3, 0x03); // 8 bits, one stop bit, no parity
    outb(SERIAL_PORT_0 + 2, 0xC7); // Enable and clear 14 byte FIFO
    outb(SERIAL_PORT_0 + 4, 0x1E); // Set in loopback mode for testing

    outb(SERIAL_PORT_0 + 0, 0xAE); // Send test byte

    assert(serial_read(SERIAL_PORT_0) == 0xAE);

    outb(SERIAL_PORT_0 + 4, 0x0F); // Disable loopback mode
    outb(SERIAL_PORT_0 + 1, 0x01); // Enable interrupts
}

uint8_t serial_read(serial_ports port)
{
    while (!(inb(port+5) & 0x01));

    return inb(port);
}

void serial_receive()
{
    uint8_t iir = inb(SERIAL_PORT_0 + 2);

    while(!(iir & 0x01)) {
        uint8_t data = inb(SERIAL_PORT_0);

        outb(SERIAL_PORT_0, data); // echo received data
        if (data == '\r')
            outb(SERIAL_PORT_0, '\n');

        iir = inb(SERIAL_PORT_0 + 2);
    }
}