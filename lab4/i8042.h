#ifndef I8042_H_
#define I8042_H_

#include <lcom/lcf.h>

/* Useful macros for lab3 */

#define ESC_BREAKCODE 0x81
#define TWO_BYTES_FIRST_FLAG 0xe0
#define WAIT_KBC 2000

/* IRQ Lines */

#define KBD_IRQ 1
#define MOUSE_IRQ 12

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

/* KBC Keyboard Commands */

#define KBC_READ_COMMAND_BYTE 0x20
#define KBC_WRITE_COMMAND_BYTE 0x60
#define KBC_SELF_TEST 0xaa
#define KBC_SELF_TEST_OK 0x55
#define KBC_SELF_TEST_ERR 0xfc
#define KBC_CHECK_KBD_I 0xab
#define KBC_DISABLE_KBD_I 0xad
#define KBC_ENABLE_KBD_I 0xae

/* KBC Mouse Commands */
#define KBC_DISABLE_MOUSE 0xa7
#define KBC_ENABLE_MOSUE 0xa8
#define KBC_CHECK_MOUSE_I 0xa9
#define KBC_WRITE_BYTE_TO_MOUSE 0xd4

/* Command Byte */

#define DISABLE_MOUSE_I BIT(5)
#define DISABLE_KEYBOARD_I BIT(4)
#define ENABLE_MOUSE_INT BIT(1)
#define ENABLE_KEYBOARD_INT BIT(0)

/* Mouse Commands */
#define MOUSE_RESET 0xff
#define MOUSE_RESEND 0xfe
#define MOUSE_SET_DEFAULTS 0xf6
#define MOUSE_DISABLE_DATA_REPORTING 0xf5
#define MOUSE_ENABLE_DATA_REPORTING 0xf4
#define MOUSE_SET_SAMPLE_RATE 0xf3
#define MOUSE_SET_REMOTE_MODE 0xf0
#define MOUSE_READ_DATA 0xeb
#define MOUSE_SET_STREAM_MODE 0xea
#define MOUSE_STATUS_REQUEST 0xe9
#define MOUSE_SET_RESOLUTION 0xe8
#define MOUSE_ACCELERATION_MODE 0xe7
#define MOUSE_LINEAR_MODE 0xe6

/* Mouse Acknowledgement */
#define MOUSE_ACK 0xfa
#define MOUSE_NACK 0xfe
#define MOUSE_ERROR 0xfc

/* Mouse Bytes */

/* First Byte */
#define MOUSE_LEFT_BUTTON BIT(0)
#define MOUSE_RIGHT_BUTTON BIT(1)
#define MOUSE_MIDDLE_BUTTON BIT(2)
#define MOUSE_FIRST_BYTE_FLAG BIT(3)
#define MOUSE_MSB_X_DELTA BIT(4)
#define MOUSE_MSB_Y_DELTA BIT(5)
#define MOUSE_X_OVERFLOW BIT(6)
#define MOUSE_Y_OVERFLOW BIT(7)

/* Second Byte */
#define MOUSE_X_DELTA_IDX 1

/* Third Byte */
#define MOUSE_Y_DELTA_IDX 2

/* Function Declarations */

int(keyboard_subscribe_int)(uint8_t *bit_no);
int(keyboard_unsubscribe_int)();
int(kbc_issue_command)(uint8_t command);
int(kbc_read_data)(uint8_t *data);
uint8_t(get_keyboard_byte)();
bool(is_breakcode)(uint8_t scancode);
bool(is_first_of_two_bytes)(uint8_t scancode);
int(kbc_enable_interrupts)();
int(mouse_subscribe_int)(uint16_t *bit_no);
int(mouse_unsubscribe_int)();
void(mouse_ih)();
int(mouse_issue_command)(uint8_t command);
uint8_t(get_mouse_byte)();
int(mouse_disable_data_reporting)();
void(mouse_assemble_packet)(uint8_t *bytes, struct packet *pp);

#endif // I8042_H_
