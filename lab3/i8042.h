#ifndef I8042_H_
#define I8042_H_

#include <lcom/lcf.h>

/* Useful macros for the lab */

#define ESC_BREAKCODE 0x81
#define TWO_BYTES_FIRST_FLAG 0xe0
#define WAIT_KBC 2000

/* IRQ Line */

#define KBD_IRQ 1

/* Data buffers */

#define KBC_OUT_BUF 0x60
#define KBC_IN_BUF_COMMANDS 0x64
#define KBC_IN_BUF_ARGS 0x60

/* Status Register */

#define KBC_STATUS_REG 0x64

#define KBC_PARITY_ERROR BIT(7)
#define KBC_TIMEOUT_ERROR BIT(6)
#define KBC_MOUSE_DATA BIT(5)
#define KBC_KBD_NOT_INHIBITED BIT(4)
#define KBC_IBF BIT(1)
#define KBC_OBF BIT(0)

/* Keyboard Commands */

#define KBC_READ_COMMAND_BYTE 0x20
#define KBC_WRITE_COMMAND_BYTE 0x60
#define KBC_SELF_TEST 0xaa
#define KBC_SELF_TEST_OK 0x55
#define KBC_SELF_TEST_ERR 0xfc
#define KBC_CHECK_KBD_I 0xab
#define KBC_DISABLE_KBD_I 0xad
#define KBC_ENABLE_KBD_I 0xae

/* Command Byte */

#define DISABLE_MOUSE_I BIT(5)
#define DISABLE_KEYBOARD_I BIT(4)
#define ENABLE_MOUSE_INT BIT(1)
#define ENABLE_KEYBOARD_INT BIT(0)

/* Function Declarations */

int(keyboard_subscribe_int)(uint8_t *bit_no);
int(keyboard_unsubscribe_int)();
int(kbc_issue_command)(uint8_t command);
int(kbc_read_data)(uint8_t *data);
uint8_t(get_keyboard_byte)();
bool(is_breakcode)(uint8_t scancode);
bool(is_first_of_two_bytes)(uint8_t scancode);

#endif // I8042_H_
