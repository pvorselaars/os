#include "kernel/device.h"
#include "arch/arch.h"
#include "lib/string.h"

static arch_result audio_open(device_t *dev);
static arch_result audio_close(device_t *dev);
static int audio_read(device_t *dev, void *buf, size_t len);
static int audio_write(device_t *dev, const void *buf, size_t len);

typedef struct {
    arch_audio_device_t *arch_device;  // Opaque arch-specific device handle
    bool is_playing;                    // Currently playing sound
    uint32_t current_frequency;        // Current tone frequency
} audio_driver_data_t;

static device_t *audio_devices = NULL;
static audio_driver_data_t *audio_data = NULL;
static int audio_device_count = 0;

static arch_result audio_open(device_t *dev)
{
    audio_driver_data_t *data = (audio_driver_data_t *)dev->driver_data;
    
    arch_result result = arch_audio_init(data->arch_device);
    if (result != ARCH_OK) {
        return result;
    }
    
    data->is_playing = false;
    data->current_frequency = 0;
    
    return ARCH_OK;
}

static arch_result audio_close(device_t *dev)
{
    audio_driver_data_t *data = (audio_driver_data_t *)dev->driver_data;
    
    if (data->is_playing) {
        arch_audio_stop(data->arch_device);
        data->is_playing = false;
    }
    
    return ARCH_OK;
}

static int audio_read(device_t *dev, void *buf, size_t len)
{
    return -1;
}

static int audio_write(device_t *dev, const void *buf, size_t len)
{
    audio_driver_data_t *data = (audio_driver_data_t *)dev->driver_data;
    
    if (len != 4) {
        return -1;
    }
    
    const uint8_t *bytes = (const uint8_t *)buf;
    uint32_t frequency = bytes[0] | (bytes[1] << 8) | (bytes[2] << 16) | (bytes[3] << 24);
    
    if (frequency == 0) {
        arch_audio_stop(data->arch_device);
        data->is_playing = false;
        data->current_frequency = 0;
    } else {
        arch_audio_play_tone(data->arch_device, frequency);
        data->is_playing = true;
        data->current_frequency = frequency;
    }
    
    return 4;
}

arch_result audio_driver_init(void)
{
    audio_device_count = arch_audio_get_count();
    
    if (audio_device_count == 0) {
        return ARCH_OK;
    }

    // TODO: allocate storage for devices and data dynamically
    #define MAX_AUDIO_DEVICES 4
    static device_t static_devices[MAX_AUDIO_DEVICES];
    static audio_driver_data_t static_data[MAX_AUDIO_DEVICES];
    
    if (audio_device_count > MAX_AUDIO_DEVICES) {
        audio_device_count = MAX_AUDIO_DEVICES;
    }
    
    audio_devices = static_devices;
    audio_data = static_data;
    
    for (int i = 0; i < audio_device_count; i++) {
        arch_audio_info_t info;
        arch_result result = arch_audio_get_info(i, &info);
        if (result != ARCH_OK) {
            continue;
        }
        
        device_t *device = &audio_devices[i];
        audio_driver_data_t *data = &audio_data[i];
        
        strncpy(device->name, info.name, sizeof(device->name) - 1);
        device->name[sizeof(device->name) - 1] = '\0';
        
        device->class = DEVICE_CLASS_CHAR;
        device->state = DEVICE_STATE_UNINITIALIZED;
        device->open = audio_open;
        device->close = audio_close;
        device->char_ops.read = audio_read;
        device->char_ops.write = audio_write;
        device->char_ops.flush = NULL;  // No flushing needed
        device->driver_data = data;
        device->next = NULL;
        
        data->arch_device = info.device;
        data->is_playing = false;
        data->current_frequency = 0;
        
        result = device_register(device);
        if (result != ARCH_OK) {
            return result;
        }
    }
    
    return ARCH_OK;
}