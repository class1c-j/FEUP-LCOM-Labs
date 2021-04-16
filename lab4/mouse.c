#include "i8042.h"

static int mouse_hook_id = MOUSE_IRQ;
static uint8_t mouse_byte = 0;

int(mouse_subscribe_int)(uint16_t *bit_no) {
  *bit_no = BIT(mouse_hook_id);
  if (sys_irqsetpolicy(MOUSE_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE,
                       &mouse_hook_id) != OK) {
    fprintf(stderr, "mouse_subscribe_int: sys_irqsetpolicy: !OK\n");
    return !OK;
  }
  return OK;
}

int(mouse_unsubscribe_int)() {
  if (sys_irqrmpolicy(&mouse_hook_id) != OK) {
    fprintf(stderr, "mouse_unsubscribe_int: sys_irqrmpolicy: !OK\n");
    return !OK;
  }
  return OK;
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

void(mouse_ih)() {
  kbc_read_data(&mouse_byte);
}

uint8_t(get_mouse_byte)() {
  return mouse_byte;
}

int(mouse_issue_command)(uint8_t command) {

  for (;;) {
    uint8_t stat = 0;
    if (util_sys_inb(KBC_STATUS_REG, &stat) != OK) {
      fprintf(stderr, "mouse_issue_command: util_sys_inb: !OK\n");
      return !OK;
    }
    if ((stat & KBC_IBF) == 0) {
      if (sys_outb(KBC_IN_BUF_COMMANDS, KBC_WRITE_BYTE_TO_MOUSE) != OK) {
        fprintf(stderr, "mouse_issue_command: sys_outb: !OK\n");
        return !OK;
      }
      if (util_sys_inb(KBC_STATUS_REG, &stat) != OK) {
        fprintf(stderr, "mouse_issue_command: util_sys_inb: !OK\n");
        return !OK;
      }
      if (sys_outb(KBC_IN_BUF_ARGS, command) != OK) {
        fprintf(stderr, "mouse_issue_command: sys_outb: !OK\n");
        return !OK;
      }
      uint8_t ack_byte = 0;
      if (util_sys_inb(KBC_OUT_BUF, &ack_byte) != OK) {
        fprintf(stderr, "mouse_issue_command: util_sys_inb: !OK\n");
        return !OK;
      }
      if (ack_byte == MOUSE_ACK) {
        return OK;
      }
    }
    tickdelay(micros_to_ticks(WAIT_KBC));
  }

  return OK;
}

int(mouse_disable_data_reporting)() {
  if (mouse_issue_command(MOUSE_DISABLE_DATA_REPORTING) != OK) {
    fprintf(stderr, "mouse_disable_data_reporting: mouse_issue_command: !OK\n");
    return !OK;
  }
  return OK;
}

int(m_mouse_enable_data_reporting)() {
  if (mouse_issue_command(MOUSE_ENABLE_DATA_REPORTING) != OK) {
    fprintf(stderr, "mouse_disable_data_reporting: mouse_issue_command: !OK\n");
    return !OK;
  }
  return OK;
}

void(mouse_assemble_packet)(uint8_t *bytes, struct packet *pp) {
  memcpy(pp->bytes, bytes, 3);
  pp->delta_x = MOUSE_MSB_X_DELTA & bytes[0] ? 0xff00 | bytes[1] : bytes[1];
  pp->delta_y = MOUSE_MSB_Y_DELTA & bytes[0] ? 0xff00 | bytes[2] : bytes[2];
  pp->lb = bytes[0] & MOUSE_LEFT_BUTTON;
  pp->mb = bytes[0] & MOUSE_MIDDLE_BUTTON;
  pp->rb = bytes[0] & MOUSE_RIGHT_BUTTON;
  pp->x_ov = bytes[0] & MOUSE_X_OVERFLOW;
  pp->y_ov = bytes[0] & MOUSE_Y_OVERFLOW;
}
