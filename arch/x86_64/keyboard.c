#include "arch/arch.h"
#include "lib/utils.h"
#include "lib/string.h"

/* x86_64 PS/2 Keyboard Implementation
 * 
 * Provides arch-layer keyboard support using the existing PS/2 infrastructure.
 * This implements the keyboard interface defined in arch.h using PS/2 hardware.
 * 
 * Uses the scancode translation from platform/pc/ps2.c
 */

/* PS/2 Controller ports */
#define PS2_DATA_PORT    0x60
#define PS2_COMMAND_PORT 0x64
#define PS2_STATUS_PORT  0x64

/* PS/2 Controller commands */
#define PS2_CMD_READ_CONFIG   0x20
#define PS2_CMD_WRITE_CONFIG  0x60
#define PS2_CMD_DISABLE_PORT1 0xAD
#define PS2_CMD_ENABLE_PORT1  0xAE
#define PS2_CMD_TEST_PORT1    0xAB

/* PS/2 Keyboard commands */
#define KB_CMD_SET_LEDS      0xED
#define KB_CMD_ECHO          0xEE
#define KB_CMD_SET_SCANCODE  0xF0
#define KB_CMD_IDENTIFY      0xF2
#define KB_CMD_SET_RATE      0xF3
#define KB_CMD_ENABLE        0xF4
#define KB_CMD_DISABLE       0xF5
#define KB_CMD_RESET         0xFF

/* Response codes */
#define KB_RESP_ACK          0xFA
#define KB_RESP_RESEND       0xFE
#define KB_RESP_ERROR        0xFC

/* PS/2 Scancode to Unicode/Logical Key Translation
 * 
 * Maps PS/2 Set 1 scancodes to Unicode codepoints and logical keys.
 * This table handles the US QWERTY layout - other layouts would need
 * different tables or a more sophisticated mapping system.
 */

typedef struct {
    uint32_t unicode_normal;    // Unicode when no modifiers
    uint32_t unicode_shift;     // Unicode when shift pressed  
    arch_logical_key_t logical; // Logical key (for special keys)
} scancode_mapping_t;

