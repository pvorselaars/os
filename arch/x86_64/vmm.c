#include "arch/x86_64/memory.h"
#include "arch/x86_64/vmm.h"

#include "arch/arch.h"
#include "lib/memory.h"
#include "lib/utils.h"

static pml4e *pml4 = virtual_address(PML4_ADDRESS);

result_t arch_vmm_map_page(uint64_t virtual_address, uint64_t physical_address, int flags)
{
	assert(IS_ALIGNED(physical_address, PAGE_SIZE));
	assert(IS_ALIGNED(virtual_address, PAGE_SIZE));

	unsigned short pml4_offset = (virtual_address >> 39) & 0x1FF;
	unsigned short pdpt_offset = (virtual_address >> 30) & 0x1FF;
	unsigned short pd_offset = (virtual_address >> 21) & 0x1FF;
	unsigned short pt_offset = (virtual_address >> 12) & 0x1FF;

	if (!(pml4[pml4_offset] & PAGE_PRESENT))
	{
		void *page = arch_memory_allocate_page();
		if (page == NULL)
			fatal("Unable to get page for PDPT\n");

		pml4[pml4_offset] = (pml4e)page | flags;
	}

	pdpte *pdpt = (pdpte *)virtual_address((pml4[pml4_offset] >> 12) << 12);

	if (!(pdpt[pdpt_offset] & PAGE_PRESENT))
	{
		void *page = arch_memory_allocate_page();
		if (page == NULL)
			fatal("Unable to get page for PD\n");

		pdpt[pdpt_offset] = (pdpte)page | flags;
	}

	pde *pd = (pde *)virtual_address((pdpt[pdpt_offset] >> 12) << 12);

	if (pd[pd_offset] & (PAGE_PS | PAGE_PRESENT))
	{
		pd[pd_offset] = flags | PAGE_PS;
		return pml4[pml4_offset];
	}

	if (!(pd[pd_offset] & PAGE_PRESENT))
	{
		void *page = arch_memory_allocate_page();
		if (page == NULL)
			fatal("Unable to get page for PT\n");

		pd[pd_offset] = (pde)page | flags;
	}

	pte *pt = (pte *)virtual_address((pd[pd_offset] >> 12) << 12);
	if (pt[pt_offset] & PAGE_PRESENT)
		return RESULT_ERROR;

	pt[pt_offset] = (pte)physical_address | flags;
	return RESULT_OK;
}

result_t arch_vmm_unmap_page(uint64_t virtual_address)
{
	unsigned short pml4_offset = (virtual_address >> 39) & 0x1FF;
	unsigned short pdpt_offset = (virtual_address >> 30) & 0x1FF;
	unsigned short pd_offset = (virtual_address >> 21) & 0x1FF;
	unsigned short pt_offset = (virtual_address >> 12) & 0x1FF;

	if (!(pml4[pml4_offset] & PAGE_PRESENT))
		return RESULT_ERROR;

	pdpte *pdpt = (pdpte *)virtual_address((pml4[pml4_offset] >> 12) << 12);
	if (!(pdpt[pdpt_offset] & PAGE_PRESENT))
		return RESULT_ERROR;

	if (pdpt[pdpt_offset] & PAGE_PS)
	{
		pdpt[pdpt_offset] = 0;
		return RESULT_OK;
	}

	pde *pd = (pde *)virtual_address((pdpt[pdpt_offset] >> 12) << 12);
	if (!(pd[pd_offset] & PAGE_PRESENT))
		return RESULT_ERROR;

	if (pd[pd_offset] & PAGE_PS)
	{
		pd[pd_offset] = 0;
		return RESULT_OK;
	}

	pte *pt = (pte *)virtual_address((pd[pd_offset] >> 12) << 12);
	if (!(pt[pt_offset] & PAGE_PRESENT))
		return RESULT_ERROR;

	pt[pt_offset] = 0;
	return RESULT_OK;
}

void x86_64_vmm_init(void)
{
	pml4[0] = 0;
	x86_64_memory_flush_tlb();
}
