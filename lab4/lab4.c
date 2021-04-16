// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>

#include <stdint.h>
#include <stdio.h>

// Any header files included below this line should have been created by you
#include "i8042.h"
#include "i8254.h"

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

typedef enum { INIT_D, GOING_UP, TOP, GOING_DOWN, DONE } drawing_state_t;
typedef enum { INIT_M, RB_PRESS, LB_PRESS, INVALID } mouse_state_t;

void(update_mouse_state)(struct packet *pp, mouse_state_t *state) {
  if (pp->rb && pp->lb) {
    *state = INVALID;
    return;
  }
  switch (*state) {
    case INIT_M:
      if (pp->lb) {
        *state = LB_PRESS;
      } else if (pp->rb) {
        *state = RB_PRESS;
      }
      break;
    case RB_PRESS:
      if (pp->lb) {
        *state = INVALID;
      } else if (!pp->rb) {
        *state = INIT_M;
      }
      break;
    case LB_PRESS:
      if (pp->rb) {
        *state = INVALID;
      } else if (!pp->lb) {
        *state = INIT_M;
      }
      break;
    case INVALID:
      if (!pp->rb && !pp->lb) {
        *state = INIT_M;
      }
      break;
    default:
      break;
  }
}

void(update_drawing_state)(uint8_t x_len, uint16_t *x_delta, uint16_t tolerance,
                           drawing_state_t *draw_state,
                           mouse_state_t *mouse_state, struct packet *pp) {
  switch (*draw_state) {
    case INIT_D:
      if (*mouse_state == LB_PRESS) {
        *draw_state = GOING_UP;
        *x_delta = 0;
      }
      break;
    case GOING_UP:
      if (*mouse_state == RB_PRESS || pp->delta_x < -tolerance ||
          pp->delta_y < -tolerance) {
        *mouse_state = INVALID;
        *draw_state = INIT_D;
        *x_delta = 0;
      } else {
        *x_delta += pp->delta_x;
        if (*mouse_state == INIT_M) {
          if (*x_delta >= x_len) {
            *draw_state = TOP;
            *x_delta = 0;
          } else {
            *draw_state = INIT_D;
          }
        }
      }
      break;
    case TOP:
      if (abs(pp->delta_x) > tolerance || abs(pp->delta_y) > tolerance) {
        *mouse_state = INVALID;
        *draw_state = INIT_D;
      } else if (*mouse_state == LB_PRESS) {
        *draw_state = GOING_UP;
      } else if (*mouse_state == RB_PRESS) {
        *draw_state = GOING_DOWN;
      }
      break;
    case GOING_DOWN:
      if (*mouse_state == LB_PRESS || pp->delta_x < -tolerance ||
          pp->delta_y > tolerance) {
        *mouse_state = INVALID;
        *draw_state = INIT_D;
        *x_delta = 0;
      } else {
        *x_delta += pp->delta_x;
        if (*mouse_state == INIT_M) {
          if (*x_delta >= x_len) {
            *draw_state = DONE;
            *x_delta = 0;
          } else {
            *draw_state = INIT_D;
          }
        }
      }
      break;
    case DONE:
      break;
    default:
      break;
  }
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
  int ipc_status, r;
  message msg;
  uint16_t mouse_irq_set;
  uint8_t timer_irq_set;
  uint8_t packet[3];
  uint8_t counter = 0;
  bool is_sync = false;
  uint32_t timer_counter = 0;
  if (m_mouse_enable_data_reporting() != OK) {
    fprintf(stderr, "mouse_test_async: mouse_enable_data_reporting: !OK\n");
    return !OK;
  }
  if (mouse_subscribe_int(&mouse_irq_set) != OK) {
    fprintf(stderr, "mouse_test_async: mouse_subscribe_int: !OK\n");
    return !OK;
  }
  if (timer_subscribe_int(&timer_irq_set) != OK) {
    fprintf(stderr, "mouse_test_async: timer_subscribe_int: !OK\n");
    return !OK;
  }
  while (timer_counter / sys_hz() < idle_time) {
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status)) {
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:
          if (msg.m_notify.interrupts & timer_irq_set) {
            timer_int_handler();
            timer_counter = get_timer_counter();
          }
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
              struct packet pp;
              mouse_assemble_packet(packet, &pp);
              mouse_print_packet(&pp);
              counter = 0;
            }
            reset_timer_counter();
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
    fprintf(stderr, "mouse_test_async: mouse_unsubscribe_int: !OK\n");
    return !OK;
  }
  if (mouse_disable_data_reporting() != OK) {
    fprintf(stderr, "mouse_test_async: mouse_disable_data_reporting: !OK\n");
    return !OK;
  }
  if (timer_unsubscribe_int() != OK) {
    fprintf(stderr, "mouse_test_async: timer_unsubscribe_int: !OK\n");
    return !OK;
  }

  return OK;
}

