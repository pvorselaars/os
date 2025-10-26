#ifndef ARCH_H
#define ARCH_H

#include "definitions.h"

typedef enum {
    ARCH_OK = 0,
    ARCH_ERROR = -1,
    ARCH_INVALID = -2,
    ARCH_UNSUPPORTED = -3
} arch_result;

arch_result arch_init(void);
void arch_halt(void);
// void arch_shutdown(void);

arch_result arch_interrupt_init(void);
int arch_register_interrupt(unsigned vector, void (*handler)(void));
void arch_handle_interrupt(unsigned vector);
void arch_interrupt_enable(void);
void arch_interrupt_disable(void);

arch_result arch_timer_init(unsigned int frequency_hz);
uint64_t arch_time_ns(void);
arch_result arch_register_default_handlers(void);

// Serial interface - arch-specific implementations  
typedef struct arch_serial_device arch_serial_device_t;  // Opaque handle

typedef struct {
    arch_serial_device_t *device;  // Opaque arch-specific device handle
    const char *name;               // Suggested device name (e.g., "serial0")
} arch_serial_info_t;

int arch_serial_get_count(void);                                         // How many devices exist?
arch_result arch_serial_get_info(int index, arch_serial_info_t *info);   // Get info for device N
arch_result arch_serial_init(arch_serial_device_t *device);             // Initialize specific device
int arch_serial_write(arch_serial_device_t *device, const void *buf, size_t len);  // Write to device
int arch_serial_read(arch_serial_device_t *device, void *buf, size_t len);         // Read from device
bool arch_serial_data_available(arch_serial_device_t *device);                     // Check if data available

// Parallel interface - arch-specific implementations
typedef struct arch_parallel_device arch_parallel_device_t;  // Opaque handle

typedef struct {
    arch_parallel_device_t *device;  // Opaque arch-specific device handle
    const char *name;                 // Suggested device name (e.g., "parallel0")
} arch_parallel_info_t;

int arch_parallel_get_count(void);                                           // How many devices exist?
arch_result arch_parallel_get_info(int index, arch_parallel_info_t *info);  // Get info for device N
arch_result arch_parallel_init(arch_parallel_device_t *device);             // Initialize specific device
int arch_parallel_write(arch_parallel_device_t *device, const void *buf, size_t len);  // Write to device

// Keyboard interface - arch-specific implementations
typedef struct arch_keyboard_device arch_keyboard_device_t;  // Opaque handle

typedef struct {
    arch_keyboard_device_t *device;  // Opaque arch-specific device handle
    const char *name;                 // Suggested device name (e.g., "keyboard0")
} arch_keyboard_info_t;

// Logical key identifiers for special keys (hardware-independent)
typedef enum {
    KEY_UNKNOWN = 0,
    
    // Modifier keys
    KEY_LEFT_SHIFT, KEY_RIGHT_SHIFT,
    KEY_LEFT_CTRL, KEY_RIGHT_CTRL, 
    KEY_LEFT_ALT, KEY_RIGHT_ALT,
    KEY_LEFT_SUPER, KEY_RIGHT_SUPER,  // Windows/Cmd keys
    
    // Lock keys
    KEY_CAPS_LOCK, KEY_NUM_LOCK, KEY_SCROLL_LOCK,
    
    // Navigation keys
    KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT,
    KEY_HOME, KEY_END, KEY_PAGE_UP, KEY_PAGE_DOWN,
    
    // Editing keys
    KEY_INSERT, KEY_DELETE, KEY_BACKSPACE,
    KEY_TAB, KEY_ENTER, KEY_ESCAPE,
    
    // Function keys
    KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6,
    KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12,
    
    // System keys
    KEY_PRINT_SCREEN, KEY_PAUSE, KEY_MENU,
    
    // Keypad keys (when num lock affects them)
    KEY_KP_0, KEY_KP_1, KEY_KP_2, KEY_KP_3, KEY_KP_4,
    KEY_KP_5, KEY_KP_6, KEY_KP_7, KEY_KP_8, KEY_KP_9,
    KEY_KP_DECIMAL, KEY_KP_ENTER, KEY_KP_PLUS, KEY_KP_MINUS,
    KEY_KP_MULTIPLY, KEY_KP_DIVIDE,
    
    KEY_MAX
} arch_logical_key_t;

typedef struct {
    // Unicode codepoint (0 if not a printable character)
    uint32_t unicode;           
    
    // Logical key identifier (for special keys and when unicode == 0)
    arch_logical_key_t key;     
    
    // Event type
    bool pressed;               // true = press, false = release
    
    // Current modifier states (independent of this event)
    bool shift;                 // Any shift key currently pressed
    bool ctrl;                  // Any ctrl key currently pressed  
    bool alt;                   // Any alt key currently pressed
    bool super;                 // Any super/windows key currently pressed
    bool caps_lock;             // Caps lock is active
    bool num_lock;              // Num lock is active
    bool scroll_lock;           // Scroll lock is active
} arch_keyboard_event_t;

