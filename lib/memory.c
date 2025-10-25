#include "lib/memory.h"

/* Platform-agnostic memory utility functions
 * 
 * These functions provide a consistent interface while delegating
 * to architecture-specific optimized implementations.
 */

/* Generic memory set - automatically chooses optimal implementation */
void memory_set(void *ptr, const uint8_t value, const uint64_t count)
{
    /* For simple byte patterns, use byte set */
    if (value == 0 || count < 8) {
        memory_set_byte(ptr, value, count);
        return;
    }
    
    /* For aligned addresses and sizes, use larger operations */
    uint64_t addr = (uint64_t)ptr;
    
    /* Use qword operations for 8-byte aligned addresses and large counts */
    if (IS_ALIGNED(addr, 8) && count >= 64 && IS_ALIGNED(count, 8)) {
        uint64_t qword_val = ((uint64_t)value << 56) | ((uint64_t)value << 48) |
                            ((uint64_t)value << 40) | ((uint64_t)value << 32) |
                            ((uint64_t)value << 24) | ((uint64_t)value << 16) |
                            ((uint64_t)value << 8) | value;
        memory_set_qword(ptr, qword_val, count / 8);
        
        /* Handle remaining bytes */
        uint64_t remaining = count % 8;
        if (remaining > 0) {
            memory_set_byte((uint8_t *)ptr + (count - remaining), value, remaining);
        }
        return;
    }
    
    /* Use dword operations for 4-byte aligned addresses */
    if (IS_ALIGNED(addr, 4) && count >= 32 && IS_ALIGNED(count, 4)) {
        uint32_t dword_val = ((uint32_t)value << 24) | ((uint32_t)value << 16) |
                            ((uint32_t)value << 8) | value;
        memory_set_dword(ptr, dword_val, count / 4);
        
        /* Handle remaining bytes */
        uint64_t remaining = count % 4;
        if (remaining > 0) {
            memory_set_byte((uint8_t *)ptr + (count - remaining), value, remaining);
        }
        return;
    }
    
    /* Use word operations for 2-byte aligned addresses */
    if (IS_ALIGNED(addr, 2) && count >= 16 && IS_ALIGNED(count, 2)) {
        uint16_t word_val = ((uint16_t)value << 8) | value;
        memory_set_word(ptr, word_val, count / 2);
        
        /* Handle remaining bytes */
        uint64_t remaining = count % 2;
        if (remaining > 0) {
            memory_set_byte((uint8_t *)ptr + (count - remaining), value, remaining);
        }
        return;
    }
    
    /* Fall back to byte operations */
    memory_set_byte(ptr, value, count);
}