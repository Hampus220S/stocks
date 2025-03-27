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

  tui_t* tui = tui_create((tui_config_t)
  {
    .color = (tui_color_t)
    {
      .bg = TUI_COLOR_BLACK,
      .fg = TUI_COLOR_WHITE
    }
  });

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
      .h = 20,
      .x = 3,
      .y = 5
    },
    .color = (tui_color_t)
    {
      .fg = TUI_COLOR_YELLOW
    },
    .border = (tui_border_t)
    {
      .is_active = true,
      .color = (tui_color_t)
      {
        .bg = TUI_COLOR_MAGENTA
      }
    }
  });

  tui_window_parent_t* box = tui_parent_child_parent_create(parent, (tui_window_parent_config_t)
  {
    .name = "box",
    .rect = (tui_rect_t)
    {
      .w = 10,
      .h = 8,
      .x = 3,
      .y = 2
    },
    .color = (tui_color_t)
    {
      .bg = TUI_COLOR_WHITE,
      .fg = TUI_COLOR_RED
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
      .x = 2,
      .y = 3
    }
  });

  char* lines[] =
  {
    "[+] Bookmark",
    "This is some text",
    "Hampus"
  };

  for (size_t index = 0; index < 3; index++)
  {
    tui_parent_child_text_create(box, (tui_window_text_config_t)
    {
      .string = lines[index],
      .rect = TUI_RECT_NONE
    });
  }


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
