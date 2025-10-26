#include "arch/arch.h"
#include "board/pc/serial.h"

struct arch_serial_device {
    serial_port port;
    bool initialized;
};

typedef struct {
    struct arch_serial_device device;
    const char *name;
    bool detected;
} x86_serial_port_t;

static x86_serial_port_t x86_serial_ports[] = {
    { .device = {SERIAL_PORT_0, false}, .name = "serial0", .detected = false }
    // TODO: add more ports
};

#define X86_SERIAL_PORT_COUNT (sizeof(x86_serial_ports) / sizeof(x86_serial_ports[0]))

static bool serial_ports_detected = false;
static uint8_t buffer[SERIAL_BUFFER_SIZE];
static uint8_t buffer_index = 0;

static void detect_serial_ports(void)
{
    if (serial_ports_detected) return;
    
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
    
    x86_serial_ports[0].detected = true;
    
    serial_ports_detected = true;
}

int arch_serial_get_count(void)
{
    detect_serial_ports();
    
    int count = 0;
    for (int i = 0; i < X86_SERIAL_PORT_COUNT; i++) {
        if (x86_serial_ports[i].detected) {
            count++;
        }
    }
    return count;
}

arch_result arch_serial_get_info(int index, arch_serial_info_t *info)
{
    if (!info) return ARCH_ERROR;
    
    detect_serial_ports();
    
    int found_count = 0;
    for (int i = 0; i < X86_SERIAL_PORT_COUNT; i++) {
        if (x86_serial_ports[i].detected) {
            if (found_count == index) {
                info->device = &x86_serial_ports[i].device;
                info->name = x86_serial_ports[i].name;
                return ARCH_OK;
            }
            found_count++;
        }
    }
    
    return ARCH_ERROR;
}

arch_result arch_serial_init(arch_serial_device_t *device)
{
    if (!device) return ARCH_ERROR;
    
    device->initialized = true;
    return ARCH_OK;
}

int arch_serial_write(arch_serial_device_t *device, const void *buf, size_t len)
{
    if (!device || !device->initialized) return -1;
    
    const char *str = (const char *)buf;
    
    for (size_t i = 0; i < len; i++) {
        // wait for transmit buffer to be empty
        while (!(inb(device->port+5) & 0x40));

        outb(device->port, str[i]);
    }
    
    return (int)len;
}

int arch_serial_read(arch_serial_device_t *device, void *buf, size_t len)
{
    // wait for buffer content
    while(!buffer_index)
        arch_halt();

    return buffer[--buffer_index];
}

bool arch_serial_data_available(arch_serial_device_t *device)
{
    if (!device || !device->initialized) return false;

    // TODO: Check if data is available in the receive buffer
    
    return false;
}