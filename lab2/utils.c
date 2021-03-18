#include <lcom/lcf.h>

#include <stdint.h>

int(util_get_LSB)(uint16_t val, uint8_t *lsb) {
  *lsb = val & 0x00FF;
  return OK;
}

int(util_get_MSB)(uint16_t val, uint8_t *msb) {
  *msb = val >> 8;
  return OK;
}

int (util_sys_inb)(int port, uint8_t *value) {
  uint32_t extended;
  int success = sys_inb(port, &extended);
  *value = (uint8_t) extended;
  return success;
}
