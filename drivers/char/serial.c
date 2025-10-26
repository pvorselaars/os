#include "kernel/device.h"
#include "arch/arch.h"
#include "lib/string.h"

/* Generic Serial Driver
 * 
 * Provides character device interface for serial communication.
 * Uses arch layer for hardware-specific operations.
 * 
 * This driver is truly arch-independent - it asks the arch layer
 * how many serial devices exist and creates devices accordingly.
 */

/* Forward declarations */
static arch_result serial_open(device_t *dev);
static arch_result serial_close(device_t *dev);
static int serial_read(device_t *dev, void *buf, size_t len);
static int serial_write(device_t *dev, const void *buf, size_t len);
static arch_result serial_flush(device_t *dev);

/* Serial driver state */
typedef struct {
    arch_serial_device_t *arch_device;  // Opaque arch-specific device handle
} serial_driver_data_t;

/* Dynamic device storage - allocated based on what arch reports */
static device_t *serial_devices = NULL;
static serial_driver_data_t *serial_data = NULL;
static int serial_device_count = 0;

static arch_result serial_open(device_t *dev)
{
    serial_driver_data_t *data = (serial_driver_data_t *)dev->driver_data;
    
    // Initialize this specific serial device through arch layer
    arch_result result = arch_serial_init(data->arch_device);
    if (result != ARCH_OK) {
        return result;
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
    serial_driver_data_t *data = (serial_driver_data_t *)dev->driver_data;
    
    // Use arch layer to read from specific serial device
    return arch_serial_read(data->arch_device, buf, len);
}

static int serial_write(device_t *dev, const void *buf, size_t len)
{
    serial_driver_data_t *data = (serial_driver_data_t *)dev->driver_data;
    
    // Use arch layer to write to specific serial device
    return arch_serial_write(data->arch_device, buf, len);
}

static arch_result serial_flush(device_t *dev)
{
    // Serial ports don't need explicit flushing in our implementation
    return ARCH_OK;
}

/* Driver initialization function - truly arch-independent! */
arch_result serial_driver_init(void)
{
    // Ask arch layer how many serial devices exist
    serial_device_count = arch_serial_get_count();
    
    if (serial_device_count == 0) {
        // No serial devices available on this architecture
        return ARCH_OK;
    }
    
    // Allocate storage for devices and data (in real kernel, use kmalloc)
    // For now, we'll use a simple static approach with max devices
    #define MAX_SERIAL_DEVICES 4
    static device_t static_devices[MAX_SERIAL_DEVICES];
    static serial_driver_data_t static_data[MAX_SERIAL_DEVICES];
    
    if (serial_device_count > MAX_SERIAL_DEVICES) {
        serial_device_count = MAX_SERIAL_DEVICES;
    }
    
    serial_devices = static_devices;
    serial_data = static_data;
    
    // Create and register a device for each serial port the arch reports
    for (int i = 0; i < serial_device_count; i++) {
        arch_serial_info_t info;
        arch_result result = arch_serial_get_info(i, &info);
        if (result != ARCH_OK) {
            continue;
        }
        
        // Initialize device structure
        device_t *device = &serial_devices[i];
        serial_driver_data_t *data = &serial_data[i];
        
        // Copy the arch-suggested name
        strncpy(device->name, info.name, sizeof(device->name) - 1);
        device->name[sizeof(device->name) - 1] = '\0';
        
        device->class = DEVICE_CLASS_CHAR;
        device->state = DEVICE_STATE_UNINITIALIZED;
        device->open = serial_open;
        device->close = serial_close;
        device->char_ops.read = serial_read;
        device->char_ops.write = serial_write;
        device->char_ops.flush = serial_flush;
        device->driver_data = data;
        device->next = NULL;
        
        // Initialize driver data
        data->arch_device = info.device;
        
        // Register the device
        result = device_register(device);
        if (result != ARCH_OK) {
            return result;
        }
    }
    
    return ARCH_OK;
}