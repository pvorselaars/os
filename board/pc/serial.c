#include "board/pc/serial.h"

#include "board/pc/io.h"
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

static result_t pc_serial_initialize(pc_serial_t *serial)
{
    if (serial->initialized) {
        return RESULT_OK;
    }

    pc_io_write8(serial->base + PC_UART_IER, 0x00);
    pc_io_write8(serial->base + PC_UART_LCR, 0x80);
    pc_io_write8(serial->base + PC_UART_DATA, 0x03);
    pc_io_write8(serial->base + PC_UART_IER, 0x00);
    pc_io_write8(serial->base + PC_UART_LCR, 0x03);
    pc_io_write8(serial->base + PC_UART_FCR, 0xC7);
    pc_io_write8(serial->base + PC_UART_MCR, 0x0F);

    serial->initialized = true;
    return RESULT_OK;
}

static result_t pc_serial_init(device_t *device)
{
    return pc_serial_initialize(device_data(device));
}

static int pc_serial_write_bytes(
    pc_serial_t *serial,
    const void *buffer,
    size_t length
) {
    const uint8_t *bytes = buffer;

    for (size_t i = 0; i < length; i++) {
        while (!(pc_io_read8(serial->base + PC_UART_LSR) & PC_UART_LSR_TX_EMPTY))
            ;

        pc_io_write8(serial->base + PC_UART_DATA, bytes[i]);
    }

    return (int)length;
}

static int pc_serial_read(device_t *device, void *buffer, size_t length)
{
    pc_serial_t *serial = device_data(device);
    uint8_t *bytes = buffer;
    size_t count = 0;

    if (!serial->initialized || !buffer)
        return -1;

    while (count < length &&
           pc_io_read8(serial->base + PC_UART_LSR) & PC_UART_LSR_DATA_READY) {
        bytes[count++] = pc_io_read8(serial->base + PC_UART_DATA);
    }

    return (int)count;
}

static int pc_serial_write(
    device_t *device,
    const void *buffer,
    size_t length
) {
    pc_serial_t *serial = device_data(device);
    if (!serial->initialized || !buffer)
        return -1;

    return pc_serial_write_bytes(serial, buffer, length);
}

static result_t pc_serial_flush(device_t *device)
{
    (void)device;
    return RESULT_OK;
}

static const device_descriptor_t pc_serial0_device = {
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
    if (pc_serial_initialize(&pc_serial0) == RESULT_OK) {
        pc_serial_write_bytes(&pc_serial0, buffer, length);
    }
}