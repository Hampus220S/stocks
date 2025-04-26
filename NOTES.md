# Notes
- prevent curl from outputting text in ncurses screen
- don't change item border color on exit or enter, set color in item_window_render
- take new screenshots
- fix stocks_window_enter or remove it (fix switching from chart back to either search or list item)
- align '=' in stocks.c
- replace 0 with TUI_PARENT_SIZE in stocks.c
- make chart display graph all the way to the left and the bottom
- fix: search text disappears when it is too long to print (tui_text_render h -> 0)

## Duplicate
- tui_..._window_..._search (only tui_window_t** windows and size_t count is needed)

## Maybe
- rename parent_window_enter to root_window_render (not genric -> specific)
- maybe, call init event from tui_..._window_append
- an interactive window must have interactive parents all the way up
  (therefor, create function that calculates
- don't render chart under range window, render them over and under each other

## Future
- 'f' for fullscreen in chart window
- create new menu with stock in table with HLOC values