int arch_keyboard_get_count(void);                                           // How many devices exist?
arch_result arch_keyboard_get_info(int index, arch_keyboard_info_t *info);  // Get info for device N
arch_result arch_keyboard_init(arch_keyboard_device_t *device);             // Initialize specific device
bool arch_keyboard_has_event(arch_keyboard_device_t *device);              // Check if event available
arch_result arch_keyboard_read_event(arch_keyboard_device_t *device, arch_keyboard_event_t *event); // Read event

// Audio interface - arch-specific implementations  
typedef struct arch_audio_device arch_audio_device_t;  // Opaque handle

typedef struct {
    arch_audio_device_t *device;  // Opaque arch-specific device handle
    const char *name;             // Suggested device name (e.g., "pcspk0")
} arch_audio_info_t;

int arch_audio_get_count(void);                                      // Get number of audio devices
arch_result arch_audio_get_info(int index, arch_audio_info_t *info); // Get info for device N  
arch_result arch_audio_init(arch_audio_device_t *device);           // Initialize specific device
arch_result arch_audio_play_tone(arch_audio_device_t *device, uint32_t frequency);  // Play tone (0 = stop)
arch_result arch_audio_stop(arch_audio_device_t *device);           // Stop current sound

// Disk interface - arch-specific implementations
typedef struct arch_disk_device arch_disk_device_t;  // Opaque handle

typedef struct {
    arch_disk_device_t *device;  // Opaque arch-specific device handle
    const char *name;            // Suggested device name (e.g., "ata0", "sda")
    uint32_t block_size;         // Block size in bytes (typically 512)
    uint64_t block_count;        // Total number of blocks
    bool read_only;              // Device is read-only
} arch_disk_info_t;

int arch_disk_get_count(void);                                      // Get number of disk devices
arch_result arch_disk_get_info(int index, arch_disk_info_t *info);  // Get info for device N
arch_result arch_disk_init(arch_disk_device_t *device);            // Initialize specific device
arch_result arch_disk_read_blocks(arch_disk_device_t *device, void *buf, uint64_t start_block, uint32_t block_count);
arch_result arch_disk_write_blocks(arch_disk_device_t *device, const void *buf, uint64_t start_block, uint32_t block_count);
arch_result arch_disk_sync(arch_disk_device_t *device);            // Flush any pending writes

// Display interface - arch-specific implementations
typedef struct arch_display_device arch_display_device_t;  // Opaque handle

typedef struct {
    arch_display_device_t *device;  // Opaque arch-specific device handle
    const char *name;                // Suggested device name (e.g., "vga0", "fb0")
    uint32_t width;                  // Display width in characters/pixels
    uint32_t height;                 // Display height in characters/pixels
    uint32_t bpp;                    // Bits per pixel (0 for text mode)
    bool text_mode;                  // True for text mode, false for graphics mode
} arch_display_info_t;

int arch_display_get_count(void);                                       // Get number of display devices
arch_result arch_display_get_info(int index, arch_display_info_t *info); // Get info for device N
arch_result arch_display_init(arch_display_device_t *device);           // Initialize specific device
arch_result arch_display_set_cursor(arch_display_device_t *device, uint32_t x, uint32_t y);
arch_result arch_display_get_cursor(arch_display_device_t *device, uint32_t *x, uint32_t *y);
arch_result arch_display_write_char(arch_display_device_t *device, uint32_t x, uint32_t y, char c, uint8_t fg, uint8_t bg);
arch_result arch_display_clear_screen(arch_display_device_t *device, uint8_t fg, uint8_t bg);
arch_result arch_display_scroll_up(arch_display_device_t *device, uint32_t lines);

arch_result arch_memory_init(void);
void arch_memory_set(void *ptr, uint8_t value, uint64_t size);
void arch_memory_set_byte(void *ptr, uint8_t value, uint64_t size);
void arch_memory_set_word(void *ptr, uint16_t value, uint64_t size);
void arch_memory_set_dword(void *ptr, uint32_t value, uint64_t size);
void arch_memory_set_qword(void *ptr, uint64_t value, uint64_t size);
void arch_memory_copy(void *dest, const void *src, uint64_t size);
void arch_memory_move(void *dest, const void *src, uint64_t size);
int arch_memory_compare(const void *ptr1, const void *ptr2, uint64_t size);

#define arch_memory_zero(ptr, count) arch_memory_set(ptr, 0, count)
#define arch_memory_zero_struct(ptr) arch_memory_set(ptr, 0, sizeof(*(ptr)))

arch_result arch_memory_map_page(uint64_t virtual_addr, uint64_t physical_addr, int flags);
arch_result arch_memory_unmap_page(uint64_t virtual_addr);
void *arch_memory_allocate_page(void);
void arch_memory_deallocate_page(void *page);
void arch_memory_flush_tlb(void);


void arch_memory_map_userpages(uint64_t pdpt);


void arch_debug_printf(const char *format, ...);

#endif