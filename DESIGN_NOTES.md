/* KERNEL API DESIGN NOTES & LEARNING ROADMAP */

/*
 * This document summarizes all design improvements and provides a clear
 * learning path for implementing a clean, portable kernel API.
 */

/* ==================== DESIGN PHILOSOPHY ==================== */

/*
 * CORE PRINCIPLES:
 * 1. SIMPLICITY OVER FEATURES - Easy to understand > powerful but complex
 * 2. PORTABILITY FIRST - Same API works on x86, ARM, RISC-V
 * 3. PROGRESSIVE LEARNING - Start simple, add complexity gradually
 * 4. SELF-DOCUMENTING - Names explain what functions do
 * 5. FAIL FAST AND LOUD - Clear errors > silent failures
 * 6. ZERO SURPRISES - Consistent behavior across architectures
 */

/* ==================== API EVOLUTION ==================== */

/*
 * PROGRESSION FROM COMPLEX TO SIMPLE:
 * 
 * ORIGINAL PROBLEMS:
 * - Magic numbers everywhere (0x20, 0x21, 0x24)
 * - Manual setup (arch_gdt_init, arch_interrupt_init, platform_pic_remap)
 * - Platform-specific knowledge (PIC, EOI, vectors)
 * - x86-only assumptions (I/O ports)
 * - Complex registration (arch_register_interrupt(0x20, handler))
 * 
 * FINAL SOLUTION:
 * - Logical names (IRQ_TIMER instead of 0x20)
 * - One-call setup (system_init() does everything)
 * - Hardware abstraction (no PIC/GIC knowledge needed)
 * - Cross-platform (works on any architecture)
 * - Simple registration (interrupt_enable(IRQ_TIMER, handler))
 */

/* ==================== FINAL API SUMMARY ==================== */

/*
 * ESSENTIAL FUNCTIONS (only ~10 functions needed):
 * 
 * LEVEL 1 - BASICS:
 * - void system_init(void)                     // Initialize everything
 * - void debug_print(char c)                   // Print for debugging
 * - void system_halt(void)                     // Stop system
 * 
 * LEVEL 2 - INTERRUPTS:
 * - result_t interrupt_enable(irq_t, handler)  // Register interrupt
 * - void interrupts_on/off(void)               // Enable/disable interrupts
 * 
 * LEVEL 3 - DEVICES:
 * - uint8_t device_read(device_t, reg)         // Read device register
 * - result_t device_write(device_t, reg, val)  // Write device register
 * 
 * LEVEL 4 - TIMING:
 * - result_t timer_start(frequency)            // Start system timer
 * - uint64_t timer_ticks(void)                 // Get current time
 * - void delay_ms(milliseconds)                // Delay function
 * 
 * LEVEL 5 - MEMORY:
 * - void memory_init(void)                     // Initialize memory system
 * - void *kmalloc/kzalloc(size)                // Allocate kernel memory
 * - void kfree(ptr)                            // Free kernel memory
 * - void *page_alloc/page_free(count)          // Page-level allocation
 * 
 * LEVEL 6 - VIRTUAL MEMORY:
 * - arch_page_table_t vm_create_table(void)    // Create page table
 * - result_t vm_map(table, vaddr, paddr, flags) // Map virtual to physical
 * - void vm_activate(table)                    // Switch to page table
 * - void vm_flush_tlb(void)                    // Flush TLB cache
 */

/* ==================== API LAYER DISTINCTION ==================== */

