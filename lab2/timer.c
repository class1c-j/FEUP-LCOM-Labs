#include <lcom/lcf.h>
#include <lcom/timer.h>

#include <stdint.h>

#include "i8254.h"

int(timer_set_frequency)(uint8_t timer, uint32_t freq) {
  /* To be implemented by the students */
  printf("%s is not yet implemented!\n", __func__);

  return 1;
}

int(timer_subscribe_int)(uint8_t *bit_no) {
  /* To be implemented by the students */
  printf("%s is not yet implemented!\n", __func__);

  return 1;
}

int(timer_unsubscribe_int)() {
  /* To be implemented by the students */
  printf("%s is not yet implemented!\n", __func__);

  return 1;
}

void(timer_int_handler)() {
  /* To be implemented by the students */
  printf("%s is not yet implemented!\n", __func__);
}

int(timer_get_conf)(uint8_t timer, uint8_t *st) {
  uint8_t read_back_cmd = TIMER_RB_CMD | TIMER_RB_COUNT_ | TIMER_RB_SEL(timer);
  if (sys_outb(TIMER_CTRL, read_back_cmd) != OK) {
    fprintf(stderr, "timer_get_conf: sys_outb: could not write to TIMER_CTRL\n");
    return !OK;
  }
  if (util_sys_inb(TIMER_0 + timer, st) != OK) {
    fprintf(stderr, "timer_get_conf: util_sys_inb: could not read from timer\n");
    return !OK;
  }
  return OK;
}

int(timer_display_conf)(uint8_t timer, uint8_t st,
                        enum timer_status_field field) {
  union timer_status_field_val val;
  switch (field) {
    case tsf_all:
      val.byte = st;
      break;
    case tsf_initial:
      val.in_mode = (st & TIMER_LSB_MSB) >> 4;
      break;
    case tsf_mode:
      val.count_mode = (st & TIMER_ST_OPERATING_MODE) >> 1;
      if (val.count_mode > 5) {
        val.count_mode &= (BIT(1) | BIT(0));
      }
      break;
    case tsf_base:
      val.bcd = st & TIMER_BCD;
      break;
    default:
      break;
  }
  if (timer_print_config(timer, field, val) != OK) {
    fprintf(stderr, "timer_display_conf: timer_print_conf: !OK");
    return !OK;
  }
  return OK;
}
