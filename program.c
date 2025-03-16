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

  tui_t* tui = tui_create();

  tui_start(tui); 

  tui_free(&tui);

  tui_quit();

  debug_file_close();

  return 0;
}
