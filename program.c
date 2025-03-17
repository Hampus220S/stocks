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
    true,
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

  tui_window_text_create(tui, "data", true,
    (tui_rect_t)
    {
      .w = (tui_size_t)
      {
        .type = TUI_SIZE_NONE,
        .value.rel = 0.3
      },
      .h = (tui_size_t)
      {
        .type = TUI_SIZE_NONE,
        .value.rel = 0.3
      },
      .xpos = TUI_POS_LEFT,
      .ypos = TUI_POS_CENTER
    },
    NULL,
    "Open terminal to close this window",
    false,
    NULL,
    TUI_POS_LEFT,
    TUI_POS_CENTER
  );

  tui_active_menu_set(tui, "main");

  tui_start(tui); 

  tui_delete(&tui);

  tui_quit();

  debug_file_close();

  return 0;
}