/*
 * ARCHITECTURE API vs KERNEL API:
 * 
 * ARCH API (Low-level hardware abstraction):
 * - Hides hardware differences between x86, ARM, RISC-V
 * - Provides minimal hardware access primitives
 * - Same interface, different implementation per architecture
 * - Examples: arch_system_init(), arch_device_read(), arch_interrupts_enable()
 * 
 * KERNEL API (High-level kernel services):
 * - Uses arch API internally to provide kernel services
 * - Adds higher-level abstractions and policies
 * - Same implementation works on all architectures
 * - Examples: kproc_create(), kmalloc(), kfs_open()
 * 
 * LAYERING:
 * [Kernel API] -> [Arch API] -> [Hardware]
 * 
 * User code calls Kernel API
 * Kernel API calls Arch API  
 * Arch API talks to hardware
 * 
 * EXAMPLE - Creating a process:
 * 1. User calls: process_create(my_function, data, 4096)
 * 2. Kernel API: Uses kmalloc() to allocate process structure
 * 3. Kernel kmalloc(): Calls arch_page_alloc() for memory
 * 4. Arch arch_page_alloc(): Talks to x86 MMU / ARM MMU / RISC-V MMU
 * 
 * EXAMPLE - Timer interrupt:
 * 1. User calls: timer_set_callback(my_callback, data) 
 * 2. Kernel API: Registers callback, calls kirq_register()
 * 3. Kernel kirq_register(): Calls arch_interrupt_register()
 * 4. Arch arch_interrupt_register(): Programs x86 PIC / ARM GIC / RISC-V PLIC
 * 
 * EXAMPLE - Device I/O:
 * 1. User calls: device_write(uart_handle, "Hello", 5)
 * 2. Kernel device API: Calls registered device driver write function
 * 3. Device driver: Calls arch_device_write(DEV_UART, UART_DATA_REG, char)
 * 4. Arch arch_device_write(): Uses x86 outb() / ARM MMIO / RISC-V MMIO
 */

/* ==================== NAMING CONSISTENCY FIXES ==================== */

/*
 * PROBLEM: Overlapping function names between arch and kernel APIs
 * 
 * CONFLICTS IDENTIFIED:
 * - memory_init() (both arch and kernel)
 * - interrupt_enable() (both arch and kernel) 
 * - device_read() (ambiguous which layer)
 * - vm_* functions (mixed between layers)
 * 
 * SOLUTION: Consistent prefixing scheme
 * 
 * ARCH API NAMING:
 * - All functions: arch_ prefix
 * - Examples: arch_system_init(), arch_device_read(), arch_interrupts_enable()
 * 
 * KERNEL API NAMING:
 * - Memory: kmem_, kvm_, kpage_ prefixes
 * - Interrupts: kirq_ prefix  
 * - Processes: kproc_ prefix (implied)
 * - Devices: kdev_ prefix (implied)
 * - Filesystem: kfs_ prefix (implied)
 * - Synchronization: No prefix (mutex_, semaphore_, spinlock_)
 * 
 * BENEFITS:
 * - No naming conflicts between layers
 * - Clear which layer you're calling
 * - Easy to grep/search for functions
 * - Consistent with kernel naming conventions
 */

/* ==================== ARCH API SPECIFICATION ==================== */

/*
 * PORTABLE ARCHITECTURE ABSTRACTION LAYER:
 * This provides hardware abstraction - same interface on all architectures
 * ALL FUNCTIONS HAVE arch_ PREFIX FOR CLARITY
 */

