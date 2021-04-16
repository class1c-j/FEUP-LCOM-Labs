#include <lcom/lcf.h>
#include <lcom/timer.h>

#include <stdint.h>

#include "i8254.h"

static uint32_t timer_counter = 0;
static int timer_hook_id = TIMER0_IRQ;

int(timer_set_frequency)(uint8_t timer, uint32_t freq) {
  uint8_t st;
  if (timer_get_conf(timer, &st) != OK) {
    fprintf(stderr, "timer_set_frequency: timer_get_conf: !OK\n");
    return !OK;
  }
  uint8_t timer_mask = timer == 0 ? TIMER_SEL0 : timer == 1 ? TIMER_SEL1
                                                            : TIMER_SEL2;
  uint8_t control_word = timer_mask | TIMER_LSB_MSB | (st & (TIMER_ST_OPERATING_MODE | TIMER_BCD));
  if (sys_outb(TIMER_CTRL, control_word) != OK) {
    fprintf(stderr, "timer_set_frequency: sys_outb: could not write to TIMER_CTRL\n");
    return !OK;
  }
  uint16_t counter = TIMER_FREQ / freq;
  uint8_t counter_LSB;
  util_get_LSB(counter, &counter_LSB);
  uint8_t counter_MSB;
  util_get_MSB(counter, &counter_MSB);
  if (sys_outb(TIMER_0 + timer, counter_LSB) != OK) {
    fprintf(stderr, "timer_set_frequency: sys_outb: could not write LSB to timer counter\n");
    return !OK;
  }
  if (sys_outb(TIMER_0 + timer, counter_MSB) != OK) {
    fprintf(stderr, "timer_set_frequency: sys_outb: could not write MSB to timer counter\n");
    return !OK;
  }
  return OK;
}

int(timer_subscribe_int)(uint8_t *bit_no) {
  *bit_no = BIT(timer_hook_id);
  if (sys_irqsetpolicy(TIMER0_IRQ, IRQ_REENABLE, &timer_hook_id) != OK) {
    fprintf(stderr, "timer_subscribe_int: sys_irqsetpolicy: could not subscribe interrupt");
    return !OK;
  }
  return OK;
}

int(timer_unsubscribe_int)() {
  if (sys_irqrmpolicy(&timer_hook_id) != OK) {
    fprintf(stderr, "timer_unsubscribe_int: sys_irqrmpolicy: could not unsubscribe interrupt\n");
    return !OK;
  }
  return OK;
}

void(timer_int_handler)() {
  ++timer_counter;
}

uint32_t(get_timer_counter)() {
  return timer_counter;
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
      if (val.count_mode > 5) { // don't care bits must be set to 0 (see handout table)
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

void(reset_timer_counter)() {
  timer_counter = 0;
}
