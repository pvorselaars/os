#include "kernel/device.h"
#include "arch/arch.h"
#include "lib/string.h"

/* Generic Parallel Port Driver
 * 
 * Provides character device interface for parallel port communication.
 * Uses arch layer for hardware-specific operations.
 * 
 * This driver is truly arch-independent - it asks the arch layer
 * how many parallel ports exist and creates devices accordingly.
 */

/* Forward declarations */
static arch_result parallel_open(device_t *dev);
static arch_result parallel_close(device_t *dev);
static int parallel_write(device_t *dev, const void *buf, size_t len);

/* Parallel driver state */
typedef struct {
    arch_parallel_device_t *arch_device;  // Opaque arch-specific device handle
} parallel_driver_data_t;

/* Dynamic device storage - allocated based on what arch reports */
static device_t *parallel_devices = NULL;
static parallel_driver_data_t *parallel_data = NULL;
static int parallel_device_count = 0;

static arch_result parallel_open(device_t *dev)
{
    parallel_driver_data_t *data = (parallel_driver_data_t *)dev->driver_data;
    
    // Initialize this specific parallel device through arch layer
    arch_result result = arch_parallel_init(data->arch_device);
    if (result != ARCH_OK) {
        return result;
    }
    
    return ARCH_OK;
}

static arch_result parallel_close(device_t *dev)
{
    // Parallel port doesn't need special cleanup
    return ARCH_OK;
}

static int parallel_write(device_t *dev, const void *buf, size_t len)
{
    parallel_driver_data_t *data = (parallel_driver_data_t *)dev->driver_data;
    
    // Use arch layer to write to specific parallel device
    return arch_parallel_write(data->arch_device, buf, len);
}

/* Driver initialization function - truly arch-independent! */
arch_result parallel_driver_init(void)
{
    // Ask arch layer how many parallel ports exist
    parallel_device_count = arch_parallel_get_count();
    
    if (parallel_device_count == 0) {
        // No parallel ports available on this architecture
        return ARCH_OK;
    }
    
    // Allocate storage for devices and data (in real kernel, use kmalloc)
    // For now, we'll use a simple static approach with max devices
    #define MAX_PARALLEL_DEVICES 4
    static device_t static_devices[MAX_PARALLEL_DEVICES];
    static parallel_driver_data_t static_data[MAX_PARALLEL_DEVICES];
    
    if (parallel_device_count > MAX_PARALLEL_DEVICES) {
        parallel_device_count = MAX_PARALLEL_DEVICES;
    }
    
    parallel_devices = static_devices;
    parallel_data = static_data;
    
    // Create and register a device for each parallel port the arch reports
    for (int i = 0; i < parallel_device_count; i++) {
        arch_parallel_info_t info;
        arch_result result = arch_parallel_get_info(i, &info);
        if (result != ARCH_OK) {
            continue;
        }
        
        // Initialize device structure
        device_t *device = &parallel_devices[i];
        parallel_driver_data_t *data = &parallel_data[i];
        
        // Copy the arch-suggested name
        strncpy(device->name, info.name, sizeof(device->name) - 1);
        device->name[sizeof(device->name) - 1] = '\0';
        
        device->class = DEVICE_CLASS_CHAR;
        device->state = DEVICE_STATE_UNINITIALIZED;
        device->open = parallel_open;
        device->close = parallel_close;
        device->char_ops.read = NULL;  // Write-only
        device->char_ops.write = parallel_write;
        device->char_ops.flush = NULL;
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