/*
#ifndef ARCH_ULTIMATE_H
#define ARCH_ULTIMATE_H

#include <stdint.h>
#include <stddef.h>

// CORE PRINCIPLES:
// 1. ONE HEADER - everything you need is here
// 2. ZERO SURPRISE - same behavior on all architectures  
// 3. FAIL LOUD - obvious errors with clear messages
// 4. PROGRESSIVE - start simple, add complexity as needed
// 5. SELF-DOCUMENTING - names explain everything

// BASIC TYPES
typedef enum {
    OK = 0,
    ERROR = -1,
    INVALID = -2,
    UNSUPPORTED = -3
} result_t;

// DEVICES - Simple device IDs, no confusing pointers
typedef enum {
    DEV_TIMER = 0,
    DEV_UART = 1,
    DEV_KEYBOARD = 2,
    DEV_NETWORK = 3,
    DEV_MAX = 8
} device_t;

// Self-documenting device register offsets
#define UART_DATA_REG       0
#define UART_STATUS_REG     1
#define UART_CONTROL_REG    2
#define TIMER_VALUE_REG     0
#define TIMER_CONTROL_REG   1

// Self-documenting bit flags
#define UART_TX_READY       0x01
#define UART_RX_READY       0x02
#define TIMER_ENABLED       0x01

// INTERRUPTS - Logical interrupt types, same on all architectures
typedef enum {
    IRQ_TIMER = 0,
    IRQ_KEYBOARD = 1,
    IRQ_UART = 2,
    IRQ_NETWORK = 3,
    IRQ_MAX = 8
} irq_t;

typedef void (*irq_handler_t)(void);

// PROGRESSIVE API LEVELS

// === LEVEL 1: ABSOLUTE BASICS ===
void arch_system_init(void);                // Initialize everything with defaults  
void arch_system_halt(void);                // Stop the system

// Debug output - arch decides best available output channel
void arch_debug_puts(const char *str);      // Output string to best debug channel
void arch_debug_putc(char c);               // Output single character

// Optional: More specific control if needed
void arch_debug_printf(const char *fmt, ...); // Printf-style formatted output (optional)
void arch_debug_write(const char *data, size_t len); // Raw byte output (optional)

// Optional: Explicit output channel selection (advanced)
// void arch_debug_serial_puts(const char *str);   // Force serial output
// void arch_debug_console_puts(const char *str);  // Force console output

// === LEVEL 2: INTERRUPTS ===  
result_t arch_interrupt_register(irq_t irq, irq_handler_t handler);
void arch_interrupts_enable(void);          // Enable all interrupts
void arch_interrupts_disable(void);         // Disable all interrupts

// === LEVEL 3: DEVICES ===
uint8_t arch_device_read(device_t dev, uint32_t reg);
result_t arch_device_write(device_t dev, uint32_t reg, uint8_t value);

// === LEVEL 4: TIMING ===
result_t arch_timer_init(uint32_t frequency_hz);
uint64_t arch_timer_get_ticks(void);
void arch_delay_ms(uint32_t milliseconds);

// === LEVEL 5: MEMORY ===
void arch_memory_init(void);
void *arch_page_alloc(size_t count);        // Allocate physical pages
void arch_page_free(void *pages, size_t count);
size_t arch_get_page_size(void);            // Get architecture page size

// === LEVEL 6: VIRTUAL MEMORY ===
typedef struct arch_page_table* arch_page_table_t;
arch_page_table_t arch_vm_create_table(void);
void arch_vm_destroy_table(arch_page_table_t table);
result_t arch_vm_map(arch_page_table_t table, uint64_t vaddr, uint64_t paddr, uint32_t flags);
result_t arch_vm_unmap(arch_page_table_t table, uint64_t vaddr);
void arch_vm_activate(arch_page_table_t table);
void arch_vm_flush_tlb(void);

// CONVENIENCE MACROS - Make common operations super readable
#define UART_SEND(c)        arch_device_write(DEV_UART, UART_DATA_REG, c)
#define UART_RECV()         arch_device_read(DEV_UART, UART_DATA_REG)
#define UART_READY()        (arch_device_read(DEV_UART, UART_STATUS_REG) & UART_TX_READY)
#define DEBUG_PRINT(str)    arch_debug_puts(str)
#define DEBUG_PRINTF(...)   arch_debug_printf(__VA_ARGS__)
#define TIMER_START(hz)     arch_timer_init(hz)
#define TIMER_GET()         arch_timer_get_ticks()
#define WAIT_MS(ms)         arch_delay_ms(ms)
#define IRQ_ON()            arch_interrupts_enable()
#define IRQ_OFF()           arch_interrupts_disable()

// ERROR HANDLING - Built-in debugging that can be enabled/disabled
#ifdef ARCH_DEBUG
    #define DEBUG(fmt, ...)     printf("[ARCH] " fmt "\n", ##__VA_ARGS__)
    #define ASSERT(cond)        do { if (!(cond)) { printf("ASSERT FAILED: %s:%d %s\n", __FILE__, __LINE__, #cond); system_halt(); } } while(0)
#else
    #define DEBUG(fmt, ...)     
    #define ASSERT(cond)        
#endif

// Safe versions that never fail silently
uint8_t arch_device_read_safe(device_t dev, uint32_t reg);  // Returns 0 on error, logs message
result_t arch_device_write_safe(device_t dev, uint32_t reg, uint8_t value);  // Logs on error

// DEBUG OUTPUT IMPLEMENTATION NOTES:
//
// WHERE DOES DEBUG OUTPUT GO? - Arch layer decides based on what's available
//
// PHILOSOPHY: arch_debug_*() outputs to the "best available debug channel"
// The arch implementation chooses the most appropriate output for the platform:
//
// x86 PC: 
//   - Serial port (COM1) - reliable, works in QEMU, real hardware
//   - VGA text mode - visible on screen but harder to capture
//   - Both serial + VGA for redundancy
//
// ARM (Raspberry Pi):
//   - UART (mini UART or PL011) - most reliable  
//   - Framebuffer console - if graphics available
//   - GPIO pins for logic analyzer - for hardware debugging
//
// QEMU/Emulator:
//   - Serial output redirected to host terminal
//   - Easy to capture and log
//
// Real Hardware:
//   - Serial cable to development machine
//   - On-screen display if available
//
// EARLY BOOT (before most hardware initialized):
//   - May fall back to simpler output (VGA text mode on x86)
//   - Or buffer output until serial is available
//
// ARCH IMPLEMENTATION EXAMPLES:
//
// void arch_debug_puts(const char *str) {
//     #ifdef ARCH_X86_64
//         serial_puts(str);           // Send to COM1
//         vga_puts(str);             // Also show on screen
//     #endif
//     
//     #ifdef ARCH_ARM64  
//         uart_puts(str);            // Send to UART
//         if (framebuffer_ready()) {
//             fb_puts(str);          // Also show on screen if available
//         }
//     #endif
// }
//
// BENEFITS OF ARCH-DECIDES APPROACH:
// - Kernel code doesn't care about output details
// - Each architecture uses its best debug channel
// - Easy to change debug output method per platform
// - Can enable multiple outputs for redundancy
// - Works in both emulators and real hardware
//
// ALTERNATIVE: Explicit output selection
// - arch_debug_puts_serial(str) - force serial output
// - arch_debug_puts_console(str) - force screen output  
// - More control but more complex API

#endif // ARCH_ULTIMATE_H
*/

