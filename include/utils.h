#ifndef UTILS_H
#define UTILS_H

void fatal(const char *format, ...);
void examine(void* ptr, unsigned long bytes);

#define assert(e) if (!(e)) { fatal("%s:%u assertion failed\n",__FILE__, __LINE__); };


#endif