static const scancode_mapping_t scancode_table[128] = {
    [0x01] = {0, 0, KEY_ESCAPE},
    [0x02] = {'1', '!', KEY_UNKNOWN},
    [0x03] = {'2', '@', KEY_UNKNOWN},
    [0x04] = {'3', '#', KEY_UNKNOWN},
    [0x05] = {'4', '$', KEY_UNKNOWN},
    [0x06] = {'5', '%', KEY_UNKNOWN},
    [0x07] = {'6', '^', KEY_UNKNOWN},
    [0x08] = {'7', '&', KEY_UNKNOWN},
    [0x09] = {'8', '*', KEY_UNKNOWN},
    [0x0A] = {'9', '(', KEY_UNKNOWN},
    [0x0B] = {'0', ')', KEY_UNKNOWN},
    [0x0C] = {'-', '_', KEY_UNKNOWN},
    [0x0D] = {'=', '+', KEY_UNKNOWN},
    [0x0E] = {0, 0, KEY_BACKSPACE},
    [0x0F] = {0, 0, KEY_TAB},
    [0x10] = {'q', 'Q', KEY_UNKNOWN},
    [0x11] = {'w', 'W', KEY_UNKNOWN},
    [0x12] = {'e', 'E', KEY_UNKNOWN},
    [0x13] = {'r', 'R', KEY_UNKNOWN},
    [0x14] = {'t', 'T', KEY_UNKNOWN},
    [0x15] = {'y', 'Y', KEY_UNKNOWN},
    [0x16] = {'u', 'U', KEY_UNKNOWN},
    [0x17] = {'i', 'I', KEY_UNKNOWN},
    [0x18] = {'o', 'O', KEY_UNKNOWN},
    [0x19] = {'p', 'P', KEY_UNKNOWN},
    [0x1A] = {'[', '{', KEY_UNKNOWN},
    [0x1B] = {']', '}', KEY_UNKNOWN},
    [0x1C] = {0, 0, KEY_ENTER},
    [0x1D] = {0, 0, KEY_LEFT_CTRL},
    [0x1E] = {'a', 'A', KEY_UNKNOWN},
    [0x1F] = {'s', 'S', KEY_UNKNOWN},
    [0x20] = {'d', 'D', KEY_UNKNOWN},
    [0x21] = {'f', 'F', KEY_UNKNOWN},
    [0x22] = {'g', 'G', KEY_UNKNOWN},
    [0x23] = {'h', 'H', KEY_UNKNOWN},
    [0x24] = {'j', 'J', KEY_UNKNOWN},
    [0x25] = {'k', 'K', KEY_UNKNOWN},
    [0x26] = {'l', 'L', KEY_UNKNOWN},
    [0x27] = {';', ':', KEY_UNKNOWN},
    [0x28] = {'\'', '"', KEY_UNKNOWN},
    [0x29] = {'`', '~', KEY_UNKNOWN},
    [0x2A] = {0, 0, KEY_LEFT_SHIFT},
    [0x2B] = {'\\', '|', KEY_UNKNOWN},
    [0x2C] = {'z', 'Z', KEY_UNKNOWN},
    [0x2D] = {'x', 'X', KEY_UNKNOWN},
    [0x2E] = {'c', 'C', KEY_UNKNOWN},
    [0x2F] = {'v', 'V', KEY_UNKNOWN},
    [0x30] = {'b', 'B', KEY_UNKNOWN},
    [0x31] = {'n', 'N', KEY_UNKNOWN},
    [0x32] = {'m', 'M', KEY_UNKNOWN},
    [0x33] = {',', '<', KEY_UNKNOWN},
    [0x34] = {'.', '>', KEY_UNKNOWN},
    [0x35] = {'/', '?', KEY_UNKNOWN},
    [0x36] = {0, 0, KEY_RIGHT_SHIFT},
    [0x37] = {'*', '*', KEY_KP_MULTIPLY},
    [0x38] = {0, 0, KEY_LEFT_ALT},
    [0x39] = {' ', ' ', KEY_UNKNOWN},
    [0x3A] = {0, 0, KEY_CAPS_LOCK},
    [0x3B] = {0, 0, KEY_F1},
    [0x3C] = {0, 0, KEY_F2},
    [0x3D] = {0, 0, KEY_F3},
    [0x3E] = {0, 0, KEY_F4},
    [0x3F] = {0, 0, KEY_F5},
    [0x40] = {0, 0, KEY_F6},
    [0x41] = {0, 0, KEY_F7},
    [0x42] = {0, 0, KEY_F8},
    [0x43] = {0, 0, KEY_F9},
    [0x44] = {0, 0, KEY_F10},
    [0x45] = {0, 0, KEY_NUM_LOCK},
    [0x46] = {0, 0, KEY_SCROLL_LOCK},
    [0x47] = {'7', '7', KEY_KP_7},
    [0x48] = {'8', '8', KEY_KP_8},
    [0x49] = {'9', '9', KEY_KP_9},
    [0x4A] = {'-', '-', KEY_KP_MINUS},
    [0x4B] = {'4', '4', KEY_KP_4},
    [0x4C] = {'5', '5', KEY_KP_5},
    [0x4D] = {'6', '6', KEY_KP_6},
    [0x4E] = {'+', '+', KEY_KP_PLUS},
    [0x4F] = {'1', '1', KEY_KP_1},
    [0x50] = {'2', '2', KEY_KP_2},
    [0x51] = {'3', '3', KEY_KP_3},
    [0x52] = {'0', '0', KEY_KP_0},
    [0x53] = {'.', '.', KEY_KP_DECIMAL},
    [0x57] = {0, 0, KEY_F11},
    [0x58] = {0, 0, KEY_F12},
    // All other entries default to {0, 0, KEY_UNKNOWN}
};

/* Keyboard state tracking */
typedef struct {
    bool shift_pressed;
    bool ctrl_pressed;  
    bool alt_pressed;
    bool caps_lock;
    bool num_lock;
    bool scroll_lock;
} keyboard_state_t;

/* Event queue for buffering keyboard events */
#define EVENT_QUEUE_SIZE 32
typedef struct {
    arch_keyboard_event_t events[EVENT_QUEUE_SIZE];
    int head;
    int tail;
    int count;
} event_queue_t;

/* PS/2 Keyboard device structure */
typedef struct {
    keyboard_state_t state;
    event_queue_t event_queue;
    bool initialized;
} ps2_keyboard_t;

/* Single PS/2 keyboard device */
static ps2_keyboard_t ps2_keyboard_device;

#include "arch/arch.h"
#include "lib/utils.h"
#include "lib/string.h"
#include "drivers/keyboard.h"

/* I/O operations (use existing definitions from io.h) */
#include "arch/x86_64/io.h"

/* Wait for PS/2 controller to be ready for output */
static void ps2_wait_output(void) {
    int timeout = 100000;
    while (timeout-- > 0) {
        if (!(inb(PS2_STATUS_PORT) & 0x02)) {
            break;
        }
    }
}

