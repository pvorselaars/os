#include "arch/x86_64/io.h"
#include "board/pc/serial.h"
#include "kernel/device.h"

#define PC_UART_DATA 0
#define PC_UART_IER  1
#define PC_UART_FCR  2
#define PC_UART_LCR  3
#define PC_UART_MCR  4
#define PC_UART_LSR  5

#define PC_UART_LSR_DATA_READY 0x01
#define PC_UART_LSR_TX_EMPTY   0x20

typedef struct {
    uint16_t base;
    bool initialized;
} pc_serial_t;

static pc_serial_t pc_serial0 = {
    .base = PC_SERIAL_PORT_0,
};

static result_t pc_serial_init(device_t *device)
{
    pc_serial_t *serial = device->data;

    if (serial->initialized) {
        return RESULT_OK;
    }

    outb(serial->base + PC_UART_IER, 0x00);
    outb(serial->base + PC_UART_LCR, 0x80);
    outb(serial->base + PC_UART_DATA, 0x03);
    outb(serial->base + PC_UART_IER, 0x00);
    outb(serial->base + PC_UART_LCR, 0x03);
    outb(serial->base + PC_UART_FCR, 0xC7);
    outb(serial->base + PC_UART_MCR, 0x0F);

    serial->initialized = true;
    return RESULT_OK;
}

static int pc_serial_write_bytes(
    pc_serial_t *serial,
    const void *buffer,
    size_t length
) {
    const uint8_t *bytes = buffer;

    for (size_t i = 0; i < length; i++) {
        while (!(inb(serial->base + PC_UART_LSR) & PC_UART_LSR_TX_EMPTY))
            ;

        outb(serial->base + PC_UART_DATA, bytes[i]);
    }

    return (int)length;
}

static int pc_serial_read(device_t *device, void *buffer, size_t length)
{
    pc_serial_t *serial = device->data;
    uint8_t *bytes = buffer;
    size_t count = 0;

    if (!serial->initialized || !buffer)
        return -1;

    while (count < length &&
           (inb(serial->base + PC_UART_LSR) & PC_UART_LSR_DATA_READY)) {
        bytes[count++] = inb(serial->base + PC_UART_DATA);
    }

    return (int)count;
}

static int pc_serial_write(
    device_t *device,
    const void *buffer,
    size_t length
) {
    pc_serial_t *serial = device->data;
    if (!serial->initialized || !buffer)
        return -1;

    return pc_serial_write_bytes(serial, buffer, length);
}

static result_t pc_serial_flush(device_t *device)
{
    (void)device;
    return RESULT_OK;
}

static device_t pc_serial0_device = {
    .name = "serial0",
    .class = DEVICE_CLASS_CHAR,
    .init = pc_serial_init,
    .char_ops = {
        .read = pc_serial_read,
        .write = pc_serial_write,
        .flush = pc_serial_flush,
    },
    .data = &pc_serial0,
};

result_t pc_serial_register(void)
{
    return device_register(&pc_serial0_device);
}

void pc_serial_write_early(const char *buffer, size_t length)
{
    if (pc_serial_init(&pc_serial0_device) == RESULT_OK) {
        pc_serial_write_bytes(&pc_serial0, buffer, length);
    }
}