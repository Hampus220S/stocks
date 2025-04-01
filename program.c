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
    .rect = TUI_RECT_NONE,
    .color = (tui_color_t)
    {
      .bg = TUI_COLOR_GREEN,
      .fg = TUI_COLOR_MAGENTA
    },
    .border = (tui_border_t)
    {
      .is_active = true,
      .color = TUI_COLOR_NONE
    },
    .is_inflated = true,
    .has_padding = true,
    .pos = TUI_POS_CENTER,
  });

  tui_window_text_create(tui, (tui_window_text_config_t)
  {
    .string = "This is some text",
    .rect = (tui_rect_t)
    {
      .w = 0,
      .h = 2,
      .y = -2
    },
    .color = (tui_color_t)
    {
      .bg = TUI_COLOR_GREEN,
      .fg = TUI_COLOR_MAGENTA
    },
    .pos = TUI_POS_END,
    .align = TUI_ALIGN_CENTER,
  });


  tui_window_parent_t* box = tui_parent_child_parent_create(parent, (tui_window_parent_config_t)
  {
    .name = "box",
    .rect = TUI_RECT_NONE,
    .color = (tui_color_t)
    {
      .fg = TUI_COLOR_RED
    },
    .border = (tui_border_t)
    {
      .is_active = false
    },
    .is_vertical = true,
    .is_inflated = true,
    .has_padding = false,
    .pos = TUI_POS_END,
    .align = TUI_ALIGN_BETWEEN
  });

  char* lines[] =
  {
    "[+] Apple",
    "[+] Pear",
    "[+] Banana"
  };

  for (size_t index = 0; index < 3; index++)
  {
    tui_parent_child_text_create(box, (tui_window_text_config_t)
    {
      .string = lines[index],
      .rect = TUI_RECT_NONE,
      .color = (tui_color_t)
      {
        .bg = TUI_COLOR_BLUE
      },
      .align = TUI_POS_CENTER,
      .pos = TUI_POS_CENTER,
    });
  }

  tui_window_parent_t* box2 = tui_parent_child_parent_create(parent, (tui_window_parent_config_t)
  {
    .name = "box2",
    .rect = TUI_RECT_NONE,
    .color = (tui_color_t)
    {
      .bg = TUI_COLOR_WHITE,
      .fg = TUI_COLOR_RED
    },
    .border = (tui_border_t)
    {
      .is_active = true
    },
    .is_vertical = true,
    .has_padding = true,
    .pos = TUI_POS_END,
  });

  tui_parent_child_text_create(box2, (tui_window_text_config_t)
  {
    .name = "box2-title",
    .string = "BOX2",
    .rect = (tui_rect_t)
    {
      .x = 1,
      .y = 0,
      .w = -2,
      .h = 1
    },
    .color = (tui_color_t)
    {
      .bg = TUI_COLOR_MAGENTA
    }
  });

  char* lines2[] =
  {
    "[+] Keyboard",
    "[+] Mouse",
    "[+] Computer",
    "[+] Case",
    "[+] Voltage",
    "[+] Fan"
  };

  for (size_t index = 0; index < 6; index++)
  {
    tui_parent_child_text_create(box2, (tui_window_text_config_t)
    {
      .string = lines2[index],
      .rect = TUI_RECT_NONE,
      .color = (tui_color_t)
      {
        .bg = TUI_COLOR_BLUE
      },
      .align = TUI_POS_START
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