/* Wait for PS/2 controller to have input */
static void ps2_wait_input(void) {
    int timeout = 100000;
    while (timeout-- > 0) {
        if (inb(PS2_STATUS_PORT) & 0x01) {
            break;
        }
    }
}

/* Send command to PS/2 controller */
static void ps2_send_command(uint8_t cmd) {
    ps2_wait_output();
    outb(PS2_COMMAND_PORT, cmd);
}

/* Send data to PS/2 keyboard */
static void ps2_send_data(uint8_t data) {
    ps2_wait_output();
    outb(PS2_DATA_PORT, data);
}

/* Read data from PS/2 keyboard */
static uint8_t ps2_read_data(void) {
    ps2_wait_input();
    return inb(PS2_DATA_PORT);
}

/* Add event to queue */
static void queue_event(arch_keyboard_event_t *event) {
    event_queue_t *queue = &ps2_keyboard_device.event_queue;
    
    if (queue->count < EVENT_QUEUE_SIZE) {
        queue->events[queue->head] = *event;
        queue->head = (queue->head + 1) % EVENT_QUEUE_SIZE;
        queue->count++;
    }
}

/* PS/2 keyboard interrupt handler */
void ps2_keyboard_interrupt(void) {
    uint8_t scancode = inb(PS2_DATA_PORT);
    keyboard_state_t *state = &ps2_keyboard_device.state;
    arch_keyboard_event_t event;
    
    // Handle key release (scancode with high bit set)
    bool key_released = (scancode & 0x80) != 0;
    if (key_released) {
        scancode &= 0x7F;
    }
    
    // Initialize event structure
    event.unicode = 0;
    event.key = KEY_UNKNOWN;
    event.pressed = !key_released;
    event.shift = state->shift_pressed;
    event.ctrl = state->ctrl_pressed;
    event.alt = state->alt_pressed;
    event.super = false;  // PS/2 doesn't typically have super keys
    event.caps_lock = state->caps_lock;
    event.num_lock = state->num_lock;
    event.scroll_lock = state->scroll_lock;
    
    // Update modifier key states and handle special keys
    if (scancode < 128) {
        const scancode_mapping_t *mapping = &scancode_table[scancode];
        
        switch (scancode) {
            case 0x2A: // Left Shift
            case 0x36: // Right Shift
                state->shift_pressed = !key_released;
                event.key = (scancode == 0x2A) ? KEY_LEFT_SHIFT : KEY_RIGHT_SHIFT;
                event.shift = state->shift_pressed;  // Update current state
                break;
                
            case 0x1D: // Ctrl
                state->ctrl_pressed = !key_released;
                event.key = KEY_LEFT_CTRL;
                event.ctrl = state->ctrl_pressed;  // Update current state
                break;
                
            case 0x38: // Alt
                state->alt_pressed = !key_released;
                event.key = KEY_LEFT_ALT;
                event.alt = state->alt_pressed;  // Update current state
                break;
                
            case 0x3A: // Caps Lock
                if (!key_released) {
                    state->caps_lock = !state->caps_lock;
                }
                event.key = KEY_CAPS_LOCK;
                event.caps_lock = state->caps_lock;  // Update current state
                break;
                
            case 0x45: // Num Lock  
                if (!key_released) {
                    state->num_lock = !state->num_lock;
                }
                event.key = KEY_NUM_LOCK;
                event.num_lock = state->num_lock;  // Update current state
                break;
                
            case 0x46: // Scroll Lock
                if (!key_released) {
                    state->scroll_lock = !state->scroll_lock;
                }
                event.key = KEY_SCROLL_LOCK;
                event.scroll_lock = state->scroll_lock;  // Update current state
                break;
                
            default:
                // Handle regular keys
                if (mapping->logical != KEY_UNKNOWN) {
                    // Special key (non-printable)
                    event.key = mapping->logical;
                } else if (mapping->unicode_normal != 0) {
                    // Printable key - determine Unicode based on modifiers
                    uint32_t base_unicode = mapping->unicode_normal;
                    
                    // Apply shift
                    if (state->shift_pressed && mapping->unicode_shift != 0) {
                        event.unicode = mapping->unicode_shift;
                    } else {
                        event.unicode = base_unicode;
                        
                        // Apply caps lock for letters
                        if (base_unicode >= 'a' && base_unicode <= 'z' && state->caps_lock) {
                            event.unicode = base_unicode - 'a' + 'A';
                        }
                    }
                }
                break;
        }
        
        // Queue the event (for all keys, including modifiers)
        queue_event(&event);
    }
    
    // Notify the generic keyboard driver that events are available
    keyboard_driver_interrupt_notify((arch_keyboard_device_t *)&ps2_keyboard_device);
}

