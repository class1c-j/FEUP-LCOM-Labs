
#include "vbe.h"

static char *video_mem; /* Process (virtual) address to which VRAM is mapped */

static unsigned h_res;          /* Horizontal resolution in pixels */
static unsigned v_res;          /* Vertical resolution in pixels */
static unsigned bits_per_pixel; /* Number of VRAM bits per pixel */

int(vbe_set_mode)(uint16_t mode) {

  reg86_t reg86;
  memset(&reg86, 0, sizeof(reg86_t));

  reg86.ah = BIOS_VBE_IF_FUNCTION;
  reg86.al = BIOS_VBE_SET_MODE;
  reg86.bx = mode | BIOS_VBE_IF_LINEAR;
  reg86.intno = BIOS_VIDEO_CARD_SERVICE;

  if (sys_int86(&reg86) != OK) {
    printf("set_vbe_mode: sys_int86() failed\n");
    return !OK;
  }

  return OK;
}

int(vbe_map_vram)(uint16_t mode) {
  struct minix_mem_range mr;
  unsigned int vram_base; /* VRAM's physical addresss */
  unsigned int vram_size; /* VRAM's size, but you can use the frame-buffer size, instead */
  int r;

  vbe_mode_info_t vbe_mode_info;

  /* Use VBE function 0x01 to initialize vram_base and vram_size */
  memset(&vbe_mode_info, 0, sizeof(vbe_mode_info_t));
  if (vbe_get_mode_info(mode, &vbe_mode_info) != OK) {
    fprintf(stderr, "vbe_map_vram: vbe_get_mode_info: !OK\n");
  }

  h_res = vbe_mode_info.XResolution;
  v_res = vbe_mode_info.YResolution;
  bits_per_pixel = vbe_mode_info.BitsPerPixel;

  printf("read mode %x\n", mode);

  vram_base = vbe_mode_info.PhysBasePtr;
  vram_size = vbe_mode_info.XResolution * vbe_mode_info.YResolution * (vbe_mode_info.BitsPerPixel + 7) / 8;

  /* Allow memory mapping */

  mr.mr_base = (phys_bytes) vram_base;
  mr.mr_limit = mr.mr_base + vram_size;

  if (OK != (r = sys_privctl(SELF, SYS_PRIV_ADD_MEM, &mr)))
    panic("sys_privctl (ADD_MEM) failed: %d\n", r);

  printf("memory mapping : base %x ; size : %d\n", vram_base, vram_size);
  /* Map memory */

  video_mem = vm_map_phys(SELF, (void *) mr.mr_base, vram_size);

  if (video_mem == MAP_FAILED)
    panic("couldn't map video memory");

  return OK;
}

int(vg_draw_pixel)(uint16_t x, uint16_t y, uint32_t color) {
  if (x > h_res || y > v_res) {
    printf("out of bounds\n");
    return !OK;
  }
  uint8_t bytes = (bits_per_pixel + 7) / 8;
  uint64_t pos = y * h_res * bytes + x * bytes;
  memcpy((char *) video_mem + pos, &color, bytes);

  return OK;
}

int(vg_draw_hline)(uint16_t x, uint16_t y, uint16_t len, uint32_t color) {
  for (int i = x; i < x + len; ++i) {
    vg_draw_pixel(i, y, color);
  }
  return OK;
}

int(vg_draw_rectangle)(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color) {
  for (int i = y; i < y + height; ++i) {
    vg_draw_hline(x, i, width, color);
  }
  return OK;
}
