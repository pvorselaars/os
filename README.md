# OS

A minimal operating system for x86_64. Designed for learning and experimenting with low-level OS concepts such as process management, memory management, and hardware interaction.

## Features

- [x] 64-bit kernel
- [x] Bootable and debuggable via QEMU
- [x] Virtual memory management
- [x] PIT support
- [x] PIC support
- [x] Basic audio driver
- [x] RS232 support
- [x] Parallel port support
- [x] VGA support
- [x] PS/2 keyboard support
- [ ] Process management
- [ ] Bootable on real hardware
- [ ] Mouse support
- [ ] Floppy support
- [ ] ATA/IDE support
- [ ] Ethernet support
- [ ] PCI support
- [ ] SATA support
- [ ] ACPI support
- [ ] USB support

## Getting started

### Requirements

* QEMU for virtualization
* GCC
* GDB
* Make

1. Clone

   ```bash
   git clone https://github.com/pvorselaars/os
   cd os
   ```
2. Build & run

   ```bash
   make run
   ```

## License

This project is licensed under the MIT License. See [LICENSE](./LICENSE) for details.

## Contributing

Contributions, suggestions, and issues are welcome. Please open a PR or file an issue in GitHub.

## Contact

Questions? Reach out on GitHub or submit an issue.