/* ==================== KERNEL API SPECIFICATIONS ==================== */

/*
 * HIGH-LEVEL KERNEL SERVICE APIS:
 * These provide kernel services and use the arch API internally
 */

/*
// KERNEL ERROR HANDLING API (uses arch API for debug output)
#ifndef KERNEL_ERROR_H
#define KERNEL_ERROR_H

typedef enum {
    KERNEL_OK = 0,
    KERNEL_ERROR = -1,
    KERNEL_INVALID_PARAM = -2,
    KERNEL_OUT_OF_MEMORY = -3,
    KERNEL_NOT_FOUND = -4,
    KERNEL_TIMEOUT = -5
} kernel_error_t;

const char *kernel_error_string(kernel_error_t error);

#endif
*/

/*
// KERNEL LOGGING API (uses arch debug_print() internally)
#ifndef KERNEL_LOG_H
#define KERNEL_LOG_H

typedef enum {
    LOG_DEBUG = 0,
    LOG_INFO = 1,
    LOG_WARNING = 2,
    LOG_ERROR = 3
} log_level_t;

void log_init(void);
void log_message(log_level_t level, const char *format, ...);

#define DEBUG(fmt, ...)   log_message(LOG_DEBUG, fmt, ##__VA_ARGS__)
#define INFO(fmt, ...)    log_message(LOG_INFO, fmt, ##__VA_ARGS__)
#define WARNING(fmt, ...) log_message(LOG_WARNING, fmt, ##__VA_ARGS__)
#define ERROR(fmt, ...)   log_message(LOG_ERROR, fmt, ##__VA_ARGS__)

#endif
*/

/*
// KERNEL MEMORY API (uses arch_page_alloc() internally)
#ifndef KERNEL_MEMORY_H
#define KERNEL_MEMORY_H

#include <stddef.h>

// Physical memory management - kernel layer provides allocation policies
void kmem_init(void);
void *kmalloc(size_t size);
void *kzalloc(size_t size);  // Zero-initialized malloc
void kfree(void *ptr);
void *kpage_alloc(size_t count);         // Kernel page allocator (uses arch layer)
void kpage_free(void *pages, size_t count);

// Virtual memory management - kernel layer provides address space management
typedef struct kvm_space* kvm_space_t;
typedef uintptr_t virtual_addr_t;
typedef uintptr_t physical_addr_t;

#define KVM_READ     0x01
#define KVM_WRITE    0x02
#define KVM_EXEC     0x04
#define KVM_USER     0x08

kvm_space_t kvm_create_space(void);          // Create address space (uses arch_vm_create_table)
void kvm_destroy_space(kvm_space_t space);
kernel_error_t kvm_map_page(kvm_space_t space, virtual_addr_t vaddr, physical_addr_t paddr, uint32_t flags);
kernel_error_t kvm_unmap_page(kvm_space_t space, virtual_addr_t vaddr);
void kvm_activate_space(kvm_space_t space);  // Switch to address space (uses arch_vm_activate)

// Memory regions for processes - higher level than single pages
kernel_error_t kvm_map_region(kvm_space_t space, virtual_addr_t vaddr, size_t size, uint32_t flags);
kernel_error_t kvm_unmap_region(kvm_space_t space, virtual_addr_t vaddr, size_t size);

typedef struct {
    size_t total_pages;
    size_t free_pages;
    size_t used_pages;
    size_t kernel_pages;
    size_t user_pages;
} memory_info_t;

memory_info_t kmem_get_info(void);

#endif
*/

