#ifndef PLATFORM_DEVICE_H
#define PLATFORM_DEVICE_H

#include "definitions.h"

/* Generic platform device hooks (block devices, framebuffer, input). These
   are optional helpers; drivers can expose more specific APIs. */

int platform_block_read(void *dst, uint64_t sector, unsigned count);
int platform_block_write(const void *src, uint64_t sector, unsigned count);

/* Framebuffer / video */
int platform_framebuffer_init(void);
void platform_framebuffer_putpixel(unsigned x, unsigned y, uint32_t color);

#endif
