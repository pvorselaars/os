#include "definitions.h"
#include "test.h"
#include "arch/x86_64/memory.h"
#include <stdio.h>
#include <stdlib.h>

static unsigned char memory[16 * 1024 * 1024] = {0}; /* 16 MiB */

#undef virtual_address
#define virtual_address(pa) ((void *)(memory + (pa)))
#undef physical_address
#define physical_address(va) ((uint64_t)((uint8_t *)(va) - memory))

void arch_debug_printf(const char *format, ... )
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
}

void fatal(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    exit(1);
}

char KERNEL_VMA[1];
char KERNEL_END[1];

#include "../arch/x86_64/memory.c"

void test_parse_memory_map()
{
    memory_map_entry_t *map = (memory_map_entry_t *)(memory + MEMORY_MAP);

    map[0].start = 0x0;
    map[0].size = 0x100000; /* 1 MiB */
    map[0].type = 1; // Usable RAM 

    map[1].start = 0x100000;
    map[1].size = 0x100000; /* 1 MiB */
    map[1].type = 2; // Reserved

    map[2].start = 0x200000;
    map[2].size = 0x100000; /* 1 MiB */
    map[2].type = 2;

    map[3].start = 0x300000;
    map[3].size = 0x200000; /* 2 MiB */
    map[3].type = 1;

    uint32_t *meta = (uint32_t *)(memory + MEMORY_MAP_ENTRIES);
    meta[0] = 4;
    meta[1] = sizeof(memory_map_entry_t);

    parse_memory_map();

    TEST_CASE("Memory map parsing") {
        TEST_ASSERT_EQUAL(map[0].start, region_head);
        TEST_ASSERT_EQUAL(map[3].start, region_at(region_head)->next);
    }
}

int main(void)
{
    test_parse_memory_map();

    TEST_REPORT();

    return tests_failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
