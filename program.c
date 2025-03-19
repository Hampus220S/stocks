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


  tui->is_running = true;

  clear();

  tui_render(tui);

  int index = 0;

  tui_color_on(tui, TUI_COLOR_WHITE, TUI_COLOR_GREEN);

  int key;

  while (tui->is_running && (key = wgetch(stdscr)))
  {
    erase();

    tui_color_t fg_color = index + 1;

    index = (index + 1) % 8;

    tui_color_on(tui, fg_color, TUI_COLOR_NONE);

    mvprintw(1, 1, "KEY: %d", key);

    tui_color_off(tui, fg_color, TUI_COLOR_NONE);

    if (key == KEY_CTRLS)
    {
      tui->is_running = false;
      break;
    }

    tui_event(tui, key);

    tui_render(tui);
  }

  tui_color_off(tui, TUI_COLOR_WHITE, TUI_COLOR_GREEN);


  tui_delete(&tui);

  info_print("Deleted TUI");


  tui_quit();

  info_print("Quitted TUI");

  debug_file_close();

  return 0;
}
