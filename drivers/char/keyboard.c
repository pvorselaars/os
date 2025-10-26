#include "kernel/device.h"
#include "arch/arch.h"
#include "lib/string.h"
#include "lib/unicode.h"

/* Generic Keyboard Driver
 * 
 * Provides character device interface for keyboard input.
 * Uses arch layer for hardware-specific operations (PS/2, USB, etc.).
 * 
 * This driver is truly arch-independent - it asks the arch layer
 * how many keyboard devices exist and creates devices accordingly.
 */

/* Forward declarations */
static arch_result keyboard_open(device_t *dev);
static arch_result keyboard_close(device_t *dev);
static int keyboard_read(device_t *dev, void *buf, size_t len);
static int keyboard_write(device_t *dev, const void *buf, size_t len);

/* Keyboard input buffer for UTF-8 characters */
#define KEYBOARD_BUFFER_SIZE 512

typedef struct {
    char buffer[KEYBOARD_BUFFER_SIZE];     // Circular buffer for UTF-8 data
    int head;                              // Write position (interrupt fills here)
    int tail;                              // Read position (user reads from here)
    int count;                             // Number of bytes in buffer
    bool reader_waiting;                   // True if a reader is blocked waiting for input
} keyboard_input_buffer_t;

/* Keyboard driver state */
typedef struct {
    arch_keyboard_device_t *arch_device;  // Opaque arch-specific device handle
    keyboard_input_buffer_t input_buffer;  // Interrupt-driven input buffer
    bool interrupt_mode;                   // True if using interrupt-driven input
} keyboard_driver_data_t;

/* Dynamic device storage - allocated based on what arch reports */
static device_t *keyboard_devices = NULL;
static keyboard_driver_data_t *keyboard_data = NULL;
static int keyboard_device_count = 0;

/* Forward declaration for interrupt callback */
static void keyboard_interrupt_callback(arch_keyboard_device_t *arch_device);

/* Add data to keyboard input buffer (called from interrupt context) */
static void keyboard_buffer_add_data(keyboard_driver_data_t *data, const char *utf8_data, int len)
{
    keyboard_input_buffer_t *buf = &data->input_buffer;
    
    // Add bytes to circular buffer
    for (int i = 0; i < len && buf->count < KEYBOARD_BUFFER_SIZE; i++) {
        buf->buffer[buf->head] = utf8_data[i];
        buf->head = (buf->head + 1) % KEYBOARD_BUFFER_SIZE;
        buf->count++;
    }
    
    // If a reader is waiting, we could wake them up here
    // For now, they'll be woken up when they next poll
}

static arch_result keyboard_open(device_t *dev)
{
    keyboard_driver_data_t *data = (keyboard_driver_data_t *)dev->driver_data;
    
    // Initialize this specific keyboard device through arch layer
    arch_result result = arch_keyboard_init(data->arch_device);
    if (result != ARCH_OK) {
        return result;
    }
    
    // Initialize the input buffer
    data->input_buffer.head = 0;
    data->input_buffer.tail = 0;
    data->input_buffer.count = 0;
    data->input_buffer.reader_waiting = false;
    data->interrupt_mode = true;
    
    return ARCH_OK;
}

static arch_result keyboard_close(device_t *dev)
{
    // Keyboard doesn't need special cleanup
    return ARCH_OK;
}

static int keyboard_read(device_t *dev, void *buf, size_t len)
{
    keyboard_driver_data_t *data = (keyboard_driver_data_t *)dev->driver_data;
    char *output = (char *)buf;
    int bytes_read = 0;
    
    if (!data->interrupt_mode) {
        // Fallback to polling mode (for compatibility)
        arch_keyboard_event_t event;
        while (arch_keyboard_has_event(data->arch_device) && bytes_read < len) {
            if (arch_keyboard_read_event(data->arch_device, &event) == ARCH_OK) {
                if (event.pressed && event.unicode != 0 && unicode_is_printable(event.unicode)) {
                    char utf8_buf[4];
                    int utf8_len = unicode_to_utf8(event.unicode, utf8_buf);
                    
                    if (utf8_len > 0 && bytes_read + utf8_len <= len) {
                        for (int i = 0; i < utf8_len; i++) {
                            output[bytes_read + i] = utf8_buf[i];
                        }
                        bytes_read += utf8_len;
                    } else {
                        break;
                    }
                }
            }
        }
        return bytes_read;
    }
    
    // Interrupt-driven mode: read from the buffer filled by interrupts
    keyboard_input_buffer_t *input_buf = &data->input_buffer;
    
    // Copy available data from circular buffer to output
    while (input_buf->count > 0 && bytes_read < len) {
        output[bytes_read] = input_buf->buffer[input_buf->tail];
        input_buf->tail = (input_buf->tail + 1) % KEYBOARD_BUFFER_SIZE;
        input_buf->count--;
        bytes_read++;
    }
    
    return bytes_read;
}

