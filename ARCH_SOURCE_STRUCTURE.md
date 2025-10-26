# ARCH API SOURCE CODE STRUCTURE

Based on your arch.h header, here's how to organize the implementation:

## 📁 Recommended Directory Structure

```
arch/
├── common/              # Architecture-independent code
│   ├── arch_common.c   # Common utilities, error handling
│   └── debug.c         # Debug output formatting (printf implementation)
│
├── x86_64/             # x86-64 specific implementation  
│   ├── arch_init.c     # arch_init() implementation
│   ├── arch_system.c   # arch_halt(), arch_shutdown()
│   ├── arch_debug.c    # Debug output to serial/VGA
│   └── internal/       # Internal x86 helpers
│       ├── serial.c    # Serial port (COM1) driver
│       ├── vga.c       # VGA text mode output
│       └── cpu.c       # CPU control (halt, shutdown)
│
├── arm64/              # ARM64 implementation (future)
│   ├── arch_init.c
│   ├── arch_system.c 
│   ├── arch_debug.c
│   └── internal/
│       ├── uart.c      # UART driver
│       ├── fb.c        # Framebuffer console
│       └── cpu.c       # ARM CPU control
│
└── riscv/              # RISC-V implementation (future)
    ├── arch_init.c
    ├── arch_system.c
    ├── arch_debug.c
    └── internal/
        ├── uart.c
        └── cpu.c
```

## 🎯 Implementation Strategy

### 1. **Start with arch/common/**
Common code shared across all architectures:

**arch/common/arch_common.c:**
```c
#include "arch/arch.h"

// Common error handling, utilities
const char *arch_result_string(result r) {
    switch (r) {
        case ARCH_OK: return "OK";
        case ARCH_ERROR: return "ERROR"; 
        case ARCH_INVALID: return "INVALID";
        case ARCH_UNSUPPORTED: return "UNSUPPORTED";
        default: return "UNKNOWN";
    }
}

// Common validation functions
result arch_validate_pointer(const void *ptr) {
    return (ptr != NULL) ? ARCH_OK : ARCH_INVALID;
}
```

**arch/common/debug.c:**
```c
#include "arch/arch.h"
#include <stdarg.h>

// Platform-neutral printf implementation
// Calls arch-specific arch_debug_putchar() for actual output

extern void arch_debug_putchar(char c);  // Implemented per-arch

void arch_debug_printf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    arch_debug_vprintf(format, args);
    va_end(args);
}

static void arch_debug_vprintf(const char *fmt, va_list args) {
    // Simple printf implementation
    // Calls arch_debug_putchar() for each character
    // You can use your existing printf code here
}
```

### 2. **Implement arch/x86_64/** 
x86-64 specific implementations:

**arch/x86_64/arch_init.c:**
```c
#include "arch/arch.h"
#include "arch/x86_64/internal/serial.h"
#include "arch/x86_64/internal/vga.h"

void arch_init(void) {
    // Initialize debug output channels
    x86_serial_init();  // Set up COM1
    x86_vga_init();     // Set up VGA text mode
    
    // Other x86 initialization
    // GDT, IDT setup would go here later
}
```

**arch/x86_64/arch_system.c:**
```c
#include "arch/arch.h"

void arch_halt(void) {
    __asm__ volatile("cli; hlt" ::: "memory");
    // Never returns
}

void arch_shutdown(void) {
    // ACPI shutdown or simple halt
    arch_halt();
}
```

**arch/x86_64/arch_debug.c:**
```c
#include "arch/arch.h"
#include "arch/x86_64/internal/serial.h" 
#include "arch/x86_64/internal/vga.h"

// This is called by common printf implementation
void arch_debug_putchar(char c) {
    // Output to multiple channels for reliability
    x86_serial_putchar(c);    // Primary: Serial (QEMU friendly)
    x86_vga_putchar(c);       // Secondary: Screen output
}
```

**arch/x86_64/internal/serial.c:**
```c
#include <stdint.h>

#define COM1_PORT 0x3F8

void x86_serial_init(void) {
    // Initialize COM1 serial port
    // Set baud rate, configure ports
}

void x86_serial_putchar(char c) {
    // Wait for transmit ready
    while (!(inb(COM1_PORT + 5) & 0x20)) {
        // Wait
    }
    // Send character
    outb(COM1_PORT, c);
}

static uint8_t inb(uint16_t port) {
    uint8_t result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

static void outb(uint16_t port, uint8_t value) {
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}
```

### 3. **Makefile Integration**
Update your Makefile to build the arch code:

```makefile
# Architecture selection
ARCH ?= x86_64

# Arch source files  
ARCH_COMMON_SOURCES = arch/common/arch_common.c arch/common/debug.c
ARCH_SOURCES = arch/$(ARCH)/arch_init.c arch/$(ARCH)/arch_system.c \
               arch/$(ARCH)/arch_debug.c arch/$(ARCH)/internal/serial.c \
               arch/$(ARCH)/internal/vga.c

ARCH_OBJECTS = $(ARCH_COMMON_SOURCES:.c=.o) $(ARCH_SOURCES:.c=.o)

# Include arch headers
CFLAGS += -Iinclude -DARCH_$(shell echo $(ARCH) | tr a-z A-Z)

# Link arch objects
kernel.bin: $(KERNEL_OBJECTS) $(ARCH_OBJECTS)
    ld $(LDFLAGS) -o $@ $^
```

## 🚀 Benefits of This Structure

### ✅ **Clean Separation**
- **Common code**: Shared printf, utilities
- **Arch-specific**: Hardware access, initialization  
- **Internal**: Private implementation details

### ✅ **Scalable**
- Easy to add ARM64: just create `arch/arm64/`
- Common code stays the same
- Each arch optimizes for its hardware

### ✅ **Testable**
- Can compile different architectures separately
- Common code can be unit tested
- Clear interface boundaries

### ✅ **Professional Structure**
- Similar to Linux kernel organization
- Easy for other developers to understand
- Good separation of concerns

## 🎯 Implementation Order

1. **Start Simple**: Just `arch/x86_64/arch_system.c` with basic halt/shutdown
2. **Add Debug**: Implement serial output for `arch_debug_printf()`  
3. **Polish Init**: Add proper `arch_init()` with serial setup
4. **Add Features**: VGA output, better error handling
5. **Optimize**: Performance improvements, multiple output channels

This structure will serve you well as your kernel grows!