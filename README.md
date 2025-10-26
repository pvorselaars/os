# OS

A minimal operating system for learning and experimenting with low-level OS concepts. Currently supports x86_64 PC architecture.

## Structure

```
├── kernel/           # Core kernel functionality
├── lib/              # Library functions
├── drivers/          # Generic device drivers
├── arch/x86_64/      # CPU architecture specific code
├── board/pc/         # Board specific code
└── include/          # Header files
```

## Features

### Core Features
- [x] 64-bit kernel
- [x] Architecture and board separation
- [x] Bootable and debuggable via QEMU
- [x] Virtual memory management

### PC Platform Features
- [x] x86_64 boot sequence
- [x] PIT (Programmable Interval Timer) support
- [x] PIC (Programmable Interrupt Controller) support
- [x] PC speaker audio driver
- [x] RS232 serial communication
- [x] Parallel port support
- [x] VGA text mode support
- [x] PS/2 keyboard support
- [x] ATA/IDE disk controller

### Planned Features
- [ ] Process management and scheduling
- [ ] Bootable on real hardware
- [ ] File system support
- [ ] Network stack
- [ ] USB support
- [ ] ACPI support

## Getting Started

### Requirements

* QEMU
* GCC
* GDB
* Make
* TMux (optional) - For debug session management

### Quick Start

1. **Clone the repository**
    ```bash
    git clone https://github.com/pvorselaars/os
    cd os
    ```

2. **Build and run**
   ```bash
   make run
   ```

### Build Commands

| Command | Description |
|---------|-------------|
| `make` | Build for default configuration (x86_64/PC) |
| `make ARCH=x86_64 BOARD=pc` | Build for specific arch/board |
| `make pc` | Build for PC (shortcut) |
| `make run` | Run the OS in QEMU |
| `make gdb` | Start debug session with GDB |
| `make clean` | Clean build artifacts |

## License

This project is licensed under the MIT License. See [LICENSE](./LICENSE) for details.

## Contributing

Contributions, suggestions, and issues are welcome. Please open a PR or file an issue in GitHub.

## Contact

Questions? Reach out on GitHub or submit an issue.