# OS

A minimal, multi-platform operating system for learning and experimenting with low-level OS concepts. Supports x86_64 PC and ARM64 Raspberry Pi 4 architectures.

## Structure

```
├── kernel/           # Core kernel functionality
├── lib/              # Library functions
├── arch/x86_64/      # Architecture-specific code
├── arch/aarch64/
├── platform/pc/      # Platform-specific drivers
├── platform/raspberrypi4
└── include/          # Header files
```


## Features

### Core Features
- [x] 64-bit kernel
- [x] Multi-platform build system
- [x] Bootable and debuggable via QEMU
- [x] Virtual memory management
- [x] Basic process management

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
- [ ] ARM64/AArch64 support for Raspberry Pi 4
- [ ] Advanced process management and scheduling
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


3. **Build and run (PC platform - default)**
   ```bash
   make run
   ```

### Build Commands

| Command | Description |
|---------|-------------|
| `make` | Build for default platform (PC) |
| `make PLATFORM=<platform>` | Build for specific platform |
| `make pc` | Build for PC (shortcut) |
| `make raspberrypi4` | Build for Raspberry Pi 4 (shortcut) |
| `make run` | Run the OS in QEMU |
| `make gdb` | Start debug session with GDB |
| `make clean` | Clean build artifacts |

## License

This project is licensed under the MIT License. See [LICENSE](./LICENSE) for details.

## Contributing

Contributions, suggestions, and issues are welcome. Please open a PR or file an issue in GitHub.

## Contact

Questions? Reach out on GitHub or submit an issue.