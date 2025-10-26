#include "arch/arch.h"
#include "arch/x86_64/io.h"
#include "lib/utils.h"

#define LPT1_BASE 0x378
#define LPT2_BASE 0x278
#define LPT3_BASE 0x3BC

struct arch_parallel_device {
    uint16_t base_port;
    bool initialized;
};

typedef struct {
    struct arch_parallel_device device;  // The actual device structure
    const char *name;                     // Device name
    bool detected;                        // Whether hardware was detected
} x86_parallel_port_t;

static x86_parallel_port_t x86_parallel_ports[] = {
    { .device = {LPT1_BASE, false}, .name = "parallel0", .detected = false },
    { .device = {LPT2_BASE, false}, .name = "parallel1", .detected = false },
    { .device = {LPT3_BASE, false}, .name = "parallel2", .detected = false }
};

#define X86_PARALLEL_PORT_COUNT (sizeof(x86_parallel_ports) / sizeof(x86_parallel_ports[0]))

static bool parallel_ports_detected = false;

static void detect_parallel_ports(void)
{
    if (parallel_ports_detected) return;
    
    for (int i = 0; i < X86_PARALLEL_PORT_COUNT; i++) {
        uint16_t base = x86_parallel_ports[i].device.base_port;
        
        // Simple detection: try to read status register
        // Real ports should have some bits readable
        uint8_t status = inb(base + 1);
        
        if (status != 0xFF) {
            x86_parallel_ports[i].detected = true;
        }
    }
    
    parallel_ports_detected = true;
}

int arch_parallel_get_count(void)
{
    detect_parallel_ports();
    
    int count = 0;
    for (int i = 0; i < X86_PARALLEL_PORT_COUNT; i++) {
        if (x86_parallel_ports[i].detected) {
            count++;
        }
    }
    return count;
}

arch_result arch_parallel_get_info(int index, arch_parallel_info_t *info)
{
    if (!info) return ARCH_ERROR;
    
    detect_parallel_ports();
    
    int found_count = 0;
    for (int i = 0; i < X86_PARALLEL_PORT_COUNT; i++) {
        if (x86_parallel_ports[i].detected) {
            if (found_count == index) {
                info->device = &x86_parallel_ports[i].device;
                info->name = x86_parallel_ports[i].name;
                return ARCH_OK;
            }
            found_count++;
        }
    }
    
    return ARCH_ERROR;
}

arch_result arch_parallel_init(arch_parallel_device_t *device)
{
    if (!device) return ARCH_ERROR;
    
    // Initialize specific parallel port
    uint16_t base_port = device->base_port;
    outb(base_port + 2, 0x04); // Control register - initialize
    device->initialized = true;
    return ARCH_OK;
}

int arch_parallel_write(arch_parallel_device_t *device, const void *buf, size_t len)
{
    if (!device || !device->initialized) return -1;
    
    const uint8_t *data = (const uint8_t *)buf;
    uint16_t port = device->base_port;
    
    for (size_t i = 0; i < len; i++) {

        // wait for receiver to not be busy
        while (!(inb(port + 1) & 0x80))
            sleep(10);
    
        // set data bits
        outb(port, data[i]);

        // strobe
        uint8_t control = inb(port + 2);
        outb(port + 2, control | 1);
        sleep(10);
        outb(port + 2, control);
    }
    
    return len;
}