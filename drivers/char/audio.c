#include "kernel/device.h"
#include "arch/arch.h"
#include "lib/string.h"

/* Generic Audio Driver
 * 
 * Provides character device interface for audio output.
 * Uses arch layer for hardware-specific operations (PC speaker, sound cards, etc.).
 * 
 * Audio devices can be used for:
 * - Playing tones at specific frequencies
 * - Simple beep sounds
 * - Basic sound effects
 * 
 * The character device interface accepts simple commands:
 * - Write frequency values to play tones
 * - Write special commands for beeps/stops
 */

/* Forward declarations */
static arch_result audio_open(device_t *dev);
static arch_result audio_close(device_t *dev);
static int audio_read(device_t *dev, void *buf, size_t len);
static int audio_write(device_t *dev, const void *buf, size_t len);

/* Audio driver state */
typedef struct {
    arch_audio_device_t *arch_device;  // Opaque arch-specific device handle
    bool is_playing;                    // Currently playing sound
    uint32_t current_frequency;        // Current tone frequency
} audio_driver_data_t;

/* Dynamic device storage - allocated based on what arch reports */
static device_t *audio_devices = NULL;
static audio_driver_data_t *audio_data = NULL;
static int audio_device_count = 0;

static arch_result audio_open(device_t *dev)
{
    audio_driver_data_t *data = (audio_driver_data_t *)dev->driver_data;
    
    // Initialize this specific audio device through arch layer
    arch_result result = arch_audio_init(data->arch_device);
    if (result != ARCH_OK) {
        return result;
    }
    
    // Initialize the driver state
    data->is_playing = false;
    data->current_frequency = 0;
    
    return ARCH_OK;
}

static arch_result audio_close(device_t *dev)
{
    audio_driver_data_t *data = (audio_driver_data_t *)dev->driver_data;
    
    // Stop any currently playing sound
    if (data->is_playing) {
        arch_audio_stop(data->arch_device);
        data->is_playing = false;
    }
    
    return ARCH_OK;
}

static int audio_read(device_t *dev, void *buf, size_t len)
{
    // Audio devices are typically output-only for simple systems
    // Could implement status reading in the future
    return -1; // Not supported
}

/* Audio device write interface - SUPER SIMPLE!
 * 
 * Write a 4-byte frequency value (little-endian) to play a tone.
 * Write 0 to stop sound.
 * That's it!
 */
static int audio_write(device_t *dev, const void *buf, size_t len)
{
    audio_driver_data_t *data = (audio_driver_data_t *)dev->driver_data;
    
    // Need exactly 4 bytes for frequency
    if (len != 4) {
        return -1;
    }
    
    // Read frequency as little-endian 32-bit value
    const uint8_t *bytes = (const uint8_t *)buf;
    uint32_t frequency = bytes[0] | (bytes[1] << 8) | (bytes[2] << 16) | (bytes[3] << 24);
    
    if (frequency == 0) {
        // Stop sound
        arch_audio_stop(data->arch_device);
        data->is_playing = false;
        data->current_frequency = 0;
    } else {
        // Play tone
        arch_audio_play_tone(data->arch_device, frequency);
        data->is_playing = true;
        data->current_frequency = frequency;
    }
    
    return 4;
}

/* Driver initialization function - truly arch-independent! */
arch_result audio_driver_init(void)
{
    // Ask arch layer how many audio devices exist
    audio_device_count = arch_audio_get_count();
    
    if (audio_device_count == 0) {
        // No audio devices available on this architecture
        return ARCH_OK;
    }
    
    // Allocate storage for devices and data (in real kernel, use kmalloc)
    // For now, we'll use a simple static approach with max devices
    #define MAX_AUDIO_DEVICES 4
    static device_t static_devices[MAX_AUDIO_DEVICES];
    static audio_driver_data_t static_data[MAX_AUDIO_DEVICES];
    
    if (audio_device_count > MAX_AUDIO_DEVICES) {
        audio_device_count = MAX_AUDIO_DEVICES;
    }
    
    audio_devices = static_devices;
    audio_data = static_data;
    
    // Create and register a device for each audio device the arch reports
    for (int i = 0; i < audio_device_count; i++) {
        arch_audio_info_t info;
        arch_result result = arch_audio_get_info(i, &info);
        if (result != ARCH_OK) {
            continue;
        }
        
        // Initialize device structure
        device_t *device = &audio_devices[i];
        audio_driver_data_t *data = &audio_data[i];
        
        // Copy the arch-suggested name
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
        
        // Initialize driver data
        data->arch_device = info.device;
        data->is_playing = false;
        data->current_frequency = 0;
        
        // Register the device
        result = device_register(device);
        if (result != ARCH_OK) {
            return result;
        }
    }
    
    return ARCH_OK;
}