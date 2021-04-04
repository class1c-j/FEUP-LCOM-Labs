#include <lcom/lcf.h>
#include "utils.h"

static uint32_t sys_inb_counter = 0;

int(util_sys_inb)(int port, uint8_t *value) {
  uint32_t extended = 0;
  int success = sys_inb(port, &extended);
#ifdef LAB3
  ++sys_inb_counter;
#endif
  *value = extended & 0xFF;
  return success;
}

uint32_t(get_sys_inb_count)(){
  return sys_inb_counter;
}
