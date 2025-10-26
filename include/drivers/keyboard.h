#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "arch/arch.h"

void keyboard_driver_interrupt_notify(arch_keyboard_device_t *arch_device);

arch_result keyboard_driver_init(void);

#endif