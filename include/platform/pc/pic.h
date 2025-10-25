#ifndef PIC_H
#define PIC_H

// Programmable Interval Controller (PIC) (Intel 8259)

// I/O port     Usage
// 0x20         Master PIC command port (write)
// 0x21         Master PIC data port (read/write)
// 0xA0         Slave PIC command port (write)
// 0xA1         Slave PIC data port (read/write)

#define PIC1			0x20
#define PIC1_DATA		0x21

#define PIC2			0xA0
#define PIC2_DATA		0xA1

#define PIC_INIT		0x10
#define PIC_4			0x01
#define PIC_8086		0x01

#endif