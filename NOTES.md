# Notes
- fix tui_tab_ functions: only is_interact windows can be tabbed to
- problem: spaces in beginning of lines are removed by tui_text_render and tui_text_ws_get
- some futures missing longName, try shortName or just save symbol (don't return error)
  (don't return error on other meta data as well)
- ESC for leaving chart and going back to list

## Duplicate
- tui_..._window_..._search (only tui_window_t** windows and size_t count is needed)
- maybe, call init event from tui_..._window_append

## Future
- 'f' for fullscreen in chart window
- fix tui_tab_forward and tui_tab_backward
- tidy up stock.h header library
- create new menu with stock in table with HLOC values
