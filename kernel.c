#include "console.h"
#include "memory.h"

void kernel()
{
	console_init();
	memory_init();

	printf("Free regions:\n");
	print_regions();
	page p1 = alloc();
	page p2 = alloc();
	page p3 = alloc();
	printf("Free regions after page allocation:\n");
	print_regions();
	free(p2);
	printf("Free regions after page free:\n");
	print_regions();
	free(p1);
	printf("Free regions after page free:\n");
	print_regions();
	free(p3);
	printf("Free regions after page free:\n");
	print_regions();

	while (1) ;
}
