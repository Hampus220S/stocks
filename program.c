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

  printf("Text:\n%s\n", text);


  for (int count = 15; count < 26; count++)
  {
    printf("\nTesting width : %d\n\n", count);
    int w = count;

    int h = tui_text_h_get(text, w);

    if (h == -1)
    {
      continue;
    }

    printf("h: %d\n", h);

    printf("w: %d\n", tui_text_w_get(text, h));


    int ws[h];

    tui_text_ws_get(ws, text, h);

    for (int index = 0; index < h; index++)
    {
      printf("[%d] w: %d\n", index, ws[index]);
    }


    for (int index = 0; index < w; index++)
    {
      printf("+");
    }

    printf("\n");


    w = tui_text_w_get(text, h);


    for (int index = 0; index < w; index++)
    {
      printf(".");
    }

    printf("\n");



    size_t length = strlen(text);

    int line_index = 0;
    int line_w = 0;

    for (size_t index = 0; index < length; index++)
    {
      char letter = text[index];

      if (letter == ' ' && line_w == 0)
      {
        line_w = 0;
      }
      else if (line_w >= ws[line_index])
      {
        line_index++;

        line_w = 0;

        printf("\n");
      }
      else
      {
        printf("%c", letter);

        line_w++;
      }
    }

    printf("\n");
  }

  /*
  debug_file_open("debug.log");

  tui_init();

  tui_t* tui = tui_create();

  info_print("tui->window_count: %d", tui->window_count);

  tui_start(tui); 

  tui_free(&tui);

  tui_quit();

  debug_file_close();
  */

  return 0;
}