/* Register keyboard interrupt handler */
extern int arch_register_interrupt(unsigned vector, void (*handler)(void));

/* Initialize PS/2 keyboard */
static arch_result ps2_keyboard_initialize(void) {
    ps2_keyboard_t *kbd = &ps2_keyboard_device;
    
    if (kbd->initialized) {
        return ARCH_OK;
    }
    
    // Initialize state
    kbd->state.shift_pressed = false;
    kbd->state.ctrl_pressed = false;
    kbd->state.alt_pressed = false;
    kbd->state.caps_lock = false;
    kbd->state.num_lock = false;
    kbd->state.scroll_lock = false;
    kbd->event_queue.head = 0;
    kbd->event_queue.tail = 0;
    kbd->event_queue.count = 0;
    
    // Disable first PS/2 port
    ps2_send_command(PS2_CMD_DISABLE_PORT1);
    
    // Flush output buffer
    int flushed = 0;
    while (inb(PS2_STATUS_PORT) & 0x01) {
        uint8_t data = inb(PS2_DATA_PORT);
        flushed++;
    }
    
    // Read controller configuration
    ps2_send_command(PS2_CMD_READ_CONFIG);
    uint8_t config = ps2_read_data();
    
    // Set configuration (enable interrupts, disable translation)
    config |= 0x01;   // Enable keyboard interrupt
    config &= ~0x40;  // Disable keyboard translation
    
    ps2_send_command(PS2_CMD_WRITE_CONFIG);
    ps2_send_data(config);
    
    // Test PS/2 port
    ps2_send_command(PS2_CMD_TEST_PORT1);
    uint8_t test_result = ps2_read_data();
    if (test_result != 0x00) {
        return ARCH_ERROR;
    }
    
    // Enable PS/2 port
    ps2_send_command(PS2_CMD_ENABLE_PORT1);
    
    // Try to reset keyboard (some emulators might not support this properly)
    ps2_send_data(KB_CMD_RESET);
    uint8_t response = ps2_read_data(); // Should be ACK
    if (response == KB_RESP_ACK) {
        // Wait for self-test result
        response = ps2_read_data(); // Should be 0xAA (passed)
        if (response != 0xAA) {
        }
    } else {
        // Some emulators/keyboards don't support reset, continue anyway
    }
    
    // Enable keyboard
    ps2_send_data(KB_CMD_ENABLE);
    response = ps2_read_data();
    if (response != KB_RESP_ACK) {
        // Some keyboards might already be enabled or respond differently
    }
    
    // Register keyboard interrupt handler
    arch_register_interrupt(0x21, ps2_keyboard_interrupt);
    
    kbd->initialized = true;
    return ARCH_OK;
}

/* Architecture interface implementations */

int arch_keyboard_get_count(void) {
    return 1; // Single PS/2 keyboard on PC
}

arch_result arch_keyboard_get_info(int index, arch_keyboard_info_t *info) {
    if (index != 0 || info == NULL) {
        return ARCH_ERROR;
    }
    
    info->name = "ps2kb0";
    info->device = (arch_keyboard_device_t *)&ps2_keyboard_device;
    
    return ARCH_OK;
}

arch_result arch_keyboard_init(arch_keyboard_device_t *device) {
    if (device != (arch_keyboard_device_t *)&ps2_keyboard_device) {
        return ARCH_ERROR;
    }
    
    return ps2_keyboard_initialize();
}

bool arch_keyboard_has_event(arch_keyboard_device_t *device) {
    if (device != (arch_keyboard_device_t *)&ps2_keyboard_device) {
        return false;
    }
    
    return ps2_keyboard_device.event_queue.count > 0;
}

arch_result arch_keyboard_read_event(arch_keyboard_device_t *device, arch_keyboard_event_t *event) {
    if (device != (arch_keyboard_device_t *)&ps2_keyboard_device || event == NULL) {
        return ARCH_ERROR;
    }
    
    event_queue_t *queue = &ps2_keyboard_device.event_queue;
    
    if (queue->count == 0) {
        return ARCH_ERROR; // No events available
    }
    
    *event = queue->events[queue->tail];
    queue->tail = (queue->tail + 1) % EVENT_QUEUE_SIZE;
    queue->count--;
    
    return ARCH_OK;
}