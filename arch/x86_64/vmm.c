#include "arch/x86_64/memory.h"
#include "arch/x86_64/vmm.h"

#include "arch/memory.h"
#include "arch/x86_64/layout.h"
#include "lib/utils.h"

static pml4e *pml4 = x86_64_virtual_address(PML4_ADDRESS);

static uint64_t x86_64_page_flags(const uint32_t flags)
{
	uint64_t page_flags = 0;
	page_flags |= PAGE_PRESENT;
	if (flags & ARCH_MEMORY_MAP_WRITE) {
		page_flags |= PAGE_WRITE;
	}
	if (flags & ARCH_MEMORY_MAP_USER) {
		page_flags |= PAGE_USER;
	}
	return page_flags;
}

result_t arch_memory_map_page(uint64_t virtual_address, uint64_t physical_address, uint32_t flags)
{
	//assert(IS_ALIGNED(physical_address, PAGE_SIZE));
	//assert(IS_ALIGNED(virtual_address, PAGE_SIZE));

	unsigned short pml4_offset = (virtual_address >> 39) & 0x1FF;
	unsigned short pdpt_offset = (virtual_address >> 30) & 0x1FF;
	unsigned short pd_offset = (virtual_address >> 21) & 0x1FF;
	unsigned short pt_offset = (virtual_address >> 12) & 0x1FF;

	uint64_t page_flags = x86_64_page_flags(flags);

	if (!(pml4[pml4_offset] & PAGE_PRESENT))
	{
		void *page = arch_memory_allocate_page();
		if (page == NULL)
			fatal("Unable to get page for PDPT\n");

		pml4[pml4_offset] = (pml4e)page | PAGE_PRESENT | flags;
	}

	pdpte *pdpt = x86_64_virtual_address((pml4[pml4_offset] >> 12) << 12);

	if (!(pdpt[pdpt_offset] & PAGE_PRESENT))
	{
		void *page = arch_memory_allocate_page();
		if (page == NULL)
			fatal("Unable to get page for PD\n");

		pdpt[pdpt_offset] = (pdpte)page | PAGE_PRESENT | flags;
	}

	pde *pd = x86_64_virtual_address((pdpt[pdpt_offset] >> 12) << 12);

	if ((pd[pd_offset] & (PAGE_PS | PAGE_PRESENT)) == (PAGE_PS | PAGE_PRESENT))
	{
		return RESULT_ERROR; // do not split huge pages
	}

	if (!(pd[pd_offset] & PAGE_PRESENT))
	{
		void *page = arch_memory_allocate_page();
		if (page == NULL)
			fatal("Unable to get page for PT\n");

		pd[pd_offset] = (pde)page | PAGE_PRESENT | flags;
	}

	pte *pt = x86_64_virtual_address((pd[pd_offset] >> 12) << 12);
	if (pt[pt_offset] & PAGE_PRESENT)
		return RESULT_ERROR;

	pt[pt_offset] = physical_address | page_flags;
	return RESULT_OK;
}

result_t arch_memory_unmap_page(uint64_t virtual_address)
{
	unsigned short pml4_offset = (virtual_address >> 39) & 0x1FF;
	unsigned short pdpt_offset = (virtual_address >> 30) & 0x1FF;
	unsigned short pd_offset = (virtual_address >> 21) & 0x1FF;
	unsigned short pt_offset = (virtual_address >> 12) & 0x1FF;

	if (!(pml4[pml4_offset] & PAGE_PRESENT))
		return RESULT_ERROR;

	pdpte *pdpt = x86_64_virtual_address((pml4[pml4_offset] >> 12) << 12);
	if (!(pdpt[pdpt_offset] & PAGE_PRESENT))
		return RESULT_ERROR;

	if (pdpt[pdpt_offset] & PAGE_PS)
	{
		pdpt[pdpt_offset] = 0;
		return RESULT_OK;
	}

	pde *pd = x86_64_virtual_address((pdpt[pdpt_offset] >> 12) << 12);
	if (!(pd[pd_offset] & PAGE_PRESENT))
		return RESULT_ERROR;

	if (pd[pd_offset] & PAGE_PS)
	{
		pd[pd_offset] = 0;
		return RESULT_OK;
	}

	pte *pt = x86_64_virtual_address((pd[pd_offset] >> 12) << 12);
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
