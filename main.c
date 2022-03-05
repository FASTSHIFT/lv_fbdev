
/**
 * @file main
 *
 */

/*********************
 *      INCLUDES
 *********************/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include "lvgl/lvgl.h"
#include "lvgl/demos/lv_demos.h"
#include "lv_drivers/display/fbdev.h"
#include "lv_drivers/indev/evdev.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void lv_port_disp_init(void);
static void lv_port_indev_init(const char * path, bool show_cursor);
static void cursor_set_hidden(bool en);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *      VARIABLES
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

int main(int argc, char **argv)
{
  if(argc != 2) {
    printf("input evdev device path\n");
    return 0;
  }

  /*Initialize LVGL*/
  lv_init();

  /*Initialize the HAL (display, input devices, tick) for LVGL*/
  lv_port_disp_init();
  lv_port_indev_init(argv[1], true);

  cursor_set_hidden(true);

  lv_demo_benchmark();

  while(1) {
      /* Periodically call the lv_task handler.
       * It could be done in a timer interrupt or an OS task too.*/
      lv_timer_handler();
      usleep(1 * 1000);
  }

  return 0;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

uint32_t custom_tick_get(void)
{
  struct timespec ts;

  clock_gettime(CLOCK_MONOTONIC, &ts);

  uint32_t tick = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;

  return tick;
}

static void lv_port_disp_init(void)
{
  /*Linux frame buffer device init*/
  fbdev_init();

  uint32_t width, height;
  fbdev_get_sizes(&width, &height);

  printf("fbdev: %" PRIu32 " x %" PRIu32 "\n", width, height);

  /*A small buffer for LittlevGL to draw the screen's content*/
  uint32_t buf_size = width * height;
  lv_color_t * buf = malloc(buf_size * sizeof(lv_color_t));
  LV_ASSERT_MALLOC(buf);

  /*Initialize a descriptor for the buffer*/
  static lv_disp_draw_buf_t disp_buf;
  lv_disp_draw_buf_init(&disp_buf, buf, NULL, buf_size);

  /*Initialize and register a display driver*/
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.draw_buf   = &disp_buf;
  disp_drv.flush_cb   = fbdev_flush;
  disp_drv.hor_res    = width;
  disp_drv.ver_res    = height;
  lv_disp_drv_register(&disp_drv);
}

static void lv_port_indev_init(const char * path, bool show_cursor)
{
  evdev_init();
  evdev_set_file(path);

  static lv_indev_drv_t indev_drv;

  /*Register a encoder input device*/
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = evdev_read;
  lv_indev_t * indev = lv_indev_drv_register(&indev_drv);

  if(show_cursor) {
    /*Set a cursor for the mouse*/
    LV_IMG_DECLARE(mouse_cursor_icon); /*Declare the image file.*/
    lv_obj_t * cursor_obj = lv_img_create(lv_scr_act()); /*Create an image object for the cursor */
    lv_img_set_src(cursor_obj, &mouse_cursor_icon);           /*Set the image source*/
    lv_indev_set_cursor(indev, cursor_obj);             /*Connect the image  object to the driver*/
  }
}

static void cursor_set_hidden(bool en)
{
  // hide cursor echo -e "\033[?25l"
  // show cursor echo -e "\033[?25h"
  int ret = system(en ? "echo -e \"\033[?25l\"" : "echo -e \"\033[?25h\"");
  printf(__func__);
  printf(ret == 0 ? " OK" : " ERROR");
  printf("\n");
}
