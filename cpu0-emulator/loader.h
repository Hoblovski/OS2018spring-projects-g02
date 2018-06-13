#ifndef LOADER_H
#define LOADER_H

#include "machine.h"

// object image cannot be greater than 8MB
#define MAX_IMG_SZ 8*1024*1024 

#define FILETYPE_ELF 1
#define FILETYPE_IMGZ 2

void load_auto(unsigned load_type, const char* filename, machine_t* rv);

extern unsigned img_sz;

#endif // LOADER_H
