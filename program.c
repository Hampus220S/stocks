/*
 * program.c
 */

#define TUI_IMPLEMENT
#include "tui.h"

/*
 * Main function
 */
int main(int argc, char* argv[])
{
  tui_init();

  tui_t* tui = tui_create();

  tui_start(tui); 

  tui_free(&tui);

  tui_quit();

  return 0;
}