/*
// KERNEL INTERRUPT API (higher level than arch interrupt_enable())
#ifndef KERNEL_INTERRUPT_H
#define KERNEL_INTERRUPT_H

typedef enum {
    IRQ_TIMER = 0,
    IRQ_KEYBOARD = 1,
    IRQ_SERIAL = 2,
    IRQ_MAX = 16
} irq_number_t;

typedef void (*interrupt_handler_t)(void);

kernel_error_t kirq_register(irq_number_t irq, interrupt_handler_t handler);
kernel_error_t kirq_enable(irq_number_t irq);
kernel_error_t kirq_disable(irq_number_t irq);
void kirq_enable_all(void);
void kirq_disable_all(void);

#endif
*/

/*
// KERNEL TIMER API (uses arch timer_start() and interrupt_enable())
#ifndef KERNEL_TIMER_H
#define KERNEL_TIMER_H

#include <stdint.h>

typedef void (*timer_callback_t)(void *data);

kernel_error_t timer_init(void);
kernel_error_t timer_start(uint32_t frequency_hz);
void timer_stop(void);
uint64_t timer_get_ticks(void);
uint64_t timer_get_milliseconds(void);
void timer_delay_ms(uint32_t milliseconds);
kernel_error_t timer_set_callback(timer_callback_t callback, void *data);

#endif
*/

/*
// KERNEL DEVICE API (uses arch device_read/write() internally)
#ifndef KERNEL_DEVICE_H
#define KERNEL_DEVICE_H

#include <stdint.h>
#include <stddef.h>

// Device handle - opaque to users
typedef struct device* device_handle_t;

// Device types
typedef enum {
    DEVICE_TYPE_CHAR = 0,     // Character devices (UART, keyboard)
    DEVICE_TYPE_BLOCK = 1,    // Block devices (disk, flash)
    DEVICE_TYPE_NETWORK = 2,  // Network devices
    DEVICE_TYPE_MAX = 8
} device_type_t;

// Device operations structure
typedef struct {
    kernel_error_t (*open)(device_handle_t dev);
    kernel_error_t (*close)(device_handle_t dev);
    ssize_t (*read)(device_handle_t dev, void *buffer, size_t size);
    ssize_t (*write)(device_handle_t dev, const void *buffer, size_t size);
    kernel_error_t (*ioctl)(device_handle_t dev, uint32_t cmd, void *arg);
} device_ops_t;

// Device registration and management
kernel_error_t device_init(void);
kernel_error_t device_register(const char *name, device_type_t type, const device_ops_t *ops);
device_handle_t device_open(const char *name);
kernel_error_t device_close(device_handle_t dev);

// High-level device I/O (uses device operations)
ssize_t device_read(device_handle_t dev, void *buffer, size_t size);
ssize_t device_write(device_handle_t dev, const void *buffer, size_t size);
kernel_error_t device_ioctl(device_handle_t dev, uint32_t cmd, void *arg);

// Standard device names
#define DEVICE_CONSOLE  "console"   // Primary console output
#define DEVICE_UART0    "uart0"     // First serial port
#define DEVICE_KEYBOARD "keyboard"  // Primary keyboard
#define DEVICE_TIMER    "timer"     // System timer
#define DEVICE_DISK0    "disk0"     // First disk drive

#endif
*/

/*
// KERNEL PROCESS API (uses arch memory and timer APIs)
#ifndef KERNEL_PROCESS_H
#define KERNEL_PROCESS_H

#include <stdint.h>

typedef uint32_t process_id_t;
typedef void (*process_function_t)(void *data);

#define PROCESS_ID_INVALID  0

typedef enum {
    PROCESS_READY = 0,
    PROCESS_RUNNING = 1,
    PROCESS_SLEEPING = 2,
    PROCESS_DEAD = 3
} process_state_t;

kernel_error_t process_init(void);
process_id_t process_create(process_function_t func, void *data, uint32_t stack_size);
kernel_error_t process_start(process_id_t pid);
kernel_error_t process_kill(process_id_t pid);
void process_exit(void);
void process_yield(void);
void process_sleep(uint32_t milliseconds);
void process_schedule(void);
uint32_t process_get_count(void);
process_id_t process_get_current(void);

#endif
*/

