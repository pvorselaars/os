#include "arch/arch.h"
#include "arch/x86_64/serial.h"

/* x86_64 Serial Implementation
 * 
 * Implements the arch serial interface using x86_64-specific hardware.
 * Detects available COM ports and provides opaque handle interface.
 */

/* x86_64-specific serial device structure */
struct arch_serial_device {
    serial_port port;      // x86_64 serial port enum (COM1, COM2, etc.)
    bool initialized;      // Whether this device is initialized
};

typedef struct {
    struct arch_serial_device device;  // The actual device structure
    const char *name;                   // Device name
    bool detected;                      // Whether hardware was detected
} x86_serial_port_t;

static x86_serial_port_t x86_serial_ports[] = {
    { .device = {SERIAL_PORT_0, false}, .name = "serial0", .detected = false }
    // Only SERIAL_PORT_0 is defined in the header currently
    // Could add more COM ports here when they're defined
};

#define X86_SERIAL_PORT_COUNT (sizeof(x86_serial_ports) / sizeof(x86_serial_ports[0]))

static bool serial_ports_detected = false;

/* Detect which serial ports actually exist */
static void detect_serial_ports(void)
{
    if (serial_ports_detected) return;
    
    // Initialize x86_64 serial subsystem first
    x86_64_serial_init();
    
    // For now, assume COM1 (SERIAL_PORT_0) always exists
    // In a full implementation, we'd probe each port
    x86_serial_ports[0].detected = true;
    
    // Could add detection logic for COM2, COM3, COM4 here
    
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
    
    return ARCH_ERROR; // Index out of range
}

arch_result arch_serial_init(arch_serial_device_t *device)
{
    if (!device) return ARCH_ERROR;
    
    // The x86_64_serial_init() was already called in detect_serial_ports()
    // Just mark this device as initialized
    device->initialized = true;
    return ARCH_OK;
}

int arch_serial_write(arch_serial_device_t *device, const void *buf, size_t len)
{
    if (!device || !device->initialized) return -1;
    
    const char *str = (const char *)buf;
    
    for (size_t i = 0; i < len; i++) {
        x86_64_serial_write(device->port, str[i]);
    }
    
    return (int)len;
}

int arch_serial_read(arch_serial_device_t *device, void *buf, size_t len)
{
    if (!device || !device->initialized) return -1;
    
    // For now, just return 0 (no data available)
    // In a full implementation, this would use x86_64_serial_read(device->port)
    return 0;
}

bool arch_serial_data_available(arch_serial_device_t *device)
{
    if (!device || !device->initialized) return false;
    
    // For now, always return false
    // In a full implementation, this would check device->port status
    return false;
}