
#include "i8042.h"
#include <lcom/lcf.h>

static int kbd_hook_id = KEYBOARD_IRQ;
static uint8_t keyboard_byte = 0;

int(keyboard_subscribe_int)(uint8_t *bit_no) {
  *bit_no = BIT(kbd_hook_id);
  if (sys_irqsetpolicy(KEYBOARD_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &kbd_hook_id) != OK) {
    fprintf(stderr, "keyboard_subscribe_int: sys_irqsetpolicy: could not subscribe interrupt\n");
    return !OK;
  }
  return OK;
}

int(keyboard_unsubscribe_int)() {
  if (sys_irqrmpolicy(&kbd_hook_id) != OK) {
    fprintf(stderr, "keyboard_unsubscribe_int: sys_irqrmpolicy: could not unsubscribe from interrupt\n");
    return !OK;
  }
  return OK;
}

void(kbc_ih)() {
  kbc_read_data(&keyboard_byte);
}

int(kbc_issue_command)(uint8_t command) {
  uint8_t stat;
  uint8_t tries = 3;
  while (tries--) {
    util_sys_inb(KBC_STATUS_REG, &stat); /*assuming it returns OK*/
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
    util_sys_inb(KBC_STATUS_REG, &stat); /*assuming it returns OK*/
    /*loop while 8042 output buffer is empty*/
    if (stat & KBC_OBF) {
      util_sys_inb(KBC_OUT_BUF, data); /*ass. it returns OK*/
      if ((stat & (KBC_PARITY_ERROR | KBC_TIMEOUT_ERROR)) == 0)
        return OK;
      else
        return !OK;
    }
    tickdelay(micros_to_ticks(WAIT_KBC));
  }
  return !OK;
}

int(kbc_enable_interrupts)() {
  if (kbc_issue_command(KBC_READ_COMMAND_BYTE) != OK) {
    fprintf(stderr, "kbc_enable_interrupts: kbc_issue_command: !OK\n");
    return !OK;
  }
  uint8_t command_byte;
  if (util_sys_inb(KBC_OUT_BUF, &command_byte) != OK) {
    fprintf(stderr, "kbc_enable_interrupts: util_sys_inb: !OK\n");
    return !OK;
  }
  command_byte |= ENABLE_KEYBOARD_INT;
  if (kbc_issue_command(KBC_WRITE_COMMAND_BYTE) != OK) {
    fprintf(stderr, "kbc_enable_interrupts: kbc_issue_command: !OK\n");
    return !OK;
  }
  if (sys_outb(KBC_IN_BUF_ARGS, command_byte) != OK) {
    fprintf(stderr, "kbc_enable_interrupts: sys_outb: !OK\n");
    return !OK;
  }
  return OK;
}

uint8_t(get_keyboard_byte()) {
  return keyboard_byte;
}

inline bool(is_breakcode)(uint8_t scancode) {
  return scancode & BIT(7);
}

inline bool(is_first_of_two_bytes)(uint8_t scancode) {
  return scancode == TWO_BYTES_FIRST_FLAG;
}