/*
// KERNEL FILESYSTEM API (essential for any real OS)
#ifndef KERNEL_FS_H
#define KERNEL_FS_H

#include <stdint.h>
#include <stddef.h>

// File handle
typedef struct file* file_handle_t;
typedef uint32_t inode_t;

// File types and permissions
#define FS_TYPE_FILE     1
#define FS_TYPE_DIR      2
#define FS_PERM_READ     0x01
#define FS_PERM_WRITE    0x02
#define FS_PERM_EXEC     0x04

typedef struct {
    char name[256];
    inode_t inode;
    size_t size;
    uint32_t type;
    uint32_t permissions;
} file_info_t;

// Core filesystem operations
kernel_error_t fs_init(void);
file_handle_t fs_open(const char *path, uint32_t flags);
kernel_error_t fs_close(file_handle_t file);
ssize_t fs_read(file_handle_t file, void *buffer, size_t size);
ssize_t fs_write(file_handle_t file, const void *buffer, size_t size);
kernel_error_t fs_seek(file_handle_t file, size_t offset);

// Directory operations
kernel_error_t fs_mkdir(const char *path);
kernel_error_t fs_rmdir(const char *path);
kernel_error_t fs_list_dir(const char *path, file_info_t *entries, size_t max_entries);

// File management
kernel_error_t fs_create(const char *path);
kernel_error_t fs_delete(const char *path);
kernel_error_t fs_rename(const char *old_path, const char *new_path);
kernel_error_t fs_get_info(const char *path, file_info_t *info);

#endif
*/

/*
// KERNEL SYNCHRONIZATION API (essential for multitasking)
#ifndef KERNEL_SYNC_H
#define KERNEL_SYNC_H

#include <stdint.h>

// Mutex for mutual exclusion
typedef struct mutex* mutex_t;
mutex_t mutex_create(void);
void mutex_destroy(mutex_t mutex);
kernel_error_t mutex_lock(mutex_t mutex);
kernel_error_t mutex_trylock(mutex_t mutex);
void mutex_unlock(mutex_t mutex);

// Semaphore for resource counting
typedef struct semaphore* semaphore_t;
semaphore_t semaphore_create(uint32_t initial_count);
void semaphore_destroy(semaphore_t sem);
kernel_error_t semaphore_wait(semaphore_t sem);
kernel_error_t semaphore_signal(semaphore_t sem);

// Spinlock for short critical sections
typedef struct {
    volatile uint32_t locked;
} spinlock_t;

void spinlock_init(spinlock_t *lock);
void spinlock_acquire(spinlock_t *lock);
kernel_error_t spinlock_tryacquire(spinlock_t *lock);
void spinlock_release(spinlock_t *lock);

#endif
*/

/*
// KERNEL SYSTEM CALLS API (user/kernel interface)
#ifndef KERNEL_SYSCALL_H
#define KERNEL_SYSCALL_H

#include <stdint.h>

// System call numbers
typedef enum {
    SYS_EXIT = 0,
    SYS_READ = 1,
    SYS_WRITE = 2,
    SYS_OPEN = 3,
    SYS_CLOSE = 4,
    SYS_FORK = 5,
    SYS_EXEC = 6,
    SYS_WAIT = 7,
    SYS_MALLOC = 8,
    SYS_FREE = 9,
    SYS_MAX = 32
} syscall_number_t;

// System call handler function type
typedef int64_t (*syscall_handler_t)(uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4);

// System call registration and handling
kernel_error_t syscall_init(void);
kernel_error_t syscall_register(syscall_number_t num, syscall_handler_t handler);
int64_t syscall_dispatch(syscall_number_t num, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4);

// Built-in system call implementations
int64_t sys_read(uint64_t fd, uint64_t buffer, uint64_t size, uint64_t unused);
int64_t sys_write(uint64_t fd, uint64_t buffer, uint64_t size, uint64_t unused);
int64_t sys_open(uint64_t path, uint64_t flags, uint64_t unused1, uint64_t unused2);
int64_t sys_close(uint64_t fd, uint64_t unused1, uint64_t unused2, uint64_t unused3);

#endif
*/

/* ==================== DESIGN PATTERNS ==================== */