/* Interrupt callback - called when keyboard events are available
 * This processes events from the arch layer and converts them to UTF-8 in the input buffer
 */
static void keyboard_interrupt_callback(arch_keyboard_device_t *arch_device)
{
    // Find the keyboard device data for this arch device
    keyboard_driver_data_t *data = NULL;
    for (int i = 0; i < keyboard_device_count; i++) {
        if (keyboard_data[i].arch_device == arch_device) {
            data = &keyboard_data[i];
            break;
        }
    }
    
    if (!data || !data->interrupt_mode) {
        return;  // Device not found or not in interrupt mode
    }
    
    // Process all available events and add them to the buffer
    arch_keyboard_event_t event;
    while (arch_keyboard_has_event(arch_device)) {
        if (arch_keyboard_read_event(arch_device, &event) == ARCH_OK) {
            // Only process key press events with printable Unicode characters
            if (event.pressed && event.unicode != 0 && unicode_is_printable(event.unicode)) {
                // Convert Unicode codepoint to UTF-8
                char utf8_buf[4];
                int utf8_len = unicode_to_utf8(event.unicode, utf8_buf);
                
                if (utf8_len > 0) {
                    // Add UTF-8 data to input buffer
                    keyboard_buffer_add_data(data, utf8_buf, utf8_len);
                }
            }
        }
    }
}

static int keyboard_write(device_t *dev, const void *buf, size_t len)
{
    // Keyboards are typically input-only devices
    return -1; // Not supported
}

/* Driver initialization function - truly arch-independent! */
arch_result keyboard_driver_init(void)
{
    // Ask arch layer how many keyboard devices exist
    keyboard_device_count = arch_keyboard_get_count();
    
    if (keyboard_device_count == 0) {
        // No keyboard devices available on this architecture
        return ARCH_OK;
    }
    
    // Allocate storage for devices and data (in real kernel, use kmalloc)
    // For now, we'll use a simple static approach with max devices
    #define MAX_KEYBOARD_DEVICES 4
    static device_t static_devices[MAX_KEYBOARD_DEVICES];
    static keyboard_driver_data_t static_data[MAX_KEYBOARD_DEVICES];
    
    if (keyboard_device_count > MAX_KEYBOARD_DEVICES) {
        keyboard_device_count = MAX_KEYBOARD_DEVICES;
    }
    
    keyboard_devices = static_devices;
    keyboard_data = static_data;
    
    // Create and register a device for each keyboard the arch reports
    for (int i = 0; i < keyboard_device_count; i++) {
        arch_keyboard_info_t info;
        arch_result result = arch_keyboard_get_info(i, &info);
        if (result != ARCH_OK) {
            continue;
        }
        
        // Initialize device structure
        device_t *device = &keyboard_devices[i];
        keyboard_driver_data_t *data = &keyboard_data[i];
        
        // Copy the arch-suggested name
        strncpy(device->name, info.name, sizeof(device->name) - 1);
        device->name[sizeof(device->name) - 1] = '\0';
        
        device->class = DEVICE_CLASS_CHAR;
        device->state = DEVICE_STATE_UNINITIALIZED;
        device->open = keyboard_open;
        device->close = keyboard_close;
        device->char_ops.read = keyboard_read;
        device->char_ops.write = keyboard_write;
        device->char_ops.flush = NULL;  // No flushing needed
        device->driver_data = data;
        device->next = NULL;
        
        // Initialize driver data
        data->arch_device = info.device;
        data->input_buffer.head = 0;
        data->input_buffer.tail = 0;
        data->input_buffer.count = 0;
        data->input_buffer.reader_waiting = false;
        data->interrupt_mode = true;
        
        // Register the device
        result = device_register(device);
        if (result != ARCH_OK) {
            return result;
        }
    }
    
    return ARCH_OK;
}

/* Function for arch layer to notify about keyboard events
 * This should be called from the arch keyboard interrupt handler
 */
void keyboard_driver_interrupt_notify(arch_keyboard_device_t *arch_device)
{
    keyboard_interrupt_callback(arch_device);
}