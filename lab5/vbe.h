#ifndef VBE_H_
#define VBE_H_

#include <lcom/lcf.h>

/* Standard BIOS services */
#define BIOS_VIDEO_CARD_SERVICE 0x10
#define BIOS_PC_CONFIGURATION_SERVICE 0x11
#define BIOS_MEMORY_CONFIGURATION_SERVICE 0x12
#define BIOS_KEYBOARD_SERVICE 0x16

/* INT 0x10 Interface */
#define BIOS_VBE_IF_FUNCTION 0x4f
#define BIOS_VBE_IF_LINEAR BIT(14)
#define BIOS_VBE_SET_MODE 0x02

/* Function Declarations */
int (vbe_set_mode)(uint16_t mode);
int(vbe_map_vram)(uint16_t mode);


#endif // VBE_H_