/*
 * 1. CONSISTENT NAMING:
 *    - module_action() pattern (timer_start, device_read, interrupts_on)
 *    - No mixed styles (arch_enable_interrupts vs timer_init vs inb)
 * 
 * 2. LOGICAL ABSTRACTIONS:
 *    - Use IRQ_TIMER instead of vector 0x20
 *    - Use DEV_UART instead of port 0x3F8 or MMIO 0x3F201000
 *    - Use UART_TX_READY instead of bit mask 0x20
 * 
 * 3. ARCHITECTURE INDEPENDENCE:
 *    - Same device_read() works with x86 ports OR ARM memory-mapped I/O
 *    - Same interrupt_enable() works with PIC, GIC, or PLIC
 *    - Same API, different implementations per architecture
 * 
 * 4. PROGRESSIVE COMPLEXITY:
 *    - Level 1: 3-line Hello World kernel
 *    - Level 2: Add interrupt handling
 *    - Level 3: Add device I/O
 *    - Level 4: Full-featured kernel
 * 
 * 5. CONVENIENCE MACROS:
 *    - UART_SEND(c) instead of device_write(DEV_UART, UART_DATA_REG, c)
 *    - IRQ_ON() instead of interrupts_on()
 *    - Self-documenting, readable code
 * 
 * 6. ERROR HANDLING:
 *    - Return result_t for functions that can fail
 *    - Provide _safe() versions that never fail silently
 *    - Built-in debug logging when ARCH_DEBUG is defined
 */

/* ==================== IMPLEMENTATION STRATEGY ==================== */

/*
 * ARCHITECTURE ABSTRACTION APPROACH:
 * 
 * 1. DEFINE PORTABLE API (arch_ultimate.h):
 *    - Logical device IDs, IRQ types, function signatures
 *    - Same header works on all architectures
 * 
 * 2. IMPLEMENT PER-ARCHITECTURE:
 *    - arch/x86_64/arch_impl.c - x86 implementation using PIC, I/O ports
 *    - arch/arm64/arch_impl.c - ARM implementation using GIC, MMIO
 *    - arch/riscv/arch_impl.c - RISC-V implementation using PLIC, MMIO
 * 
 * 3. INTERNAL MAPPING TABLES:
 *    - Map logical IRQ_TIMER to architecture interrupt numbers
 *    - Map logical DEV_UART to hardware addresses/ports
 *    - Hide all architecture-specific details
 * 
 * EXAMPLE MAPPINGS:
 * 
 * IRQ_TIMER:
 *   x86:    Vector 0x20 (PIC timer)
 *   ARM:    IRQ 64 (Generic Timer)
 *   RISC-V: IRQ 7 (Machine Timer)
 * 
 * DEV_UART:
 *   x86:    I/O port 0x3F8 (COM1)
 *   ARM:    MMIO 0x3F201000 (UART0)
 *   RISC-V: MMIO 0x10013000 (UART)
 * 
 * device_read(DEV_UART, reg):
 *   x86:    inb(0x3F8 + reg)
 *   ARM:    *(volatile uint8_t*)(0x3F201000 + reg)
 *   RISC-V: *(volatile uint8_t*)(0x10013000 + reg)
 */

/* ==================== LEARNING ROADMAP ==================== */

/*
 * PHASE 1: BASIC SYSTEM (Week 1)
 * Implement Level 1 functions:
 * - system_init() - Initialize hardware (GDT, IDT, basic setup)
 * - debug_print() - Output single character (serial or VGA)
 * - system_halt() - Stop system (hlt instruction)
 * 
 * TEST: 3-line Hello World kernel
 * 
 * PHASE 2: INTERRUPT FOUNDATION (Week 2)  
 * Implement Level 2 functions:
 * - interrupts_on/off() - CLI/STI instructions
 * - interrupt_enable() - Register interrupt handlers
 * - Basic interrupt handler dispatch
 * 
 * TEST: Timer interrupt prints dots
 * 
 * PHASE 3: DEVICE ABSTRACTION (Week 3)
 * Implement Level 3 functions:
 * - device_read/write() - Abstract I/O operations
 * - Device mapping tables (IRQ->vector, device->address)
 * - Error handling and validation
 * 
 * TEST: UART echo program
 * 
 * PHASE 4: TIMING SYSTEM (Week 4)
 * Implement Level 4 functions:
 * - timer_start() - Configure hardware timer
 * - timer_ticks() - Track system uptime
 * - delay_ms() - Busy-wait delays
 * 
 * TEST: Multitasking kernel with time slicing
 * 
 * PHASE 5: MEMORY MANAGEMENT (Week 5)
 * Implement Level 5 functions:
 * - memory_init() - Set up memory allocators
 * - kmalloc/kfree() - Dynamic memory allocation
 * - page_alloc/page_free() - Physical page management
 * 
 * TEST: Dynamic data structures, process stacks
 * 
 * PHASE 6: VIRTUAL MEMORY (Week 6)
 * Implement Level 6 functions:
 * - vm_create_table() - Create page tables
 * - vm_map/unmap() - Map virtual addresses to physical
 * - vm_activate() - Switch address spaces
 * 
 * TEST: Process isolation, user/kernel separation
 * 
 * PHASE 7: PORTABILITY (Week 7+)
 * Port to second architecture:
 * - Implement same API for ARM or RISC-V
 * - Verify identical kernel code works
 * - Learn architecture differences
 * 
 * TEST: Same kernel binary runs on multiple architectures
 */

