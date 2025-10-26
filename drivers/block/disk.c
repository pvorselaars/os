#include "kernel/device.h"
#include "arch/arch.h"
#include "lib/string.h"

static arch_result disk_open(device_t *dev);
static arch_result disk_close(device_t *dev);
static int disk_read_blocks(device_t *dev, void *buf, uint64_t start_block, uint32_t block_count);
static int disk_write_blocks(device_t *dev, const void *buf, uint64_t start_block, uint32_t block_count);
static arch_result disk_sync(device_t *dev);
static uint32_t disk_get_block_size(device_t *dev);
static uint64_t disk_get_block_count(device_t *dev);

typedef struct {
    arch_disk_device_t *arch_device;  // Opaque arch-specific device handle
    uint32_t block_size;               // Block size in bytes
    uint64_t block_count;              // Total number of blocks
    bool read_only;                    // Device is read-only
} disk_driver_data_t;

static device_t *disk_devices = NULL;
static disk_driver_data_t *disk_data = NULL;
static int disk_device_count = 0;

static arch_result disk_open(device_t *dev)
{
    disk_driver_data_t *data = (disk_driver_data_t *)dev->driver_data;
    
    arch_result result = arch_disk_init(data->arch_device);
    if (result != ARCH_OK) {
        return result;
    }
    
    return ARCH_OK;
}

static arch_result disk_close(device_t *dev)
{
    disk_driver_data_t *data = (disk_driver_data_t *)dev->driver_data;
    
    arch_disk_sync(data->arch_device);
    
    return ARCH_OK;
}

static int disk_read_blocks(device_t *dev, void *buf, uint64_t start_block, uint32_t block_count)
{
    disk_driver_data_t *data = (disk_driver_data_t *)dev->driver_data;
    
    if (!buf || block_count == 0) {
        return -1;
    }
    
    if (start_block >= data->block_count || 
        start_block + block_count > data->block_count) {
        return -1;
    }
    
    arch_result result = arch_disk_read_blocks(data->arch_device, buf, start_block, block_count);
    if (result != ARCH_OK) {
        return -1;
    }
    
    return block_count;
}

static int disk_write_blocks(device_t *dev, const void *buf, uint64_t start_block, uint32_t block_count)
{
    disk_driver_data_t *data = (disk_driver_data_t *)dev->driver_data;
    
    if (!buf || block_count == 0) {
        return -1;
    }
    
    if (data->read_only) {
        return -1;
    }
    
    if (start_block >= data->block_count || 
        start_block + block_count > data->block_count) {
        return -1;
    }
    
    arch_result result = arch_disk_write_blocks(data->arch_device, buf, start_block, block_count);
    if (result != ARCH_OK) {
        return -1;
    }
    
    return block_count;
}

static arch_result disk_sync(device_t *dev)
{
    disk_driver_data_t *data = (disk_driver_data_t *)dev->driver_data;
    return arch_disk_sync(data->arch_device);
}

static uint32_t disk_get_block_size(device_t *dev)
{
    disk_driver_data_t *data = (disk_driver_data_t *)dev->driver_data;
    return data->block_size;
}

static uint64_t disk_get_block_count(device_t *dev)
{
    disk_driver_data_t *data = (disk_driver_data_t *)dev->driver_data;
    return data->block_count;
}

arch_result disk_driver_init(void)
{
    disk_device_count = arch_disk_get_count();
    
    if (disk_device_count == 0) {
        return ARCH_OK;
    }
    
    // TODO: allocate storage for devices and data dynamically
    #define MAX_DISK_DEVICES 8
    static device_t static_devices[MAX_DISK_DEVICES];
    static disk_driver_data_t static_data[MAX_DISK_DEVICES];
    
    if (disk_device_count > MAX_DISK_DEVICES) {
        disk_device_count = MAX_DISK_DEVICES;
    }
    
    disk_devices = static_devices;
    disk_data = static_data;
    
    for (int i = 0; i < disk_device_count; i++) {
        arch_disk_info_t info;
        arch_result result = arch_disk_get_info(i, &info);
        if (result != ARCH_OK) {
            continue;
        }
        
        device_t *device = &disk_devices[i];
        disk_driver_data_t *data = &disk_data[i];
        
        strncpy(device->name, info.name, sizeof(device->name) - 1);
        device->name[sizeof(device->name) - 1] = '\0';
        
        device->class = DEVICE_CLASS_BLOCK;
        device->state = DEVICE_STATE_UNINITIALIZED;
        device->open = disk_open;
        device->close = disk_close;
        device->block_ops.read_blocks = disk_read_blocks;
        device->block_ops.write_blocks = disk_write_blocks;
        device->block_ops.sync = disk_sync;
        device->block_ops.get_block_size = disk_get_block_size;
        device->block_ops.get_block_count = disk_get_block_count;
        device->driver_data = data;
        device->next = NULL;
        
        data->arch_device = info.device;
        data->block_size = info.block_size;
        data->block_count = info.block_count;
        data->read_only = info.read_only;
        
        result = device_register(device);
        if (result != ARCH_OK) {
            return result;
        }
    }
    
    return ARCH_OK;
}