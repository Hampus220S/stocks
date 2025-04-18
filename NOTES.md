# Notes
- fix tui_tab_ functions: only is_interact windows can be tabbed to
- print prices on every second line vertically on left side of chart
  (0, 0, w:10, h:0) with .has_padding = true
- print titles of every window border

## Duplicate
- tui_..._window_..._search (only tui_window_t** windows and size_t count is needed)
- maybe, call init event from tui_..._window_append

## Future
- 'f' for fullscreen in chart window
- fix tui_tab_forward and tui_tab_backward
- tidy up stock.h header library
- create new menu with stock in table with HLOC values
