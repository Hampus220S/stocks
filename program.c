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

  getch();

  tui_quit();

  return 0;
}