/* ==================== IMPLEMENTATION TIPS ==================== */

/*
 * START SIMPLE:
 * - Get basic functions working first
 * - Don't try to implement everything at once
 * - Test each level before moving to next
 * 
 * USE DEBUGGING:
 * - Add lots of debug prints to understand flow
 * - Use QEMU monitor to inspect hardware state
 * - Enable ARCH_DEBUG for automatic logging
 * 
 * FOCUS ON ONE ARCHITECTURE:
 * - Master x86 implementation first
 * - Then port to ARM to verify portability
 * - Don't try to support multiple architectures initially
 * 
 * KEEP IT WORKING:
 * - Always have a working kernel
 * - Make small, incremental changes
 * - Test frequently in QEMU
 * 
 * LEARN BY COMPARISON:
 * - Look at Linux, xv6, or other simple kernels
 * - Understand how they abstract hardware
 * - Adapt their patterns to your simpler API
 */

/* ==================== COMMON PITFALLS ==================== */

/*
 * AVOID THESE MISTAKES:
 * 
 * 1. TRYING TO BE TOO GENERIC:
 *    - Don't over-abstract - keep it simple
 *    - Focus on 2-3 architectures max
 * 
 * 2. FORGETTING ERROR HANDLING:
 *    - Always check return values
 *    - Provide clear error messages
 * 
 * 3. MIXING ABSTRACTION LEVELS:
 *    - Don't put arch-specific code in portable layer
 *    - Keep hardware details in arch implementation
 * 
 * 4. PREMATURE OPTIMIZATION:
 *    - Focus on correctness first
 *    - Make it work, then make it fast
 * 
 * 5. COMPLEX INITIALIZATION:
 *    - system_init() should handle everything
 *    - Avoid complex setup sequences
 */

/* ==================== SUCCESS METRICS ==================== */

/*
 * YOU'LL KNOW IT'S WORKING WHEN:
 * 
 * 1. 3-LINE HELLO WORLD:
 *    system_init();
 *    debug_print('H');
 *    system_halt();
 * 
 * 2. READABLE KERNEL CODE:
 *    interrupt_enable(IRQ_TIMER, my_handler);
 *    TIMER_START(100);
 *    IRQ_ON();
 * 
 * 3. PORTABLE IMPLEMENTATION:
 *    Same kernel source compiles and runs on x86 and ARM
 * 
 * 4. NO MAGIC NUMBERS:
 *    All hardware details hidden behind logical names
 * 
 * 5. EASY TO EXTEND:
 *    Adding new device type takes <10 lines of code
 */

/* ==================== FINAL THOUGHTS ==================== */

/*
 * LEARNING GOALS:
 * - Understand hardware abstraction principles
 * - Learn portable kernel design patterns  
 * - Master interrupt handling concepts
 * - Gain experience with multiple architectures
 * 
 * SKILLS DEVELOPED:
 * - API design and abstraction
 * - Cross-platform programming
 * - Low-level systems programming
 * - Hardware interface design
 * 
 * REAL-WORLD APPLICATION:
 * - These patterns are used in production kernels
 * - Portable device driver frameworks
 * - Embedded systems programming
 * - Hypervisor and virtualization software
 * 
 * The goal isn't just to build a kernel - it's to understand
 * how to create clean, maintainable system software that
 * works across different hardware platforms.
 */