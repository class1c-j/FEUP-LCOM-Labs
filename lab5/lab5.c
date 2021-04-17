// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>

#include <lcom/lab5.h>

#include <stdint.h>
#include <stdio.h>

// Any header files included below this line should have been created by you
#include "i8042.h"
#include "vbe.h"

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/lab5/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab5/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int(video_test_init)(uint16_t mode, uint8_t delay) {

  if (vbe_set_mode(mode) != OK) {
    fprintf(stderr, "video_test_init: vbe_video_init: !OK\n");
    return OK;
  }
  sleep(delay);
  if (vg_exit() != OK) {
    fprintf(stderr, "video_test_init: vg_exit: !OK\n");
    return !OK;
  }
  return OK;
}

int(video_test_rectangle)(uint16_t mode, uint16_t x, uint16_t y,
                          uint16_t width, uint16_t height, uint32_t color) {

  printf("MOde passed %x\n", mode);

  // map the vram
  if (vbe_map_vram(mode) != OK) {
    return !OK;
  }

  printf("mapped vram\n");

    // change the video mode
  if (vbe_set_mode(mode) != OK) {
    fprintf(stderr, "video_test_rectangle: vbe_set_mode: !OK\n");
    return !OK;
  }



  printf("changed mode\n");

  // draw a rectangle
  vg_draw_rectangle(x, y, width, height, color);

  printf("drawn pixel\n");

  // recieve esc breakcode
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
            } else {
              index = 0;
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
  if (keyboard_unsubscribe_int() != OK) {
    fprintf(stderr, "kbd_test_scan: keyboard_unsubscribe_int: !OK\n");
    return !OK;
  }

  printf("leaving...\n");

  vg_exit();

  return OK;
}

int(video_test_pattern)(uint16_t mode, uint8_t no_rectangles, uint32_t first, uint8_t step) {
  /* To be completed */
  printf("%s(0x%03x, %u, 0x%08x, %d): under construction\n", __func__,
         mode, no_rectangles, first, step);

  return 1;
}

int(video_test_xpm)(xpm_map_t xpm, uint16_t x, uint16_t y) {
  /* To be completed */
  printf("%s(%8p, %u, %u): under construction\n", __func__, xpm, x, y);

  return 1;
}

int(video_test_move)(xpm_map_t xpm, uint16_t xi, uint16_t yi, uint16_t xf, uint16_t yf,
                     int16_t speed, uint8_t fr_rate) {
  /* To be completed */
  printf("%s(%8p, %u, %u, %u, %u, %d, %u): under construction\n",
         __func__, xpm, xi, yi, xf, yf, speed, fr_rate);

  return 1;
}

int(video_test_controller)() {
  /* To be completed */
  printf("%s(): under construction\n", __func__);

  return 1;
}
