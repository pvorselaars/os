#include "kernel/device.h"
#include "arch/arch.h"
#include "lib/string.h"

/* Generic Serial Driver
 * 
 * Provides a character device interface for serial communication.
 * Uses the arch layer for hardware-specific operations.
 */

/* Serial driver state */
typedef struct {
    uint32_t port_id;
    bool initialized;
} serial_driver_data_t;

/* Forward declarations */
static arch_result serial_open(device_t *dev);
static arch_result serial_close(device_t *dev);
static int serial_read(device_t *dev, void *buf, size_t len);
static int serial_write(device_t *dev, const void *buf, size_t len);
static arch_result serial_flush(device_t *dev);

/* Serial device instances */
static serial_driver_data_t serial0_data = {
    .port_id = 0,
    .initialized = false
};

static device_t serial0_device = {
    .name = "serial0",
    .class = DEVICE_CLASS_CHAR,
    .state = DEVICE_STATE_UNINITIALIZED,
    .open = serial_open,
    .close = serial_close,
    .char_ops = {
        .read = serial_read,
        .write = serial_write,
        .flush = serial_flush
    },
    .driver_data = &serial0_data
};

static arch_result serial_open(device_t *dev)
{
    serial_driver_data_t *data = (serial_driver_data_t *)dev->driver_data;
    
    if (!data->initialized) {
        // Initialize the serial port through arch layer
        arch_result result = arch_serial_init();
        if (result != ARCH_OK) {
            return result;
        }
        data->initialized = true;
    }
    
    return ARCH_OK;
}

static arch_result serial_close(device_t *dev)
{
    // Serial port doesn't need special cleanup
    return ARCH_OK;
}

static int serial_read(device_t *dev, void *buf, size_t len)
{
    // Use arch layer to read from serial port
    return arch_serial_read(buf, len);
}

static int serial_write(device_t *dev, const void *buf, size_t len)
{
    // Use arch layer to write to serial port
    return arch_serial_write(buf, len);
}

static arch_result serial_flush(device_t *dev)
{
    // Serial ports on x86 don't need explicit flushing
    return ARCH_OK;
}

/* Driver initialization function */
arch_result serial_driver_init(void)
{
    return device_register(&serial0_device);
}