#ifndef REGISTERS_H
#define REGISTERS_H

#include "kernel/base.h"

#define X86_64_MSR_IA32_EFER  0xc0000080
#define X86_64_MSR_IA32_STAR  0xc0000081
#define X86_64_MSR_IA32_LSTAR 0xc0000082
#define X86_64_MSR_IA32_FMASK 0xc0000084

void x86_64_write_msr(uint64_t msr, uint64_t value);
uint64_t x86_64_read_msr(uint64_t msr);

#endif
