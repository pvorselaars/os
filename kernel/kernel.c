#include "gdt.h"
#include "memory.h"
#include "interrupt.h"
#include "process.h"
#include "console.h"
#include "disk.h"
#include "utils.h"
#include "audio.h"
#include "platform/init.h"
#include "parallel.h"
#include "video.h"
#include "ps2.h"

void kernel(void)
{
	platform_init();

	while (1)
		halt();
}
