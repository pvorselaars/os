#include "kernel/device.h"
#include "arch/arch.h"
#include "lib/string.h"

static arch_result serial_open(device_t *dev);
static arch_result serial_close(device_t *dev);
static int serial_read(device_t *dev, void *buf, size_t len);
static int serial_write(device_t *dev, const void *buf, size_t len);
static arch_result serial_flush(device_t *dev);

typedef struct {
    arch_serial_device_t *arch_device;
} serial_driver_data_t;

static device_t *serial_devices = NULL;
static serial_driver_data_t *serial_data = NULL;
static int serial_device_count = 0;

static arch_result serial_open(device_t *dev)
{
    serial_driver_data_t *data = (serial_driver_data_t *)dev->driver_data;
    
    arch_result result = arch_serial_init(data->arch_device);
    if (result != ARCH_OK) {
        return result;
    }
    
    return ARCH_OK;
}

static arch_result serial_close(device_t *dev)
{
    return ARCH_OK;
}

static int serial_read(device_t *dev, void *buf, size_t len)
{
    serial_driver_data_t *data = (serial_driver_data_t *)dev->driver_data;
    
    return arch_serial_read(data->arch_device, buf, len);
}

static int serial_write(device_t *dev, const void *buf, size_t len)
{
    serial_driver_data_t *data = (serial_driver_data_t *)dev->driver_data;
    
    return arch_serial_write(data->arch_device, buf, len);
}

static arch_result serial_flush(device_t *dev)
{
    return ARCH_OK;
}

arch_result serial_driver_init(void)
{
    serial_device_count = arch_serial_get_count();
    
    if (serial_device_count == 0) {
        return ARCH_OK;
    }
    
    #define MAX_SERIAL_DEVICES 4
    static device_t static_devices[MAX_SERIAL_DEVICES];
    static serial_driver_data_t static_data[MAX_SERIAL_DEVICES];
    
    if (serial_device_count > MAX_SERIAL_DEVICES) {
        serial_device_count = MAX_SERIAL_DEVICES;
    }
    
    serial_devices = static_devices;
    serial_data = static_data;

    for (int i = 0; i < serial_device_count; i++) {
        arch_serial_info_t info;
        arch_result result = arch_serial_get_info(i, &info);
        if (result != ARCH_OK) {
            continue;
        }
        
        device_t *device = &serial_devices[i];
        serial_driver_data_t *data = &serial_data[i];
        
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
        
        data->arch_device = info.device;
        
        result = device_register(device);
        if (result != ARCH_OK) {
            return result;
        }
    }
    
    return ARCH_OK;
}