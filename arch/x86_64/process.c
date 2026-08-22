#include "arch/memory.h"
#include "arch/x86_64/memory.h"
#include "kernel/base.h"

struct arch_process_context {
   uint64_t rip;
   uint64_t rsp;
};

#include "arch/process.h"

static struct arch_process_context context;

arch_process_context_t *arch_process_context_create(uint64_t entry) {
   uint64_t stack = (uint64_t)arch_memory_allocate_page();

   arch_memory_map_page(PAGE_SIZE, stack, ARCH_MEMORY_MAP_WRITE | ARCH_MEMORY_MAP_USER);

   context.rip = entry;
   context.rsp = 2 * PAGE_SIZE;
   return &context;
}