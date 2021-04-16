
#include "vbe.h"


int vbe_set_mode(uint16_t mode) {

  reg86_t reg86;
  memset(&reg86, 0, sizeof(reg86_t));

  reg86.ah = BIOS_VBE_IF_FUNCTION;
  reg86.al = BIOS_VBE_SET_MODE;
  reg86.bx = mode | BIOS_VBE_IF_LINEAR;
  reg86.intno = BIOS_VIDEO_CARD_SERVICE;

  if(sys_int86(&reg86) != OK){
    printf("set_vbe_mode: sys_int86() failed\n");
    return !OK;
  }

  return OK;
}
