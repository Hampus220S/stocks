/*
 * program.c
 */

#define TUI_IMPLEMENT
#include "tui.h"

#define DEBUG_IMPLEMENT
#include "debug.h"

/*
 * Main function
 */
int main(int argc, char* argv[])
{
  char* text = "Hello I am what are you doing\nnot a clue what your sock\npig piggy";

  debug_file_open("debug.log");

  tui_init();

  tui_t* tui = tui_create(40, 26, NULL);

  tui_menu_t* menu = tui_menu_create(tui, "main", NULL);

  tui_menu_window_text_create(menu, "info1", true,
   (tui_rect_t)
    {
      .w = (tui_size_t)
      {
        .type = TUI_SIZE_NONE,
        .value.rel = 0.5
      },
      .h = (tui_size_t)
      {
        .type = TUI_SIZE_NONE,
        .value.rel = 0.5
      },
      .xpos = TUI_POS_RIGHT,
      .ypos = TUI_POS_BOTTOM
    },
    NULL,
    text,
    false,
    "Close",
    TUI_POS_CENTER,
    TUI_POS_CENTER
  );

  tui_menu_window_text_create(menu, "info2", true,
    (tui_rect_t)
    {
      .w = (tui_size_t)
      {
        .type = TUI_SIZE_NONE,
        .value.rel = 0.5
      },
      .h = (tui_size_t)
      {
        .type = TUI_SIZE_NONE,
        .value.rel = 0.5
      },
      .xpos = TUI_POS_LEFT,
      .ypos = TUI_POS_BOTTOM
    },
    NULL,
    text,
    true,
    "Done",
    TUI_POS_RIGHT,
    TUI_POS_CENTER
  );

  tui_active_menu_set(tui, "main");

  info_print("tui->menu_count: %d", tui->menu_count);
  info_print("tui->menu: %d", tui->menu);

  info_print("menu->window_count: %d", menu->window_count);

  tui_start(tui); 

  tui_delete(&tui);

  tui_quit();

  debug_file_close();

  return 0;
}
