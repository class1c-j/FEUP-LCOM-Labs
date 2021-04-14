#include "i8042.h"

int(mouse_subscribe_int)(uint8_t *bit_no) {
  return OK;
}

int(mouse_unsubcribe_int)() {
  return OK;
}

void(mouse_ih)() {

}
