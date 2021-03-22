#include <lcom/lcf.h>
#include "i8042.h"


static int kbd_hook_id = KEYBOARD_IRQ;
static uint8_t keyboard_byte = 0;

int(keyboard_subscribe_int)(uint8_t *bit_no) {
  *bit_no = BIT(kbd_hook_id);
  if (sys_irqsetpolicy(KEYBOARD_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, kbd_hook_id) != OK) {
    fprintf(stderr, "keyboard_subscribe_int: sys_irqsetpolicy: could not subscribe interrupt\n");
    return !OK;
  }
  return OK;
}

int(keyboard_unsubscribe_int)(int hook_id) {
  if (sys_irqrmpolicy(hook_id) != OK) {
    fprintf(stderr, "keyboard_unsubscribe_int: sys_irqrmpolicy: could not unsubscribe from interrupt\n");
    return !OK;
  }
  return OK;
}

void(kbc_ih)() {
  // read the status
  uint8_t stat;

  // check for errors

  // read the scancode from out_buf
}

int(kbc_issue_command)(uint8_t command) {
  uint8_t stat;
  uint8_t tries;
  while (tries--) {
    sys_inb(KBC_STATUS_REG, &stat); /*assuming it returns OK*/
    /*loop while 8042 input buffer is not empty*/
    if ((stat & KBC_IBF) == 0) {
      sys_outb(KBC_IN_BUF_COMMANDS, command); /*no args command*/
      return OK;
    }
    tickdelay(micros_to_ticks(WAIT_KBC));
  }
  return !OK;
}

int(kbc_read_data)(uint8_t *data) {
  uint8_t stat;
  uint8_t tries = 3;
  while (tries--) {
    sys_inb(KBC_STATUS_REG, &stat); /*assuming it returns OK*/
    /*loop while 8042 output buffer is empty*/
    if (stat & KBC_OBF) {
      sys_inb(KBC_OUT_BUF, &data); /*ass. it returns OK*/
      if ((stat & (KBC_PARITY_ERROR | KBC_TIMEOUT_ERROR)) == 0)
        return OK;
      else
        return !OK;
    }
    tickdelay(micros_to_ticks(WAIT_KBC));
  }
  return !OK;
}

uint8_t get_keyboard_byte() {
  return keyboard_byte;
}
