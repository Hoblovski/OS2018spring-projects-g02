#include "loader.h"
#include <string.h>

static uint8_t img[MAX_IMG_SZ];

unsigned img_sz = 0;

void load_elf(const char* filename, machine_t* rv)
{
  assert(0 && "for raw machines, load elf is disabled");
}


void load_imgz(const char* filename, machine_t* rv) {
  // load object into memory
  FILE* fin = fopen(filename, "r");
  assert(fin);
  int n_read = fread(img, 1, MAX_IMG_SZ, fin);
  assert(n_read < MAX_IMG_SZ && "object too big!");
  assert(n_read < MEMSZ_BYTES && "image too big for memory");
  printf("loaded raw memory image (%d bytes)\n", n_read);
  memcpy(rv->mem, img, n_read);
  img_sz = n_read;
}


void load_auto(unsigned load_type, const char* filename, machine_t* rv)
{
  switch (load_type) {
    case FILETYPE_ELF:
      load_elf(filename, rv);
      break;
    case FILETYPE_IMGZ:
      load_imgz(filename, rv);
      break;
    default:
      assert(0 && "bad load type");
  }
}

