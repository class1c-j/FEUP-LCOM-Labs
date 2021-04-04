#include <lcom/lcf.h>

#include <lcom/lab3.h>

#include <stdbool.h>
#include <stdint.h>

#include "i8042.h"

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/lab3/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab3/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int(kbd_test_scan)() {
  int ipc_status, r;
  message msg;
  uint8_t kbd_hook_id, kbd_byte = 0;
  uint8_t index = 0;
  uint8_t scancode[2];
  if (keyboard_subscribe_int(&kbd_hook_id) != OK) {
    fprintf(stderr, "kbd_test_scan: keyboard_subscribe_int: !OK\n");
    return !OK;
  }
  while (kbd_byte != ESC_BREAKCODE) {
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status)) {
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:
          if (msg.m_notify.interrupts & kbd_hook_id) {
            kbc_ih();
            kbd_byte = get_keyboard_byte();
            scancode[index] = kbd_byte;
            if (is_first_of_two_bytes(kbd_byte)) {
              ++index;
            }
            else {
              kbd_print_scancode(!is_breakcode(kbd_byte), index + 1, scancode);
              index = 0;
            }
          }
          break;
        default:
          break; /*no other notifications expected: do nothing*/
      }
    }
    else { /*received a standard message, not a notification*/
           /*no standard messages expected: do nothing*/
    }
  }
  if (keyboard_unsubscribe_int() != OK) {
    fprintf(stderr, "kbd_test_scan: keyboard_unsubscribe_int: !OK\n");
    return !OK;
  }
  return OK;
}

int(kbd_test_poll)() {
  uint8_t kbd_byte = 0, index = 0;
  uint8_t scancode[2];
  while (kbd_byte != ESC_BREAKCODE) {
    if (kbc_read_data(&kbd_byte) != OK) {
      tickdelay(micros_to_ticks(WAIT_KBC));
    }
    scancode[index] = kbd_byte;
    if (is_first_of_two_bytes(kbd_byte)) {
      ++index;
    }
    else {
      kbd_print_scancode(!is_breakcode(kbd_byte), index + 1, scancode);
      index = 0;
    }
  }
  if (kbc_enable_interrupts() != OK) {
    fprintf(stderr, "kbd_test_poll: kbc_enable_interrupts: !OK\n");
    return !OK;
  }
  return OK;
}

int(kbd_test_timed_scan)(uint8_t n) {
  /* To be completed by the students */
  printf("%s is not yet implemented!\n", __func__);

  return 1;
}
