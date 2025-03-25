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
  debug_file_open("debug.log");

  if(tui_init() != 0)
  {
    info_print("Failed to initialize TUI");

    debug_file_close();

    return 1;
  }

  info_print("Initialized TUI");

  tui_t* tui = tui_create(NULL);

  if (!tui)
  {
    info_print("Failed to create TUI");

    tui_quit();

    debug_file_close();

    return 2;
  }

  info_print("Created TUI");


  tui_window_parent_t* parent = tui_window_parent_create(tui, (tui_window_parent_config_t)
  {
    .name = "parent",
    .rect = (tui_rect_t)
    {
      .w = 20,
      .h = 10,
      .x = 3,
      .y = 5
    },
    .color = (tui_color_t)
    {
      .bg = COLOR_BLUE,
      .fg = COLOR_RED
    },
    .border = (tui_border_t)
    {
      .is_active = true
    }
  });

  tui_parent_child_parent_create(parent, (tui_window_parent_config_t)
  {
    .name = "box",
    .rect = (tui_rect_t)
    {
      .w = 10,
      .h = 3,
      .x = 3,
      .y = 5
    },
    .color = (tui_color_t)
    {
      .bg = COLOR_BLUE,
      .fg = COLOR_RED
    },
    .border = (tui_border_t)
    {
      .is_active = true
    }
  });

  tui_parent_child_text_create(parent, (tui_window_text_config_t)
  {
    .name = "key",
    .string = "Hej",
    .rect = (tui_rect_t)
    {
      .w = 6,
      .h = 1,
      .x = 3,
      .y = 5
    },
    .color = (tui_color_t)
    {
      .bg = COLOR_BLUE,
      .fg = COLOR_RED
    }
  });


  tui->is_running = true;

  tui_render(tui);

  int key;

  while (tui->is_running && (key = wgetch(stdscr)))
  {
    if (key == KEY_CTRLS)
    {
      tui->is_running = false;
      break;
    }

    tui_event(tui, key);

    tui_render(tui);
  }


  tui_delete(&tui);

  info_print("Deleted TUI");


  tui_quit();

  info_print("Quitted TUI");

  debug_file_close();

  return 0;
}
