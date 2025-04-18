# Notes
- fix tui_tab_ functions: only is_interact windows can be tabbed to
- problem: spaces in beginning of lines are removed by tui_text_render and tui_text_ws_get
- some futures missing longName, try shortName or just save symbol (don't return error)
  (don't return error on other meta data as well)
- ESC for leaving chart and going back to list
- problem: prices don't event.update on resize, only on keypress

==16644== 4 bytes in 1 blocks are definitely lost in loss record 6 of 100
==16644==    at 0x4848899: malloc (in /usr/libexec/valgrind/vgpreload_memcheck-amd64-linux.so)
==16644==    by 0x4A3758E: strdup (strdup.c:42)
==16644==    by 0x111D84: stock_meta_parse (stock.h:414)
==16644==    by 0x1127BE: stock_fetch (stock.h:700)
==16644==    by 0x112BB3: stock_update (stock.h:814)

==16644== 3 bytes in 1 blocks are definitely lost in loss record 1 of 100
==16644==    at 0x4848899: malloc (in /usr/libexec/valgrind/vgpreload_memcheck-amd64-linux.so)
==16644==    by 0x4A3758E: strdup (strdup.c:42)
==16644==    by 0x112B87: stock_update (stock.h:810)

## Duplicate
- tui_..._window_..._search (only tui_window_t** windows and size_t count is needed)
- maybe, call init event from tui_..._window_append

## Future
- 'f' for fullscreen in chart window
- fix tui_tab_forward and tui_tab_backward
- tidy up stock.h header library
- create new menu with stock in table with HLOC values
