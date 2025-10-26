#include "arch/arch.h"
#include "arch/x86_64/serial.h"

/* x86_64 Serial Implementation
 * 
 * Implements the arch serial interface using x86_64-specific hardware.
 * Provides the arch_serial_* functions for the generic serial device.
 */

arch_result arch_serial_init(void)
{
    x86_64_serial_init();
    return ARCH_OK;
}

int arch_serial_write(const void *buf, size_t len)
{
    const char *str = (const char *)buf;
    
    for (size_t i = 0; i < len; i++) {
        x86_64_serial_write(SERIAL_PORT_0, str[i]);
    }
    
    return (int)len;
}

int arch_serial_read(void *buf, size_t len)
{
    // For now, just return 0 (no data available)
    // In a full implementation, this would check for incoming data
    // using x86_64_serial_read() or similar
    return 0;
}

bool arch_serial_data_available(void)
{
    // For now, always return false
    // In a full implementation, this would check serial port status
    return false;
}