#include "platform/pc/serial.h"

static uint8_t buffer[SERIAL_BUFFER_SIZE];
static uint8_t buffer_index = 0;

void serial_init()
{
    // TODO: support more ports
    outb(SERIAL_PORT_0 + 1, 0x00); // Disable interrupts

    outb(SERIAL_PORT_0 + 3, 0x80); // Set DLAB
    outb(SERIAL_PORT_0 + 0, 0x03); // Set divisor low byte (115200 / 3 = 38400 baud)
    outb(SERIAL_PORT_0 + 1, 0x00); // Set divisor high byte

    outb(SERIAL_PORT_0 + 3, 0x03); // 8 bits, one stop bit, no parity
    outb(SERIAL_PORT_0 + 2, 0xC7); // Enable and clear 14 byte FIFO
    outb(SERIAL_PORT_0 + 4, 0x1E); // Set in loopback mode for testing

    outb(SERIAL_PORT_0 + 0, 0xAE); // Send test byte

    while (!(inb(SERIAL_PORT_0+5) & 0x01));
    assert(inb(SERIAL_PORT_0) == 0xAE);

    outb(SERIAL_PORT_0 + 4, 0x0F); // Disable loopback mode
    outb(SERIAL_PORT_0 + 1, 0x01); // Enable interrupts
}

uint8_t serial_read()
{
    // wait for buffer content
    while(!buffer_index)
        platform_halt();

    return buffer[--buffer_index];
}

void serial_write(serial_port port, uint8_t data)
{
    // wait for transmit buffer to be empty
    while (!(inb(port+5) & 0x40));

    outb(SERIAL_PORT_0, data);
}

static void write_buffer(uint8_t data)
{
    buffer[buffer_index++] = data;
    if (buffer_index > SERIAL_BUFFER_SIZE - 1)
        buffer_index = 0;
}

void serial_receive_interrupt()
{
    uint8_t iir = inb(SERIAL_PORT_0 + 2);

    while(!(iir & 0x01)) {
        uint8_t data = inb(SERIAL_PORT_0);

        serial_write(SERIAL_PORT_0, data); // echo received data
        write_buffer(data);
        if (data == '\r') {
            serial_write(SERIAL_PORT_0, '\n');
            write_buffer('\n');
        }

        iir = inb(SERIAL_PORT_0 + 2);
    }
}

void serial_output_func(const char *str, uint64_t len)
{
    for (uint64_t i = 0; i < len; i++) {
        serial_write(SERIAL_PORT_0, (uint8_t)str[i]);
    }
}