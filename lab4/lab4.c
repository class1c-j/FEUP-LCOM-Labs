// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>

#include <stdint.h>
#include <stdio.h>

// Any header files included below this line should have been created by you
#include "i8042.h"

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need/ it]
  lcf_trace_calls("/home/lcom/labs/lab4/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab4/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int(mouse_test_packet)(uint32_t cnt) {
  int ipc_status, r;
  message msg;
  uint16_t mouse_irq_set;
  uint8_t packet[3];
  uint8_t counter = 0;
  bool is_sync = false;
  if (m_mouse_enable_data_reporting() != OK) {
    fprintf(stderr, "mouse_test_packet: mouse_enable_data_reporting: !OK\n");
    return !OK;
  }
  if (mouse_subscribe_int(&mouse_irq_set) != OK) {
    fprintf(stderr, "mouse_test_packet: mouse_subscribe_int: !OK\n");
    return !OK;
  }
  while (cnt) {
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status)) {
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:
          if (msg.m_notify.interrupts & mouse_irq_set) {
            mouse_ih();
            uint8_t mouse_byte = get_mouse_byte();

            if (counter != 0 && (mouse_byte & MOUSE_FIRST_BYTE_FLAG)) {
              is_sync = false;
            }

            if (!is_sync && (mouse_byte & MOUSE_FIRST_BYTE_FLAG)) {
              is_sync = true;
            }

            if (is_sync) {
              packet[counter++] = mouse_byte;
            } else {
              tickdelay(micros_to_ticks(WAIT_KBC));
              continue;
            }

            if (counter == 3) {
              cnt--;
              struct packet pp;
              mouse_assemble_packet(packet, &pp);
              mouse_print_packet(&pp);
              counter = 0;
            }
          }
          break;
        default:
          break; /*no other notifications expected: do nothing*/
      }
    } else { /*received a standard message, not a notification*/
             /*no standard messages expected: do nothing*/
    }
  }
  if (mouse_unsubscribe_int() != OK) {
    fprintf(stderr, "mouse_test_packet: mouse_unsubscribe_int: !OK\n");
    return !OK;
  }
  if (mouse_disable_data_reporting() != OK) {
    fprintf(stderr, "mouse_test_packet: mouse_disable_data_reporting: !OK\n");
    return !OK;
  }

  return OK;
}

int(mouse_test_async)(uint8_t idle_time) {
  /* To be completed */
  printf("%s(%u): under construction\n", __func__, idle_time);
  return 1;
}

int(mouse_test_gesture)(uint8_t x_len, uint8_t tolerance) {
  /* To be completed */
  printf("%s: under construction\n", __func__);
  return 1;
}

int(mouse_test_remote)(uint16_t period, uint8_t cnt) {
  /* To be completed */
  printf("%s(%u, %u): under construction\n", __func__, period, cnt);
  return 1;
}
