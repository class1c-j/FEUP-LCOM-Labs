#include <lcom/lcf.h>

int(util_sys_inb)(int port, uint8_t *value) {
  uint32_t extended = 0;
  int success = sys_inb(port, &extended);
  *value = extended & 0xFF;
  return success;
}