int(mouse_test_gesture)(uint8_t x_len, uint8_t tolerance) {
  int ipc_status, r;
  message msg;
  uint16_t mouse_irq_set;
  uint8_t packet[3];
  uint8_t counter = 0;
  bool is_sync = false;
  mouse_state_t mouse_state = INIT_M;
  drawing_state_t drawing_state = INIT_D;
  uint16_t x_delta = 0;
  if (m_mouse_enable_data_reporting() != OK) {
    fprintf(stderr, "mouse_test_packet: mouse_enable_data_reporting: !OK\n");
    return !OK;
  }
  if (mouse_subscribe_int(&mouse_irq_set) != OK) {
    fprintf(stderr, "mouse_test_packet: mouse_subscribe_int: !OK\n");
    return !OK;
  }
  while (drawing_state != DONE) {
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
              struct packet pp;
              mouse_assemble_packet(packet, &pp);
              mouse_print_packet(&pp);
              update_mouse_state(&pp, &mouse_state);
              update_drawing_state(x_len, &x_delta, tolerance, &drawing_state,
                                   &mouse_state, &pp);
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

int(mouse_test_remote)(uint16_t period, uint8_t cnt) {

  uint8_t counter = 0;
  uint8_t packet[3];
  bool is_sync = false;
  uint8_t mouse_byte = 0;

  while (cnt) {

    if (counter == 0 && mouse_issue_command(MOUSE_READ_DATA) != OK) {
      printf("aaaaa\n");
    }

    // ph
    if (mouse_ph() != OK) {
      printf("aaaa\n");
      tickdelay(micros_to_ticks(period * 1000));
      continue;
    }

    printf("got here\n");

    mouse_byte = get_mouse_byte();

    if (counter != 0 && (mouse_byte & MOUSE_FIRST_BYTE_FLAG)) {
      is_sync = false;
    }

    if (!is_sync && (mouse_byte & MOUSE_FIRST_BYTE_FLAG)) {
      is_sync = true;
    }

    if (is_sync) {
      packet[counter++] = mouse_byte;
    } else {
      continue;
    }
    if (counter == 3) {
      cnt--;
      struct packet pp;
      mouse_assemble_packet(packet, &pp);
      mouse_print_packet(&pp);
      counter = 0;
      tickdelay(micros_to_ticks(period * 1000));
    }

    // set stream mode
    if (mouse_reset_stream_mode() != OK) {
      fprintf(stderr, "mouse_test_remote: mouse_reset_stream_mode: !OK\n");
      return !OK;
    }
    // disable data reporting
    if (mouse_disable_data_reporting() != OK) {
      fprintf(stderr, "mouse_test_remote: mouse_disable_data_reporting: !OK\n");
      return !OK;
    }
    // write default kbc command byte
    uint8_t default_cmd_byte = minix_get_dflt_kbc_cmd_byte();
    kbc_write_command_byte(default_cmd_byte);
  }

  return OK;
}
